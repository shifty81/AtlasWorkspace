// AtlasWorkspace — primary executable entrypoint
//
// AtlasWorkspace.exe is the outer workspace shell — NOT a game editor.
// It hosts and launches child tools (NovaForgeEditor, NovaForgeGame,
// NovaForgeServer, …) via WorkspaceLaunchService.
//
// Render pipeline:
//   WorkspaceShell (state) → WorkspaceRenderer → UIRenderer → GDIBackend → Win32 window
//
// Rule: no child process may be launched outside WorkspaceLaunchService.
#include "NF/Core/Core.h"
#include "NF/UI/UI.h"
#include "NF/UI/UIBackend.h"
#include "NF/UI/UIWidgets.h"
#include "NF/Input/Input.h"
#include "NF/Workspace/WorkspaceShell.h"
#include "NF/Workspace/WorkspaceBootstrap.h"
#include "NF/Workspace/WorkspaceFrameController.h"
#include "NF/Workspace/WorkspaceRenderer.h"
#include "NF/Workspace/WorkspaceAppRegistry.h"
#include "NF/Workspace/WorkspaceLaunchContract.h"
#include "NF/Workspace/IGameProjectAdapter.h"
#include "NF/Editor/CoreToolRoster.h"
#include "NovaForge/EditorAdapter/NovaForgeAdapter.h"
#if defined(_WIN32)
#  include <windows.h>
#  include <shlobj.h>     // SHBrowseForFolderW / SHGetPathFromIDListW
#  include <commdlg.h>    // GetOpenFileNameW
#  include "NF/Input/Win32InputAdapter.h"
#  include "NF/UI/GDIBackend.h"
#endif
#include <chrono>
#include <filesystem>
#include <string>

// ── Global state for Win32 WndProc ───────────────────────────────
#if defined(_WIN32)

// ── LocalProjectAdapter ───────────────────────────────────────────
// Minimal IGameProjectAdapter backed by a filesystem directory or .atlas file.
// Used when the user opens or creates a generic (non-NovaForge) project from
// the workspace welcome screen.  The adapter has no content-specific logic —
// it simply records the project path and lets the shell host the registered
// tools and panels.

class LocalProjectAdapter final : public NF::IGameProjectAdapter {
public:
    LocalProjectAdapter(std::string id, std::string displayName, std::string path)
        : m_id(std::move(id))
        , m_displayName(std::move(displayName))
        , m_path(std::move(path)) {}

    std::string projectId()          const override { return m_id;          }
    std::string projectDisplayName() const override { return m_displayName; }

    bool initialize() override { return true; }
    void shutdown()   override {}

    std::vector<NF::GameplaySystemPanelDescriptor> panelDescriptors() const override {
        return {};
    }
    std::vector<std::string> contentRoots() const override { return {m_path}; }

private:
    std::string m_id;
    std::string m_displayName;
    std::string m_path;
};

// ── String conversion helpers ─────────────────────────────────────

static std::string wideToUtf8(const wchar_t* ws) {
    if (!ws || !*ws) return {};
    int len = WideCharToMultiByte(CP_UTF8, 0, ws, -1, nullptr, 0, nullptr, nullptr);
    if (len <= 0) return {};
    std::string s(static_cast<size_t>(len), '\0');
    int written = WideCharToMultiByte(CP_UTF8, 0, ws, -1, s.data(), len, nullptr, nullptr);
    if (written <= 0) return {};
    // WideCharToMultiByte with -1 includes the null terminator in the count.
    if (!s.empty() && s.back() == '\0') s.pop_back();
    return s;
}

static std::wstring utf8ToWide(const std::string& u8) {
    if (u8.empty()) return {};
    int len = MultiByteToWideChar(CP_UTF8, 0, u8.c_str(), -1, nullptr, 0);
    if (len <= 0) return {};
    std::wstring ws(static_cast<size_t>(len), L'\0');
    int written = MultiByteToWideChar(CP_UTF8, 0, u8.c_str(), -1, ws.data(), len);
    if (written <= 0) return {};
    if (!ws.empty() && ws.back() == L'\0') ws.pop_back();
    return ws;
}

// ── NovaForge project-root detection ─────────────────────────────
// Returns true if |root| is the NovaForge project directory.
// Checks for NovaForge.atlas (canonical) or novaforge.project.json (legacy marker).

static bool isNovaForgeProjectRoot(const std::filesystem::path& root) {
    return std::filesystem::exists(root / "NovaForge.atlas") ||
           std::filesystem::exists(root / "novaforge.project.json");
}

