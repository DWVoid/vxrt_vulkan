#pragma once

#include "window.h"

namespace SDL {
    struct WindowCreateInfo {
        int Width, Height, Left, Top;
        const char* Title;
        uint32_t Flags;
    };

    class FullScreenFacet {
    public:
        FullScreenFacet(const FullScreenFacet&) = delete;

        FullScreenFacet& operator=(const FullScreenFacet&) = delete;

        FullScreenFacet(FullScreenFacet&&) = delete;

        FullScreenFacet& operator=(FullScreenFacet&&) = delete;

    private:
        explicit FullScreenFacet(std::weak_ptr<Window> window) noexcept : _window(std::move(window)) {}

        std::shared_ptr<Window> Pin() noexcept { return _window.lock(); }

        friend class WindowFactory;

        std::weak_ptr<Window> _window;
    };

    class WindowFactory {
    public:
        static std::shared_ptr<Window> CreateWindow(const WindowCreateInfo& createInfo) noexcept {
            auto ptr = std::shared_ptr<Window>(CreateWindowUnique(createInfo));
            if (ptr) {
                ptr->SetReference(ptr);
            }
            return ptr;
        }

        static std::unique_ptr<FullScreenFacet> EnableFullScreen(const std::shared_ptr<Window>& window) {
            return std::unique_ptr<FullScreenFacet>(new FullScreenFacet(std::weak_ptr<Window>(window)));
        }
    private:
        static std::unique_ptr<Window> CreateWindowUnique(const WindowCreateInfo& createInfo) noexcept {
            try {
                const auto win = SDL_CreateWindow(createInfo.Title,
                        createInfo.Left, createInfo.Top, createInfo.Width, createInfo.Height,
                        createInfo.Flags
                );
                return win ? std::unique_ptr<Window>(new Window(win)) : nullptr;
            }
            catch (...) {
                return nullptr;
            }
        }
    };
}
