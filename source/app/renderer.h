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
    /*void CreateRenderPass() {
        vk::AttachmentDescription attachmentDescriptions[1];
        attachmentDescriptions[0] = vk::AttachmentDescription(vk::AttachmentDescriptionFlags(), , vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear,
                vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR);
        vk::SubpassDescription subpass(vk::SubpassDescriptionFlags(), vk::PipelineBindPoint::eGraphics, 0, nullptr, 1, &colorReference, nullptr, &depthReference);
        vk::UniqueRenderPass renderPass = device->createRenderPassUnique(vk::RenderPassCreateInfo(vk::RenderPassCreateFlags(), 2, attachmentDescriptions, 1, &subpass));
    }*/

    void RenderThread(SDL::Window& window) {
        Setup(window);
        //CreateRenderPass();
    }

    void Setup(SDL::Window& window);
};