// ── Win32 project dialogs ─────────────────────────────────────────
// Both helpers open a modal dialog and, on success, load a project adapter
// into the shell.  They must be called from OUTSIDE WM_PAINT (the main loop)
// so that the nested COM/common-dialog message pump does not corrupt paint state.

static void doNewProject(NF::WorkspaceShell& shell, HWND hwnd) {
    BROWSEINFOW bi{};
    bi.hwndOwner  = hwnd;
    bi.lpszTitle  = L"Select a folder for the new project";
    bi.ulFlags    = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
    PIDLIST_ABSOLUTE pidl = SHBrowseForFolderW(&bi);
    if (!pidl) return; // user cancelled

    wchar_t path[MAX_PATH] = {};
    if (!SHGetPathFromIDListW(pidl, path)) {
        CoTaskMemFree(pidl);
        MessageBoxW(hwnd,
            L"Could not resolve the selected folder path.",
            L"New Project", MB_OK | MB_ICONERROR);
        return;
    }
    CoTaskMemFree(pidl);

    // Derive project name from the chosen folder's last path component.
    std::wstring wpath(path);
    auto sep = wpath.find_last_of(L"\\/");
    std::wstring wname = (sep != std::wstring::npos) ? wpath.substr(sep + 1) : wpath;

    std::string name    = wideToUtf8(wname.c_str());
    std::string pathStr = wideToUtf8(path);

    // If the selected folder is a NovaForge project root, use the NovaForgeAdapter
    // so its gameplay panels, content roots, and commands are registered correctly.
    std::unique_ptr<NF::IGameProjectAdapter> adapter;
    if (isNovaForgeProjectRoot(std::filesystem::path(pathStr))) {
        adapter = std::make_unique<NovaForge::NovaForgeAdapter>(pathStr);
    } else {
        adapter = std::make_unique<LocalProjectAdapter>("local." + name, name, pathStr);
    }

    if (!shell.loadProject(std::move(adapter))) {
        MessageBoxW(hwnd,
            L"The workspace could not load the selected folder as a project.",
            L"New Project", MB_OK | MB_ICONERROR);
    } else {
        NF_LOG_INFO("AtlasWorkspace", "New project loaded: " + name + " @ " + pathStr);
    }
}

static void doOpenProject(NF::WorkspaceShell& shell, HWND hwnd) {
    wchar_t buf[MAX_PATH] = {};
    OPENFILENAMEW ofn{};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner   = hwnd;
    // Filter: show .atlas files primarily, but allow all files as a fallback.
    ofn.lpstrFilter = L"Atlas Project (*.atlas)\0*.atlas\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile   = buf;
    ofn.nMaxFile    = MAX_PATH;
    ofn.Flags       = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrTitle  = L"Open Atlas Project";

    if (!GetOpenFileNameW(&ofn)) return; // user cancelled or error

    std::wstring wpath(buf);
    // Use the file stem (no extension) as the display name.
    auto sep = wpath.find_last_of(L"\\/");
    std::wstring wname = (sep != std::wstring::npos) ? wpath.substr(sep + 1) : wpath;
    auto dot = wname.rfind(L'.');
    if (dot != std::wstring::npos) wname = wname.substr(0, dot);

    std::string name    = wideToUtf8(wname.c_str());
    std::string pathStr = wideToUtf8(buf);

    // If the opened .atlas file belongs to the NovaForge project root, use the
    // NovaForgeAdapter so its gameplay panels and content roots are registered.
    // For any other project file, fall back to the generic LocalProjectAdapter.
    std::filesystem::path atlaspath(pathStr);
    std::filesystem::path projectDir = atlaspath.parent_path();

    std::unique_ptr<NF::IGameProjectAdapter> adapter;
    if (isNovaForgeProjectRoot(projectDir)) {
        adapter = std::make_unique<NovaForge::NovaForgeAdapter>(projectDir.string());
    } else {
        adapter = std::make_unique<LocalProjectAdapter>("local." + name, name, pathStr);
    }

    if (!shell.loadProject(std::move(adapter))) {
        MessageBoxW(hwnd,
            L"The workspace could not load the selected file as a project.",
            L"Open Project", MB_OK | MB_ICONERROR);
    } else {
        NF_LOG_INFO("AtlasWorkspace", "Project opened: " + name + " @ " + pathStr);
    }
}

// ── Handle deferred workspace actions ────────────────────────────
// Called from the main loop after PeekMessage dispatch so that modal dialogs
// open outside WM_PAINT and do not create unsafe nested message loops.

