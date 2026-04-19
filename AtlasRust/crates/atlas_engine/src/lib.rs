//! atlas_engine — Engine config, ECS world, event bus, logger, and tick loop.
//!
//! Mirrors the C++ atlas::Engine, atlas::EventBus, and atlas::Logger.
//! Bevy-specific ECS integration lives at the app level.

use std::{
    any::{Any, TypeId},
    collections::HashMap,
    sync::Arc,
};

use atlas_physics::PhysicsWorld;
use serde::{Deserialize, Serialize};

// ── Logger ────────────────────────────────────────────────────────────────────

type SinkFn = Arc<dyn Fn(&str) + Send + Sync>;

/// Simple structured logger with an optional callback sink.
pub struct Logger {
    sink: Option<SinkFn>,
}

impl Logger {
    pub fn new() -> Self { Self { sink: None } }

    pub fn set_sink<F: Fn(&str) + Send + Sync + 'static>(&mut self, f: F) {
        self.sink = Some(Arc::new(f));
    }

    fn write(&self, level: &str, msg: &str) {
        let line = format!("[{level}] {msg}");
        println!("{line}");
        if let Some(sink) = &self.sink { sink(&line); }
    }

    pub fn info(&self, msg: &str) { self.write("INFO", msg); }
    pub fn warn(&self, msg: &str) { self.write("WARN", msg); }
    pub fn error(&self, msg: &str) { self.write("ERROR", msg); }
}

impl Default for Logger {
    fn default() -> Self { Self::new() }
}

// ── EventBus ──────────────────────────────────────────────────────────────────

/// Lightweight event payload — mirrors C++ atlas::Event.
#[derive(Debug, Clone)]
pub struct Event {
    pub event_type:  String,
    pub sender_id:   u32,
    pub int_param:   i64,
    pub float_param: f64,
    pub str_param:   String,
}

impl Event {
    pub fn new(event_type: impl Into<String>) -> Self {
        Self {
            event_type: event_type.into(),
            sender_id: 0,
            int_param: 0,
            float_param: 0.0,
            str_param: String::new(),
        }
    }
}

pub type SubscriptionId = u64;
type Callback = Arc<dyn Fn(&Event) + Send + Sync>;

/// Publish/subscribe event bus with wildcard support and deferred delivery.
pub struct EventBus {
    subs:            HashMap<String, Vec<(SubscriptionId, Callback)>>,
    wildcard_subs:   Vec<(SubscriptionId, Callback)>,
    queue:           Vec<Event>,
    next_id:         SubscriptionId,
    total_published: u64,
}

impl EventBus {
    pub fn new() -> Self {
        Self {
            subs: HashMap::new(),
            wildcard_subs: Vec::new(),
            queue: Vec::new(),
            next_id: 1,
            total_published: 0,
        }
    }

    pub fn subscribe<F>(&mut self, event_type: impl Into<String>, cb: F) -> SubscriptionId
    where F: Fn(&Event) + Send + Sync + 'static
    {
        let id = self.next_id;
        self.next_id += 1;
        let event_type = event_type.into();
        if event_type == "*" {
            self.wildcard_subs.push((id, Arc::new(cb)));
        } else {
            self.subs.entry(event_type).or_default().push((id, Arc::new(cb)));
        }
        id
    }

    pub fn unsubscribe(&mut self, id: SubscriptionId) {
        for list in self.subs.values_mut() {
            list.retain(|(sid, _)| *sid != id);
        }
        self.wildcard_subs.retain(|(sid, _)| *sid != id);
    }

    fn dispatch(&mut self, event: &Event) {
        self.total_published += 1;
        if let Some(list) = self.subs.get(&event.event_type) {
            for (_, cb) in list.iter() { cb(event); }
        }
        for (_, cb) in &self.wildcard_subs { cb(event); }
    }

    pub fn publish(&mut self, event: Event) { self.dispatch(&event); }

    pub fn enqueue(&mut self, event: Event) { self.queue.push(event); }

    pub fn flush(&mut self) {
        let events: Vec<_> = self.queue.drain(..).collect();
        for e in events { self.dispatch(&e); }
    }

    pub fn subscription_count(&self) -> usize {
        self.subs.values().map(|v| v.len()).sum::<usize>() + self.wildcard_subs.len()
    }

    pub fn queue_size(&self) -> usize { self.queue.len() }
    pub fn total_published(&self) -> u64 { self.total_published }

    pub fn reset(&mut self) {
        self.subs.clear();
        self.wildcard_subs.clear();
        self.queue.clear();
    }
}

impl Default for EventBus {
    fn default() -> Self { Self::new() }
}

// ── Entity / Component Store ──────────────────────────────────────────────────

