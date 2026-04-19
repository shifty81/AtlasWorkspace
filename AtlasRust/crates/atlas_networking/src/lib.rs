//! atlas_networking — Packet types, connection management, and network manager.
//!
//! Mirrors the C++ NF::Networking system: PacketType, Packet, Connection,
//! ConnectionState, and NetworkManager. Platform sockets are stubbed — wiring
//! to bevy_replicon or tokio::net happens at the app level.

use serde::{Deserialize, Serialize};
use serde_json::{json, Value as JsonValue};
use std::collections::HashMap;

// ── PacketType ────────────────────────────────────────────────────────────────

#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash, Serialize, Deserialize)]
#[repr(u8)]
pub enum PacketType {
    Connect,
    Disconnect,
    Heartbeat,
    EntitySpawn,
    EntityDestroy,
    EntityUpdate,
    WorldChunk,
    VoxelEdit,
    ChatMessage,
    Rpc,
    Ping,
    Pong,
    AuthRequest,
    AuthResponse,
    PlayerInput,
    StateSnapshot,
    AckReliable,
}

// ── Packet ────────────────────────────────────────────────────────────────────

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Packet {
    pub packet_type:  PacketType,
    pub sender_id:    u32,
    pub sequence_num: u32,
    pub reliable:     bool,
    pub timestamp:    f32,
    pub payload:      JsonValue,
}

impl Packet {
    pub fn new(packet_type: PacketType, sender_id: u32) -> Self {
        Self {
            packet_type,
            sender_id,
            sequence_num: 0,
            reliable: false,
            timestamp: 0.0,
            payload: JsonValue::Null,
        }
    }

    pub fn to_json(&self) -> JsonValue {
        json!({
            "type":        self.packet_type as u8,
            "senderId":    self.sender_id,
            "sequenceNum": self.sequence_num,
            "reliable":    self.reliable,
            "timestamp":   self.timestamp,
            "payload":     self.payload,
        })
    }
}

// ── ConnectionState ───────────────────────────────────────────────────────────

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize, Default)]
pub enum ConnectionState {
    #[default]
    Disconnected,
    Connecting,
    Connected,
    Authenticated,
    Disconnecting,
}

// ── Connection ────────────────────────────────────────────────────────────────

/// Represents a single peer connection.
pub struct Connection {
    id:              u32,
    state:           ConnectionState,
    address:         String,
    port:            u16,
    latency:         f32,
    last_heartbeat:  f32,
    packets_sent:    u32,
    packets_received: u32,
    send_queue:      Vec<Packet>,
    receive_queue:   Vec<Packet>,
    reliable_buffer: Vec<Packet>,
}

impl Connection {
    pub fn new(id: u32, address: impl Into<String>, port: u16) -> Self {
        Self {
            id,
            state: ConnectionState::Disconnected,
            address: address.into(),
            port,
            latency: 0.0,
            last_heartbeat: 0.0,
            packets_sent: 0,
            packets_received: 0,
            send_queue: Vec::new(),
            receive_queue: Vec::new(),
            reliable_buffer: Vec::new(),
        }
    }

    pub fn id(&self) -> u32 { self.id }
    pub fn state(&self) -> ConnectionState { self.state }
    pub fn set_state(&mut self, s: ConnectionState) { self.state = s; }
    pub fn address(&self) -> &str { &self.address }
    pub fn port(&self) -> u16 { self.port }
    pub fn latency(&self) -> f32 { self.latency }
    pub fn set_latency(&mut self, lat: f32) { self.latency = lat; }
    pub fn last_heartbeat(&self) -> f32 { self.last_heartbeat }
    pub fn set_last_heartbeat(&mut self, t: f32) { self.last_heartbeat = t; }
    pub fn packets_sent(&self) -> u32 { self.packets_sent }
    pub fn packets_received(&self) -> u32 { self.packets_received }
    pub fn send_queue(&self) -> &[Packet] { &self.send_queue }
    pub fn receive_queue(&self) -> &[Packet] { &self.receive_queue }
    pub fn reliable_buffer(&self) -> &[Packet] { &self.reliable_buffer }

