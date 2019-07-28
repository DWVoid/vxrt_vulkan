
#pragma once

#include <map>
#include <vector>
#include <utility>
#include <algorithm>
#include <vulkan/vulkan.hpp>

namespace Vulkan {
    class QueueProperty {
    public:
        explicit QueueProperty(
                size_t index,
                vk::PhysicalDevice device,
                vk::QueueFamilyProperties  properties)
                : Property(std::move(properties)), Device(device), Index(index){}

        bool CheckFlag(const vk::QueueFlags& flags) const noexcept {
            return bool(Property.queueFlags & flags);
        }

        auto GetCount() const noexcept { return Property.queueCount; }

        auto GetIndex() const noexcept { return Index; }

        bool CheckSurfaceSupport(vk::SurfaceKHR surface) const {
            return Device.getSurfaceSupportKHR(Index, surface);
        }
    private:
        vk::QueueFamilyProperties Property;
        vk::PhysicalDevice Device;
        size_t Index;
    };

    class Queues {
        static const auto MakeList(const std::vector<vk::QueueFamilyProperties>& vector, vk::PhysicalDevice dev) {
            std::vector<QueueProperty> ret;
            for (int i = 0; i < vector.size(); ++i) {
                ret.emplace_back(i, dev, vector[i]);
            }
            return ret;
        }
    public:
        explicit Queues(vk::PhysicalDevice device)
                :Device(device), FamilyProperties(MakeList(device.getQueueFamilyProperties(), device)) { }

        std::pair<size_t, size_t> GetGraphicsAndPresentFast(VkSurfaceKHR surface) {
            SelectFirstGraphicsQueueFamilyIndex();
            FindPresentQueue(surface);
            return {GraphicsIndex, PresentIndex};
        }

        void SelectFirstGraphicsQueueFamilyIndex() {
            GraphicsIndex = std::find_if(FamilyProperties.begin(), FamilyProperties.end(),
                    [](const auto& qfp) { return qfp.CheckFlag(vk::QueueFlagBits::eGraphics); })->GetIndex();
        }

        void SelectFirstCmputeQueueFamilyIndex() {
            ComputeIndex = std::find_if(FamilyProperties.begin(), FamilyProperties.end(),
                    [](const auto& qfp) { return qfp.CheckFlag(vk::QueueFlagBits::eCompute); })->GetIndex();
        }

        void FindPresentQueue(VkSurfaceKHR surface) {
            CheckCurrentGraphics(surface);
            FindIfGraphicsHasNoPresentSupport(surface);
            FindIfGraphicAndPresentNotCoExist(surface);
            AssertSuccess();
        }

        void CheckCurrentGraphics(VkSurfaceKHR surface) {
            PresentIndex = Device.getSurfaceSupportKHR(static_cast<uint32_t>(GraphicsIndex), surface) ? GraphicsIndex
                    : FamilyProperties.size();
        }

        void AssertSuccess() const {
            if ((GraphicsIndex==FamilyProperties.size()) || (PresentIndex==FamilyProperties.size())) {
                throw std::runtime_error("Could not find a queue for graphics or present");
            }
        }

        void FindIfGraphicAndPresentNotCoExist(VkSurfaceKHR surface) {
            if (PresentIndex==FamilyProperties.size()) {
                // look for an other family index that supports present
                for (auto&& x : FamilyProperties) {
                    if (x.CheckSurfaceSupport(surface)) {
                        PresentIndex = x.GetIndex();
                        return;
                    }
                }
            }
        }

        void FindIfGraphicsHasNoPresentSupport(VkSurfaceKHR surface) {
            if (PresentIndex==FamilyProperties.size()) {
                // look for an other family index that supports both graphics and present
                for (auto&& x : FamilyProperties) {
                    if (x.CheckFlag(vk::QueueFlagBits::eGraphics) && x.CheckSurfaceSupport(surface)) {
                        GraphicsIndex = PresentIndex = x.GetIndex();
                        return;
                    }
                }
            }
        }
    private:
        vk::PhysicalDevice Device;
        size_t GraphicsIndex{}, PresentIndex{}, ComputeIndex{};
        const std::vector<QueueProperty> FamilyProperties;
    };
}