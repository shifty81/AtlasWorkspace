#pragma once
// NF::Editor — asset import job queue
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

enum class ImportPriority : uint8_t { Low, Normal, High, Critical };
inline const char* importPriorityName(ImportPriority v) {
    switch (v) {
        case ImportPriority::Low:      return "Low";
        case ImportPriority::Normal:   return "Normal";
        case ImportPriority::High:     return "High";
        case ImportPriority::Critical: return "Critical";
    }
    return "Unknown";
}

enum class ImportStatus : uint8_t { Waiting, Processing, Complete, Error };
inline const char* importStatusName(ImportStatus v) {
    switch (v) {
        case ImportStatus::Waiting:    return "Waiting";
        case ImportStatus::Processing: return "Processing";
        case ImportStatus::Complete:   return "Complete";
        case ImportStatus::Error:      return "Error";
    }
    return "Unknown";
}

class ImportJob {
public:
    explicit ImportJob(uint32_t id, const std::string& assetPath)
        : m_id(id), m_assetPath(assetPath) {}

    void setPriority(ImportPriority v)     { m_priority = v; }
    void setStatus(ImportStatus v)         { m_status   = v; }
    void setProgress(float v)              { m_progress = v; }
    void setMessage(const std::string& v)  { m_message  = v; }

    [[nodiscard]] uint32_t           id()        const { return m_id;        }
    [[nodiscard]] const std::string& assetPath() const { return m_assetPath; }
    [[nodiscard]] ImportPriority     priority()  const { return m_priority;  }
    [[nodiscard]] ImportStatus       status()    const { return m_status;    }
    [[nodiscard]] float              progress()  const { return m_progress;  }
    [[nodiscard]] const std::string& message()   const { return m_message;   }

private:
    uint32_t       m_id;
    std::string    m_assetPath;
    ImportPriority m_priority = ImportPriority::Normal;
    ImportStatus   m_status   = ImportStatus::Waiting;
    float          m_progress = 0.0f;
    std::string    m_message;
};

class AssetImportQueue {
public:
    bool enqueue(const ImportJob& j) {
        for (auto& x : m_jobs) if (x.id() == j.id()) return false;
        m_jobs.push_back(j); return true;
    }
    bool cancel(uint32_t id) {
        auto it = std::find_if(m_jobs.begin(), m_jobs.end(),
            [&](const ImportJob& j){ return j.id() == id; });
        if (it == m_jobs.end()) return false;
        m_jobs.erase(it); return true;
    }
    [[nodiscard]] ImportJob* findJob(uint32_t id) {
        for (auto& j : m_jobs) if (j.id() == id) return &j;
        return nullptr;
    }
    [[nodiscard]] size_t jobCount() const { return m_jobs.size(); }
    [[nodiscard]] size_t waitingCount() const {
        size_t n = 0;
        for (auto& j : m_jobs) if (j.status() == ImportStatus::Waiting) ++n;
        return n;
    }
    bool setProgress(uint32_t id, float progress) {
        auto* j = findJob(id);
        if (!j) return false;
        j->setProgress(progress);
        return true;
    }
    bool setStatus(uint32_t id, ImportStatus status) {
        auto* j = findJob(id);
        if (!j) return false;
        j->setStatus(status);
        return true;
    }

private:
    std::vector<ImportJob> m_jobs;
};

} // namespace NF
