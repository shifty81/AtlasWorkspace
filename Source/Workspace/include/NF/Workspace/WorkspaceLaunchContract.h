#pragma once
// NF::Editor — Workspace launch contract: launch context, CLI args, IPC,
//              and the WorkspaceLaunchService interface.
//
// Rule: no panel, subsystem, or plugin may spawn child executables directly.
//       All launches must go through WorkspaceLaunchService.
#include "NF/Workspace/WorkspaceAppRegistry.h"
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace NF {

// ── WorkspaceLaunchMode ──────────────────────────────────────────
// How the child process is being started.

enum class WorkspaceLaunchMode : uint8_t {
    Hosted   = 0,  // launched by Workspace; full contract active
    Headless = 1,  // CI / server; no UI, log-only
    Preview  = 2,  // quick run-in-editor preview mode
};

inline const char* workspaceLaunchModeName(WorkspaceLaunchMode m) {
    switch (m) {
        case WorkspaceLaunchMode::Hosted:   return "hosted";
        case WorkspaceLaunchMode::Headless: return "headless";
        case WorkspaceLaunchMode::Preview:  return "preview";
        default:                            return "unknown";
    }
}

// ── WorkspaceLaunchContext ───────────────────────────────────────
// Everything Workspace provides to a child process at startup.
// This is the authoritative launch data object.

struct WorkspaceLaunchContext {
    std::string workspaceRoot;    // absolute path to workspace root
    std::string projectPath;      // absolute path to project.atlas.json
    std::string sessionId;        // unique ID for this launch session
    std::string ipcEndpoint;      // named pipe or socket path for IPC
    std::string logSinkPath;      // child process writes logs here
    WorkspaceLaunchMode mode = WorkspaceLaunchMode::Hosted;

    [[nodiscard]] bool isValid() const {
        return !workspaceRoot.empty()
            && !projectPath.empty()
            && !sessionId.empty();
    }

    // Build the canonical CLI argument list from this context.
    // Child executables must accept these arguments.
    [[nodiscard]] std::vector<std::string> toArgs() const {
        std::vector<std::string> args;
        args.push_back("--" + std::string(workspaceLaunchModeName(mode)));
        args.push_back("--workspace-root=" + workspaceRoot);
        args.push_back("--project=" + projectPath);
        args.push_back("--session-id=" + sessionId);
        if (!ipcEndpoint.empty())
            args.push_back("--ipc=" + ipcEndpoint);
        if (!logSinkPath.empty())
            args.push_back("--log=" + logSinkPath);
        return args;
    }
};

// ── WorkspaceLaunchStatus ────────────────────────────────────────

enum class WorkspaceLaunchStatus : uint8_t {
    Success            = 0,
    AppNotFound        = 1,   // not in registry
    ExecutableNotFound = 2,   // binary does not exist on disk
    InvalidContext     = 3,   // WorkspaceLaunchContext::isValid() == false
    AlreadyRunning     = 4,   // one instance already active
    SpawnFailed        = 5,   // OS-level process creation failed
};

inline const char* workspaceLaunchStatusName(WorkspaceLaunchStatus s) {
    switch (s) {
        case WorkspaceLaunchStatus::Success:             return "Success";
        case WorkspaceLaunchStatus::AppNotFound:         return "AppNotFound";
        case WorkspaceLaunchStatus::ExecutableNotFound:  return "ExecutableNotFound";
        case WorkspaceLaunchStatus::InvalidContext:      return "InvalidContext";
        case WorkspaceLaunchStatus::AlreadyRunning:      return "AlreadyRunning";
        case WorkspaceLaunchStatus::SpawnFailed:         return "SpawnFailed";
        default:                                         return "Unknown";
    }
}

// ── WorkspaceLaunchResult ────────────────────────────────────────

struct WorkspaceLaunchResult {
    WorkspaceLaunchStatus status = WorkspaceLaunchStatus::SpawnFailed;
    uint32_t              pid    = 0;   // OS process ID (0 if not running)
    std::string           errorDetail;  // human-readable error for logging

    [[nodiscard]] bool succeeded() const {
        return status == WorkspaceLaunchStatus::Success;
    }
    [[nodiscard]] bool isRunning() const { return succeeded() && pid != 0; }
};

// ── WorkspaceLaunchService ───────────────────────────────────────
// Owns all child process launch logic.
// Concrete platform implementations live in Source/Programs/AtlasWorkspace/.
// The interface lives here so it can be tested and stubbed.

class WorkspaceLaunchService {
public:
    virtual ~WorkspaceLaunchService() = default;

    // Launch the app described by |app| using |context|.
    // Returns a result with PID on success.
    virtual WorkspaceLaunchResult launchApp(const WorkspaceAppDescriptor& app,
                                            const WorkspaceLaunchContext& context) = 0;

    // Returns true if the app with |id| is currently running.
    virtual bool isRunning(WorkspaceAppId id) const = 0;

    // Request a graceful shutdown of the running app with |id|.
    virtual void shutdownApp(WorkspaceAppId id) = 0;

    // Returns the PID of the running app, or 0 if not running.
    virtual uint32_t pidOf(WorkspaceAppId id) const = 0;
};

// ── NullLaunchService ────────────────────────────────────────────
// No-op implementation used in headless tests and the editor layer.
// Does not spawn any processes.

class NullLaunchService final : public WorkspaceLaunchService {
public:
    WorkspaceLaunchResult launchApp(const WorkspaceAppDescriptor& app,
                                    const WorkspaceLaunchContext& context) override {
        WorkspaceLaunchResult r;
        if (!app.isValid()) {
            r.status      = WorkspaceLaunchStatus::AppNotFound;
            r.errorDetail = "Invalid descriptor for " + app.name;
            return r;
        }
        if (!context.isValid()) {
            r.status      = WorkspaceLaunchStatus::InvalidContext;
            r.errorDetail = "WorkspaceLaunchContext is incomplete";
            return r;
        }
        // Null service records the launch without actually spawning
        m_running[static_cast<uint16_t>(app.id)] = true;
        r.status = WorkspaceLaunchStatus::Success;
        r.pid    = 1;   // synthetic PID
        return r;
    }

    bool isRunning(WorkspaceAppId id) const override {
        auto it = m_running.find(static_cast<uint16_t>(id));
        return it != m_running.end() && it->second;
    }

    void shutdownApp(WorkspaceAppId id) override {
        m_running[static_cast<uint16_t>(id)] = false;
    }

    uint32_t pidOf(WorkspaceAppId id) const override {
        return isRunning(id) ? 1u : 0u;
    }

private:
    // uint16_t key maps WorkspaceAppId to running state
    std::unordered_map<uint16_t, bool> m_running;
};


} // namespace NF