static void handlePendingWorkspaceActions(NF::WorkspaceRenderer& renderer,
                                          NF::WorkspaceShell& shell, HWND hwnd)
{
    // Show any launch or runtime error dialog first (non-blocking, informational).
    NF::WorkspacePendingError err = renderer.takePendingError();
    if (!err.empty()) {
        std::wstring wtitle = utf8ToWide(err.title);
        std::wstring wmsg   = utf8ToWide(err.message);
        MessageBoxW(hwnd, wmsg.c_str(), wtitle.c_str(), MB_OK | MB_ICONWARNING);
    }

    // Then handle any queued project action.
    NF::WorkspaceAction action = renderer.takePendingAction();
    switch (action) {
        case NF::WorkspaceAction::NewProject:
            doNewProject(shell, hwnd);
            break;
        case NF::WorkspaceAction::OpenProject:
            doOpenProject(shell, hwnd);
            break;
        case NF::WorkspaceAction::Exit:
            DestroyWindow(hwnd);
            break;
        default:
            break;
    }
}

static NF::WorkspaceShell*    g_shell       = nullptr;
static NF::WorkspaceRenderer* g_renderer    = nullptr;
static NF::UIRenderer*        g_ui          = nullptr;
static NF::Win32InputAdapter* g_inputAdapter = nullptr;
static NF::GDIBackend*        g_gdiBackend  = nullptr;
static NF::InputSystem*       g_inputSystem = nullptr;
static NF::WorkspaceLaunchService* g_launchSvc = nullptr;
static int g_clientW = 1280, g_clientH = 800;

// Per-frame left-button state for edge detection (leftPressed / leftReleased).
// Updated each WM_PAINT so transitions fire for exactly one rendered frame.

