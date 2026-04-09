// NF::Editor — AnimationEditorTool implementation.
//
// Fourth real NF::IHostedTool from Phase 3 consolidation.
// Manages animation clip and sequencer authoring within AtlasWorkspace.

#include "NF/Editor/AnimationEditorTool.h"

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

} // namespace NF
