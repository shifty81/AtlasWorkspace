// S141 editor tests: AtlasAIPanelHost, AIPanelSession, AIActionSurface
#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

// ── AtlasAIPanelHost ──────────────────────────────────────────────────────────

TEST_CASE("AIPanelMode names", "[Editor][S141]") {
    REQUIRE(std::string(aiPanelModeName(AIPanelMode::Chat))    == "Chat");
    REQUIRE(std::string(aiPanelModeName(AIPanelMode::CodeGen)) == "CodeGen");
    REQUIRE(std::string(aiPanelModeName(AIPanelMode::Review))  == "Review");
    REQUIRE(std::string(aiPanelModeName(AIPanelMode::Debug))   == "Debug");
    REQUIRE(std::string(aiPanelModeName(AIPanelMode::Explain)) == "Explain");
}

TEST_CASE("AIPanelState names", "[Editor][S141]") {
    REQUIRE(std::string(aiPanelStateName(AIPanelState::Idle))       == "Idle");
    REQUIRE(std::string(aiPanelStateName(AIPanelState::Thinking))   == "Thinking");
    REQUIRE(std::string(aiPanelStateName(AIPanelState::Responding)) == "Responding");
    REQUIRE(std::string(aiPanelStateName(AIPanelState::Error))      == "Error");
}

TEST_CASE("AIPanelPanel defaults", "[Editor][S141]") {
    AIPanelPanel p(1, "AI Chat");
    REQUIRE(p.id()      == 1u);
    REQUIRE(p.title()   == "AI Chat");
    REQUIRE(p.mode()    == AIPanelMode::Chat);
    REQUIRE(p.state()   == AIPanelState::Idle);
    REQUIRE(p.visible() == true);
    REQUIRE(p.pinned()  == false);
}

TEST_CASE("AIPanelPanel setters", "[Editor][S141]") {
    AIPanelPanel p(2, "Code Gen", AIPanelMode::CodeGen);
    p.setMode(AIPanelMode::Review);
    p.setState(AIPanelState::Thinking);
    p.setVisible(false);
    p.setPinned(true);
    REQUIRE(p.mode()    == AIPanelMode::Review);
    REQUIRE(p.state()   == AIPanelState::Thinking);
    REQUIRE(p.visible() == false);
    REQUIRE(p.pinned()  == true);
}

TEST_CASE("AtlasAIPanelHost add and visiblePanels", "[Editor][S141]") {
    AtlasAIPanelHost host;
    host.addPanel(AIPanelPanel(1, "Chat"));
    host.addPanel(AIPanelPanel(2, "Debug"));
    REQUIRE(host.panelCount()    == 2u);
    REQUIRE(host.visiblePanels() == 2u);
    host.findPanel(1)->setVisible(false);
    REQUIRE(host.visiblePanels() == 1u);
}

TEST_CASE("AtlasAIPanelHost duplicate and remove", "[Editor][S141]") {
    AtlasAIPanelHost host;
    REQUIRE(host.addPanel(AIPanelPanel(5, "panel")) == true);
    REQUIRE(host.addPanel(AIPanelPanel(5, "panel")) == false);
    REQUIRE(host.removePanel(5) == true);
    REQUIRE(host.panelCount()   == 0u);
    REQUIRE(host.removePanel(5) == false);
}

TEST_CASE("AtlasAIPanelHost findPanel missing", "[Editor][S141]") {
    AtlasAIPanelHost host;
    REQUIRE(host.findPanel(99) == nullptr);
}

// ── AIPanelSession ────────────────────────────────────────────────────────────

TEST_CASE("SessionRole names", "[Editor][S141]") {
    REQUIRE(std::string(sessionRoleName(SessionRole::User))      == "User");
    REQUIRE(std::string(sessionRoleName(SessionRole::Assistant)) == "Assistant");
    REQUIRE(std::string(sessionRoleName(SessionRole::System))    == "System");
    REQUIRE(std::string(sessionRoleName(SessionRole::Tool))      == "Tool");
}

TEST_CASE("SessionStatus names", "[Editor][S141]") {
    REQUIRE(std::string(sessionStatusName(SessionStatus::Active))    == "Active");
    REQUIRE(std::string(sessionStatusName(SessionStatus::Paused))    == "Paused");
    REQUIRE(std::string(sessionStatusName(SessionStatus::Completed)) == "Completed");
    REQUIRE(std::string(sessionStatusName(SessionStatus::Cancelled)) == "Cancelled");
}

