// NF::Editor — BuildTool implementation.
//
// Seventh real NF::IHostedTool from Phase 3 consolidation.
// Manages build, packaging, and deploy workflows within AtlasWorkspace.

#include "NF/Editor/BuildTool.h"

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

} // namespace NF