pub type EntityId = u32;
pub const INVALID_ENTITY: EntityId = 0;

/// Pool of live entity IDs with a free list.
pub struct EntityManager {
    alive:    Vec<EntityId>,
    free:     Vec<EntityId>,
    next_id:  EntityId,
}

impl EntityManager {
    pub fn new() -> Self { Self { alive: Vec::new(), free: Vec::new(), next_id: 1 } }

    pub fn create(&mut self) -> EntityId {
        let id = if let Some(id) = self.free.pop() { id } else {
            let id = self.next_id; self.next_id += 1; id
        };
        self.alive.push(id);
        id
    }

    pub fn destroy(&mut self, id: EntityId) {
        self.alive.retain(|&a| a != id);
        self.free.push(id);
    }

    pub fn is_alive(&self, id: EntityId) -> bool { self.alive.contains(&id) }
    pub fn alive(&self) -> &[EntityId] { &self.alive }
    pub fn count(&self) -> usize { self.alive.len() }
    pub fn clear(&mut self) { self.alive.clear(); self.free.clear(); self.next_id = 1; }
}

impl Default for EntityManager {
    fn default() -> Self { Self::new() }
}

/// Type-erased per-entity component storage.
pub struct ComponentStore {
    pools: HashMap<TypeId, Box<dyn Any + Send + Sync>>,
}

impl ComponentStore {
    pub fn new() -> Self { Self { pools: HashMap::new() } }

    fn pool<T: 'static + Send + Sync>(&mut self) -> &mut HashMap<EntityId, T> {
        self.pools
            .entry(TypeId::of::<T>())
            .or_insert_with(|| Box::new(HashMap::<EntityId, T>::new()))
            .downcast_mut()
            .expect("pool type mismatch")
    }

    pub fn add<T: 'static + Send + Sync>(&mut self, entity: EntityId, component: T) {
        self.pool::<T>().insert(entity, component);
    }

    pub fn remove<T: 'static + Send + Sync>(&mut self, entity: EntityId) {
        if let Some(pool) = self.pools.get_mut(&TypeId::of::<T>()) {
            if let Some(map) = pool.downcast_mut::<HashMap<EntityId, T>>() {
                map.remove(&entity);
            }
        }
    }

    pub fn get<T: 'static + Send + Sync>(&self, entity: EntityId) -> Option<&T> {
        self.pools.get(&TypeId::of::<T>())?
            .downcast_ref::<HashMap<EntityId, T>>()
            .and_then(|m| m.get(&entity))
    }

    pub fn get_mut<T: 'static + Send + Sync>(&mut self, entity: EntityId) -> Option<&mut T> {
        self.pool::<T>().get_mut(&entity)
    }

    pub fn has<T: 'static + Send + Sync>(&self, entity: EntityId) -> bool {
        self.pools.get(&TypeId::of::<T>())
            .and_then(|p| p.downcast_ref::<HashMap<EntityId, T>>())
            .map_or(false, |m| m.contains_key(&entity))
    }
}

impl Default for ComponentStore {
    fn default() -> Self { Self::new() }
}

// ── EngineConfig ──────────────────────────────────────────────────────────────

#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum EngineMode { Editor, Client, Server }

impl Default for EngineMode { fn default() -> Self { Self::Client } }

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct EngineConfig {
    pub mode:             EngineMode,
    pub asset_root:       String,
    pub tick_rate:        u32,
    pub max_ticks:        u32,
    pub window_width:     i32,
    pub window_height:    i32,
    pub headless:         bool,
    pub autosave_interval: u32,
    pub autosave_path:    String,
}

impl Default for EngineConfig {
    fn default() -> Self {
        Self {
            mode:              EngineMode::Client,
            asset_root:        "assets".into(),
            tick_rate:         30,
            max_ticks:         0,
            window_width:      1280,
            window_height:     720,
            headless:          false,
            autosave_interval: 0,
            autosave_path:     "autosave.asav".into(),
        }
    }
}

// ── Engine ────────────────────────────────────────────────────────────────────

/// Core engine — owns the world, event bus, and tick loop.
pub struct Engine {
    pub config:   EngineConfig,
    pub entities: EntityManager,
    pub components: ComponentStore,
    pub events:   EventBus,
    pub physics:  PhysicsWorld,
    pub logger:   Logger,
    running:      bool,
    tick_count:   u64,
    systems:      Vec<String>,
    frame_cb:     Option<Box<dyn Fn(f32) + Send + Sync>>,
}

impl Engine {
    pub fn new(config: EngineConfig) -> Self {
        Self {
            config,
            entities:    EntityManager::new(),
            components:  ComponentStore::new(),
            events:      EventBus::new(),
            physics:     PhysicsWorld::new(),
            logger:      Logger::new(),
            running:     false,
            tick_count:  0,
            systems:     Vec::new(),
            frame_cb:    None,
        }
    }

