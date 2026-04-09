#pragma once
// NF::Editor — Minimal editor core include.
//
// This header includes only the essential editor infrastructure.
// For tool-specific or feature-specific includes, use:
//   - EditorSharedPanels.h   (shared reusable panels)
//   - EditorToolRegistry.h   (tool registration and ecosystem)
//
// Individual headers can always be included directly for faster compilation.

// ── Core editor infrastructure ──────────────────────────────────
#include "NF/Editor/EditorApp.h"
#include "NF/Editor/EditorPanel.h"
#include "NF/Editor/EditorEventBus.h"
#include "NF/Editor/EditorTheme.h"
#include "NF/Editor/DockLayout.h"
#include "NF/Editor/LayoutPersistence.h"
#include "NF/Editor/MenuBar.h"
#include "NF/Editor/CommandPalette.h"
#include "NF/Editor/ShortcutManager.h"
#include "NF/Editor/SelectionService.h"
#include "NF/Editor/UndoRedoSystem.h"
#include "NF/Editor/EditorSettings.h"
#include "NF/Editor/EditorCamera.h"

// ── Workspace shell ─────────────────────────────────────────────
#include "NF/Editor/WorkspaceAppRegistry.h"
#include "NF/Editor/WorkspaceLaunchContract.h"
#include "NF/Editor/WorkspacePanelHost.h"
#include "NF/Editor/WorkspaceInputBridge.h"
#include "NF/Editor/WorkspaceShellContract.h"
#include "NF/Editor/WorkspaceLayout.h"

// ── Shared panels (always available) ────────────────────────────
#include "NF/Editor/EditorSharedPanels.h"

// ── Tool ecosystem and registration ─────────────────────────────
#include "NF/Editor/EditorToolRegistry.h"
