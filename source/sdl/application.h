#pragma once

#include "window.h"

namespace SDL {
    class Application {
    public:
        using SignalType = boost::signals2::signal<void(const SDL_Event&)>;

        template<class Func>
        static auto Connect(int type, Func function) {
            return _signals[type].connect(function);
        }

        static void Init() {
            SDL_Init(SDL_INIT_VIDEO);
            Connect(SDL_QUIT, [](const SDL_Event&) {Stop();});
            Connect(SDL_WINDOWEVENT, [](const SDL_Event& e) {
                auto window = SDL_GetWindowFromID(e.window.windowID);
                Window::GetWindowClassFromWindow(window)->TriggerEvent(e);
            });
        }

        static void Run() {
            while(!AppQuit.load()) {
                ProcessEvents();
            }
            ShutDown();
        }

        static void Stop() {
            AppQuit = true;
        }

    private:
        static void HandleEvent(const SDL_Event& event) {
            _signals[event.type](event);
        }

        static void DrainEvents(SDL_Event* event) {
            do {
                HandleEvent(*event);
            } while (SDL_PollEvent(event));
        }

        static void ProcessEvents() {
            SDL_Event event;
            SDL_WaitEvent(&event);
            DrainEvents(&event);
        }

        static void ShutDown() {
            SDL_Quit();
        }
    private:
        inline static std::atomic_bool AppQuit = false;
        inline static std::map<int, SignalType> _signals;
    };
}