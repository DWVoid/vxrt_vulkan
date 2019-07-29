#pragma once

#include <iostream>
#include "../vulkan/builder.h"
#include "../vulkan/application.h"
#include "../vulkan/queue.h"
#include "../vulkan/shader.h"

namespace {
    struct ResultPack {
        vk::UniqueDevice Device;
        vk::UniqueSwapchainKHR SwapChain;
        std::vector<vk::UniqueImageView> ImageViews;
        vk::UniqueRenderPass RenderPass;
        vk::UniquePipeline Pipeline;
        std::shared_ptr<SDL::Window> Window;
        std::unique_ptr<Vulkan::VulkanFacet> WindowVk;
    };

    constexpr const char* PhysicalDeviceName = "select.physical_device";
    constexpr const char* DeviceQueueName = "select.device_queue";
    constexpr const char* ResultName = "select.result";
    constexpr const char* QueueIndexName = "select.queue_index";

    class InitializeBuildStep : public Vulkan::IBuilder {
    protected:
        static ResultPack& GetResults(Vulkan::Builder& builder) {
            auto clone = builder.Fetch<std::shared_ptr<ResultPack>>(ResultName);
            return *Utils::RequireNonNull(clone);
        }
    };

    class ConsoleDeviceSelector : public InitializeBuildStep {
    public:
        void Build(Vulkan::Builder& builder) override {
            builder.Push(PhysicalDeviceName, DeviceSelectDialog(Vulkan::Application::EnumeratePhysicalDevices()));
        }
    private:
        static vk::PhysicalDevice DeviceSelectDialog(const std::vector<vk::PhysicalDevice>& devices) {
            int counter = 0;
            std::cout << "Devices:" << std::endl;
            for (auto&& x : devices) {
                auto properties = x.getProperties();
                std::cout << counter++ << ": " << properties.deviceName << std::endl;
            }
            int select;
            if (devices.size()>1) {
                do {
                    std::cout << "Input Your ID for selection [0-" << devices.size()-1 << "]:";
                    std::cin >> select;
                }
                while (select<0 || select>=devices.size());
                std::cout << "Device " << select << " selected, proceeding" << std::endl;
            }
            else {
                std::cout << "Has only 1 device, auto select device 0" << std::endl;
                select = 0;
            }
            return devices[select];
        }
    };

    class EnableWindow : public InitializeBuildStep {
    public:
        explicit EnableWindow(std::shared_ptr<SDL::Window> window) noexcept
                :_window(std::move(window)) { }

        void Build(Vulkan::Builder& builder) override {
            GetResults(builder).WindowVk = Vulkan::Application::EnableWindow(_window);
            GetResults(builder).Window = std::move(_window);
        }
    private:
        std::shared_ptr<SDL::Window> _window;
    };

    class QueueSelector : public InitializeBuildStep {
    public:
        void Build(Vulkan::Builder& builder) override {
            Vulkan::Queues queues(builder.Fetch<vk::PhysicalDevice>(PhysicalDeviceName));
            auto index = queues.GetGraphicsAndPresentFast(GetResults(builder).WindowVk->GetSurface());
            static constexpr float priority = 0.1f;
            std::vector<vk::DeviceQueueCreateInfo> queueInfos{
                    vk::DeviceQueueCreateInfo(vk::DeviceQueueCreateFlags(), index.first, 1, &priority)
            };
            if (index.first!=index.second) {
                queueInfos.emplace_back(vk::DeviceQueueCreateFlags(), index.second, 1, &priority);
            }
            builder.Push(DeviceQueueName, queueInfos);
            builder.Push(QueueIndexName, index);
        }
    };

    class DeviceCreator : public InitializeBuildStep {
    public:
        explicit DeviceCreator(std::vector<const char*> extensions) noexcept
                :_extensions(std::move(extensions)) { }

        void Build(Vulkan::Builder& builder) override {
            auto deviceQueues = builder.Fetch<std::vector<vk::DeviceQueueCreateInfo>>(DeviceQueueName);
            GetResults(builder).Device =
                    builder.Fetch<vk::PhysicalDevice>(PhysicalDeviceName).createDeviceUnique(
                            {
                                    {},
                                    static_cast<uint32_t>(deviceQueues.size()), deviceQueues.data(),
                                    0, nullptr,
                                    static_cast<uint32_t>(_extensions.size()), _extensions.data()
                            }
                    );
        }
    private:
        std::vector<const char*> _extensions;
    };

