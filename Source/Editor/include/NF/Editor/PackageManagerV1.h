#pragma once
// NF::Editor — package manager
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

enum class PkgStatus : uint8_t { NotInstalled, Installing, Installed, UpdateAvailable, Error };
inline const char* pkgStatusName(PkgStatus v) {
    switch (v) {
        case PkgStatus::NotInstalled:    return "NotInstalled";
        case PkgStatus::Installing:      return "Installing";
        case PkgStatus::Installed:       return "Installed";
        case PkgStatus::UpdateAvailable: return "UpdateAvailable";
        case PkgStatus::Error:           return "Error";
    }
    return "Unknown";
}

enum class PkgSource : uint8_t { Registry, Local, Git, Archive };
inline const char* pkgSourceName(PkgSource v) {
    switch (v) {
        case PkgSource::Registry: return "Registry";
        case PkgSource::Local:    return "Local";
        case PkgSource::Git:      return "Git";
        case PkgSource::Archive:  return "Archive";
    }
    return "Unknown";
}

class PkgEntry {
public:
    explicit PkgEntry(uint32_t id, const std::string& name) : m_id(id), m_name(name) {}

    void setVersion(const std::string& v)     { m_version     = v; }
    void setStatus(PkgStatus v)               { m_status      = v; }
    void setSource(PkgSource v)               { m_source      = v; }
    void setDescription(const std::string& v) { m_description = v; }
    void setEnabled(bool v)                   { m_enabled     = v; }

    [[nodiscard]] uint32_t           id()          const { return m_id;          }
    [[nodiscard]] const std::string& name()        const { return m_name;        }
    [[nodiscard]] const std::string& version()     const { return m_version;     }
    [[nodiscard]] PkgStatus          status()      const { return m_status;      }
    [[nodiscard]] PkgSource          source()      const { return m_source;      }
    [[nodiscard]] const std::string& description() const { return m_description; }
    [[nodiscard]] bool               enabled()     const { return m_enabled;     }

private:
    uint32_t    m_id;
    std::string m_name;
    std::string m_version     = "1.0.0";
    PkgStatus   m_status      = PkgStatus::NotInstalled;
    PkgSource   m_source      = PkgSource::Registry;
    std::string m_description = "";
    bool        m_enabled     = true;
};

class PackageManagerV1 {
public:
    bool addPackage(const PkgEntry& p) {
        for (auto& x : m_packages) if (x.id() == p.id()) return false;
        m_packages.push_back(p); return true;
    }
    bool removePackage(uint32_t id) {
        auto it = std::find_if(m_packages.begin(), m_packages.end(),
            [&](const PkgEntry& p){ return p.id() == id; });
        if (it == m_packages.end()) return false;
        m_packages.erase(it); return true;
    }
    [[nodiscard]] PkgEntry* findPackage(uint32_t id) {
        for (auto& p : m_packages) if (p.id() == id) return &p;
        return nullptr;
    }
    [[nodiscard]] size_t packageCount() const { return m_packages.size(); }
    [[nodiscard]] size_t installedCount() const {
        size_t n = 0;
        for (auto& p : m_packages) if (p.status() == PkgStatus::Installed) ++n;
        return n;
    }
    bool install(uint32_t id) {
        auto* p = findPackage(id);
        if (!p) return false;
        p->setStatus(PkgStatus::Installed); return true;
    }
    bool uninstall(uint32_t id) {
        auto* p = findPackage(id);
        if (!p) return false;
        p->setStatus(PkgStatus::NotInstalled); return true;
    }
    [[nodiscard]] std::vector<PkgEntry> filterByStatus(PkgStatus s) const {
        std::vector<PkgEntry> result;
        for (auto& p : m_packages) if (p.status() == s) result.push_back(p);
        return result;
    }

private:
    std::vector<PkgEntry> m_packages;
};

} // namespace NF
