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

    // Stub file list for the tree panel (used when no project is open)
    static const char* kStubFiles[] = {
        "GameManager.cpp", "Player.cpp", "Enemy.cpp",
        "InputHandler.cpp", "SceneLoader.cpp"
    };
    static constexpr int kStubCount = 5;

    // ── File Tree panel ───────────────────────────────────────────
    ctx.drawPanel(ctx.x, ctx.y, treeW, ctx.h, "File Tree");

    float ty = ctx.y + 28.f;
    if (ctx.shell && ctx.shell->hasProject()) {
        const AssetCatalog& catalog = ctx.shell->assetCatalog();
        size_t scriptCount = catalog.countByType(AssetTypeTag::Script);

        if (scriptCount > 0) {
            auto scripts = catalog.query([](const AssetDescriptor& d) {
                return d.typeTag == AssetTypeTag::Script;
            });
            for (size_t i = 0; i < scripts.size() && ty + 18.f < ctx.y + ctx.h - 4.f; ++i) {
                const auto* desc = scripts[i];
                if (!desc) continue;
                bool isActive = (desc->sourcePath == m_activeFile);
                bool hov = ctx.isHovered({ctx.x + 2.f, ty - 2.f, treeW - 4.f, 18.f});
                uint32_t bg = isActive ? 0x1A3A6AFF : (hov ? 0x2A2A3AFF : 0x00000000u);
                if (bg) ctx.ui.drawRect({ctx.x + 2.f, ty - 2.f, treeW - 4.f, 18.f}, bg);
                std::string name = desc->displayName;
                static constexpr size_t kMaxNameDisplayLen = 18;
                static constexpr size_t kTruncatedNameLen  = 15;
                if (name.size() > kMaxNameDisplayLen)
                    name = name.substr(0, kTruncatedNameLen) + "...";
                ctx.ui.drawText(ctx.x + 10.f, ty, name.c_str(),
                                isActive ? ctx.kTextPrimary : ctx.kTextSecond);
                if (ctx.hitRegion({ctx.x + 2.f, ty - 2.f, treeW - 4.f, 18.f}, false)) {
                    m_viewSelectedFile = static_cast<int>(i);
                    // Note: openFile() is non-const; we route through command bus.
                    if (ctx.shell)
                        (void)ctx.shell->commandBus().execute("ide.open_file:" + desc->sourcePath);
                }
                ty += 18.f;
            }
        } else {
            ctx.ui.drawText(ctx.x + 8.f, ty, "No scripts in project", ctx.kTextMuted);
        }
    } else {
        // Stub file list when no project is open
        for (int i = 0; i < kStubCount && ty + 18.f < ctx.y + ctx.h - 4.f; ++i) {
            bool isActive = (m_viewSelectedFile == i);
            bool hov = ctx.isHovered({ctx.x + 2.f, ty - 2.f, treeW - 4.f, 18.f});
            uint32_t bg = isActive ? 0x1A3A6AFF : (hov ? 0x2A2A3AFF : 0x00000000u);
            if (bg) ctx.ui.drawRect({ctx.x + 2.f, ty - 2.f, treeW - 4.f, 18.f}, bg);
            ctx.ui.drawText(ctx.x + 10.f, ty, kStubFiles[i],
                            isActive ? ctx.kTextPrimary : ctx.kTextSecond);
            if (ctx.hitRegion({ctx.x + 2.f, ty - 2.f, treeW - 4.f, 18.f}, false))
                m_viewSelectedFile = isActive ? -1 : i;
            ty += 18.f;
        }
    }

    // ── Code Editor panel ─────────────────────────────────────────
    const float ex = ctx.x + treeW;
    ctx.drawPanel(ex, ctx.y, editorW, ctx.h, "Code Editor");

    // Edit mode buttons
    static constexpr struct { const char* label; IDEEditMode mode; } kIDEModes[] = {
        {"Code",      IDEEditMode::Code},
        {"Diff",      IDEEditMode::Diff},
        {"Read-Only", IDEEditMode::ReadOnly},
    };
    {
        float bx = ex + 8.f;
        for (const auto& m : kIDEModes) {
            float bw = static_cast<float>(std::strlen(m.label)) * 7.f + 14.f;
            bool active = (m_editMode == m.mode);
            if (ctx.drawButton(bx, ctx.y + 28.f, bw, 16.f, m.label,
                               active ? ctx.kAccentBlue : ctx.kButtonBg))
                m_editMode = m.mode;
            bx += bw + 4.f;
        }
    }

    if (m_stats.isDirty)
        ctx.ui.drawText(ex + editorW - 100.f, ctx.y + 30.f, "* unsaved", ctx.kRed);

    // Open file tab bar with clickable tabs
    if (!m_openFiles.empty()) {
        float tabX = ex + 8.f;
        float tabY = ctx.y + 50.f;
        for (size_t i = 0; i < m_openFiles.size() && tabX < ex + editorW - 4.f; ++i) {
            const auto& f = m_openFiles[i];
            std::string fname = f;
            auto sep = fname.find_last_of("/\\");
            if (sep != std::string::npos) fname = fname.substr(sep + 1);
            if (fname.size() > 14) fname = fname.substr(0, 11) + "...";

            bool active = (m_openFiles[i] == m_activeFile);
            float tw = static_cast<float>(fname.size()) * 8.f + 16.f;
            uint32_t bg = active ? ctx.kAccentBlue : ctx.kCardBg;
            ctx.ui.drawRect({tabX, tabY, tw, 20.f}, bg);
            ctx.ui.drawRectOutline({tabX, tabY, tw, 20.f}, ctx.kBorder, 1.f);
            ctx.ui.drawText(tabX + 6.f, tabY + 3.f, fname.c_str(),
                            active ? ctx.kTextPrimary : ctx.kTextSecond);
            // Click: switch active file via command bus
            if (ctx.hitRegion({tabX, tabY, tw, 20.f}, false) && ctx.shell)
                (void)ctx.shell->commandBus().execute("ide.open_file:" + m_openFiles[i]);
            tabX += tw + 2.f;
        }
    } else if (m_viewSelectedFile >= 0) {
        // Show the stub file name in a pseudo tab
        const char* fname = m_viewSelectedFile < kStubCount
                          ? kStubFiles[m_viewSelectedFile] : kStubFiles[0];
        float tw = static_cast<float>(std::strlen(fname)) * 8.f + 16.f;
        ctx.ui.drawRect({ex + 8.f, ctx.y + 50.f, tw, 20.f}, ctx.kAccentBlue);
        ctx.ui.drawRectOutline({ex + 8.f, ctx.y + 50.f, tw, 20.f}, ctx.kBorder, 1.f);
        ctx.ui.drawText(ex + 14.f, ctx.y + 53.f, fname, ctx.kTextPrimary);
    }

    // Editor body — line number gutter + code area
    {
        const float bodyY = ctx.y + 76.f;
        const float bodyH = ctx.h - 76.f - 24.f;
        const float gutterW = 32.f;

        ctx.ui.drawRect({ex, bodyY, gutterW, bodyH}, 0x1E1E1EFF);
        ctx.ui.drawRect({ex + gutterW, bodyY, 1.f, bodyH}, ctx.kBorder);

        for (float ly = bodyY + 4.f; ly < bodyY + bodyH - 4.f; ly += 18.f) {
            int lineNo = static_cast<int>((ly - bodyY) / 18.f) + 1;
            char lnBuf[16];
            std::snprintf(lnBuf, sizeof(lnBuf), "%3d", lineNo);
            ctx.ui.drawText(ex + 2.f, ly, lnBuf, ctx.kTextMuted);
        }

        ctx.ui.drawRect({ex + gutterW + 1.f, bodyY, editorW - gutterW - 1.f, bodyH},
                        0x1A1A1AFF);

        bool hasFile = !m_activeFile.empty() || m_viewSelectedFile >= 0;
        if (!hasFile) {
            float hx = ex + gutterW + (editorW - gutterW - 160.f) * 0.5f;
            float hy = bodyY + (bodyH - 14.f) * 0.5f;
            ctx.ui.drawText(hx, hy, "Select a file to edit", ctx.kTextMuted);
        } else {
            // Active line highlight + cursor
            ctx.ui.drawRect({ex + gutterW + 1.f, bodyY + 2.f,
                             editorW - gutterW - 1.f, 18.f}, 0x2A3A2AFF);
            ctx.ui.drawText(ex + gutterW + 8.f, bodyY + 4.f, "|", ctx.kAccentBlue);

            // Show filename as first comment line
            const char* fn = !m_activeFile.empty() ? m_activeFile.c_str()
                           : (m_viewSelectedFile >= 0 && m_viewSelectedFile < kStubCount
                              ? kStubFiles[m_viewSelectedFile] : "");
            if (*fn) {
                std::string comment = std::string("// ") + fn;
                ctx.ui.drawText(ex + gutterW + 8.f, bodyY + 22.f, comment, ctx.kTextMuted);
            }
        }
    }

    // ── Output / Symbol panel ─────────────────────────────────────
    const float ox = ex + editorW;
    ctx.drawPanel(ox, ctx.y, outputW, ctx.h, "Output");

    {
        float oy = ctx.y + 28.f;
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
        oy += 18.f;

        // Run / Stop buttons
        if (!m_stats.isRunning) {
            if (ctx.drawButton(ox + 8.f, ctx.y + ctx.h - 30.f,
                               outputW - 16.f, 20.f, "Run Script")) {
                if (ctx.shell)
                    (void)ctx.shell->commandBus().execute("ide.run_script");
            }
        } else {
            ctx.drawStatusPill(ox + 8.f, ctx.y + ctx.h - 28.f, "Running...", ctx.kGreen);
        }
    }
}

} // namespace NF
