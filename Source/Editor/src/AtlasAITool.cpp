// NF::Editor — AtlasAITool implementation.
//
// Eighth real NF::IHostedTool from Phase 3 consolidation.
// Manages the AtlasAI assistant, inline suggestions, and Codex browser
// within AtlasWorkspace.

#include "NF/Editor/AtlasAITool.h"

namespace NF {

AtlasAITool::AtlasAITool() {
    buildDescriptor();
}

void AtlasAITool::buildDescriptor() {
    m_descriptor.toolId      = kToolId;
    m_descriptor.displayName = "AtlasAI";
    m_descriptor.category    = HostedToolCategory::AIAssistant;
    m_descriptor.isPrimary   = true;
    m_descriptor.acceptsProjectExtensions = false;

    m_descriptor.supportedPanels = {
        "panel.ai_chat",
        "panel.ai_suggestions",
        "panel.console",
    };

    m_descriptor.commands = {
        "ai.query",
        "ai.apply_suggestion",
        "ai.clear_history",
        "ai.toggle_inline",
        "ai.promote_snippet",
    };
}

bool AtlasAITool::initialize() {
    if (m_state != HostedToolState::Unloaded) return false;
    m_assistMode = AIAssistMode::Chat;
    m_stats      = {};
    m_state      = HostedToolState::Ready;
    return true;
}

void AtlasAITool::shutdown() {
    m_state           = HostedToolState::Unloaded;
    m_assistMode      = AIAssistMode::Chat;
    m_stats           = {};
    m_activeProjectId = {};
}

void AtlasAITool::activate() {
    if (m_state == HostedToolState::Ready || m_state == HostedToolState::Suspended) {
        m_state = HostedToolState::Active;
    }
}

void AtlasAITool::suspend() {
    if (m_state == HostedToolState::Active) {
        m_stats.isProcessing = false;
        m_state = HostedToolState::Suspended;
    }
}

void AtlasAITool::update(float /*dt*/) {
    // AI responses are driven by async broker callbacks, not per-frame polling.
}

void AtlasAITool::onProjectLoaded(const std::string& projectId) {
    m_activeProjectId = projectId;
    clearSession();
}

void AtlasAITool::onProjectUnloaded() {
    m_activeProjectId = {};
    clearSession();
}

void AtlasAITool::setAssistMode(AIAssistMode mode) {
    m_assistMode = mode;
}

void AtlasAITool::setProcessing(bool processing) {
    m_stats.isProcessing = processing;
}

void AtlasAITool::setMessageCount(uint32_t count) {
    m_stats.messageCount = count;
}

void AtlasAITool::setPendingSuggestionCount(uint32_t count) {
    m_stats.pendingSuggestionCount = count;
}

void AtlasAITool::setCodexSnippetCount(uint32_t count) {
    m_stats.codexSnippetCount = count;
}

void AtlasAITool::clearSession() {
    m_stats = {};
}

} // namespace NF
