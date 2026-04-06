# AtlasWorkspace — Development task shortcuts
# Usage: make <target>

.PHONY: help configure build build-debug build-release test clean

help: ## Show this help
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | \
		awk 'BEGIN {FS = ":.*?## "}; {printf "  \033[36m%-20s\033[0m %s\n", $$1, $$2}'

# ── Configure ─────────────────────────────────────────────────────

configure: ## Configure debug build with tests
	cmake --preset debug

configure-release: ## Configure release build
	cmake --preset release

# ── Build ─────────────────────────────────────────────────────────

build: configure ## Build debug (all targets)
	cmake --build --preset debug --parallel

build-release: configure-release ## Build release (all targets)
	cmake --build --preset release --parallel

# ── Test ──────────────────────────────────────────────────────────

test: build ## Run all tests
	ctest --preset debug

# ── Clean ─────────────────────────────────────────────────────────

clean: ## Remove all build artifacts
	rm -rf Builds/ build/ out/ dist/
