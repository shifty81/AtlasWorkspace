# Repo Audit Checklist

Use this checklist to audit the repo for missing features and direction drift.

## A. Direction and naming
- [ ] AtlasUI is named and treated as the canonical tooling UI framework
- [ ] Old ImGui references are removed or clearly archived/deferred
- [ ] Old Arbiter naming is replaced by AtlasAI where intended
- [ ] Workspace naming is consistent and user-facing naming is not drifting
- [ ] Legacy experimental tool names do not appear as active canonical paths

## B. UI framework
- [ ] backend abstraction exists
- [ ] panels are backend-agnostic
- [ ] GDI fallback exists
- [ ] GPU path is present or actively scaffolded
- [ ] shared widget kit exists
- [ ] theme/token system exists
- [ ] command/shortcut layer exists
- [ ] menu/context menu layer exists
- [ ] docking exists
- [ ] floating host exists
- [ ] tabs/chrome exist
- [ ] tooltip system exists

## C. Tooling shell
- [ ] workspace shell exists or is clearly scaffolded
- [ ] notification center exists or is scaffolded
- [ ] AtlasAI panel host exists or is scaffolded
- [ ] settings/control panel exists or is scaffolded
- [ ] intake flow exists or is scaffolded

## D. Persistence and usability
- [ ] layout persistence exists
- [ ] panel ids are stable
- [ ] focus handling is standardized
- [ ] property grid exists
- [ ] tree/list/table systems exist
- [ ] scroll/virtualization exists
- [ ] typography and icon rules are defined

## E. Logging and debugging
- [ ] build logs route to a logger format
- [ ] logs can be surfaced in tooling
- [ ] error notifications can escalate to AtlasAI
- [ ] debug/fix workflow is spec'd

## F. Roadmap order
- [ ] tooling-first roadmap is reflected in docs
- [ ] game work is behind tooling unless explicitly selected
- [ ] AtlasUI/framework tasks are near the top of the roadmap

## G. Repo hygiene
- [ ] obsolete experiments are archived or clearly marked
- [ ] duplicate UI implementations are reconciled
- [ ] docs match current direction
- [ ] test outputs route to bin/Tests
