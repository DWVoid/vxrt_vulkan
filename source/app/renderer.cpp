#include "renderer.h"
#include "initialize.h"

void Vulkan_Renderer::Setup(SDL::Window& window) {
    auto result = std::make_shared<ResultPack>();
    Vulkan::Builder()
            .Push(ResultName, result)
            .Use(std::make_unique<ConsoleDeviceSelector>())
            .Use(std::make_unique<EnableWindow>(window.GetReference()))
            .Use(std::make_unique<QueueSelector>())
            .Use(std::make_unique<DeviceCreator>(std::vector<const char*>({VK_KHR_SWAPCHAIN_EXTENSION_NAME})))
            .Use(std::make_unique<SwapChainBuilder>())
            .Use(std::make_unique<RenderPassBuilder>())
            .Use(std::make_unique<PipelineBuilder>())
            .Build();
}