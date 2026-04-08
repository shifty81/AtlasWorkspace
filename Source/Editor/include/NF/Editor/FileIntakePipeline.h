#pragma once
// NF::Editor — file intake and processing pipeline
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

enum class IntakeStage : uint8_t { Validate, Fingerprint, Convert, Register, Done };
inline const char* intakeStageName(IntakeStage v) {
    switch (v) {
        case IntakeStage::Validate:     return "Validate";
        case IntakeStage::Fingerprint:  return "Fingerprint";
        case IntakeStage::Convert:      return "Convert";
        case IntakeStage::Register:     return "Register";
        case IntakeStage::Done:         return "Done";
    }
    return "Unknown";
}

enum class IntakeResult : uint8_t { Ok, Skipped, Failed, Duplicate };
inline const char* intakeResultName(IntakeResult v) {
    switch (v) {
        case IntakeResult::Ok:        return "Ok";
        case IntakeResult::Skipped:   return "Skipped";
        case IntakeResult::Failed:    return "Failed";
        case IntakeResult::Duplicate: return "Duplicate";
    }
    return "Unknown";
}

class IntakeFile {
public:
    explicit IntakeFile(uint32_t id, const std::string& path)
        : m_id(id), m_path(path) {}

    void setStage(IntakeStage v)        { m_stage     = v; }
    void setResult(IntakeResult v)      { m_result    = v; }
    void setError(const std::string& v) { m_error     = v; }
    void setSizeBytes(uint64_t v)       { m_sizeBytes = v; }

    [[nodiscard]] uint32_t           id()        const { return m_id;        }
    [[nodiscard]] const std::string& path()      const { return m_path;      }
    [[nodiscard]] IntakeStage        stage()     const { return m_stage;     }
    [[nodiscard]] IntakeResult       result()    const { return m_result;    }
    [[nodiscard]] const std::string& error()     const { return m_error;     }
    [[nodiscard]] uint64_t           sizeBytes() const { return m_sizeBytes; }

private:
    uint32_t     m_id;
    std::string  m_path;
    IntakeStage  m_stage     = IntakeStage::Validate;
    IntakeResult m_result    = IntakeResult::Ok;
    std::string  m_error;
    uint64_t     m_sizeBytes = 0;
};

class FileIntakePipeline {
public:
    bool enqueue(const IntakeFile& f) {
        for (auto& x : m_files) if (x.id() == f.id()) return false;
        m_files.push_back(f); return true;
    }
    bool dequeue(uint32_t id) {
        auto it = std::find_if(m_files.begin(), m_files.end(),
            [&](const IntakeFile& f){ return f.id() == id; });
        if (it == m_files.end()) return false;
        m_files.erase(it); return true;
    }
    [[nodiscard]] IntakeFile* findFile(uint32_t id) {
        for (auto& f : m_files) if (f.id() == id) return &f;
        return nullptr;
    }
    [[nodiscard]] size_t fileCount() const { return m_files.size(); }
    [[nodiscard]] size_t pendingCount() const {
        size_t n = 0;
        for (auto& f : m_files) if (f.stage() != IntakeStage::Done) ++n;
        return n;
    }
    bool advance(uint32_t id) {
        auto* f = findFile(id);
        if (!f || f->stage() == IntakeStage::Done) return false;
        f->setStage(static_cast<IntakeStage>(static_cast<uint8_t>(f->stage()) + 1));
        return true;
    }

private:
    std::vector<IntakeFile> m_files;
};

} // namespace NF