static LRESULT CALLBACK WorkspaceWndProc(HWND hwnd, UINT msg,
                                          WPARAM wParam, LPARAM lParam) {
    if (g_inputAdapter)
        g_inputAdapter->processMessage(hwnd, msg, wParam, lParam);

    switch (msg) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        if (g_shell && g_renderer && g_ui && g_gdiBackend && g_inputSystem) {
            // Route input through the shell's InputRouter.  beginFrame() extracts
            // mouse position, button edge transitions, scroll, and typed text from
            // the InputSystem snapshot.  The shell owns the prevLeftDown tracking.
            g_shell->inputRouter().beginFrame(g_inputSystem->state());

            const NF::UIMouseState& mouse = g_shell->inputRouter().mouseState();

            g_gdiBackend->setTargetDC(hdc);
            g_gdiBackend->beginFrame(g_clientW, g_clientH);
            g_renderer->render(*g_ui,
                               static_cast<float>(g_clientW),
                               static_cast<float>(g_clientH),
                               *g_shell, mouse, g_launchSvc);
            g_gdiBackend->endFrame();
            g_shell->inputRouter().endFrame();
        }
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_SIZE:
        g_clientW = LOWORD(lParam);
        g_clientH = HIWORD(lParam);
        InvalidateRect(hwnd, nullptr, FALSE);
        return 0;
    // Trigger repaints on mouse activity so hover states update immediately.
    case WM_MOUSEMOVE:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
        InvalidateRect(hwnd, nullptr, FALSE);
        break;
    // Trigger repaints on keyboard events so text input updates immediately.
    case WM_CHAR:
    case WM_KEYDOWN:
        InvalidateRect(hwnd, nullptr, FALSE);
        break;
    case WM_CLOSE:
        DestroyWindow(hwnd);
        return 0;
    case WM_ACTIVATE:
        // On deactivation (losing focus), reset input router focus to workspace chrome
        // so that stale TextInput focus doesn't persist into the next activation.
        if (LOWORD(wParam) == WA_INACTIVE && g_shell) {
            g_shell->inputRouter().clearFocus();
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}
#endif // _WIN32

// ── Register child apps ───────────────────────────────────────────
// Registers all known child tool executables into the shell's app registry.
// Executable paths are relative to the workspace binary directory.
static void registerApps(NF::WorkspaceAppRegistry& registry) {
    {
        NF::WorkspaceAppDescriptor d;
        d.id              = NF::WorkspaceAppId::NovaForgeEditor;
        d.name            = "NovaForge Editor";
        d.executablePath  = "NovaForgeEditor.exe";
        d.isProjectScoped = true;
        d.allowDirectLaunch = false;
        d.defaultArgs     = { "--hosted" };
        registry.registerApp(std::move(d));
    }
    {
        NF::WorkspaceAppDescriptor d;
        d.id              = NF::WorkspaceAppId::NovaForgeGame;
        d.name            = "NovaForge Game";
        d.executablePath  = "NovaForgeGame.exe";
        d.isProjectScoped = true;
        d.allowDirectLaunch = false;
        d.defaultArgs     = { "--hosted" };
        registry.registerApp(std::move(d));
    }
    {
        NF::WorkspaceAppDescriptor d;
        d.id              = NF::WorkspaceAppId::NovaForgeServer;
        d.name            = "NovaForge Server";
        d.executablePath  = "NovaForgeServer.exe";
        d.isProjectScoped = true;
        d.allowDirectLaunch = false;
        d.defaultArgs     = { "--headless" };
        registry.registerApp(std::move(d));
    }
}

// ── main ─────────────────────────────────────────────────────────

// ── Resolve the directory that contains AtlasWorkspace.exe ───────
// Child executables (NovaForgeEditor.exe etc.) are expected to live in
// the same directory as the host binary.  We derive that directory from
// argv[0] so the launch service can build absolute paths at runtime.
static std::string resolveBaseDir(const char* argv0) {
#if defined(_WIN32)
    // GetModuleFileNameW is the most reliable source on Windows.
    wchar_t buf[MAX_PATH] = {};
    DWORD len = GetModuleFileNameW(nullptr, buf, MAX_PATH);
    if (len > 0 && len < MAX_PATH) {
        // Strip the filename to get the directory.
        wchar_t* lastSep = wcsrchr(buf, L'\\');
        if (lastSep) *lastSep = L'\0';
        // Convert back to UTF-8.
        int mbLen = WideCharToMultiByte(CP_UTF8, 0, buf, -1, nullptr, 0, nullptr, nullptr);
        std::string dir(static_cast<size_t>(mbLen), '\0');
        WideCharToMultiByte(CP_UTF8, 0, buf, -1, dir.data(), mbLen, nullptr, nullptr);
        // Remove trailing null inserted by WideCharToMultiByte.
        if (!dir.empty() && dir.back() == '\0') dir.pop_back();
        return dir;
    }
#endif
    // Fallback: derive from argv[0].
    if (argv0 && *argv0) {
        std::string p(argv0);
        auto pos = p.find_last_of("/\\");
        if (pos != std::string::npos) return p.substr(0, pos);
    }
    return ".";
}

int main(int argc, char* argv[]) {
    // Resolve the directory containing AtlasWorkspace.exe so the launch
    // service can locate sibling executables (NovaForgeEditor.exe etc.).
    std::string binDir = resolveBaseDir(argc > 0 ? argv[0] : nullptr);
    NF_LOG_INFO("AtlasWorkspace", "Binary directory: " + binDir);

    NF::coreInit();
    NF_LOG_INFO("AtlasWorkspace", "=== Atlas Workspace ===");
    NF_LOG_INFO("AtlasWorkspace", std::string("Version: ") + NF::NF_VERSION_STRING);

#if defined(_WIN32)
    // COM is required by SHBrowseForFolderW (folder picker) and CoTaskMemFree.
    // COINIT_APARTMENTTHREADED matches the Win32 message-loop threading model.
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
#endif

    // ── Workspace shell ───────────────────────────────────────────
    NF::WorkspaceShell shell;

    // Register child apps before bootstrap (bootstrap only calls initialize()).
    registerApps(shell.appRegistry());
    NF_LOG_INFO("AtlasWorkspace",
        std::string("App registry: ") + std::to_string(shell.appRegistry().count())
        + " registered apps");

    // Register the canonical primary tool roster (SceneEditor, AssetEditor,
    // MaterialEditor, AnimationEditor, DataEditor, VisualLogicEditor, BuildTool,
    // AtlasAI).  Factories are invoked by WorkspaceBootstrap during initialize().
    NF::registerCoreTools(shell);

    // Bootstrap the shell (validates config, invokes tool factories, initializes).
    NF::WorkspaceBootstrapConfig bootCfg;
    bootCfg.launchMode      = NF::WorkspaceStartupMode::Hosted;
    bootCfg.startupMessages = { "Workspace initialized." };
    // windowConfig defaults to {1280, 800, "Atlas Workspace"} — no override needed

    NF::WorkspaceBootstrap bootstrap;
    auto bootResult = bootstrap.run(bootCfg, shell);
    if (bootResult.failed()) {
        NF_LOG_ERROR("AtlasWorkspace",
            std::string("Bootstrap failed: ") + bootResult.errorName()
            + " — " + bootResult.errorDetail);
        NF::coreShutdown();
        return 1;
    }
    NF_LOG_INFO("AtlasWorkspace",
        std::string("Bootstrap complete. Tools: ")
        + std::to_string(bootResult.toolsRegistered));

    // ── UI renderer ───────────────────────────────────────────────
    NF::UIRenderer ui;
    ui.init();

    // ── Input ─────────────────────────────────────────────────────
    NF::InputSystem input;
    input.init();

    // ── Frame controller ──────────────────────────────────────────
    NF::WorkspaceFrameController frameCtrl;
    frameCtrl.setTargetFPS(60.f);

#if defined(_WIN32)
    NF::WorkspaceRenderer wsRenderer;

    // GDI fallback backend — the active Windows render path.
    // See Docs/Canon/04_UI_BACKEND_STRATEGY.md for the D3D11 target direction.
    NF::GDIBackend gdiBackend;
    gdiBackend.init(1280, 800);
    ui.setBackend(&gdiBackend);

    // Win32LaunchService: spawns real child processes from the binary directory.
    // Falls back to NullLaunchService on non-Windows platforms.
    NF::Win32LaunchService win32LaunchSvc(binDir);

    g_shell       = &shell;
    g_renderer    = &wsRenderer;
    g_ui          = &ui;
    g_gdiBackend  = &gdiBackend;
    g_inputSystem = &input;
    g_launchSvc   = &win32LaunchSvc;

    NF::Win32InputAdapter inputAdapter(input);
    g_inputAdapter = &inputAdapter;

    WNDCLASSEXW wc{};
    wc.cbSize        = sizeof(wc);
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = WorkspaceWndProc;
    wc.hInstance     = GetModuleHandleW(nullptr);
    wc.hCursor       = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = L"AtlasWorkspaceWnd";
    RegisterClassExW(&wc);

    RECT wr{0, 0, 1280, 800};
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
    HWND hwnd = CreateWindowExW(0, L"AtlasWorkspaceWnd",
        L"Atlas Workspace",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        wr.right - wr.left, wr.bottom - wr.top,
        nullptr, nullptr, wc.hInstance, nullptr);

    gdiBackend.setWindowHandle(hwnd);
    inputAdapter.setWindowHandle(hwnd);  // enable mouse capture on click
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
    NF_LOG_INFO("AtlasWorkspace", "Workspace window created (1280x800)");
#else
    (void)binDir;
    NF::NullBackend nullBackend;
    nullBackend.init(1280, 800);
    ui.setBackend(&nullBackend);
#endif

    NF_LOG_INFO("AtlasWorkspace", "Workspace ready — entering main loop");

    auto lastTime = std::chrono::high_resolution_clock::now();
    bool running  = true;

    while (running) {
#if defined(_WIN32)
        MSG msg{};
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) { running = false; break; }
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        if (!running) break;

        // Handle any UI actions or errors that were queued during WM_PAINT.
        // These are processed here — outside WM_PAINT — so that modal dialogs
        // (SHBrowseForFolderW, GetOpenFileNameW, MessageBoxW) can open safely
        // without creating a nested message loop inside the paint callback.
        handlePendingWorkspaceActions(*g_renderer, *g_shell, hwnd);
#else
        running = false; // headless: single frame then exit
#endif
        auto now  = std::chrono::high_resolution_clock::now();
        float rawDt = std::chrono::duration<float>(now - lastTime).count();
        lastTime = now;

        auto fr = frameCtrl.beginFrame(rawDt);

        input.update();
        shell.update(fr.dt);
        frameCtrl.markUpdateDone();

#if defined(_WIN32)
        InvalidateRect(hwnd, nullptr, FALSE);

        // Sleep to pace to ~60 FPS (render happens in WM_PAINT).
        float sleepMs = frameCtrl.sleepMs(rawDt * 1000.f);
        if (sleepMs > 1.f)
            Sleep(static_cast<DWORD>(sleepMs));
#endif
        frameCtrl.markRenderDone();
        frameCtrl.endFrame();
    }

#if defined(_WIN32)
    gdiBackend.shutdown();
    g_shell       = nullptr;
    g_renderer    = nullptr;
    g_ui          = nullptr;
    g_inputAdapter = nullptr;
    g_inputSystem  = nullptr;
    g_launchSvc    = nullptr;
    g_gdiBackend  = nullptr;
#endif

    input.shutdown();
    shell.shutdown();
    ui.shutdown();
#if defined(_WIN32)
    CoUninitialize();
#endif
    NF::coreShutdown();
    return 0;
}