TEST_CASE("SessionMessage defaults", "[Editor][S141]") {
    SessionMessage msg(1, SessionRole::User, "Hello!");
    REQUIRE(msg.id()        == 1u);
    REQUIRE(msg.role()      == SessionRole::User);
    REQUIRE(msg.content()   == "Hello!");
    REQUIRE(msg.tokens()    == 0);
    REQUIRE(msg.timestamp() == 0u);
}

TEST_CASE("AIPanelSession messages and tokenCount", "[Editor][S141]") {
    AIPanelSession session(42);
    REQUIRE(session.id()           == 42u);
    REQUIRE(session.status()       == SessionStatus::Active);
    REQUIRE(session.messageCount() == 0u);
    REQUIRE(session.tokenCount()   == 0);

    SessionMessage m1(1, SessionRole::User,      "Hi");
    SessionMessage m2(2, SessionRole::Assistant, "Hello");
    m1.setTokens(5);
    m2.setTokens(10);
    session.addMessage(m1);
    session.addMessage(m2);
    REQUIRE(session.messageCount() == 2u);
    REQUIRE(session.tokenCount()   == 15);
}

TEST_CASE("AIPanelSession duplicate message", "[Editor][S141]") {
    AIPanelSession session(1);
    SessionMessage m(1, SessionRole::User, "text");
    REQUIRE(session.addMessage(m) == true);
    REQUIRE(session.addMessage(m) == false);
    REQUIRE(session.messageCount() == 1u);
}

TEST_CASE("AIPanelSession clearMessages", "[Editor][S141]") {
    AIPanelSession session(1);
    session.addMessage(SessionMessage(1, SessionRole::User, "a"));
    session.addMessage(SessionMessage(2, SessionRole::Assistant, "b"));
    session.clearMessages();
    REQUIRE(session.messageCount() == 0u);
    REQUIRE(session.tokenCount()   == 0);
}

// ── AIActionSurface ───────────────────────────────────────────────────────────

TEST_CASE("AIActionType names", "[Editor][S141]") {
    REQUIRE(std::string(aiActionTypeName(AIActionType::Insert))   == "Insert");
    REQUIRE(std::string(aiActionTypeName(AIActionType::Replace))  == "Replace");
    REQUIRE(std::string(aiActionTypeName(AIActionType::Delete))   == "Delete");
    REQUIRE(std::string(aiActionTypeName(AIActionType::Refactor)) == "Refactor");
    REQUIRE(std::string(aiActionTypeName(AIActionType::Generate)) == "Generate");
}

TEST_CASE("AIActionState names", "[Editor][S141]") {
    REQUIRE(std::string(aiActionStateName(AIActionState::Pending))  == "Pending");
    REQUIRE(std::string(aiActionStateName(AIActionState::Applied))  == "Applied");
    REQUIRE(std::string(aiActionStateName(AIActionState::Rejected)) == "Rejected");
    REQUIRE(std::string(aiActionStateName(AIActionState::Undone))   == "Undone");
}

TEST_CASE("AIAction defaults", "[Editor][S141]") {
    AIAction a(1, AIActionType::Insert, "insert snippet");
    REQUIRE(a.id()          == 1u);
    REQUIRE(a.type()        == AIActionType::Insert);
    REQUIRE(a.state()       == AIActionState::Pending);
    REQUIRE(a.description() == "insert snippet");
    REQUIRE(a.targetPath()  == "");
    REQUIRE(a.canUndo()     == true);
}

TEST_CASE("AIActionSurface submit and pendingCount", "[Editor][S141]") {
    AIActionSurface surface;
    surface.submitAction(AIAction(1, AIActionType::Insert, "a"));
    surface.submitAction(AIAction(2, AIActionType::Replace, "b"));
    REQUIRE(surface.actionCount() == 2u);
    REQUIRE(surface.pendingCount() == 2u);
}

TEST_CASE("AIActionSurface rejectAction", "[Editor][S141]") {
    AIActionSurface surface;
    surface.submitAction(AIAction(1, AIActionType::Delete, "del"));
    REQUIRE(surface.rejectAction(1) == true);
    REQUIRE(surface.findAction(1)->state() == AIActionState::Rejected);
    REQUIRE(surface.pendingCount() == 0u);
    REQUIRE(surface.rejectAction(99) == false);
}

TEST_CASE("AIActionSurface undoAction", "[Editor][S141]") {
    AIActionSurface surface;
    AIAction a(1, AIActionType::Refactor, "refactor");
    a.setCanUndo(false);
    surface.submitAction(a);
    REQUIRE(surface.undoAction(1) == false);

    AIAction b(2, AIActionType::Generate, "gen");
    surface.submitAction(b);
    REQUIRE(surface.undoAction(2) == true);
    REQUIRE(surface.findAction(2)->state() == AIActionState::Undone);
}
