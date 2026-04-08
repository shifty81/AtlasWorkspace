#pragma once
// NF::Editor — source repository surface
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

enum class RsvBranchStatus : uint8_t { Clean, Modified, Ahead, Behind, Conflict };
inline const char* rsvBranchStatusName(RsvBranchStatus v) {
    switch (v) {
        case RsvBranchStatus::Clean:    return "Clean";
        case RsvBranchStatus::Modified: return "Modified";
        case RsvBranchStatus::Ahead:    return "Ahead";
        case RsvBranchStatus::Behind:   return "Behind";
        case RsvBranchStatus::Conflict: return "Conflict";
    }
    return "Unknown";
}

enum class RsvRemote : uint8_t { None, GitHub, GitLab, Bitbucket, Custom };
inline const char* rsvRemoteName(RsvRemote v) {
    switch (v) {
        case RsvRemote::None:      return "None";
        case RsvRemote::GitHub:    return "GitHub";
        case RsvRemote::GitLab:    return "GitLab";
        case RsvRemote::Bitbucket: return "Bitbucket";
        case RsvRemote::Custom:    return "Custom";
    }
    return "Unknown";
}

class RsvCommit {
public:
    explicit RsvCommit(uint32_t id, const std::string& sha, const std::string& message,
                       const std::string& author)
        : m_id(id), m_sha(sha), m_message(message), m_author(author) {}

    void setTimestamp(uint64_t v) { m_timestamp = v; }

    [[nodiscard]] uint32_t           id()        const { return m_id;        }
    [[nodiscard]] const std::string& sha()       const { return m_sha;       }
    [[nodiscard]] const std::string& message()   const { return m_message;   }
    [[nodiscard]] const std::string& author()    const { return m_author;    }
    [[nodiscard]] uint64_t           timestamp() const { return m_timestamp; }

private:
    uint32_t    m_id;
    std::string m_sha;
    std::string m_message;
    std::string m_author;
    uint64_t    m_timestamp = 0;
};

class RepoSurfaceV1 {
public:
    explicit RepoSurfaceV1(uint32_t id, const std::string& name)
        : m_id(id), m_name(name) {}

    void setBranchName(const std::string& v) { m_branchName = v; }
    void setStatus(RsvBranchStatus v)         { m_status     = v; }
    void setRemote(RsvRemote v)               { m_remote     = v; }

    bool addCommit(const RsvCommit& c) {
        for (auto& x : m_commits) if (x.id() == c.id()) return false;
        m_commits.push_back(c); return true;
    }
    void clearCommits() { m_commits.clear(); }

    [[nodiscard]] uint32_t              id()           const { return m_id;              }
    [[nodiscard]] const std::string&    name()         const { return m_name;            }
    [[nodiscard]] const std::string&    branchName()   const { return m_branchName;      }
    [[nodiscard]] RsvBranchStatus       status()       const { return m_status;          }
    [[nodiscard]] RsvRemote             remote()       const { return m_remote;          }
    [[nodiscard]] size_t                commitCount()  const { return m_commits.size();  }
    [[nodiscard]] const std::string&    currentBranch() const { return m_branchName;     }

private:
    uint32_t               m_id;
    std::string            m_name;
    std::string            m_branchName = "main";
    RsvBranchStatus        m_status     = RsvBranchStatus::Clean;
    RsvRemote              m_remote     = RsvRemote::None;
    std::vector<RsvCommit> m_commits;
};

} // namespace NF
