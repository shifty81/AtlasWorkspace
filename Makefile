# AtlasWorkspace — Rust workspace task shortcuts
# Run `cargo ws` (alias) or use these Make targets from the repo root.

CARGO     := cargo
LOG_DIR   := Logs
LOG_FILE  := $(LOG_DIR)/build.log
TEST_LOG  := $(LOG_DIR)/test.log

# Binaries produced by the workspace
BIN_WORKSPACE := atlas_workspace
BIN_EDITOR    := novaforge_editor
BIN_SERVER    := novaforge_server
BIN_GAME      := novaforge_game

.PHONY: help check build build-release test lint fmt doc \
        run-workspace run-editor run-server \
        clean clean-target

# ── Help ──────────────────────────────────────────────────────────────────────

help: ## Show this help
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | \
		awk 'BEGIN {FS = ":.*?## "}; {printf "  \033[36m%-20s\033[0m %s\n", $$1, $$2}'

# ── Type-check (fast) ─────────────────────────────────────────────────────────

check: ## Fast type-check all workspace crates (no codegen)
	@mkdir -p $(LOG_DIR)
	$(CARGO) check --workspace 2>&1 | tee -a $(LOG_FILE)

# ── Build ─────────────────────────────────────────────────────────────────────

build: ## Debug build — all workspace crates and binaries
	@mkdir -p $(LOG_DIR)
	$(CARGO) build --workspace 2>&1 | tee -a $(LOG_FILE)

build-release: ## Optimised release build
	@mkdir -p $(LOG_DIR)
	$(CARGO) build --workspace --release 2>&1 | tee -a $(LOG_FILE)

build-ci: ## CI profile build (opt-level=1, fast)
	@mkdir -p $(LOG_DIR)
	$(CARGO) build --workspace --profile ci 2>&1 | tee -a $(LOG_FILE)

# ── Test ──────────────────────────────────────────────────────────────────────

test: ## Run all workspace tests
	@mkdir -p $(LOG_DIR)
	$(CARGO) test --workspace 2>&1 | tee $(TEST_LOG)

test-release: ## Run all workspace tests in release mode
	@mkdir -p $(LOG_DIR)
	$(CARGO) test --workspace --release 2>&1 | tee $(TEST_LOG)

# ── Lint & format ─────────────────────────────────────────────────────────────

lint: ## Run Clippy — warnings become errors
	$(CARGO) clippy --workspace -- -D warnings

fmt: ## Check formatting (use `make fmt-fix` to apply)
	$(CARGO) fmt --all -- --check

fmt-fix: ## Apply rustfmt to all source files
	$(CARGO) fmt --all

# ── Documentation ─────────────────────────────────────────────────────────────

doc: ## Build and open workspace documentation
	$(CARGO) doc --workspace --no-deps --open

doc-ci: ## Build documentation without opening (CI)
	$(CARGO) doc --workspace --no-deps

# ── Run binaries ──────────────────────────────────────────────────────────────

run-workspace: build ## Launch the AtlasWorkspace IDE host
	$(CARGO) run --bin $(BIN_WORKSPACE)

run-editor: build ## Launch the NovaForge editor
	$(CARGO) run --bin $(BIN_EDITOR)

run-server: build ## Launch the NovaForge dedicated server (headless)
	$(CARGO) run --bin $(BIN_SERVER)

run-game: build ## Launch the NovaForge game client
	$(CARGO) run --bin $(BIN_GAME)

# ── Clean ─────────────────────────────────────────────────────────────────────

clean: ## Remove Cargo build artifacts (target/) and logs
	$(CARGO) clean
	rm -rf $(LOG_DIR)

clean-logs: ## Remove log files only
	rm -rf $(LOG_DIR)

