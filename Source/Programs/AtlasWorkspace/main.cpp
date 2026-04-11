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
#include "NF/Input/Input.h"
#include "NF/Workspace/WorkspaceShell.h"
#include "NF/Workspace/WorkspaceBootstrap.h"
#include "NF/Workspace/WorkspaceFrameController.h"
#include "NF/Workspace/WorkspaceRenderer.h"
#include "NF/Workspace/WorkspaceAppRegistry.h"
#if defined(_WIN32)
#  include "NF/Input/Win32InputAdapter.h"
#  include "NF/UI/GDIBackend.h"
#  include <windows.h>
#endif
#include <chrono>
#include <string>

// ── Global state for Win32 WndProc ───────────────────────────────
#if defined(_WIN32)
static NF::WorkspaceShell*    g_shell       = nullptr;
static NF::WorkspaceRenderer* g_renderer    = nullptr;
static NF::UIRenderer*        g_ui          = nullptr;
static NF::Win32InputAdapter* g_inputAdapter = nullptr;
static NF::GDIBackend*        g_gdiBackend  = nullptr;
static int g_clientW = 1280, g_clientH = 800;

static LRESULT CALLBACK WorkspaceWndProc(HWND hwnd, UINT msg,
                                          WPARAM wParam, LPARAM lParam) {
    if (g_inputAdapter)
        g_inputAdapter->processMessage(hwnd, msg, wParam, lParam);

    switch (msg) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        if (g_shell && g_renderer && g_ui && g_gdiBackend) {
            g_gdiBackend->setTargetDC(hdc);
            g_gdiBackend->beginFrame(g_clientW, g_clientH);
            g_renderer->render(*g_ui,
                               static_cast<float>(g_clientW),
                               static_cast<float>(g_clientH),
                               *g_shell);
            g_gdiBackend->endFrame();
        }
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_SIZE:
        g_clientW = LOWORD(lParam);
        g_clientH = HIWORD(lParam);
        InvalidateRect(hwnd, nullptr, FALSE);
        return 0;
    case WM_CLOSE:
        DestroyWindow(hwnd);
        return 0;
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

int main(int argc, char* argv[]) {
    (void)argc; (void)argv;

    NF::coreInit();
    NF_LOG_INFO("AtlasWorkspace", "=== Atlas Workspace ===");
    NF_LOG_INFO("AtlasWorkspace", std::string("Version: ") + NF::NF_VERSION_STRING);

    // ── Workspace shell ───────────────────────────────────────────
    NF::WorkspaceShell shell;

    // Register child apps before bootstrap (bootstrap only calls initialize()).
    registerApps(shell.appRegistry());
    NF_LOG_INFO("AtlasWorkspace",
        std::string("App registry: ") + std::to_string(shell.appRegistry().count())
        + " registered apps");

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

    g_shell       = &shell;
    g_renderer    = &wsRenderer;
    g_ui          = &ui;
    g_gdiBackend  = &gdiBackend;

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
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
    NF_LOG_INFO("AtlasWorkspace", "Workspace window created (1280x800)");
#else
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
    g_gdiBackend  = nullptr;
#endif

    input.shutdown();
    shell.shutdown();
    ui.shutdown();
    NF::coreShutdown();
    return 0;
}
