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

    // ── Skeleton / Clips panel ────────────────────────────────────
    ctx.drawPanel(ctx.x, ctx.y, skelW, ctx.h, "Skeleton / Clips");
    {
        char boneBuf[32];
        std::snprintf(boneBuf, sizeof(boneBuf), "%u bones sel.", m_stats.selectedBoneCount);
        ctx.ui.drawText(ctx.x + 8.f, ctx.y + 30.f, boneBuf, ctx.kTextSecond);
    }

    // ── Timeline panel ────────────────────────────────────────────
    ctx.drawPanel(ctx.x + skelW, ctx.y, timelineW, ctx.h, "Timeline");
    // Mode pill
    ctx.drawStatusPill(ctx.x + skelW + 8.f, ctx.y + 30.f,
                       animationEditModeName(m_editMode), ctx.kAccentBlue);
    // Playback state
    if (m_stats.isPlaying) {
        ctx.drawStatusPill(ctx.x + skelW + 80.f, ctx.y + 30.f, "PLAYING", ctx.kGreen);
    }
    if (m_stats.isRecording) {
        ctx.drawStatusPill(ctx.x + skelW + 150.f, ctx.y + 30.f, "REC", ctx.kRed);
    }
    {
        char frameBuf[48];
        std::snprintf(frameBuf, sizeof(frameBuf), "%.0f ms  |  %u keyframes",
                      m_stats.clipDurationMs, m_stats.frameCount);
        ctx.ui.drawText(ctx.x + skelW + 8.f, ctx.y + 54.f, frameBuf, ctx.kTextSecond);
    }

    // ── Clip Properties panel ─────────────────────────────────────
    ctx.drawPanel(ctx.x + skelW + timelineW, ctx.y, propW, ctx.h, "Clip Properties");
    {
        char durBuf[32];
        std::snprintf(durBuf, sizeof(durBuf), "%.1f ms", m_stats.clipDurationMs);
        ctx.drawStatRow(ctx.x + skelW + timelineW + 8.f, ctx.y + 30.f, "Duration:", durBuf);
        char fcBuf[16];
        std::snprintf(fcBuf, sizeof(fcBuf), "%u", m_stats.frameCount);
        ctx.drawStatRow(ctx.x + skelW + timelineW + 8.f, ctx.y + 48.f, "Frames:", fcBuf);
    }
    if (m_stats.isDirty) {
        ctx.ui.drawText(ctx.x + skelW + timelineW + 8.f, ctx.y + ctx.h - 20.f,
                        "* unsaved", ctx.kRed);
    }
}

} // namespace NF
