#include "renderer.h"
#include "initialize.h"

void Vulkan_Renderer::Setup(SDL::Window& window) {
    auto result = std::make_shared<ResultPack>();
    Vulkan::Builder()
            .Push(ResultName, result)
            .Use<ConsoleDeviceSelector>()
            .Use<EnableWindow>(window.GetReference())
            .Use<QueueSelector>()
            .Use<DeviceCreator>(std::vector<const char*>({VK_KHR_SWAPCHAIN_EXTENSION_NAME}))
            .Use<SwapChainBuilder>()
            .Use<RenderPassBuilder>()
            .Use<ShaderCompile>()
            .Use<PipelineBuilder>()
            .Build();
}