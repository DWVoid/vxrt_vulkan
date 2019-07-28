#include <thread>
#include <iostream>

#include "sdl/application.h"
#include "sdl/window_factory.h"
#include "vulkan/application.h"

#include "app/renderer.h"

int main() {
    static std::thread renderThread;
    SDL::Application::Init();
    auto window = SDL::WindowFactory::CreateWindow({
            800, 800, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            "Vulkan Application",
            SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN
    });
    window->Connect(SDL_WINDOWEVENT_SHOWN, [](SDL::Window& window, const SDL_Event&) {
        Vulkan::Application::CreateInstance({{}, "vxrt", "vxrt", 1, 1});
        renderThread = std::thread([&]() { Vulkan_Renderer().RenderThreadSecure(window); });
    });
    window->Connect(SDL_WINDOWEVENT_CLOSE, [](SDL::Window& window, const SDL_Event&) {
        if (renderThread.joinable()) {
            renderThread.join();
        }
    });

    SDL::Application::Run();
    return 0;
}
