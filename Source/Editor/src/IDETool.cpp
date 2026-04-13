// NF::Editor — IDETool implementation.
//
// Ninth primary IHostedTool from Phase 3 consolidation.
// Provides an integrated code/script editor with a file tree, code editor
// area, and output panel within the AtlasWorkspace.

#include "NF/Editor/IDETool.h"
#include "NF/Workspace/ToolViewRenderContext.h"
#include "NF/Workspace/WorkspaceShell.h"
#include "NF/Workspace/AssetCatalog.h"
#include <cstdio>
#include <algorithm>

namespace NF {

IDETool::IDETool() {
    buildDescriptor();
}

void IDETool::buildDescriptor() {
    m_descriptor.toolId      = kToolId;
    m_descriptor.displayName = "IDE";
    m_descriptor.category    = HostedToolCategory::Utility;
    m_descriptor.isPrimary   = true;
    m_descriptor.acceptsProjectExtensions = true;

    m_descriptor.supportedPanels = {
        "panel.file_tree",
        "panel.code_editor",
        "panel.find_references",
        "panel.console",
    };

    m_descriptor.commands = {
        "ide.open_file",
        "ide.save_file",
        "ide.save_all",
        "ide.go_to_definition",
        "ide.find_in_files",
        "ide.toggle_comment",
        "ide.format_document",
        "ide.run_script",
        "ide.new_script",
    };
}

bool IDETool::initialize() {
    if (m_state != HostedToolState::Unloaded) return false;
    m_editMode   = IDEEditMode::Code;
    m_stats      = {};
    m_activeFile = {};
    m_openFiles  = {};
    m_state      = HostedToolState::Ready;
    return true;
}

void IDETool::shutdown() {
    m_state           = HostedToolState::Unloaded;
    m_editMode        = IDEEditMode::Code;
    m_stats           = {};
    m_activeFile      = {};
    m_openFiles       = {};
    m_activeProjectId = {};
}

void IDETool::activate() {
    if (m_state == HostedToolState::Ready || m_state == HostedToolState::Suspended) {
        m_state = HostedToolState::Active;
    }
}

void IDETool::suspend() {
    if (m_state == HostedToolState::Active) {
        m_state = HostedToolState::Suspended;
    }
}

void IDETool::update(float /*dt*/) {
    // IDE updates are event-driven (file save, compile, index changes).
}

void IDETool::onProjectLoaded(const std::string& projectId) {
    m_activeProjectId = projectId;
    // Reset open file state; the project indexer will re-populate.
    m_openFiles.clear();
    m_activeFile.clear();
    m_stats = {};
}

void IDETool::onProjectUnloaded() {
    m_activeProjectId = {};
    m_openFiles.clear();
    m_activeFile.clear();
    m_stats = {};
}

void IDETool::setEditMode(IDEEditMode mode) {
    m_editMode = mode;
}

void IDETool::openFile(const std::string& path) {
    // Add to open files if not already present.
    auto it = std::find(m_openFiles.begin(), m_openFiles.end(), path);
    if (it == m_openFiles.end()) {
        m_openFiles.push_back(path);
        ++m_stats.openFileCount;
    }
    m_activeFile = path;
}

void IDETool::closeFile(const std::string& path) {
    auto it = std::find(m_openFiles.begin(), m_openFiles.end(), path);
    if (it != m_openFiles.end()) {
        m_openFiles.erase(it);
        if (m_stats.openFileCount > 0) --m_stats.openFileCount;
    }
    if (m_activeFile == path) {
        m_activeFile = m_openFiles.empty() ? std::string{} : m_openFiles.back();
    }
}

void IDETool::markDirty()  { m_stats.isDirty = true;  }
void IDETool::clearDirty() { m_stats.isDirty = false; }

void IDETool::renderToolView(const ToolViewRenderContext& ctx) const {
    // Three-column layout: File Tree (20%) | Code Editor (60%) | Output (20%)
    const float treeW   = ctx.w * 0.20f;
    const float editorW = ctx.w * 0.60f;
    const float outputW = ctx.w - treeW - editorW;

    // ── File Tree panel ───────────────────────────────────────────
    ctx.drawPanel(ctx.x, ctx.y, treeW, ctx.h, "File Tree");

    // Show indexed script files from the asset catalog when a project is open.
    float ty = ctx.y + 28.f;
    if (ctx.shell && ctx.shell->hasProject()) {
        const AssetCatalog& catalog = ctx.shell->assetCatalog();
        size_t scriptCount = catalog.countByType(AssetTypeTag::Script);

        if (scriptCount > 0) {
            // Render script file entries from the catalog.
            auto scripts = catalog.query([](const AssetDescriptor& d) {
                return d.typeTag == AssetTypeTag::Script;
            });
            for (size_t i = 0; i < scripts.size() && ty + 18.f < ctx.y + ctx.h - 4.f; ++i) {
                const auto* desc = scripts[i];
                bool isActive = (desc->sourcePath == m_activeFile);
                std::string name = desc->displayName;
                if (name.size() > 18) name = name.substr(0, 15) + "...";
                ctx.ui.drawText(ctx.x + 10.f, ty,
                                name.c_str(),
                                isActive ? ctx.kTextPrimary : ctx.kTextSecond);
                ty += 18.f;
            }
        } else {
            ctx.ui.drawText(ctx.x + 8.f, ty, "No scripts in project", ctx.kTextMuted);
        }
    } else {
        ctx.ui.drawText(ctx.x + 8.f, ty, "Open a project", ctx.kTextMuted);
        ctx.ui.drawText(ctx.x + 8.f, ty + 16.f, "to browse files", ctx.kTextMuted);
    }

    // ── Code Editor panel ─────────────────────────────────────────
    const float ex = ctx.x + treeW;
    ctx.drawPanel(ex, ctx.y, editorW, ctx.h, "Code Editor");

    // Edit mode pill
    ctx.drawStatusPill(ex + 8.f, ctx.y + 30.f, ideEditModeName(m_editMode), ctx.kAccentBlue);

    if (m_stats.isDirty) {
        ctx.ui.drawText(ex + 8.f, ctx.y + ctx.h - 20.f, "* unsaved changes", ctx.kRed);
    }

    // Open file tab bar
    if (!m_openFiles.empty()) {
        float tabX = ex + 8.f;
        float tabY = ctx.y + 52.f;
        for (size_t i = 0; i < m_openFiles.size() && tabX < ex + editorW - 4.f; ++i) {
            const auto& f = m_openFiles[i];
            // Extract filename
            std::string fname = f;
            auto sep = fname.find_last_of("/\\");
            if (sep != std::string::npos) fname = fname.substr(sep + 1);
            if (fname.size() > 14) fname = fname.substr(0, 11) + "...";

            bool active = (m_openFiles[i] == m_activeFile);
            float tw = static_cast<float>(fname.size()) * 8.f + 16.f;
            ctx.ui.drawRect({tabX, tabY, tw, 20.f},
                            active ? ctx.kAccentBlue : ctx.kCardBg);
            ctx.ui.drawRectOutline({tabX, tabY, tw, 20.f}, ctx.kBorder, 1.f);
            ctx.ui.drawText(tabX + 6.f, tabY + 3.f, fname.c_str(),
                            active ? ctx.kTextPrimary : ctx.kTextSecond);
            tabX += tw + 2.f;
        }
    }

    // Editor body — show line number gutter + code area placeholder
    {
        const float bodyY = ctx.y + 76.f;
        const float bodyH = ctx.h - 76.f - 24.f;
        const float gutterW = 32.f;

        // Gutter
        ctx.ui.drawRect({ex, bodyY, gutterW, bodyH}, 0x1E1E1EFF);
        ctx.ui.drawRect({ex + gutterW, bodyY, 1.f, bodyH}, ctx.kBorder);

        // Line numbers
        for (float ly = bodyY + 4.f; ly < bodyY + bodyH - 4.f; ly += 18.f) {
            int lineNo = static_cast<int>((ly - bodyY) / 18.f) + 1;
            char lnBuf[16];
            std::snprintf(lnBuf, sizeof(lnBuf), "%3d", lineNo);
            ctx.ui.drawText(ex + 2.f, ly, lnBuf, ctx.kTextMuted);
        }

        // Code body
        ctx.ui.drawRect({ex + gutterW + 1.f, bodyY, editorW - gutterW - 1.f, bodyH},
                        0x1A1A1AFF);

        if (m_activeFile.empty()) {
            float hx = ex + gutterW + (editorW - gutterW - 160.f) * 0.5f;
            float hy = bodyY + (bodyH - 14.f) * 0.5f;
            ctx.ui.drawText(hx, hy, "Select a file to edit", ctx.kTextMuted);
        } else {
            // Show a cursor line indicator at line 1
            ctx.ui.drawRect({ex + gutterW + 1.f, bodyY + 2.f,
                             editorW - gutterW - 1.f, 18.f},
                            0x2A3A2AFF); // active line highlight
            ctx.ui.drawText(ex + gutterW + 8.f, bodyY + 4.f, "|", ctx.kAccentBlue);
        }
    }

    // ── Output / Symbol panel ─────────────────────────────────────
    const float ox = ex + editorW;
    ctx.drawPanel(ox, ctx.y, outputW, ctx.h, "Output");

    {
        float oy = ctx.y + 28.f;
        // File counts from project index
        if (ctx.shell && ctx.shell->hasProject()) {
            const AssetCatalog& catalog = ctx.shell->assetCatalog();
            size_t scripts  = catalog.countByType(AssetTypeTag::Script);
            size_t shaders  = catalog.countByType(AssetTypeTag::Shader);

            char buf[48];
            std::snprintf(buf, sizeof(buf), "Scripts: %zu", scripts);
            ctx.ui.drawText(ox + 8.f, oy, buf, ctx.kTextSecond);
            oy += 18.f;
            if (shaders > 0) {
                std::snprintf(buf, sizeof(buf), "Shaders: %zu", shaders);
                ctx.ui.drawText(ox + 8.f, oy, buf, ctx.kTextSecond);
                oy += 18.f;
            }
            oy += 8.f;
            ctx.ui.drawRect({ox + 4.f, oy, outputW - 8.f, 1.f}, ctx.kBorder);
            oy += 10.f;
        }

        if (m_stats.openFileCount > 0) {
            char buf[32];
            std::snprintf(buf, sizeof(buf), "%u open", m_stats.openFileCount);
            ctx.ui.drawText(ox + 8.f, oy, buf, ctx.kTextMuted);
            oy += 18.f;
        }

        ctx.ui.drawText(ox + 8.f, oy, m_stats.isRunning ? "Running..." : "Ready",
                        m_stats.isRunning ? ctx.kGreen : ctx.kTextMuted);
    }
}

} // namespace NF
