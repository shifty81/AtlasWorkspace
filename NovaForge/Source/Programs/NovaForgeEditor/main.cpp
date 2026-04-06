// NovaForge Editor — Development tool
// Uses the unified Atlas UI renderer pipeline with pluggable backends.
// Win32: GDI backend.  All platforms: OpenGL backend (when GL context available).
// ImGui is banned — see ATLAS_CORE_CONTRACT.md §6.
#include "NF/Core/Core.h"
#include "NF/Editor/Editor.h"
#include "NF/Input/Input.h"
#include "NF/UI/UIBackend.h"
#if defined(_WIN32)
#  include "NF/Input/Win32InputAdapter.h"
#  include "NF/UI/GDIBackend.h"
#  include <windows.h>
#endif
#include <chrono>

#if defined(_WIN32)
static NF::EditorApp* g_editor    = nullptr;
static NF::Win32InputAdapter* g_inputAdapter = nullptr;
static NF::GDIBackend* g_gdiBackend = nullptr;
static int g_clientW = 1280, g_clientH = 800;

static LRESULT CALLBACK EditorWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (g_inputAdapter)
        g_inputAdapter->processMessage(hwnd, msg, wParam, lParam);
    switch (msg) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        // Render the full editor UI through the unified pipeline
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
        g_clientW = LOWORD(lParam); g_clientH = HIWORD(lParam);
        InvalidateRect(hwnd,nullptr,FALSE);
        return 0;
    case WM_CLOSE:
        DestroyWindow(hwnd); return 0;
    case WM_DESTROY:
        PostQuitMessage(0); return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}
#endif // _WIN32

int main(int argc, char* argv[]) {
    (void)argc; (void)argv;

    NF::coreInit();
    NF_LOG_INFO("Main", "=== NovaForge Editor ===");
    NF_LOG_INFO("Main", std::string("Version: ") + NF::NF_VERSION_STRING);

    NF::EditorApp editor;
    std::string execPath = (argc > 0) ? argv[0] : ".";
    if (!editor.init(1280, 800, execPath)) {
        NF_LOG_ERROR("Main", "Failed to initialize editor");
        return 1;
    }

    NF::InputSystem input;
    input.init();

    // ── Backend setup ────────────────────────────────────────────
#if defined(_WIN32)
    // GDI backend for Win32
    NF::GDIBackend gdiBackend;
    gdiBackend.init(1280, 800);
    editor.uiRenderer().setBackend(&gdiBackend);
    g_gdiBackend = &gdiBackend;
    NF_LOG_INFO("Main", "GDI backend active");
#else
    NF::NullBackend nullBackend;
    nullBackend.init(1280, 800);
    editor.uiRenderer().setBackend(&nullBackend);
    NF_LOG_INFO("Main", "Null backend active (headless)");
#endif

#if defined(_WIN32)
    g_editor = &editor;
    NF::Win32InputAdapter inputAdapter(input);
    g_inputAdapter = &inputAdapter;
    NF_LOG_INFO("Main", "Win32 input adapter initialized");

    WNDCLASSEXW wc{};
    wc.cbSize        = sizeof(wc);
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = EditorWndProc;
    wc.hInstance     = GetModuleHandleW(nullptr);
    wc.hCursor       = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = L"NovaForgeEditorWnd";
    RegisterClassExW(&wc);

    RECT wr{0,0,1280,800};
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
    std::wstring winTitle = std::wstring(L"NovaForge Editor  v") + std::wstring(NF::NF_VERSION_STRING, NF::NF_VERSION_STRING + strlen(NF::NF_VERSION_STRING));
    HWND hwnd = CreateWindowExW(0, L"NovaForgeEditorWnd",
        winTitle.c_str(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        wr.right-wr.left, wr.bottom-wr.top,
        nullptr, nullptr, wc.hInstance, nullptr);

    gdiBackend.setWindowHandle(hwnd);
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
    NF_LOG_INFO("Main", "Editor window created (1280x800)");
#endif

    NF_LOG_INFO("Main", "Editor ready — entering main loop");

    auto lastTime = std::chrono::high_resolution_clock::now();
    bool running = true;
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
        running = false;
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
    g_editor = nullptr;
    g_inputAdapter = nullptr;
    g_gdiBackend = nullptr;
#endif
    input.shutdown();
    editor.shutdown();
    NF::coreShutdown();
    return 0;
}
