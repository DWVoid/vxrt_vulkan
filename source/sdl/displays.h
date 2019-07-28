#pragma once

#include <SDL2/SDL.h>
#include <vector>

namespace SDL {
    class DisplayInfo {
    public:
        const char * GetName() const noexcept { return SDL_GetDisplayName(_index); }

        SDL_Rect GetBounds() const noexcept {
            SDL_Rect result;
            SDL_GetDisplayBounds(_index, &result);
            return result;
        }

        SDL_Rect GetUsableBounds() const noexcept {
            SDL_Rect result;
            SDL_GetDisplayUsableBounds(_index, &result);
            return result;
        }

        struct DPIInfo {
            float Diagonal, Horizontal, Vertical;
        };

        bool TryGetDPI(DPIInfo& infoOut) const noexcept {
            return SDL_GetDisplayDPI(_index, &infoOut.Diagonal, &infoOut.Horizontal, &infoOut.Vertical) == 0;
        }

        SDL_DisplayOrientation GetOrientation() const noexcept { return SDL_GetDisplayOrientation(_index); }

        std::vector<SDL_DisplayMode> EnumerateModes() const {
            std::vector<SDL_DisplayMode> modes { static_cast<size_t>(SDL_GetNumDisplayModes(_index)) };
            for (auto i = 0; i < modes.size(); ++i) {
                SDL_GetDisplayMode(_index, i, modes.data() + i);
            }
            return modes;
        }

        SDL_DisplayMode GetDesktopMode() const noexcept {
            SDL_DisplayMode result;
            SDL_GetDesktopDisplayMode(_index, &result);
            return result;
        }

        SDL_DisplayMode GetCurrentMode() const noexcept {
            SDL_DisplayMode result;
            SDL_GetCurrentDisplayMode(_index, &result);
            return result;
        }

        bool TryGetClosestMode(const SDL_DisplayMode& mode, SDL_DisplayMode& closest) const noexcept {
             return SDL_GetClosestDisplayMode(_index, &mode, &closest) != nullptr;
        }

        size_t GetIndex() const noexcept { return _index; }
    private:
        friend class Displays;
        constexpr explicit DisplayInfo(size_t index) noexcept : _index(index) {}
        size_t _index;
    };

    class Displays {
    public:
        static int Count() noexcept { return SDL_GetNumVideoDisplays(); }

        static std::vector<DisplayInfo> Enumerate() {
            std::vector<DisplayInfo> infos {static_cast<size_t>(Count())};
            for (auto i = 0; i < infos.size(); ++i) {
                infos[i] = DisplayInfo(i);
            }
            return infos;
        }
    };
}