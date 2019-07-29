#pragma once

#include <iostream>
#include <algorithm>
#include "../sdl/window.h"
#include <vulkan/vulkan.hpp>

class Vulkan_Renderer {
public:
    void RenderThreadSecure(SDL::Window& window) noexcept {
        try {
            RenderThread(window);
            return;
        }
        catch (vk::SystemError& err)
        {
            std::cout << "vk::SystemError: " << err.what() << std::endl;
        }
        catch (std::exception& err) {
            std::cout << typeid(err).name() << ": " << err.what() << std::endl;
        }
        catch (...)
        {
            std::cout << "unknown error" << std::endl;
        }
        std::cout << "Abnormal Render Exit, Initiate Exit Cleanup" << std::endl;
    }
private:
    void RenderThread(SDL::Window& window) {
        Setup(window);
        //CreateRenderPass();
    }

    void Setup(SDL::Window& window);
};