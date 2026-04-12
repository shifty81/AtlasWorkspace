// NF::Editor — AtlasAITool implementation.
//
// Eighth real NF::IHostedTool from Phase 3 consolidation.
// Manages the AtlasAI assistant, inline suggestions, and Codex browser
// within AtlasWorkspace.

#include "NF/Editor/AtlasAITool.h"
#include "NF/Workspace/ToolViewRenderContext.h"
#include <cstdio>

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

void AtlasAITool::renderToolView(const ToolViewRenderContext& ctx) const {
    // Three-column layout: Session (20%) | Chat/Codex/Suggestions (60%) | Context (20%)
    const float sessionW = ctx.w * 0.20f;
    const float mainW    = ctx.w * 0.60f;
    const float ctxW     = ctx.w - sessionW - mainW;

    // ── Session panel ─────────────────────────────────────────────
    ctx.drawPanel(ctx.x, ctx.y, sessionW, ctx.h, "Session");
    {
        char msgBuf[32];
        std::snprintf(msgBuf, sizeof(msgBuf), "%u messages", m_stats.messageCount);
        ctx.ui.drawText(ctx.x + 8.f, ctx.y + 30.f, msgBuf, ctx.kTextSecond);
        char snippetBuf[32];
        std::snprintf(snippetBuf, sizeof(snippetBuf), "%u snippets", m_stats.codexSnippetCount);
        ctx.ui.drawText(ctx.x + 8.f, ctx.y + 48.f, snippetBuf, ctx.kTextSecond);
        if (m_stats.pendingSuggestionCount > 0) {
            char pendBuf[32];
            std::snprintf(pendBuf, sizeof(pendBuf), "%u pending", m_stats.pendingSuggestionCount);
            ctx.drawStatusPill(ctx.x + 8.f, ctx.y + 70.f, pendBuf, ctx.kAccentBlue);
        }
    }

    // ── Main panel — Chat / Codex / Suggestions ───────────────────
    ctx.drawPanel(ctx.x + sessionW, ctx.y, mainW, ctx.h, aiAssistModeName(m_assistMode));
    // Mode pill
    ctx.drawStatusPill(ctx.x + sessionW + 8.f, ctx.y + 30.f,
                       aiAssistModeName(m_assistMode), ctx.kAccentBlue);
    if (m_stats.isProcessing) {
        ctx.drawStatusPill(ctx.x + sessionW + 80.f, ctx.y + 30.f, "Processing...", ctx.kGreen);
    }
    // Placeholder for chat/codex content
    const char* hint = (m_stats.messageCount == 0)
        ? "Ask AtlasAI anything..."
        : nullptr;
    if (hint) {
        float hx = ctx.x + sessionW + (mainW - static_cast<float>(std::strlen(hint)) * 8.f) * 0.5f;
        float hy = ctx.y + (ctx.h - 14.f) * 0.5f;
        ctx.ui.drawText(hx, hy, hint, ctx.kTextMuted);
    }

    // ── Context panel ─────────────────────────────────────────────
    ctx.drawPanel(ctx.x + sessionW + mainW, ctx.y, ctxW, ctx.h, "Context");
    ctx.ui.drawText(ctx.x + sessionW + mainW + 8.f, ctx.y + 30.f,
                    "Active File", ctx.kTextSecond);
    ctx.ui.drawText(ctx.x + sessionW + mainW + 8.f, ctx.y + 48.f,
                    "(none)", ctx.kTextMuted);
}

} // namespace NF
