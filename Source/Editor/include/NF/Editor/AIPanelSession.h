#pragma once
// NF::Editor — AI conversation session management
#include "NF/Core/Core.h"
#include "NF/Engine/Engine.h"
#include "NF/Renderer/Renderer.h"
#include "NF/UI/UI.h"
#include "NF/GraphVM/GraphVM.h"
#include "NF/Input/Input.h"
#include "NF/UI/UIWidgets.h"
#include <string>
#include <vector>
#include <cstdint>
#include <algorithm>

namespace NF {

enum class SessionRole : uint8_t { User, Assistant, System, Tool };
inline const char* sessionRoleName(SessionRole v) {
    switch (v) {
        case SessionRole::User:      return "User";
        case SessionRole::Assistant: return "Assistant";
        case SessionRole::System:    return "System";
        case SessionRole::Tool:      return "Tool";
    }
    return "Unknown";
}

enum class SessionStatus : uint8_t { Active, Paused, Completed, Cancelled };
inline const char* sessionStatusName(SessionStatus v) {
    switch (v) {
        case SessionStatus::Active:    return "Active";
        case SessionStatus::Paused:    return "Paused";
        case SessionStatus::Completed: return "Completed";
        case SessionStatus::Cancelled: return "Cancelled";
    }
    return "Unknown";
}

class SessionMessage {
public:
    explicit SessionMessage(uint32_t id, SessionRole role, const std::string& content)
        : m_id(id), m_role(role), m_content(content) {}

    void setTokens(int v)        { m_tokens    = v; }
    void setTimestamp(uint64_t v){ m_timestamp = v; }

    [[nodiscard]] uint32_t           id()        const { return m_id;        }
    [[nodiscard]] SessionRole        role()      const { return m_role;      }
    [[nodiscard]] const std::string& content()   const { return m_content;   }
    [[nodiscard]] int                tokens()    const { return m_tokens;    }
    [[nodiscard]] uint64_t           timestamp() const { return m_timestamp; }

private:
    uint32_t    m_id;
    SessionRole m_role;
    std::string m_content;
    int         m_tokens    = 0;
    uint64_t    m_timestamp = 0;
};

class AIPanelSession {
public:
    explicit AIPanelSession(uint32_t id) : m_id(id) {}

    void setStatus(SessionStatus v) { m_status = v; }

    bool addMessage(const SessionMessage& msg) {
        for (auto& x : m_messages) if (x.id() == msg.id()) return false;
        m_messages.push_back(msg); return true;
    }
    void clearMessages() { m_messages.clear(); }

    [[nodiscard]] uint32_t       id()           const { return m_id;             }
    [[nodiscard]] SessionStatus  status()       const { return m_status;         }
    [[nodiscard]] size_t         messageCount() const { return m_messages.size(); }
    [[nodiscard]] int            tokenCount()   const {
        int total = 0;
        for (auto& m : m_messages) total += m.tokens();
        return total;
    }

private:
    uint32_t                  m_id;
    SessionStatus             m_status   = SessionStatus::Active;
    std::vector<SessionMessage> m_messages;
};

} // namespace NF