    class SwapChainBuilder : public InitializeBuildStep {
    public:
        void Build(Vulkan::Builder& builder) override {
            auto& result = GetResults(builder);
            Setup(builder, result);
            auto surface = result.WindowVk->GetSurface();
            vk::Format format = SelectFormat(surface);
            SwapChain = Device.createSwapchainKHRUnique(BuildCreateInfo(surface, format));
            BuildImageView(format);
            result.SwapChain = std::move(SwapChain);
            result.ImageViews = std::move(ImageViews);
        }
    private:
        void Setup(Vulkan::Builder& builder, const ResultPack& result) {
            auto index = builder.Fetch<std::pair<size_t, size_t>>(QueueIndexName);
            SetQueueIndex(index.first, index.second);
            Window = result.Window->GetHandleDangerous();
            Device = result.Device.get();
            PhysicalDevice = builder.Fetch<vk::PhysicalDevice>(PhysicalDeviceName);
        }

        void SetQueueIndex(size_t graphics, size_t present) {
            graphicIndex = graphics;
            presentIndex = present;
        }

        VkExtent2D DefineExtent(const vk::SurfaceCapabilitiesKHR& capabilities) const {
            VkExtent2D swapchainExtent;
            int width, height;
            SDL_Vulkan_GetDrawableSize(Window, &width, &height);
            if (capabilities.currentExtent.width==std::numeric_limits<uint32_t>::max()) {
                // If the surface size is undefined, the size is set to the size of the images requested.
                swapchainExtent.width = std::clamp<uint32_t>(width, capabilities.minImageExtent.width,
                        capabilities.maxImageExtent.width);
                swapchainExtent.height = std::clamp<uint32_t>(height, capabilities.minImageExtent.height,
                        capabilities.maxImageExtent.height);
            }
            else {
                // If the surface size is defined, the swap chain size must match
                swapchainExtent = capabilities.currentExtent;
            }
            return swapchainExtent;
        }

        vk::CompositeAlphaFlagBitsKHR SelectCompositeAlpha(const vk::SurfaceCapabilitiesKHR& capabilities) const {
            return (capabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::ePreMultiplied)
                    ? vk::CompositeAlphaFlagBitsKHR::ePreMultiplied :
                    (capabilities.supportedCompositeAlpha
                            & vk::CompositeAlphaFlagBitsKHR::ePostMultiplied)
                            ? vk::CompositeAlphaFlagBitsKHR::ePostMultiplied :
                            (capabilities.supportedCompositeAlpha
                                    & vk::CompositeAlphaFlagBitsKHR::eInherit)
                                    ? vk::CompositeAlphaFlagBitsKHR::eInherit
                                    : vk::CompositeAlphaFlagBitsKHR::eOpaque;
        }

        void BuildImageView(const vk::Format& format) {
            std::vector<vk::Image> swapChainImages = Device.getSwapchainImagesKHR(SwapChain.get());
            ImageViews.reserve(swapChainImages.size());
            vk::ComponentMapping componentMapping(vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG,
                    vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA);
            vk::ImageSubresourceRange subResourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
            for (auto image : swapChainImages) {
                vk::ImageViewCreateInfo imageViewCreateInfo(vk::ImageViewCreateFlags(), image, vk::ImageViewType::e2D,
                        format, componentMapping, subResourceRange);
                ImageViews.push_back(Device.createImageViewUnique(imageViewCreateInfo));
            }
        }

        const vk::SwapchainCreateInfoKHR BuildCreateInfo(VkSurfaceKHR surface, vk::Format format) const {
            // get the supported VkFormats
            const auto surfaceCapabilities = PhysicalDevice.getSurfaceCapabilitiesKHR(surface);
            const auto swapchainExtent = DefineExtent(surfaceCapabilities);
            // The FIFO present mode is guaranteed by the spec to be supported
            const auto swapchainPresentMode = vk::PresentModeKHR::eFifo;
            const auto preTransform = SelectSurfaceTransform(surfaceCapabilities);
            const auto compositeAlpha = SelectCompositeAlpha(surfaceCapabilities);
            vk::SwapchainCreateInfoKHR swapChainCreateInfo(vk::SwapchainCreateFlagsKHR(), surface,
                    surfaceCapabilities.minImageCount, format, vk::ColorSpaceKHR::eSrgbNonlinear,
                    swapchainExtent, 1, vk::ImageUsageFlagBits::eColorAttachment, vk::SharingMode::eExclusive, 0,
                    nullptr, preTransform, compositeAlpha, swapchainPresentMode, true, nullptr);
            AdjustCreateInfoByQueueConfiguration(swapChainCreateInfo);
            return swapChainCreateInfo;
        }

