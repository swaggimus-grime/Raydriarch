#include "raydpch.h"
#include "Model.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;

    bool operator==(const Vertex& other) const {
        return pos == other.pos && color == other.color && texCoord == other.texCoord;
    }
};

namespace std {
    template<> struct hash<Vertex> {
        size_t operator()(Vertex const& vertex) const {
            return ((hash<glm::vec3>()(vertex.pos) ^
                (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
                (hash<glm::vec2>()(vertex.texCoord) << 1);
        }
    };
}

Model::Model(RefPtr<Device> device, const std::string& modelPath)
    :m_Device(device)
{
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> mats;
	tinyobj::attrib_t attrib;
	std::string warn, err;
	RAYD_ASSERT(tinyobj::LoadObj(&attrib, &shapes, &mats, &warn, &err, modelPath.c_str()), warn + err);

    std::unordered_map<Vertex, uint32_t> uniqueVertices{};
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            Vertex vertex{};

            vertex.pos = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
            };

            vertex.texCoord = {
                attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
            };

            vertex.color = { 1.0f, 1.0f, 1.0f };

            if (uniqueVertices.count(vertex) == 0) {
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }

            indices.push_back(uniqueVertices[vertex]);
        }

    }

    m_VLayout.AddAttribute(0, 0, VK_FORMAT_R32G32B32_SFLOAT);
    m_VLayout.AddAttribute(1, 0, VK_FORMAT_R32G32B32_SFLOAT);
    m_VLayout.AddAttribute(2, 0, VK_FORMAT_R32G32_SFLOAT);
    m_VLayout.AddBinding(0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX);
    m_VBuffer = MakeScopedPtr<VertexBuffer>(m_Device, vertices.size(), vertices.size() * sizeof(Vertex), vertices.data());
    m_IBuffer = MakeScopedPtr<IndexBuffer>(m_Device, indices.size(), indices.size() * sizeof(uint32_t), indices.data());
}

void Model::Render(VkCommandBuffer& cbuff)
{
    m_VBuffer->Bind(cbuff);
    m_IBuffer->Bind(cbuff);

    vkCmdDrawIndexed(cbuff, m_IBuffer->GetIndexCount(), 1, 0, 0, 0);
}
