// NovaForge Server — Dedicated headless server
#include "NF/Core/Core.h"
#include "NF/Engine/Engine.h"
#include "NF/Networking/Networking.h"
#include "NF/Game/Game.h"
#include "NF/World/World.h"
#include "NF/AI/AI.h"
#ifdef _WIN32
#  include <windows.h>
#endif
#include <chrono>
#include <thread>
#include <atomic>
#include <csignal>

static std::atomic<bool> g_running{true};

static void handleSignal(int) {
    g_running = false;
}

int main(int argc, char* argv[]) {
    (void)argc; (void)argv;

    std::signal(SIGINT,  handleSignal);
    std::signal(SIGTERM, handleSignal);

    NF::coreInit();
    NF_LOG_INFO("Main","=== NovaForge Dedicated Server ===");
    NF_LOG_INFO("Main", std::string("Version: ") + NF::NF_VERSION_STRING);

    NF::NetworkManager network;
    network.init(NF::NetRole::Server);

    NF::WorldGenerator worldGen;
    worldGen.init(42);

    NF::AISystem ai;
    ai.init();

    NF_LOG_INFO("Main","Server ready — listening on port 7777");
    NF_LOG_INFO("Main","Press Ctrl+C to stop");

    using Clock = std::chrono::high_resolution_clock;
    auto lastTick = Clock::now();
    constexpr float kTickRate = 1.f / 30.f; // 30Hz server tick

    while (g_running) {
        auto now = Clock::now();
        float dt = std::chrono::duration<float>(now - lastTick).count();
        lastTick = now;

        network.tick(dt);
        ai.update(dt);

#ifdef _WIN32
        // Check for ESC key on Windows console
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
            g_running = false;
        Sleep(static_cast<DWORD>(kTickRate * 1000.f));
#else
        std::this_thread::sleep_for(
            std::chrono::duration<float>(kTickRate));
#endif
    }

    NF_LOG_INFO("Main","Server shutting down...");
    ai.shutdown();
    worldGen.shutdown();
    network.shutdown();
    NF::coreShutdown();
    return 0;
}