    pub fn init_core(&mut self) {
        self.running = true;
        self.physics.init();
        self.register_system("Core");
        tracing::info!(target: "Engine", "Atlas Engine core initialized");
    }

    pub fn register_system(&mut self, name: impl Into<String>) {
        self.systems.push(name.into());
    }

    pub fn set_frame_callback<F: Fn(f32) + Send + Sync + 'static>(&mut self, cb: F) {
        self.frame_cb = Some(Box::new(cb));
    }

    pub fn tick(&mut self, dt: f32) {
        self.tick_count += 1;
        self.events.flush();
        self.physics.step(dt);
        if let Some(cb) = &self.frame_cb { cb(dt); }
    }

    /// Run a fixed-rate headless tick loop for `ticks` iterations.
    pub fn run_headless(&mut self, ticks: u32) {
        let dt = 1.0 / self.config.tick_rate as f32;
        for _ in 0..ticks {
            if !self.running { break; }
            self.tick(dt);
            if self.config.max_ticks > 0 && self.tick_count >= self.config.max_ticks as u64 {
                self.running = false;
                break;
            }
        }
    }

    pub fn request_exit(&mut self) { self.running = false; }
    pub fn is_running(&self) -> bool { self.running }
    pub fn tick_count(&self) -> u64 { self.tick_count }
    pub fn system_execution_order(&self) -> &[String] { &self.systems }

    pub fn shutdown(&mut self) {
        self.running = false;
        tracing::info!(target: "Engine", "Atlas Engine shutdown (ticks={})", self.tick_count);
    }
}

impl Default for Engine {
    fn default() -> Self { Self::new(EngineConfig::default()) }
}

// ── Tests ─────────────────────────────────────────────────────────────────────

#[cfg(test)]
mod tests {
    use super::*;

    // ── Logger ───────────────────────────────────────────────────────────────

    #[test]
    fn logger_sink_receives_messages() {
        use std::sync::{Arc, Mutex};
        let captured = Arc::new(Mutex::new(Vec::<String>::new()));
        let cap2 = captured.clone();
        let mut logger = Logger::new();
        logger.set_sink(move |msg: &str| {
            cap2.lock().unwrap().push(msg.to_string());
        });
        logger.info("hello");
        logger.warn("watch out");
        logger.error("oops");
        let msgs = captured.lock().unwrap();
        assert_eq!(msgs.len(), 3);
        assert!(msgs[0].contains("INFO"));
        assert!(msgs[1].contains("WARN"));
        assert!(msgs[2].contains("ERROR"));
    }

    // ── EventBus ─────────────────────────────────────────────────────────────

    #[test]
    fn event_bus_publish_fires_subscriber() {
        let mut bus = EventBus::new();
        let fired = std::sync::Arc::new(std::sync::Mutex::new(false));
        let f2 = fired.clone();
        bus.subscribe("TestEvent", move |_e| { *f2.lock().unwrap() = true; });
        bus.publish(Event::new("TestEvent"));
        assert!(*fired.lock().unwrap(), "subscriber must be called on publish");
    }

    #[test]
    fn event_bus_deferred_queue_flushed() {
        let mut bus = EventBus::new();
        let count = std::sync::Arc::new(std::sync::Mutex::new(0u32));
        let c2 = count.clone();
        bus.subscribe("Tick", move |_| { *c2.lock().unwrap() += 1; });
        bus.enqueue(Event::new("Tick"));
        bus.enqueue(Event::new("Tick"));
        assert_eq!(bus.queue_size(), 2);
        bus.flush();
        assert_eq!(bus.queue_size(), 0);
        assert_eq!(*count.lock().unwrap(), 2);
    }

    #[test]
    fn event_bus_unsubscribe_stops_delivery() {
        let mut bus = EventBus::new();
        let count = std::sync::Arc::new(std::sync::Mutex::new(0u32));
        let c2 = count.clone();
        let id = bus.subscribe("E", move |_| { *c2.lock().unwrap() += 1; });
        bus.publish(Event::new("E"));
        bus.unsubscribe(id);
        bus.publish(Event::new("E"));
        assert_eq!(*count.lock().unwrap(), 1, "after unsubscribe, should receive only 1 event");
    }

    #[test]
    fn event_bus_wildcard_matches_all() {
        let mut bus = EventBus::new();
        let count = std::sync::Arc::new(std::sync::Mutex::new(0u32));
        let c2 = count.clone();
        bus.subscribe("*", move |_| { *c2.lock().unwrap() += 1; });
        bus.publish(Event::new("A"));
        bus.publish(Event::new("B"));
        bus.publish(Event::new("C"));
        assert_eq!(*count.lock().unwrap(), 3);
    }