    pub fn queue_send(&mut self, packet: Packet) {
        self.packets_sent += 1;
        if packet.reliable { self.reliable_buffer.push(packet.clone()); }
        self.send_queue.push(packet);
    }

    pub fn queue_receive(&mut self, packet: Packet) {
        self.packets_received += 1;
        self.receive_queue.push(packet);
    }

    pub fn clear_send_queue(&mut self) { self.send_queue.clear(); }
    pub fn clear_receive_queue(&mut self) { self.receive_queue.clear(); }

    pub fn ack_reliable(&mut self, seq_num: u32) {
        self.reliable_buffer.retain(|p| p.sequence_num != seq_num);
    }

    pub fn is_timed_out(&self, now: f32, timeout_secs: f32) -> bool {
        self.state == ConnectionState::Connected && now - self.last_heartbeat > timeout_secs
    }
}

// ── NetRole ───────────────────────────────────────────────────────────────────

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize, Default)]
pub enum NetRole {
    #[default]
    None,
    Client,
    Server,
    ListenServer,
}

// ── NetworkManager ────────────────────────────────────────────────────────────

/// Central networking hub — manages connections and packet dispatch.
pub struct NetworkManager {
    role:         NetRole,
    connections:  HashMap<u32, Connection>,
    next_conn_id: u32,
    sequence_num: u32,
}

impl NetworkManager {
    pub fn new(role: NetRole) -> Self {
        Self {
            role,
            connections:  HashMap::new(),
            next_conn_id: 1,
            sequence_num: 0,
        }
    }

    pub fn init(&self) {
        tracing::info!(target: "Networking", "Network manager initialized (role: {:?})", self.role);
    }

    pub fn shutdown(&mut self) {
        self.connections.clear();
        tracing::info!(target: "Networking", "Network manager shutdown");
    }

    pub fn role(&self) -> NetRole { self.role }

    pub fn connect(&mut self, address: impl Into<String>, port: u16) -> u32 {
        let id = self.next_conn_id;
        self.next_conn_id += 1;
        let mut conn = Connection::new(id, address, port);
        conn.set_state(ConnectionState::Connecting);
        tracing::info!(target: "Networking", "Connecting to {}:{}", conn.address(), conn.port());
        self.connections.insert(id, conn);
        id
    }

    pub fn disconnect(&mut self, conn_id: u32) {
        if let Some(conn) = self.connections.get_mut(&conn_id) {
            conn.set_state(ConnectionState::Disconnecting);
            tracing::info!(target: "Networking", "Disconnecting connection {}", conn_id);
        }
        self.connections.remove(&conn_id);
    }

    pub fn send(&mut self, conn_id: u32, mut packet: Packet) {
        self.sequence_num += 1;
        packet.sequence_num = self.sequence_num;
        if let Some(conn) = self.connections.get_mut(&conn_id) {
            conn.queue_send(packet);
        }
    }

    pub fn broadcast(&mut self, packet: Packet) {
        let ids: Vec<u32> = self.connections.keys().copied().collect();
        for id in ids {
            self.send(id, packet.clone());
        }
    }

    pub fn tick(&mut self, dt: f32, now: f32) {
        const TIMEOUT: f32 = 30.0;
        let timed_out: Vec<u32> = self.connections
            .values()
            .filter(|c| c.is_timed_out(now, TIMEOUT))
            .map(|c| c.id())
            .collect();
        for id in timed_out {
            tracing::warn!(target: "Networking", "Connection {} timed out", id);
            self.connections.remove(&id);
        }
        let _ = dt;
    }

    pub fn connection(&self, id: u32) -> Option<&Connection> { self.connections.get(&id) }
    pub fn connection_mut(&mut self, id: u32) -> Option<&mut Connection> { self.connections.get_mut(&id) }
    pub fn connection_count(&self) -> usize { self.connections.len() }
    pub fn connections(&self) -> impl Iterator<Item = &Connection> { self.connections.values() }
}

impl Default for NetworkManager {
    fn default() -> Self { Self::new(NetRole::None) }
}