        vk::Format SelectFormat(VkSurfaceKHR surface) const {
            const auto formats = PhysicalDevice.getSurfaceFormatsKHR(surface);
            return (formats[0].format==vk::Format::eUndefined) ? vk::Format::eB8G8R8A8Unorm : formats[0].format;
        }

        vk::SurfaceTransformFlagBitsKHR SelectSurfaceTransform(const vk::SurfaceCapabilitiesKHR& capabilities) const {
            return (capabilities.supportedTransforms & vk::SurfaceTransformFlagBitsKHR::eIdentity)
                    ? vk::SurfaceTransformFlagBitsKHR::eIdentity : capabilities.currentTransform;
        }

        void AdjustCreateInfoByQueueConfiguration(vk::SwapchainCreateInfoKHR& swapChainCreateInfo) const {
            uint32_t queueFamilyIndices[2] = {static_cast<uint32_t>(graphicIndex), static_cast<uint32_t>(presentIndex)};
            if (graphicIndex!=presentIndex) {
                // If the graphics and present queues are from different queue families, we either have to explicitly transfer ownership of images between
                // the queues, or we have to create the swapchain with imageSharingMode as VK_SHARING_MODE_CONCURRENT
                swapChainCreateInfo.imageSharingMode = vk::SharingMode::eConcurrent;
                swapChainCreateInfo.queueFamilyIndexCount = 2;
                swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
            }
        }

        SDL_Window* Window{};
        vk::Device Device{};
        vk::UniqueSwapchainKHR SwapChain;
        std::vector<vk::UniqueImageView> ImageViews;
        vk::PhysicalDevice PhysicalDevice;
        size_t graphicIndex{}, presentIndex{};
    };

    class RenderPassBuilder : public InitializeBuildStep {
    public:
        void Build(Vulkan::Builder& builder) override {
            auto& results = GetResults(builder);
            vk::AttachmentDescription attachmentDescriptions[1];
            attachmentDescriptions[0] = vk::AttachmentDescription(vk::AttachmentDescriptionFlags(), ,
                    vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear,
                    vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
                    vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR
            );
            vk::AttachmentReference colorReference(0, vk::ImageLayout::eColorAttachmentOptimal);
            vk::AttachmentReference depthReference(1, vk::ImageLayout::eDepthStencilAttachmentOptimal);
            vk::SubpassDescription subpass(vk::SubpassDescriptionFlags(), vk::PipelineBindPoint::eGraphics, 0, nullptr,
                    1, &colorReference, nullptr, &depthReference);
            results.RenderPass = results.Device->createRenderPassUnique(
                    vk::RenderPassCreateInfo(vk::RenderPassCreateFlags(), 2, attachmentDescriptions, 1, &subpass)
            );
        }
    };

    class ShaderCompile : public InitializeBuildStep {
    public:
        void Build(Vulkan::Builder& builder) override {
            auto& result = GetResults(builder);
            using C = Vulkan::Compiler;
            C::Load();
            C::CreateModule(result.Device, C::CompileGlslang(vk::ShaderStageFlagBits::eVertex, ""));
            C::CreateModule(result.Device, C::CompileGlslang(vk::ShaderStageFlagBits::eFragment, ""));
            C::Unload();
        }
    };

    class PipelineBuilder : public InitializeBuildStep {
    public:
        void Build(Vulkan::Builder& builder) override {
            auto& result = GetResults(builder);
            vk::DescriptorSetLayoutBinding descriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex);
            vk::UniqueDescriptorSetLayout descriptorSetLayout = result.Device->createDescriptorSetLayoutUnique(vk::DescriptorSetLayoutCreateInfo(vk::DescriptorSetLayoutCreateFlags(), 1, &descriptorSetLayoutBinding));

            // create a PipelineLayout using that DescriptorSetLayout
            vk::UniquePipelineLayout pipelineLayout = result.Device->createPipelineLayoutUnique(vk::PipelineLayoutCreateInfo(vk::PipelineLayoutCreateFlags(), 1, &descriptorSetLayout.get()));

            vk::PipelineShaderStageCreateInfo pipelineShaderStageCreateInfos[2] =
                    {
                            vk::PipelineShaderStageCreateInfo({}, vk::ShaderStageFlagBits::eVertex, vertexShaderModule.get(), "main"),
                            vk::PipelineShaderStageCreateInfo({}, vk::ShaderStageFlagBits::eFragment, fragmentShaderModule.get(), "main")
                    };