    #[test]
    fn event_bus_total_published_counter() {
        let mut bus = EventBus::new();
        bus.publish(Event::new("X"));
        bus.publish(Event::new("Y"));
        assert_eq!(bus.total_published(), 2);
    }

    // ── EntityManager ────────────────────────────────────────────────────────

    #[test]
    fn entity_manager_create_and_destroy() {
        let mut em = EntityManager::new();
        let e1 = em.create();
        let e2 = em.create();
        assert_ne!(e1, e2);
        assert!(em.is_alive(e1));
        em.destroy(e1);
        assert!(!em.is_alive(e1));
        assert!(em.is_alive(e2));
        assert_eq!(em.count(), 1);
    }

    #[test]
    fn entity_manager_reuses_destroyed_ids() {
        let mut em = EntityManager::new();
        let e1 = em.create();
        em.destroy(e1);
        let e2 = em.create();
        // The free list is LIFO, so the next create should reuse e1's id.
        assert_eq!(e1, e2, "destroyed entity id should be reused");
    }

    #[test]
    fn entity_manager_clear() {
        let mut em = EntityManager::new();
        em.create(); em.create(); em.create();
        em.clear();
        assert_eq!(em.count(), 0);
    }

    // ── ComponentStore ───────────────────────────────────────────────────────

    #[derive(Debug, PartialEq)]
    struct Position { x: f32, y: f32 }

    #[derive(Debug, PartialEq)]
    struct Health { hp: f32 }

    #[test]
    fn component_store_add_and_get() {
        let mut store = ComponentStore::new();
        store.add(1, Position { x: 10.0, y: 20.0 });
        let pos = store.get::<Position>(1).unwrap();
        assert_eq!(pos.x, 10.0);
        assert_eq!(pos.y, 20.0);
    }

    #[test]
    fn component_store_get_missing_returns_none() {
        let store = ComponentStore::new();
        assert!(store.get::<Position>(99).is_none());
    }

    #[test]
    fn component_store_has() {
        let mut store = ComponentStore::new();
        store.add(1, Health { hp: 100.0 });
        assert!(store.has::<Health>(1));
        assert!(!store.has::<Health>(2));
    }

    #[test]
    fn component_store_remove() {
        let mut store = ComponentStore::new();
        store.add(1, Health { hp: 50.0 });
        store.remove::<Health>(1);
        assert!(!store.has::<Health>(1));
    }

    #[test]
    fn component_store_mutate() {
        let mut store = ComponentStore::new();
        store.add(1, Health { hp: 50.0 });
        store.get_mut::<Health>(1).unwrap().hp = 75.0;
        assert_eq!(store.get::<Health>(1).unwrap().hp, 75.0);
    }

    // ── Engine ───────────────────────────────────────────────────────────────

    #[test]
    fn engine_tick_increments_counter() {
        let mut engine = Engine::new(EngineConfig { headless: true, ..Default::default() });
        engine.init_core();
        engine.tick(1.0 / 30.0);
        engine.tick(1.0 / 30.0);
        assert_eq!(engine.tick_count(), 2);
    }

    #[test]
    fn engine_run_headless() {
        let mut engine = Engine::new(EngineConfig { headless: true, ..Default::default() });
        engine.init_core();
        engine.run_headless(10);
        assert_eq!(engine.tick_count(), 10);
    }

    #[test]
    fn engine_max_ticks_stops_early() {
        let mut engine = Engine::new(EngineConfig {
            headless:  true,
            max_ticks: 5,
            ..Default::default()
        });
        engine.init_core();
        engine.run_headless(100);  // ask for 100, should stop at 5
        assert_eq!(engine.tick_count(), 5);
    }

    #[test]
    fn engine_register_system() {
        let mut engine = Engine::new(EngineConfig::default());
        engine.register_system("Physics");
        engine.register_system("Rendering");
        let order = engine.system_execution_order();
        // "Core" is registered in init_core; without calling init_core here we
        // only see the two we registered.
        assert!(order.contains(&"Physics".to_string()));
        assert!(order.contains(&"Rendering".to_string()));
    }

    #[test]
    fn engine_frame_callback_is_called() {
        use std::sync::{Arc, Mutex};
        let count = Arc::new(Mutex::new(0u32));
        let c2 = count.clone();
        let mut engine = Engine::new(EngineConfig { headless: true, ..Default::default() });
        engine.init_core();
        engine.set_frame_callback(move |_dt| { *c2.lock().unwrap() += 1; });
        engine.run_headless(3);
        assert_eq!(*count.lock().unwrap(), 3);
    }
}
