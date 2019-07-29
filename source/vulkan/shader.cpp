#include "shader.h"
#include <SPIRV/GlslangToSpv.h>
#include <StandAlone/ResourceLimits.h>

namespace Vulkan {
    namespace { ;
        EShLanguage translateShaderStage(vk::ShaderStageFlagBits stage) {
            switch (stage) {
            case vk::ShaderStageFlagBits::eVertex: return EShLangVertex;
            case vk::ShaderStageFlagBits::eTessellationControl: return EShLangTessControl;
            case vk::ShaderStageFlagBits::eTessellationEvaluation: return EShLangTessEvaluation;
            case vk::ShaderStageFlagBits::eGeometry: return EShLangGeometry;
            case vk::ShaderStageFlagBits::eFragment: return EShLangFragment;
            case vk::ShaderStageFlagBits::eCompute: return EShLangCompute;
            case vk::ShaderStageFlagBits::eRaygenNVX: return EShLangRayGenNV;
            case vk::ShaderStageFlagBits::eAnyHitNVX: return EShLangAnyHitNV;
            case vk::ShaderStageFlagBits::eClosestHitNVX: return EShLangClosestHitNV;
            case vk::ShaderStageFlagBits::eMissNVX: return EShLangMissNV;
            case vk::ShaderStageFlagBits::eIntersectionNVX: return EShLangIntersectNV;
            case vk::ShaderStageFlagBits::eCallableNVX: return EShLangCallableNV;
            case vk::ShaderStageFlagBits::eTaskNV: return EShLangTaskNV;
            case vk::ShaderStageFlagBits::eMeshNV: return EShLangMeshNV;
            default: throw Compiler::UnknownShaderStageException();
            }
        }
    }

    void Compiler::Load() {
        glslang::InitializeProcess();
    }

    std::vector<unsigned int> Compiler::CompileGlslang(const vk::ShaderStageFlagBits type, const std::string& source) {
        std::vector<unsigned int> spvShader;
        EShLanguage stage = translateShaderStage(type);

        const char* shaderStrings[1] = {source.data()};
        glslang::TShader shader(stage);
        shader.setStrings(shaderStrings, 1);
        // Enable SPIR-V and Vulkan rules when parsing GLSL
        auto messages = (EShMessages) (EShMsgSpvRules | EShMsgVulkanRules);
        if (!shader.parse(&glslang::DefaultTBuiltInResource, 100, false, messages)) {
            throw Compiler::GlslangCompileFailure(shader);
        }

        glslang::TProgram program;
        program.addShader(&shader);
        if (!program.link(messages)) {
            throw Compiler::GlslangLinkFailure(program);
        }
        glslang::GlslangToSpv(*program.getIntermediate(stage), spvShader);
        return spvShader;
    }

    vk::UniqueShaderModule Compiler::CreateModule(vk::UniqueDevice& device, const std::vector<unsigned int>& spv) {
        return device->createShaderModuleUnique(
                vk::ShaderModuleCreateInfo(
                        vk::ShaderModuleCreateFlags(),
                        spv.size()*sizeof(unsigned int), spv.data()
                )
        );
    }

    void Compiler::Unload() {
        glslang::FinalizeProcess();
    }
}