            vk::VertexInputBindingDescription vertexInputBindingDescription(0, sizeof(coloredCubeData[0]));
            vk::VertexInputAttributeDescription vertexInputAttributeDescriptions[2] =
                    {
                            vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32A32Sfloat, 0),
                            vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32B32A32Sfloat, 16)
                    };
            vk::PipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo
                    (
                            vk::PipelineVertexInputStateCreateFlags(),  // flags
                            1,                                          // vertexBindingDescriptionCount
                            &vertexInputBindingDescription,             // pVertexBindingDescription
                            2,                                          // vertexAttributeDescriptionCount
                            vertexInputAttributeDescriptions            // pVertexAttributeDescriptions
                    );

            vk::PipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo(vk::PipelineInputAssemblyStateCreateFlags(), vk::PrimitiveTopology::eTriangleList);

            vk::PipelineViewportStateCreateInfo pipelineViewportStateCreateInfo(vk::PipelineViewportStateCreateFlags(), 1, nullptr, 1, nullptr);

            vk::PipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo
                    (
                            vk::PipelineRasterizationStateCreateFlags(),  // flags
                            false,                                        // depthClampEnable
                            false,                                        // rasterizerDiscardEnable
                            vk::PolygonMode::eFill,                       // polygonMode
                            vk::CullModeFlagBits::eBack,                  // cullMode
                            vk::FrontFace::eClockwise,                    // frontFace
                            false,                                        // depthBiasEnable
                            0.0f,                                         // depthBiasConstantFactor
                            0.0f,                                         // depthBiasClamp
                            0.0f,                                         // depthBiasSlopeFactor
                            1.0f                                          // lineWidth
                    );

            vk::PipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo;

            vk::StencilOpState stencilOpState(vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::CompareOp::eAlways);
            vk::PipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo
                    (
                            vk::PipelineDepthStencilStateCreateFlags(), // flags
                            true,                                       // depthTestEnable
                            true,                                       // depthWriteEnable
                            vk::CompareOp::eLessOrEqual,                // depthCompareOp
                            false,                                      // depthBoundTestEnable
                            false,                                      // stencilTestEnable
                            stencilOpState,                             // front
                            stencilOpState                              // back
                    );

            vk::ColorComponentFlags colorComponentFlags(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
            vk::PipelineColorBlendAttachmentState pipelineColorBlendAttachmentState
                    (
                            false,                      // blendEnable
                            vk::BlendFactor::eZero,     // srcColorBlendFactor
                            vk::BlendFactor::eZero,     // dstColorBlendFactor
                            vk::BlendOp::eAdd,          // colorBlendOp
                            vk::BlendFactor::eZero,     // srcAlphaBlendFactor
                            vk::BlendFactor::eZero,     // dstAlphaBlendFactor
                            vk::BlendOp::eAdd,          // alphaBlendOp
                            colorComponentFlags         // colorWriteMask
                    );
            vk::PipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo
                    (
                            vk::PipelineColorBlendStateCreateFlags(),   // flags
                            false,                                      // logicOpEnable
                            vk::LogicOp::eNoOp,                         // logicOp
                            1,                                          // attachmentCount
                            &pipelineColorBlendAttachmentState,         // pAttachments
                            { { (1.0f, 1.0f, 1.0f, 1.0f) } }            // blendConstants
                    );

            vk::DynamicState dynamicStates[2] = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };
            vk::PipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo(vk::PipelineDynamicStateCreateFlags(), 2, dynamicStates);

            vk::GraphicsPipelineCreateInfo graphicsPipelineCreateInfo
                    (
                            vk::PipelineCreateFlags(),                  // flags
                            2,                                          // stageCount
                            pipelineShaderStageCreateInfos,             // pStages
                            &pipelineVertexInputStateCreateInfo,        // pVertexInputState
                            &pipelineInputAssemblyStateCreateInfo,      // pInputAssemblyState
                            nullptr,                                    // pTessellationState
                            &pipelineViewportStateCreateInfo,           // pViewportState
                            &pipelineRasterizationStateCreateInfo,      // pRasterizationState
                            &pipelineMultisampleStateCreateInfo,        // pMultisampleState
                            &pipelineDepthStencilStateCreateInfo,       // pDepthStencilState
                            &pipelineColorBlendStateCreateInfo,         // pColorBlendState
                            &pipelineDynamicStateCreateInfo,            // pDynamicState
                            pipelineLayout.get(),                       // layout
                            result.RenderPass.get()                            // renderPass
                    );

            result.Pipeline = result.Device->createGraphicsPipelineUnique(nullptr, graphicsPipelineCreateInfo);
        }
    private:
    };
}
