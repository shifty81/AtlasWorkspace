// NF::Editor — BuildTool implementation.
//
// Seventh real NF::IHostedTool from Phase 3 consolidation.
// Manages build, packaging, and deploy workflows within AtlasWorkspace.

#include "NF/Editor/BuildTool.h"
#include "NF/Workspace/ToolViewRenderContext.h"
#include "NF/Workspace/WorkspaceShell.h"
#include <cstdio>
#include <cstring>

namespace NF {

BuildTool::BuildTool() {
    buildDescriptor();
}

void BuildTool::buildDescriptor() {
    m_descriptor.toolId      = kToolId;
    m_descriptor.displayName = "Build Tool";
    m_descriptor.category    = HostedToolCategory::BuildPackaging;
    m_descriptor.isPrimary   = true;
    m_descriptor.acceptsProjectExtensions = true;

    m_descriptor.supportedPanels = {
        "panel.build_log",
        "panel.inspector",
        "panel.console",
    };

    m_descriptor.commands = {
        "build.run",
        "build.clean",
        "build.cancel",
        "build.open_output",
        "build.set_target",
        "build.run_tests",
    };
}

bool BuildTool::initialize() {
    if (m_state != HostedToolState::Unloaded) return false;
    m_buildMode    = BuildMode::Incremental;
    m_stats        = {};
    m_activeTarget = {};
    m_state        = HostedToolState::Ready;
    return true;
}

void BuildTool::shutdown() {
    m_state           = HostedToolState::Unloaded;
    m_buildMode       = BuildMode::Incremental;
    m_stats           = {};
    m_activeTarget    = {};
    m_activeProjectId = {};
}

void BuildTool::activate() {
    if (m_state == HostedToolState::Ready || m_state == HostedToolState::Suspended) {
        m_state = HostedToolState::Active;
    }
}

void BuildTool::suspend() {
    if (m_state == HostedToolState::Active) {
        m_state = HostedToolState::Suspended;
    }
}

void BuildTool::update(float /*dt*/) {
    // Build progress is driven by async build-log events, not per-frame polling.
}

void BuildTool::onProjectLoaded(const std::string& projectId) {
    m_activeProjectId = projectId;
    m_stats           = {};
    m_activeTarget    = {};
}

void BuildTool::onProjectUnloaded() {
    m_activeProjectId = {};
    m_stats           = {};
    m_activeTarget    = {};
}

void BuildTool::setBuildMode(BuildMode mode) {
    m_buildMode = mode;
}

void BuildTool::setBuilding(bool building) {
    m_stats.isBuilding = building;
}

void BuildTool::setLastBuildMs(float ms) {
    m_stats.lastBuildMs = ms;
}

void BuildTool::setWarningCount(uint32_t count) {
    m_stats.warningCount = count;
}

void BuildTool::setErrorCount(uint32_t count) {
    m_stats.errorCount = count;
}

void BuildTool::clearStats() {
    m_stats = {};
}

void BuildTool::setActiveTarget(const std::string& target) {
    m_activeTarget = target;
}

void BuildTool::renderToolView(const ToolViewRenderContext& ctx) const {
    // Three-column layout: Build Config (25%) | Build Log (55%) | Metrics (20%)
    const float configW  = ctx.w * 0.25f;
    const float logW     = ctx.w * 0.55f;
    const float metricsW = ctx.w - configW - logW;

    // Build mode options
    static constexpr struct { const char* label; BuildMode mode; } kModes[] = {
        {"Incremental", BuildMode::Incremental},
        {"Full",        BuildMode::Full},
        {"ShaderOnly",  BuildMode::ShaderOnly},
    };

    // ── Build Config panel ────────────────────────────────────────
    ctx.drawPanel(ctx.x, ctx.y, configW, ctx.h, "Build Config");
    ctx.drawStatRow(ctx.x + 8.f, ctx.y + 30.f, "Mode:", buildModeName(m_buildMode));
    if (!m_activeTarget.empty()) {
        std::string tgt = m_activeTarget;
        if (tgt.size() > 20) tgt = tgt.substr(0, 17) + "...";
        ctx.drawStatRow(ctx.x + 8.f, ctx.y + 48.f, "Target:", tgt.c_str());
    }

    // Build mode selector buttons
    float mbY = ctx.y + 72.f;
    for (const auto& m : kModes) {
        if (mbY + 20.f > ctx.y + ctx.h - 36.f) break;
        bool active = (m_buildMode == m.mode);
        uint32_t bg = active ? ctx.kAccentBlue : ctx.kButtonBg;
        if (ctx.drawButton(ctx.x + 8.f, mbY, configW - 16.f, 18.f, m.label, bg)) {
            m_buildMode = m.mode;
        }
        mbY += 22.f;
    }

    // Build button at the bottom of the config panel
    bool isBuilding = m_stats.isBuilding || m_viewBuildRequested;
    if (!isBuilding) {
        if (ctx.drawButton(ctx.x + 8.f, ctx.y + ctx.h - 30.f,
                           configW - 16.f, 22.f, "Build Now",
                           0x005A9EFF, ctx.kAccentBlue)) {
            m_viewBuildRequested = true;
            if (ctx.shell)
                (void)ctx.shell->commandBus().execute("build.start");
        }
    } else {
        ctx.drawStatusPill(ctx.x + 8.f, ctx.y + ctx.h - 28.f, "Building...", ctx.kAccentBlue);
    }

    // ── Build Log panel ───────────────────────────────────────────
    ctx.drawPanel(ctx.x + configW, ctx.y, logW, ctx.h, "Build Log");
    {
        float ly = ctx.y + 30.f;
        if (m_stats.errorCount > 0) {
            char errBuf[32];
            std::snprintf(errBuf, sizeof(errBuf), "%u errors", m_stats.errorCount);
            ctx.drawStatusPill(ctx.x + configW + 8.f, ly, errBuf, ctx.kRed);
            ly += 24.f;
        }
        if (m_stats.warningCount > 0) {
            char warnBuf[32];
            std::snprintf(warnBuf, sizeof(warnBuf), "%u warnings", m_stats.warningCount);
            ctx.drawStatusPill(ctx.x + configW + 8.f, ly, warnBuf, 0xE09020FFu);
            ly += 24.f;
        }
        if (m_stats.errorCount == 0 && m_stats.warningCount == 0 && !isBuilding) {
            ctx.ui.drawText(ctx.x + configW + 8.f, ly, "Build clean", ctx.kGreen);
            ly += 18.f;
        }

        // Static stub log lines
        static const char* kLogLines[] = {
            "[INFO]  Build system ready",
            "[INFO]  Target: Windows x64",
            "[INFO]  Mode: Incremental",
            "[INFO]  Awaiting start...",
        };
        for (auto* line : kLogLines) {
            if (ly + 16.f > ctx.y + ctx.h - 4.f) break;
            ctx.ui.drawText(ctx.x + configW + 8.f, ly, line, ctx.kTextSecond);
            ly += 16.f;
        }
    }

    // ── Metrics panel ─────────────────────────────────────────────
    ctx.drawPanel(ctx.x + configW + logW, ctx.y, metricsW, ctx.h, "Metrics");
    const float mx = ctx.x + configW + logW;
    if (m_stats.lastBuildMs > 0.f) {
        char timeBuf[32];
        std::snprintf(timeBuf, sizeof(timeBuf), "%.0f ms", m_stats.lastBuildMs);
        ctx.drawStatRow(mx + 8.f, ctx.y + 30.f, "Last build:", timeBuf);
    } else {
        ctx.ui.drawText(mx + 8.f, ctx.y + 30.f, "No build yet", ctx.kTextMuted);
    }
    ctx.drawStatRow(mx + 8.f, ctx.y + 50.f, "Mode:", buildModeName(m_buildMode));
    char errStr[16]; std::snprintf(errStr, sizeof(errStr), "%u", m_stats.errorCount);
    ctx.drawStatRow(mx + 8.f, ctx.y + 68.f, "Errors:", errStr);
    char warnStr[16]; std::snprintf(warnStr, sizeof(warnStr), "%u", m_stats.warningCount);
    ctx.drawStatRow(mx + 8.f, ctx.y + 86.f, "Warnings:", warnStr);
}

} // namespace NF
