#pragma once

#include <vulkan/vulkan.hpp>
#include "../util/exceptions.h"

namespace Vulkan {
    class Compiler {
    public:
        class GlslangFailure : public std::exception {
        public:
            const char* what() const noexcept override { return _info.c_str(); }
            const char* debug() const noexcept { return _info.c_str(); }
        protected:
            GlslangFailure(std::string info, std::string debug) noexcept
                    :_info(std::move((info))), _debug(std::move(debug)) { }
        private:
            std::string _info, _debug;
        };

        class GlslangCompileFailure : public GlslangFailure {
            friend class Vulkan::Compiler;
            template <class T>
            explicit GlslangCompileFailure(T& shader)
                    :GlslangFailure(shader.getInfoLog(), shader.getInfoDebugLog()) { }
        };

        class GlslangLinkFailure : public GlslangFailure {
            friend class Vulkan::Compiler;
            template <class T>
            explicit GlslangLinkFailure(T& program)
                    :GlslangFailure(program.getInfoLog(), program.getInfoDebugLog()) { }
        };

        VXRT_EXCEPTION(UnknownShaderStageException, "Unknown Shader Stage")

        static void Load();

        static std::vector<unsigned int> CompileGlslang(vk::ShaderStageFlagBits type, const std::string& source);

        static vk::UniqueShaderModule CreateModule(vk::UniqueDevice& device, const std::vector<unsigned int>& spv);

        static void Unload();
    };
}