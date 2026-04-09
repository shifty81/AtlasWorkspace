// AtlasWorkspace — primary executable entrypoint
//
// AtlasWorkspace.exe is the only top-level user entrypoint.
// All child tools (NovaForgeEditor, NovaForgeGame, NovaForgeServer, …)
// are launched exclusively through WorkspaceLaunchService.
//
// Rule: no child process may be launched outside this service.
#include "NF/Core/Core.h"
#include "NF/Editor/Editor.h"
#include "NF/Editor/WorkspaceAppRegistry.h"
#include "NF/Editor/WorkspaceLaunchContract.h"
#include "NF/Input/Input.h"
#include "NF/UI/UIBackend.h"
#if defined(_WIN32)
#  include "NF/Input/Win32InputAdapter.h"
#  include "NF/UI/GDIBackend.h"
#  include <windows.h>
#endif
#include <chrono>
#include <string>

// ── Workspace app registry bootstrap ────────────────────────────
// Registers all known child apps that this workspace owns.
// Executable paths are relative to the workspace binary location.

static NF::WorkspaceAppRegistry buildDefaultRegistry() {
    NF::WorkspaceAppRegistry reg;

    {
        NF::WorkspaceAppDescriptor d;
        d.id             = NF::WorkspaceAppId::NovaForgeEditor;
        d.name           = "NovaForgeEditor";
        d.executablePath = "NovaForgeEditor.exe";
        d.isProjectScoped = true;
        d.allowDirectLaunch = false;
        d.defaultArgs    = { "--hosted" };
        reg.registerApp(std::move(d));
    }
    {
        NF::WorkspaceAppDescriptor d;
        d.id             = NF::WorkspaceAppId::NovaForgeGame;
        d.name           = "NovaForgeGame";
        d.executablePath = "NovaForgeGame.exe";
        d.isProjectScoped = true;
        d.allowDirectLaunch = false;
        d.defaultArgs    = { "--hosted" };
        reg.registerApp(std::move(d));
    }
    {
        NF::WorkspaceAppDescriptor d;
        d.id             = NF::WorkspaceAppId::NovaForgeServer;
        d.name           = "NovaForgeServer";
        d.executablePath = "NovaForgeServer.exe";
        d.isProjectScoped = true;
        d.allowDirectLaunch = false;
        d.defaultArgs    = { "--headless" };
        reg.registerApp(std::move(d));
    }
    return reg;
}

// ── Win32 window procedure ───────────────────────────────────────
#if defined(_WIN32)
static NF::EditorApp*          g_editor      = nullptr;
static NF::Win32InputAdapter*  g_inputAdapter = nullptr;
static NF::GDIBackend*         g_gdiBackend  = nullptr;
static int g_clientW = 1280, g_clientH = 800;

static LRESULT CALLBACK WorkspaceWndProc(HWND hwnd, UINT msg,
                                          WPARAM wParam, LPARAM lParam) {
    if (g_inputAdapter)
        g_inputAdapter->processMessage(hwnd, msg, wParam, lParam);

    switch (msg) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        if (g_editor && g_gdiBackend) {
            g_gdiBackend->setTargetDC(hdc);
            g_gdiBackend->beginFrame(g_clientW, g_clientH);
            g_editor->renderAll(static_cast<float>(g_clientW),
                                static_cast<float>(g_clientH));
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

// ── main ─────────────────────────────────────────────────────────

int main(int argc, char* argv[]) {
    (void)argc; (void)argv;

    NF::coreInit();
    NF_LOG_INFO("AtlasWorkspace", "=== Atlas Workspace ===");
    NF_LOG_INFO("AtlasWorkspace",
        std::string("Version: ") + NF::NF_VERSION_STRING);

    // Build the default app registry — all child processes go through this.
    NF::WorkspaceAppRegistry registry = buildDefaultRegistry();
    NF_LOG_INFO("AtlasWorkspace",
        std::string("App registry: ") + std::to_string(registry.count())
        + " registered apps");

    // Workspace owns the EditorApp (the shared tool surface).
    NF::EditorApp editor;
    std::string execPath = (argc > 0) ? argv[0] : ".";
    if (!editor.init(1280, 800, execPath)) {
        NF_LOG_ERROR("AtlasWorkspace", "Failed to initialize workspace editor surface");
        NF::coreShutdown();
        return 1;
    }

    NF::InputSystem input;
    input.init();

#if defined(_WIN32)
    // Backend initialization — currently using GDI fallback backend.
    // TODO: Replace with UIBackendSelector when D3D11 backend is ready.
    // See Docs/Canon/04_UI_BACKEND_STRATEGY.md for target direction.
    NF::GDIBackend gdiBackend;
    gdiBackend.init(1280, 800);
    editor.uiRenderer().setBackend(&gdiBackend);
    g_gdiBackend  = &gdiBackend;
    g_editor      = &editor;

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
    editor.uiRenderer().setBackend(&nullBackend);
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
        running = false;   // headless: single frame then exit
#endif
        auto now = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float>(now - lastTime).count();
        lastTime = now;
        if (dt > 0.1f) dt = 0.1f;

        input.update();
        editor.update(dt, input);

#if defined(_WIN32)
        InvalidateRect(hwnd, nullptr, FALSE);
        Sleep(16);
#endif
    }

#if defined(_WIN32)
    gdiBackend.shutdown();
    g_editor       = nullptr;
    g_inputAdapter = nullptr;
    g_gdiBackend   = nullptr;
#endif
    input.shutdown();
    editor.shutdown();
    NF::coreShutdown();
    return 0;
}
