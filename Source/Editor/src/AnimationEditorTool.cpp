// NF::Editor — AnimationEditorTool implementation.
//
// Fourth real NF::IHostedTool from Phase 3 consolidation.
// Manages animation clip and sequencer authoring within AtlasWorkspace.

#include "NF/Editor/AnimationEditorTool.h"
#include "NF/Workspace/ToolViewRenderContext.h"
#include <cstdio>

namespace NF {

AnimationEditorTool::AnimationEditorTool() {
    buildDescriptor();
}

void AnimationEditorTool::buildDescriptor() {
    m_descriptor.toolId      = kToolId;
    m_descriptor.displayName = "Animation Editor";
    m_descriptor.category    = HostedToolCategory::AnimationAuthoring;
    m_descriptor.isPrimary   = true;
    m_descriptor.acceptsProjectExtensions = false;

    m_descriptor.supportedPanels = {
        "panel.viewport_animation",
        "panel.timeline",
        "panel.outliner",
        "panel.inspector",
        "panel.console",
    };

    m_descriptor.commands = {
        "animation.play",
        "animation.pause",
        "animation.stop",
        "animation.record",
        "animation.add_keyframe",
        "animation.delete_keyframe",
        "animation.export_clip",
    };
}

bool AnimationEditorTool::initialize() {
    if (m_state != HostedToolState::Unloaded) return false;
    m_editMode = AnimationEditMode::Timeline;
    m_stats    = {};
    m_state    = HostedToolState::Ready;
    return true;
}

void AnimationEditorTool::shutdown() {
    m_state           = HostedToolState::Unloaded;
    m_editMode        = AnimationEditMode::Timeline;
    m_stats           = {};
    m_activeProjectId = {};
}

void AnimationEditorTool::activate() {
    if (m_state == HostedToolState::Ready || m_state == HostedToolState::Suspended) {
        m_state = HostedToolState::Active;
    }
}

void AnimationEditorTool::suspend() {
    if (m_state == HostedToolState::Active) {
        // Stop playback / recording when backgrounded
        m_stats.isPlaying  = false;
        m_stats.isRecording = false;
        m_state = HostedToolState::Suspended;
    }
}

void AnimationEditorTool::update(float /*dt*/) {
    // Playback cursor advancement is driven by the engine runtime, not here.
}

void AnimationEditorTool::onProjectLoaded(const std::string& projectId) {
    m_activeProjectId  = projectId;
    m_stats            = {};
}

void AnimationEditorTool::onProjectUnloaded() {
    m_activeProjectId  = {};
    m_stats            = {};
}

void AnimationEditorTool::setEditMode(AnimationEditMode mode) {
    m_editMode = mode;
}

void AnimationEditorTool::markDirty() {
    m_stats.isDirty = true;
}

void AnimationEditorTool::clearDirty() {
    m_stats.isDirty = false;
}

void AnimationEditorTool::play() {
    m_stats.isPlaying = true;
}

void AnimationEditorTool::pause() {
    m_stats.isPlaying = false;
}

void AnimationEditorTool::stop() {
    m_stats.isPlaying  = false;
    m_stats.isRecording = false;
}

void AnimationEditorTool::record(bool enable) {
    m_stats.isRecording = enable;
}

void AnimationEditorTool::setClipDurationMs(float ms) {
    m_stats.clipDurationMs = ms;
}

void AnimationEditorTool::setFrameCount(uint32_t count) {
    m_stats.frameCount = count;
}

void AnimationEditorTool::setSelectedBoneCount(uint32_t count) {
    m_stats.selectedBoneCount = count;
}

void AnimationEditorTool::renderToolView(const ToolViewRenderContext& ctx) const {
    // Three-column layout: Skeleton/Clips (20%) | Timeline (55%) | Clip Properties (25%)
    const float skelW     = ctx.w * 0.20f;
    const float timelineW = ctx.w * 0.55f;
    const float propW     = ctx.w - skelW - timelineW;

    // Stub clip list
    static const char* kClipNames[] = {"Idle", "Run", "Jump", "Attack", "Death"};
    static constexpr int kClipCount = 5;

    // ── Skeleton / Clips panel ────────────────────────────────────
    ctx.drawPanel(ctx.x, ctx.y, skelW, ctx.h, "Skeleton / Clips");
    {
        char boneBuf[32];
        std::snprintf(boneBuf, sizeof(boneBuf), "%u bones sel.", m_stats.selectedBoneCount);
        ctx.ui.drawText(ctx.x + 8.f, ctx.y + 30.f, boneBuf, ctx.kTextSecond);

        // Clip list rows
        ctx.ui.drawText(ctx.x + 8.f, ctx.y + 50.f, "Clips:", ctx.kTextMuted);
        float cy2 = ctx.y + 66.f;
        for (int i = 0; i < kClipCount; ++i) {
            if (cy2 + 18.f > ctx.y + ctx.h - 4.f) break;
            bool sel = (m_viewSelectedClip == i);
            bool hov = ctx.isHovered({ctx.x + 2.f, cy2, skelW - 4.f, 16.f});
            uint32_t bg = sel ? ctx.kAccentBlue : (hov ? 0x2A2A3AFF : 0x00000000u);
            if (bg) ctx.ui.drawRect({ctx.x + 2.f, cy2, skelW - 4.f, 16.f}, bg);
            ctx.ui.drawText(ctx.x + 8.f, cy2 + 1.f, kClipNames[i],
                            sel ? ctx.kTextPrimary : ctx.kTextSecond);
            if (ctx.hitRegion({ctx.x + 2.f, cy2, skelW - 4.f, 16.f}, false))
                m_viewSelectedClip = sel ? -1 : i;
            cy2 += 18.f;
        }
    }

    // ── Timeline panel ────────────────────────────────────────────
    ctx.drawPanel(ctx.x + skelW, ctx.y, timelineW, ctx.h, "Timeline");

    // Playback control buttons: Play / Pause / Stop / Record
    {
        float bx = ctx.x + skelW + 8.f;
        float by = ctx.y + 28.f;
        if (ctx.drawButton(bx,      by, 40.f, 18.f, m_viewIsPlaying ? "Pause" : "Play",
                           m_viewIsPlaying ? ctx.kGreen : ctx.kButtonBg)) {
            m_viewIsPlaying = !m_viewIsPlaying;
        }
        if (ctx.drawButton(bx + 44.f, by, 36.f, 18.f, "Stop")) {
            m_viewIsPlaying  = false;
            m_viewIsRecording = false;
        }
        uint32_t recBg = m_viewIsRecording ? ctx.kRed : ctx.kButtonBg;
        if (ctx.drawButton(bx + 84.f, by, 36.f, 18.f, "REC", recBg)) {
            m_viewIsRecording = !m_viewIsRecording;
        }
    }

    // Status pills
    if (m_viewIsPlaying || m_stats.isPlaying)
        ctx.drawStatusPill(ctx.x + skelW + 8.f, ctx.y + 50.f, "PLAYING", ctx.kGreen);
    if (m_viewIsRecording || m_stats.isRecording)
        ctx.drawStatusPill(ctx.x + skelW + 80.f, ctx.y + 50.f, "REC", ctx.kRed);

    // Timeline ruler (simple frame markers)
    {
        float rulY = ctx.y + 70.f;
        float rulW = timelineW - 16.f;
        ctx.ui.drawRect({ctx.x + skelW + 8.f, rulY, rulW, 18.f}, 0x1E1E1EFF);
        ctx.ui.drawRectOutline({ctx.x + skelW + 8.f, rulY, rulW, 18.f}, ctx.kBorder, 1.f);
        // Tick marks at 10% intervals
        for (int t = 0; t <= 10; ++t) {
            float tx2 = ctx.x + skelW + 8.f + rulW * (t / 10.f);
            ctx.ui.drawRect({tx2, rulY, 1.f, 8.f}, ctx.kTextMuted);
        }
        // Playhead (blue line at 0 for now)
        ctx.ui.drawRect({ctx.x + skelW + 8.f, rulY, 2.f, 18.f}, ctx.kAccentBlue);
    }

    {
        char frameBuf[48];
        std::snprintf(frameBuf, sizeof(frameBuf), "%.0f ms  |  %u keyframes",
                      m_stats.clipDurationMs, m_stats.frameCount);
        ctx.ui.drawText(ctx.x + skelW + 8.f, ctx.y + 94.f, frameBuf, ctx.kTextSecond);
    }

    // ── Clip Properties panel ─────────────────────────────────────
    ctx.drawPanel(ctx.x + skelW + timelineW, ctx.y, propW, ctx.h, "Clip Properties");
    const float px = ctx.x + skelW + timelineW;
    if (m_viewSelectedClip >= 0 && m_viewSelectedClip < kClipCount) {
        ctx.ui.drawText(px + 8.f, ctx.y + 30.f, kClipNames[m_viewSelectedClip], ctx.kTextPrimary);
        ctx.ui.drawRect({px + 4.f, ctx.y + 44.f, propW - 8.f, 1.f}, ctx.kBorder);
        char durBuf[32];
        std::snprintf(durBuf, sizeof(durBuf), "%.1f ms", m_stats.clipDurationMs);
        ctx.drawStatRow(px + 8.f, ctx.y + 50.f, "Duration:", durBuf);
        char fcBuf[16];
        std::snprintf(fcBuf, sizeof(fcBuf), "%u", m_stats.frameCount);
        ctx.drawStatRow(px + 8.f, ctx.y + 68.f, "Frames:", fcBuf);
        ctx.drawStatRow(px + 8.f, ctx.y + 86.f, "Loop:", "Yes");
    } else {
        char durBuf[32];
        std::snprintf(durBuf, sizeof(durBuf), "%.1f ms", m_stats.clipDurationMs);
        ctx.drawStatRow(px + 8.f, ctx.y + 30.f, "Duration:", durBuf);
        char fcBuf[16];
        std::snprintf(fcBuf, sizeof(fcBuf), "%u", m_stats.frameCount);
        ctx.drawStatRow(px + 8.f, ctx.y + 48.f, "Frames:", fcBuf);
        ctx.ui.drawText(px + 8.f, ctx.y + 70.f, "Select a clip", ctx.kTextMuted);
    }
    if (m_stats.isDirty) {
        ctx.ui.drawText(px + 8.f, ctx.y + ctx.h - 20.f, "* unsaved", ctx.kRed);
    }
}

} // namespace NF
