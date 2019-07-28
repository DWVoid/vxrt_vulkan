#pragma once
#include <map>
#include <atomic>
#include <utility>
#include <SDL2/SDL.h>
#include <type_traits>
#include <boost/signals2.hpp>

namespace SDL {
class Window {
    public:
        using SignalType = boost::signals2::signal<void(Window& window, const SDL_Event&)>;

        Window(const Window&) = delete;

        Window& operator=(const Window&) = delete;

        Window(Window&&) = delete;

        Window& operator=(Window&&) = delete;

        template<class Func>
        auto Connect(int type, Func function) {
            return _signals[type].connect(function);
        }

        Uint32 GetFlags() const noexcept { return SDL_GetWindowFlags(_window); }

        void SetTitle(const char* title) noexcept { SDL_SetWindowTitle(_window, title); }

        const char* GetTitle() const noexcept { return SDL_GetWindowTitle(_window); }

        void SetIcon(SDL_Surface* icon) noexcept { SDL_SetWindowIcon(_window, icon); }

        void SetPosition(int x, int y) noexcept { SDL_SetWindowPosition(_window, x, y); }

        void GetPosition(int& x, int& y) const noexcept { SDL_GetWindowPosition(_window, &x, &y); }

        void SetSize(int w, int h) noexcept { SDL_SetWindowSize(_window, w, h); }

        void GetSize(int& w, int& h) const noexcept { SDL_GetWindowSize(_window, &w, &h); }

        bool TryGetBordersSize(int& top, int& left, int& bottom, int& right) const noexcept {
            return SDL_GetWindowBordersSize(_window, &top, &left, &bottom, &right)==0;
        }

        void SetMinimumSize(int w, int h) noexcept { SDL_SetWindowMinimumSize(_window, w, h); }

        void GetMinimumSize(int& w, int& h) const noexcept { SDL_GetWindowMinimumSize(_window, &w, &h); }

        void SetMaximumSize(int w, int h) noexcept { SDL_SetWindowMaximumSize(_window, w, h); }

        void GetMaximumSize(int& w, int& h) const noexcept { SDL_GetWindowMaximumSize(_window, &w, &h); }

        void SetBordered(bool bordered) noexcept {
            return SDL_SetWindowBordered(_window, bordered ? SDL_TRUE : SDL_FALSE);
        }

        void SetResizable(bool resizable) noexcept {
            return SDL_SetWindowResizable(_window, resizable ? SDL_TRUE : SDL_FALSE);
        }

        void Show() noexcept { return SDL_ShowWindow(_window); }

        void Hide() noexcept { return SDL_HideWindow(_window); }

        void Raise() noexcept { return SDL_RaiseWindow(_window); }

        void Maximize() noexcept { return SDL_MaximizeWindow(_window); }

        void Minimize() noexcept { return SDL_MinimizeWindow(_window); }

        void Restore() noexcept { return SDL_RestoreWindow(_window); }

        bool TrySetFullscreen(Uint32 flags) noexcept {
            return SDL_SetWindowFullscreen(_window, flags)==0;
        }

        void SetGrab(bool grabbed) noexcept { return SDL_SetWindowGrab(_window, grabbed ? SDL_TRUE : SDL_FALSE); }

        bool GetGrab() const noexcept { return SDL_GetWindowGrab(_window); }

        static Window* GetGrabbed() noexcept {
            if (auto window = SDL_GetGrabbedWindow(); window) {
                return GetWindowClassFromWindow(window);
            }
            return nullptr;
        }

        bool TrySetBrightness(float brightness) noexcept {
            return SDL_SetWindowBrightness(_window, brightness)==0;
        }

        float GetBrightness() const noexcept { return SDL_GetWindowBrightness(_window); }

        bool TrySetOpacity(float opacity) noexcept { return SDL_SetWindowOpacity(_window, opacity)==0; }

        bool TryGetOpacity(float& opacity) const noexcept { return SDL_GetWindowOpacity(_window, &opacity)==0; }

        bool TryFocusInput() noexcept { return SDL_SetWindowInputFocus(_window)==0; }

        bool TrySetGammaRamp(const Uint16* red, const Uint16* green, const Uint16* blue) {
            return SDL_SetWindowGammaRamp(_window, red, green, blue)==0;
        }

        bool TryGetGammaRamp(Uint16* red, Uint16* green, Uint16* blue) {
            return SDL_GetWindowGammaRamp(_window, red, green, blue);
        }

        template <class Func, class = std::is_invocable_r<SDL_HitTestResult, Func, Window&, const SDL_Point&>>
        bool TrySetHitTest(Func func) {
            if constexpr (std::is_nothrow_move_constructible_v<Func>) {
                return SetupHitTestOperation(std::make_unique<Func>(std::move(func)));
            }
            else {
                return SetupHitTestOperation(std::make_unique<Func>(func));
            }
        }

        void DisableHitTest() noexcept {
            SDL_SetWindowHitTest(_window, nullptr, nullptr);
            if (_hitTestDestruct) {
                _hitTestDestruct();
                _hitTestDestruct = nullptr;
            }
        }

        auto GetReference() const noexcept { return _weak_ref.lock(); }

        auto GetHandleDangerous() const noexcept { return _window; }
    private:
        static constexpr const char* MetaName = "__VK_SDK_APP_WIN_META";

        explicit Window(SDL_Window* window) {
            _window = window;
            SDL_SetWindowData(_window, MetaName, this);
        }

        static Window* GetWindowClassFromWindow(SDL_Window* window) noexcept {
            return reinterpret_cast<Window*>(SDL_GetWindowData(window, MetaName));
        }

        void TriggerEvent(const SDL_Event& event) {
            _signals[event.window.event](*this, event);
        }

        void SetReference(std::weak_ptr<Window> window) noexcept { _weak_ref = std::move(window); }

        template <class Func>
        bool SetupHitTestOperation(std::unique_ptr<Func> func) {
            DisableHitTest();
            _hitTestDestruct = [func = func.get()]() { delete func; };
            auto result = SDL_SetWindowHitTest(_window, [](SDL_Window* win, const SDL_Point* area, void* data) -> auto {
                return (*reinterpret_cast<Func*>(data))(GetWindowClassFromWindow(win), *area);
            }, func.get());
            func.release();
            return result==0;
        }

        friend class WindowFactory;
        friend class Application;

        SDL_Window* _window;
        std::weak_ptr<Window> _weak_ref;
        std::map<int, SignalType> _signals;
        std::function<void()> _hitTestDestruct;
    };
}
