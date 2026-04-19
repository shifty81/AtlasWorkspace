//! NovaForge dedicated server — headless entry point.
//!
//! Runs the engine in Server mode: no window, no rendering. Accepts client
//! connections via atlas_networking and simulates at the configured tick rate.
//!
//! Launch arguments:
//!   --port=<u16>       listen port (default: 7777)
//!   --tick-rate=<u32>  server tick rate (default: 30)
//!   --max-ticks=<u32>  stop after N ticks (0 = run forever)

use std::env;
use tracing_subscriber::EnvFilter;

use atlas_core::NF_VERSION_STRING;
use atlas_engine::{Engine, EngineConfig, EngineMode};
use atlas_networking::{NetworkManager, NetRole};

fn main() {
    tracing_subscriber::fmt()
        .with_env_filter(EnvFilter::from_default_env().add_directive("info".parse().unwrap()))
        .with_target(true)
        .init();

    // ── CLI ───────────────────────────────────────────────────────────────────
    let mut port:      u16 = 7777;
    let mut tick_rate: u32 = 30;
    let mut max_ticks: u32 = 0;

    for arg in env::args().skip(1) {
        if let Some(v) = arg.strip_prefix("--port=") {
            port = v.parse().unwrap_or(7777);
        } else if let Some(v) = arg.strip_prefix("--tick-rate=") {
            tick_rate = v.parse().unwrap_or(30);
        } else if let Some(v) = arg.strip_prefix("--max-ticks=") {
            max_ticks = v.parse().unwrap_or(0);
        }
    }

    tracing::info!(target: "NovaForgeServer", "=== NovaForge Dedicated Server v{NF_VERSION_STRING} ===");
    tracing::info!(target: "NovaForgeServer", "Port: {port}  TickRate: {tick_rate}  MaxTicks: {max_ticks}");

    // ── Networking ────────────────────────────────────────────────────────────
    let mut net = NetworkManager::new(NetRole::Server);
    net.init();

    // ── Engine ─────────────────────────────────────────────────────────────────
    let mut engine = Engine::new(EngineConfig {
        mode:      EngineMode::Server,
        tick_rate,
        max_ticks,
        headless:  true,
        ..Default::default()
    });
    engine.init_core();
    engine.register_system("Networking");
    engine.register_system("Physics");
    engine.register_system("Simulation");

    tracing::info!(target: "NovaForgeServer", "Server ready — listening on :{port}");

    let run_ticks = if max_ticks > 0 { max_ticks } else { 300 };
    engine.run_headless(run_ticks);

    net.shutdown();
    tracing::info!(target: "NovaForgeServer", "Server shutdown (ticks={})", engine.tick_count());
}
