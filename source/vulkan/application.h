#pragma once

#include "../sdl/application.h"
#include <vulkan/vulkan.hpp>
#include <SDL2/SDL_vulkan.h>

namespace Vulkan {
    struct InstanceCreateInfo {
        std::vector<const char*> Extensions;
        const char* AppName;
        const char* EngineName;
        size_t AppVer, EngineVer;
    };

    class VulkanFacet {
    public:
        VulkanFacet(const VulkanFacet&) = delete;

        VulkanFacet& operator=(const VulkanFacet&) = delete;

        VulkanFacet(VulkanFacet&&) = delete;

        VulkanFacet& operator=(VulkanFacet&&) = delete;

        vk::SurfaceKHR GetSurface() const noexcept { return _surface.get(); }
    private:
        explicit VulkanFacet(std::weak_ptr<SDL::Window> window, VkSurfaceKHR surface) noexcept
                :_surface(surface), _window(std::move(window)) {
            Pin()->Connect(SDL_WINDOWEVENT_CLOSE, [this](SDL::Window&, const SDL_Event&) {
                _surface.reset(vk::SurfaceKHR());
            });
        }

        std::shared_ptr<SDL::Window> Pin() noexcept { return _window.lock(); }

        friend class Application;

        vk::UniqueSurfaceKHR _surface;
        std::weak_ptr<SDL::Window> _window;
    };
    
    class Application {
    public:
        static void CreateInstance(const InstanceCreateInfo& create) {
            auto appInfo = vk::ApplicationInfo(create.AppName, create.AppVer, create.EngineName, create.EngineVer, VK_API_VERSION_1_1);
            auto extensions = GetInstanceRequiredExtensions(create.Extensions);
            Instance = vk::createInstanceUnique({
                    {}, &appInfo,
                    0, nullptr,
                    static_cast<uint32_t>(extensions.size()), extensions.data()
            });
        }

        static auto EnumeratePhysicalDevices() {
            return Instance->enumeratePhysicalDevices();
        }

        static auto EnableWindow(const std::shared_ptr<SDL::Window>& window) {
            VkSurfaceKHR surface;
            if (!SDL_Vulkan_CreateSurface(window->GetHandleDangerous(), Instance.get(), &surface)) {
                VulkanHandleSDLError();
            }
            return std::unique_ptr<VulkanFacet>(new VulkanFacet(std::weak_ptr<SDL::Window>(window), surface));
        }
    private:
        [[noreturn]] static void VulkanHandleSDLError() {
            throw std::runtime_error(SDL_GetError());
        }

        static std::vector<const char*> GetInstanceRequiredExtensions(std::vector<const char*> extensions) {
            unsigned int count;
            if (!SDL_Vulkan_GetInstanceExtensions(nullptr, &count, nullptr)) VulkanHandleSDLError();

            size_t additional_extension_count = extensions.size();
            extensions.resize(additional_extension_count+count);

            if (!SDL_Vulkan_GetInstanceExtensions(nullptr, &count,
                    extensions.data()+additional_extension_count))
                VulkanHandleSDLError();

            return extensions;
        }

        static inline vk::UniqueInstance Instance;
    };

}