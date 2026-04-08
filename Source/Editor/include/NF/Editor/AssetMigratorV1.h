#pragma once
// NF::Editor — asset migrator editor
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

enum class AmvMigrateStatus : uint8_t { Pending, InProgress, Done, Failed, Skipped };
inline const char* amvMigrateStatusName(AmvMigrateStatus v) {
    switch (v) {
        case AmvMigrateStatus::Pending:    return "Pending";
        case AmvMigrateStatus::InProgress: return "InProgress";
        case AmvMigrateStatus::Done:       return "Done";
        case AmvMigrateStatus::Failed:     return "Failed";
        case AmvMigrateStatus::Skipped:    return "Skipped";
    }
    return "Unknown";
}

class AmvTask {
public:
    explicit AmvTask(uint32_t id, const std::string& assetPath)
        : m_id(id), m_assetPath(assetPath) {}

    void setStatus(AmvMigrateStatus v)        { m_status    = v; }
    void setTargetVersion(uint32_t v)         { m_targetVer = v; }
    void setEnabled(bool v)                   { m_enabled   = v; }
    void setErrorMsg(const std::string& v)    { m_errorMsg  = v; }

    [[nodiscard]] uint32_t           id()            const { return m_id;          }
    [[nodiscard]] const std::string& assetPath()     const { return m_assetPath;   }
    [[nodiscard]] AmvMigrateStatus   status()        const { return m_status;      }
    [[nodiscard]] uint32_t           targetVersion() const { return m_targetVer;   }
    [[nodiscard]] bool               enabled()       const { return m_enabled;     }
    [[nodiscard]] const std::string& errorMsg()      const { return m_errorMsg;    }

private:
    uint32_t         m_id;
    std::string      m_assetPath;
    AmvMigrateStatus m_status    = AmvMigrateStatus::Pending;
    uint32_t         m_targetVer = 0;
    bool             m_enabled   = true;
    std::string      m_errorMsg  = "";
};

class AssetMigratorV1 {
public:
    bool addTask(const AmvTask& t) {
        for (auto& x : m_tasks) if (x.id() == t.id()) return false;
        m_tasks.push_back(t); return true;
    }
    bool removeTask(uint32_t id) {
        auto it = std::find_if(m_tasks.begin(), m_tasks.end(),
            [&](const AmvTask& t){ return t.id() == id; });
        if (it == m_tasks.end()) return false;
        m_tasks.erase(it); return true;
    }
    [[nodiscard]] AmvTask* findTask(uint32_t id) {
        for (auto& t : m_tasks) if (t.id() == id) return &t;
        return nullptr;
    }
    [[nodiscard]] size_t taskCount()  const { return m_tasks.size(); }
    [[nodiscard]] size_t doneCount()  const {
        size_t n = 0;
        for (auto& t : m_tasks) if (t.status() == AmvMigrateStatus::Done) ++n;
        return n;
    }
    [[nodiscard]] size_t failedCount() const {
        size_t n = 0;
        for (auto& t : m_tasks) if (t.status() == AmvMigrateStatus::Failed) ++n;
        return n;
    }

private:
    std::vector<AmvTask> m_tasks;
};

} // namespace NF
