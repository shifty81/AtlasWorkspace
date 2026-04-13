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

    // Platform targets (DeploymentTargetEditorV1)
    static constexpr struct { const char* name; uint32_t color; } kPlatforms[] = {
        {"Win64",   0x4488CCFF},
        {"Mac",     0x88AABBFF},
        {"Linux",   0xCCAA44FF},
        {"iOS",     0x44AACCFF},
        {"Android", 0x4EC94EFF},
        {"Console", 0xAA6633FF},
    };
    static constexpr int kPlatCount = 6;

    // Pipeline stages (BuildPipelineEditorV1: Compile/Link/Package/Cook/Deploy)
    static constexpr struct {
        const char* name;
        bool        enabled;
        uint32_t    color;
    } kStages[] = {
        {"Compile",  true,  0x4488CCFF},
        {"Link",     true,  0x44CC88FF},
        {"Cook",     true,  0xCCAA44FF},
        {"Package",  true,  0x884488FF},
        {"Sign",     false, 0x886644FF},
        {"Deploy",   false, 0xCC4466FF},
    };
    static constexpr int kStageCount = 6;

    // ── Build Config panel ────────────────────────────────────────
    ctx.drawPanel(ctx.x, ctx.y, configW, ctx.h, "Build Config");
    {
        ctx.drawStatRow(ctx.x + 8.f, ctx.y + 28.f, "Mode:", buildModeName(m_buildMode));
        if (!m_activeTarget.empty()) {
            std::string tgt = m_activeTarget;
            if (tgt.size() > 20) tgt = tgt.substr(0, 17) + "...";
            ctx.drawStatRow(ctx.x + 8.f, ctx.y + 46.f, "Target:", tgt.c_str());
        }

        // Build mode selector buttons
        float mbY = ctx.y + 68.f;
        ctx.ui.drawText(ctx.x + 8.f, mbY - 10.f, "Mode:", ctx.kTextMuted);
        for (const auto& m : kModes) {
            if (mbY + 18.f > ctx.y + ctx.h - 90.f) break;
            bool active = (m_buildMode == m.mode);
            uint32_t bg = active ? ctx.kAccentBlue : ctx.kButtonBg;
            if (ctx.drawButton(ctx.x + 8.f, mbY, configW - 16.f, 16.f, m.label, bg))
                m_buildMode = m.mode;
            mbY += 20.f;
        }

        // Platform selector (DeploymentTargetEditorV1)
        mbY += 6.f;
        ctx.ui.drawText(ctx.x + 8.f, mbY, "Platform:", ctx.kTextMuted);
        mbY += 14.f;
        float pbx = ctx.x + 8.f;
        for (int i = 0; i < kPlatCount; ++i) {
            if (pbx + 40.f > ctx.x + configW - 4.f) { pbx = ctx.x + 8.f; mbY += 20.f; }
            if (mbY + 18.f > ctx.y + ctx.h - 36.f) break;
            ctx.ui.drawRect({pbx, mbY, 36.f, 16.f}, kPlatforms[i].color);
            ctx.ui.drawRectOutline({pbx, mbY, 36.f, 16.f}, ctx.kBorder, 1.f);
            ctx.ui.drawText(pbx + 4.f, mbY + 2.f, kPlatforms[i].name, ctx.kTextPrimary);
            pbx += 40.f;
        }
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
        float ly = ctx.y + 10.f;

        // Pipeline stage list (BuildPipelineEditorV1)
        ctx.ui.drawText(ctx.x + configW + 8.f, ly, "Pipeline stages:", ctx.kTextMuted);
        ly += 14.f;
        float stX = ctx.x + configW + 8.f;
        for (int i = 0; i < kStageCount; ++i) {
            float stW = static_cast<float>(std::strlen(kStages[i].name)) * 6.f + 14.f;
            if (stX + stW > ctx.x + configW + logW - 4.f) { stX = ctx.x + configW + 8.f; ly += 18.f; }
            uint32_t stageBg = kStages[i].enabled ? kStages[i].color : 0x333333FF;
            ctx.ui.drawRect({stX, ly, stW, 14.f}, stageBg);
            ctx.ui.drawRectOutline({stX, ly, stW, 14.f}, ctx.kBorder, 1.f);
            ctx.ui.drawText(stX + 4.f, ly + 1.f, kStages[i].name,
                            kStages[i].enabled ? ctx.kTextPrimary : ctx.kTextMuted);
            // Arrow between stages
            if (i < kStageCount - 1 && kStages[i].enabled && kStages[i + 1].enabled)
                ctx.ui.drawText(stX + stW + 2.f, ly + 1.f, ">", ctx.kTextMuted);
            stX += stW + 18.f;
        }
        ly += 22.f;
        ctx.ui.drawRect({ctx.x + configW + 4.f, ly, logW - 8.f, 1.f}, ctx.kBorder);
        ly += 8.f;

        // Build status & log lines
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

        // Stub log lines
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
