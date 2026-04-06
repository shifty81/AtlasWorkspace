# Atlas Workspace — Deployable AI Tool Packages

> **Source code** lives in `AtlasAI/` (repo root).
> **Deployable packages** live here in `Atlas/Workspace/`.
> **Runtime IPC** goes through `.novaforge/pipeline/`.

## Directory Layout

```
Atlas/
└── Workspace/
    ├── Arbiter/              ← Atlas_Arbiter deployable package
    │   ├── arbiter_cli.py    ← CLI entry point
    │   ├── rules/            ← .arbiter.json rule files
    │   ├── Dockerfile        ← Container deployment
    │   └── config.toml       ← Runtime configuration
    ├── SwissAgent/           ← Atlas_SwissAgent deployable package
    │   ├── cli.py            ← CLI entry point
    │   ├── core/             ← Agent core modules
    │   ├── llm/              ← LLM backend adapters
    │   ├── tools/            ← Tool integrations
    │   ├── Dockerfile        ← Container deployment
    │   └── config.toml       ← Runtime configuration
    └── Sessions/             ← Runtime session data (gitignored)
        └── <session_id>/
```

## Relationship to Other Directories

| Directory | Purpose | Committed |
|-----------|---------|-----------|
| `AtlasAI/` | AI tool **source code** and reference snippets | Yes |
| `Atlas/Workspace/` | **Deployable** tool packages (staged for execution) | Yes (except Sessions/) |
| `.novaforge/pipeline/` | **Runtime IPC** — ChangeEvent JSON files between tools | No (gitignored) |

## Pipeline Contract

All tools communicate exclusively through `.novaforge/pipeline/`:

- **Input:** Reads `pipeline/changes/*.change.json` events
- **Output:** Writes `pipeline/changes/<timestamp>_atlasai_<type>.change.json`
- **No direct file writes** to source — everything goes through pipeline events

## Usage

```bash
# Run Arbiter from workspace
cd Atlas/Workspace/Arbiter
python arbiter_cli.py --help

# Run SwissAgent from workspace
cd Atlas/Workspace/SwissAgent
python cli.py --help

# Or via Docker
docker build -t atlas-arbiter Atlas/Workspace/Arbiter/
docker build -t atlas-swissagent Atlas/Workspace/SwissAgent/
```

## Build Script Integration

The build scripts (`Scripts/build.sh`, `Scripts/build_win.cmd`) scaffold this
directory as a post-build step, ensuring the workspace is always ready after
a successful C++ build.
