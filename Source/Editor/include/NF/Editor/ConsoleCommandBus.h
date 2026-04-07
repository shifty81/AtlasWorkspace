#pragma once
// NF::Editor — Console command bus and command palette backend
#include "NF/Core/Core.h"
#include "NF/Engine/Engine.h"
#include "NF/Renderer/Renderer.h"
#include "NF/UI/UI.h"
#include "NF/GraphVM/GraphVM.h"
#include "NF/Input/Input.h"
#include "NF/UI/UIWidgets.h"
#include <filesystem>
#include <fstream>
#include <set>
#include <map>
#include <deque>
#include <unordered_map>

namespace NF {

enum class ConsoleCmdScope : uint8_t {
    Global, Editor, Game, Server, Client, Plugin
};

inline const char* consoleCmdScopeName(ConsoleCmdScope s) {
    switch (s) {
        case ConsoleCmdScope::Global: return "Global";
        case ConsoleCmdScope::Editor: return "Editor";
        case ConsoleCmdScope::Game:   return "Game";
        case ConsoleCmdScope::Server: return "Server";
        case ConsoleCmdScope::Client: return "Client";
        case ConsoleCmdScope::Plugin: return "Plugin";
    }
    return "Unknown";
}

enum class ConsoleCmdArgType : uint8_t {
    None, Bool, Int, Float, String, Enum
};

inline const char* consoleCmdArgTypeName(ConsoleCmdArgType t) {
    switch (t) {
        case ConsoleCmdArgType::None:   return "None";
        case ConsoleCmdArgType::Bool:   return "Bool";
        case ConsoleCmdArgType::Int:    return "Int";
        case ConsoleCmdArgType::Float:  return "Float";
        case ConsoleCmdArgType::String: return "String";
        case ConsoleCmdArgType::Enum:   return "Enum";
    }
    return "Unknown";
}

enum class ConsoleCmdExecResult : uint8_t {
    Ok, NotFound, InvalidArgs, PermissionDenied, Error
};

inline const char* consoleCmdExecResultName(ConsoleCmdExecResult r) {
    switch (r) {
        case ConsoleCmdExecResult::Ok:               return "Ok";
        case ConsoleCmdExecResult::NotFound:         return "NotFound";
        case ConsoleCmdExecResult::InvalidArgs:      return "InvalidArgs";
        case ConsoleCmdExecResult::PermissionDenied: return "PermissionDenied";
        case ConsoleCmdExecResult::Error:            return "Error";
    }
    return "Unknown";
}

class ConsoleCommand {
public:
    explicit ConsoleCommand(const std::string& name,
                             ConsoleCmdScope scope,
                             ConsoleCmdArgType argType)
        : m_name(name), m_scope(scope), m_argType(argType) {}

    void setDescription(const std::string& d) { m_description = d; }
    void setEnabled(bool v)                   { m_enabled     = v; }
    void setHidden(bool v)                    { m_hidden      = v; }

    [[nodiscard]] const std::string& name()        const { return m_name;        }
    [[nodiscard]] ConsoleCmdScope    scope()        const { return m_scope;       }
    [[nodiscard]] ConsoleCmdArgType  argType()      const { return m_argType;     }
    [[nodiscard]] const std::string& description()  const { return m_description; }
    [[nodiscard]] bool               isEnabled()    const { return m_enabled;     }
    [[nodiscard]] bool               isHidden()     const { return m_hidden;      }

private:
    std::string       m_name;
    ConsoleCmdScope   m_scope;
    ConsoleCmdArgType m_argType;
    std::string       m_description;
    bool              m_enabled = true;
    bool              m_hidden  = false;
};

class ConsoleCommandBus {
public:
    static constexpr size_t MAX_COMMANDS = 1024;

    [[nodiscard]] bool registerCommand(const ConsoleCommand& cmd) {
        for (auto& c : m_commands) if (c.name() == cmd.name()) return false;
        if (m_commands.size() >= MAX_COMMANDS) return false;
        m_commands.push_back(cmd);
        return true;
    }

    [[nodiscard]] bool unregisterCommand(const std::string& name) {
        for (auto it = m_commands.begin(); it != m_commands.end(); ++it) {
            if (it->name() == name) { m_commands.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] ConsoleCommand* findCommand(const std::string& name) {
        for (auto& c : m_commands) if (c.name() == name) return &c;
        return nullptr;
    }

    [[nodiscard]] ConsoleCmdExecResult execute(const std::string& name) {
        auto* cmd = findCommand(name);
        if (!cmd) return ConsoleCmdExecResult::NotFound;
        if (!cmd->isEnabled()) return ConsoleCmdExecResult::PermissionDenied;
        m_lastExec = name;
        return ConsoleCmdExecResult::Ok;
    }

    [[nodiscard]] size_t commandCount()  const { return m_commands.size(); }
    [[nodiscard]] const std::string& lastExec() const { return m_lastExec; }

    [[nodiscard]] size_t countByScope(ConsoleCmdScope s) const {
        size_t c = 0; for (auto& cmd : m_commands) if (cmd.scope() == s) ++c; return c;
    }
    [[nodiscard]] size_t hiddenCount() const {
        size_t c = 0; for (auto& cmd : m_commands) if (cmd.isHidden()) ++c; return c;
    }
    [[nodiscard]] size_t enabledCount() const {
        size_t c = 0; for (auto& cmd : m_commands) if (cmd.isEnabled()) ++c; return c;
    }

private:
    std::vector<ConsoleCommand> m_commands;
    std::string                 m_lastExec;
};

} // namespace NF
