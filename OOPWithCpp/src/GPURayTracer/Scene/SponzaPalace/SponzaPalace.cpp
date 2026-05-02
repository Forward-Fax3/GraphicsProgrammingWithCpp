//
// Created by forwardfax3 on 17/04/2026.
//


#include "SponzaPalace.hpp"
#include "Log.hpp"

#include "SceneMesh.hpp"

#include <tiny_gltf_v3.h>
#include <filesystem>

#include "glm/ext/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"


namespace OWC
{
    SponzaPalace::SponzaPalace()
        : m_TLAS(BaseTLAS::CreateTopLevelAccelerationStructure())
    {
        constexpr std::string_view filename = "./../../../../../GLTFs/Sponza/NewSponza_Main_glTF_003.gltf";

        tg3_parse_options options;
        tg3_parse_options_init(&options);

        tg3_error_stack errorStack;
        tg3_parse_file(&m_Model, &errorStack, filename.data(), filename.size(), &options);

        if (errorStack.has_error)
        {
            for (u32 i = 0; i < errorStack.count; i++)
                switch (const tg3_error_entry& e = errorStack.entries[i]; e.severity)
                {
                case TG3_SEVERITY_INFO:
                    Log<LogLevel::Trace>("TinyGLTF: {}", e.message);
                    break;
                case TG3_SEVERITY_WARNING:
                    Log<LogLevel::Warn>("TinyGLTF: {}", e.message);
                    break;
                case TG3_SEVERITY_ERROR:
                    Log<LogLevel::Error>("TinyGLTF: {}", e.message);
                    break;
                default:
                    Log<LogLevel::Error>("Unknown TinyGLTF message: {}", e.message);
                    break;
                }
            return;
        }

        const auto bufferSize = m_Model.buffers[0].data.count;
        m_GPUBuffer = Graphics::GeneralBuffer::CreateGeneralBuffer(bufferSize);
        m_GPUBuffer->UpdateBufferData(std::bit_cast<const u8*>(m_Model.buffers[0].data.data));

        const auto sceneIndex = m_Model.default_scene == -1 ? 0 : m_Model.default_scene;
        const auto& scene = m_Model.scenes[sceneIndex];

        for (u32 i = 0; i < scene.nodes_count; i++)
            IterateThroughNodes(m_Model, scene.nodes[i], Mat4(1.0f));

        m_TLAS->CreateTLAS();

        tg3_error_stack_free(&errorStack);
    }

    SponzaPalace::~SponzaPalace()
    {
        tg3_model_free(&m_Model);
    }

    void SponzaPalace::IterateThroughNodes(const tg3_model& model, const u32 nodeIndex, Mat4 parentTransform)
    {
        const tg3_node& node = m_Model.nodes[nodeIndex];

        // casts are used in this next section because tinygltf gives the matrix with doubles but vulkan uses floats, and we change it right away
        if (node.has_matrix)
            parentTransform = parentTransform * static_cast<Mat4>(glm::make_mat4(node.matrix));
        else
        {
            const auto floatRotation = static_cast<Vec4>(glm::make_vec4(node.rotation));

            const Mat4 translation = glm::translate(glm::mat4(1.0f), static_cast<Vec3>(glm::make_vec3(node.translation)));
            const Mat4 rotation = glm::mat4_cast(glm::quat(floatRotation.w, floatRotation.x, floatRotation.y, floatRotation.z));
            const Mat4 scale = glm::scale(Mat4(1), static_cast<Vec3>(glm::make_vec3(node.scale)));

            parentTransform = parentTransform * (translation * rotation * scale);
        }

        if (node.mesh != -1)
        {
            if (!m_Meshes.contains(node.mesh))
                m_Meshes.emplace(node.mesh, SceneMesh::CreateFromGLTFModelWithMeshIndex(m_Model, node.mesh, m_GPUBuffer));

            m_TLAS->AddInstance(parentTransform, m_Meshes[node.mesh]);
        }

        if (node.light != -1)
        {
            
        }

        for (u32 i = 0; i < node.children_count; i++)
            IterateThroughNodes(model, node.children[i], parentTransform);
    }
} // OWC