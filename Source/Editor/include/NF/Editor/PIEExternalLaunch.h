#pragma once
// NF::PIEExternalLaunch — External game process launcher for Play-In-Editor.
//
// Manages the launch of the NovaForge game client or server as an external
// process directly from the workspace.  The build→launch pipeline:
//   1. Trigger a build via BuildTool (represented by a build config here)
//   2. Resolve the output executable path from the build artifact
//   3. Launch the external process with the current project's .atlas file
//   4. Route console output to the ConsolePanel notification sink
//   5. Track process lifecycle (running / exit code / crash detection)
//
// Phase F.3 — External Game Launch

#include "NF/Core/Core.h"
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

// ── PIELaunchConfig ───────────────────────────────────────────────────────────

struct PIELaunchConfig {
    std::string executablePath;     ///< path to the game binary
    std::string projectFilePath;    ///< .atlas project file to pass as argument
    std::string workingDirectory;   ///< working dir for the process
    std::vector<std::string> args;  ///< additional command-line arguments
    bool        waitForDebugger   = false;
    bool        fullscreen        = false;
    uint32_t    targetWidth       = 1280;
    uint32_t    targetHeight      = 720;
    std::string buildConfiguration; ///< "Debug", "Release", etc.

    [[nodiscard]] bool isValid() const {
        return !executablePath.empty() && !projectFilePath.empty();
    }
};

// ── PIELaunchResult ───────────────────────────────────────────────────────────

struct PIELaunchResult {
    bool        launched   = false;
    std::string errorMsg;
    uint32_t    processId  = 0;     ///< OS PID of the launched process (stub: nonzero on success)

    [[nodiscard]] bool ok()     const { return launched; }
    [[nodiscard]] bool failed() const { return !launched; }
};

// ── PIEProcessState ───────────────────────────────────────────────────────────

enum class PIEProcessState : uint8_t {
    Idle,      ///< No process running
    Launching, ///< Process is being started
    Running,   ///< Process is active
    Exited,    ///< Process exited normally
    Crashed,   ///< Process terminated unexpectedly
};

inline const char* pieProcessStateName(PIEProcessState s) {
    switch (s) {
    case PIEProcessState::Idle:      return "Idle";
    case PIEProcessState::Launching: return "Launching";
    case PIEProcessState::Running:   return "Running";
    case PIEProcessState::Exited:    return "Exited";
    case PIEProcessState::Crashed:   return "Crashed";
    }
    return "Unknown";
}

// ── PIEExternalLaunch ─────────────────────────────────────────────────────────

class PIEExternalLaunch {
public:
    PIEExternalLaunch()  = default;
    ~PIEExternalLaunch() = default;

    // ── Configuration ─────────────────────────────────────────────────────

    void setConfig(const PIELaunchConfig& cfg) { m_config = cfg; }
    [[nodiscard]] const PIELaunchConfig& config() const { return m_config; }
    [[nodiscard]] bool hasValidConfig() const { return m_config.isValid(); }

    // ── Launch ────────────────────────────────────────────────────────────

    /// Launch the external game process using the stored configuration.
    /// This is a stub implementation: records the launch attempt and
    /// simulates a successful launch by setting state to Running.
    PIELaunchResult launch() {
        if (!m_config.isValid()) {
            return { false, "Launch config is invalid (no executable or project file)" };
        }
        if (m_state == PIEProcessState::Running ||
            m_state == PIEProcessState::Launching) {
            return { false, "A process is already running" };
        }

        m_state        = PIEProcessState::Launching;
        m_launchCount++;
        m_lastExitCode = 0;
        m_stdoutLines.clear();

        // Stub: immediately transition to Running (no real fork/exec).
        // Real implementation: CreateProcess (Win32) or fork+execv (POSIX).
        NF_LOG_INFO("PIE", "Launching external process: " + m_config.executablePath);
        m_state     = PIEProcessState::Running;
        m_processId = 1000 + m_launchCount; // synthetic PID for stubs

        if (m_onLaunched) m_onLaunched(m_processId);
        return { true, "", m_processId };
    }

    /// Terminate the running external process.
    bool terminate() {
        if (m_state != PIEProcessState::Running &&
            m_state != PIEProcessState::Launching) return false;
        NF_LOG_INFO("PIE", "Terminating external process (PID " +
                    std::to_string(m_processId) + ")");
        m_state        = PIEProcessState::Exited;
        m_lastExitCode = -1; // terminated by editor
        if (m_onExited) m_onExited(m_lastExitCode);
        return true;
    }

    /// Simulate process exit (for testing and stub callbacks).
    void simulateExit(int32_t exitCode) {
        if (m_state != PIEProcessState::Running) return;
        m_lastExitCode = exitCode;
        m_state = (exitCode == 0) ? PIEProcessState::Exited
                                  : PIEProcessState::Crashed;
        if (m_onExited) m_onExited(exitCode);
    }

    // ── Console output routing ────────────────────────────────────────────

    /// Route a line of stdout output from the external process to listeners.
    void pushStdoutLine(const std::string& line) {
        m_stdoutLines.push_back(line);
        if (m_onStdoutLine) m_onStdoutLine(line);
    }

    [[nodiscard]] const std::vector<std::string>& stdoutLines() const {
        return m_stdoutLines;
    }

    [[nodiscard]] uint32_t stdoutLineCount() const {
        return static_cast<uint32_t>(m_stdoutLines.size());
    }

    // ── State query ───────────────────────────────────────────────────────

    [[nodiscard]] PIEProcessState state()        const { return m_state; }
    [[nodiscard]] bool            isRunning()    const { return m_state == PIEProcessState::Running; }
    [[nodiscard]] bool            hasExited()    const { return m_state == PIEProcessState::Exited ||
                                                                m_state == PIEProcessState::Crashed; }
    [[nodiscard]] bool            hasCrashed()   const { return m_state == PIEProcessState::Crashed; }
    [[nodiscard]] uint32_t        processId()    const { return m_processId; }
    [[nodiscard]] int32_t         lastExitCode() const { return m_lastExitCode; }
    [[nodiscard]] uint32_t        launchCount()  const { return m_launchCount; }

    // ── Callbacks ─────────────────────────────────────────────────────────

    using LaunchCb  = std::function<void(uint32_t pid)>;
    using ExitCb    = std::function<void(int32_t exitCode)>;
    using StdoutCb  = std::function<void(const std::string& line)>;

    void setOnLaunched(LaunchCb cb)     { m_onLaunched    = std::move(cb); }
    void setOnExited(ExitCb cb)         { m_onExited      = std::move(cb); }
    void setOnStdoutLine(StdoutCb cb)   { m_onStdoutLine  = std::move(cb); }

private:
    PIELaunchConfig            m_config;
    PIEProcessState            m_state        = PIEProcessState::Idle;
    uint32_t                   m_processId    = 0;
    int32_t                    m_lastExitCode = 0;
    uint32_t                   m_launchCount  = 0;
    std::vector<std::string>   m_stdoutLines;

    LaunchCb  m_onLaunched;
    ExitCb    m_onExited;
    StdoutCb  m_onStdoutLine;
};

} // namespace NF
