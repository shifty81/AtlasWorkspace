#pragma once
// NF::Editor — render pass editor
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

enum class RpeLoadOp : uint8_t { Load, Clear, DontCare };
inline const char* rpeLoadOpName(RpeLoadOp v) {
    switch (v) {
        case RpeLoadOp::Load:     return "Load";
        case RpeLoadOp::Clear:    return "Clear";
        case RpeLoadOp::DontCare: return "DontCare";
    }
    return "Unknown";
}

enum class RpeStoreOp : uint8_t { Store, DontCare };
inline const char* rpeStoreOpName(RpeStoreOp v) {
    switch (v) {
        case RpeStoreOp::Store:    return "Store";
        case RpeStoreOp::DontCare: return "DontCare";
    }
    return "Unknown";
}

class RpeAttachment {
public:
    explicit RpeAttachment(uint32_t id, const std::string& name) : m_id(id), m_name(name) {}

    void setLoadOp(RpeLoadOp v)            { m_loadOp     = v; }
    void setStoreOp(RpeStoreOp v)          { m_storeOp    = v; }
    void setClearColor(const std::string& v){ m_clearColor = v; }
    void setIsDepth(bool v)                { m_isDepth    = v; }

    [[nodiscard]] uint32_t           id()         const { return m_id;         }
    [[nodiscard]] const std::string& name()       const { return m_name;       }
    [[nodiscard]] RpeLoadOp          loadOp()     const { return m_loadOp;     }
    [[nodiscard]] RpeStoreOp         storeOp()    const { return m_storeOp;    }
    [[nodiscard]] const std::string& clearColor() const { return m_clearColor; }
    [[nodiscard]] bool               isDepth()    const { return m_isDepth;    }

private:
    uint32_t    m_id;
    std::string m_name;
    RpeLoadOp   m_loadOp     = RpeLoadOp::Clear;
    RpeStoreOp  m_storeOp    = RpeStoreOp::Store;
    std::string m_clearColor = "0,0,0,1";
    bool        m_isDepth    = false;
};

class RpePass {
public:
    explicit RpePass(uint32_t id, const std::string& name) : m_id(id), m_name(name) {}

    void setEnabled(bool v) { m_enabled = v; }
    void setOrder(int v)    { m_order   = v; }

    [[nodiscard]] uint32_t                        id()          const { return m_id;          }
    [[nodiscard]] const std::string&              name()        const { return m_name;        }
    [[nodiscard]] bool                            enabled()     const { return m_enabled;     }
    [[nodiscard]] int                             order()       const { return m_order;       }
    [[nodiscard]] const std::vector<RpeAttachment>& attachments() const { return m_attachments; }

    bool addAttachment(const RpeAttachment& a) {
        for (auto& x : m_attachments) if (x.id() == a.id()) return false;
        m_attachments.push_back(a); return true;
    }

private:
    uint32_t                  m_id;
    std::string               m_name;
    bool                      m_enabled = true;
    int                       m_order   = 0;
    std::vector<RpeAttachment> m_attachments;
};

class RenderPassEditorV1 {
public:
    bool addPass(const RpePass& p) {
        for (auto& x : m_passes) if (x.id() == p.id()) return false;
        m_passes.push_back(p); return true;
    }
    bool removePass(uint32_t id) {
        auto it = std::find_if(m_passes.begin(), m_passes.end(),
            [&](const RpePass& p){ return p.id() == id; });
        if (it == m_passes.end()) return false;
        m_passes.erase(it); return true;
    }
    [[nodiscard]] RpePass* findPass(uint32_t id) {
        for (auto& p : m_passes) if (p.id() == id) return &p;
        return nullptr;
    }
    [[nodiscard]] size_t passCount() const { return m_passes.size(); }
    [[nodiscard]] size_t enabledCount() const {
        size_t n = 0;
        for (auto& p : m_passes) if (p.enabled()) ++n;
        return n;
    }
    bool addAttachment(uint32_t passId, const RpeAttachment& a) {
        auto* p = findPass(passId);
        if (!p) return false;
        return p->addAttachment(a);
    }
    [[nodiscard]] std::vector<RpePass> sortedPasses() const {
        std::vector<RpePass> result = m_passes;
        std::sort(result.begin(), result.end(),
            [](const RpePass& a, const RpePass& b){ return a.order() < b.order(); });
        return result;
    }

private:
    std::vector<RpePass> m_passes;
};

} // namespace NF
