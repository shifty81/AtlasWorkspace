# AtlasWorkspace — Rust multi-stage build container
#
# Stages:
#   planner  — compute the dependency recipe (cargo-chef)
#   deps     — build and cache all Cargo dependencies
#   builder  — compile workspace crates and produce binaries
#   server   — minimal runtime image for the dedicated server
#   workspace — minimal runtime image for the workspace IDE host
#
# Build a specific stage:
#   docker build --target server   -t novaforge-server .
#   docker build --target workspace -t atlas-workspace .

# ── Toolchain version ─────────────────────────────────────────────────────────
ARG RUST_VERSION=1.86

# ── 1. Planner — generate a cacheable dependency recipe ───────────────────────
FROM rust:${RUST_VERSION}-slim-bookworm AS planner

# cargo-chef caches layer rebuilds when only source code changes
RUN cargo install cargo-chef --locked

WORKDIR /app
COPY . .
RUN cargo chef prepare --recipe-path recipe.json

# ── 2. Deps — compile all third-party dependencies ────────────────────────────
FROM rust:${RUST_VERSION}-slim-bookworm AS deps

RUN apt-get update && apt-get install -y --no-install-recommends \
    pkg-config \
    libssl-dev \
    && rm -rf /var/lib/apt/lists/*

RUN cargo install cargo-chef --locked

WORKDIR /app
COPY --from=planner /app/recipe.json recipe.json
RUN cargo chef cook --release --recipe-path recipe.json

# ── 3. Builder — compile workspace code ───────────────────────────────────────
FROM rust:${RUST_VERSION}-slim-bookworm AS builder

RUN apt-get update && apt-get install -y --no-install-recommends \
    pkg-config \
    libssl-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Reuse the pre-built dependency cache
COPY --from=deps /app/target            target
COPY --from=deps /usr/local/cargo       /usr/local/cargo

# Copy full source tree and build release binaries
COPY . .
RUN cargo build --release \
    --bin atlas_workspace \
    --bin novaforge_editor \
    --bin novaforge_server

# ── 4. Server — minimal production image for the dedicated server ─────────────
FROM debian:bookworm-slim AS server

RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY --from=builder /app/target/release/novaforge_server /usr/local/bin/novaforge_server

ENV RUST_LOG=info
EXPOSE 7777/udp
EXPOSE 7777/tcp

ENTRYPOINT ["/usr/local/bin/novaforge_server"]
CMD ["--port=7777", "--tick-rate=30"]

# ── 5. Workspace — image for running the AtlasWorkspace IDE host ──────────────
FROM debian:bookworm-slim AS workspace

RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY --from=builder /app/target/release/atlas_workspace /usr/local/bin/atlas_workspace

ENV RUST_LOG=info

ENTRYPOINT ["/usr/local/bin/atlas_workspace"]
CMD []

