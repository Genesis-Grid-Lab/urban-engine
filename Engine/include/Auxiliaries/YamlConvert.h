#pragma once
#include "Renderer/Mesh.h"
#include <yaml-cpp/yaml.h>

using namespace UE;

namespace YAML {

	template<>
	struct convert<glm::vec2>
	{
		static Node encode(const glm::vec2& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::vec2& rhs)
		{
			if (!node.IsSequence() || node.size() != 2)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec3>
	{
		static Node encode(const glm::vec3& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::vec3& rhs)
		{
			if (!node.IsSequence() || node.size() != 3)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec4>
	{
		static Node encode(const glm::vec4& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::vec4& rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.w = node[3].as<float>();
			return true;
		}
	};

    template<>
    struct convert<Vertex>
    {
        static Node encode(const Vertex& vert)
        {
            Node node;
            node.push_back(vert.Position);
            node.push_back(vert.Normal);
            node.push_back(vert.TexCoords);
            node.push_back(vert.Tangent);
            node.push_back(vert.Bitangent);
            // node.push_back(vert.m_BoneIDs);
            // node.push_back(vert.m_Weights);
            node.push_back(vert.EntityID);
        }

        static bool decode(const Node& node, Vertex& vert)
        {
            //TO DO: arranger
            // if (!node.IsSequence() || node.size() != 4)
				// return false;

            vert.Position = node[0].as<glm::vec3>();
            vert.Normal = node[1].as<glm::vec3>();
            vert.TexCoords = node[2].as<glm::vec2>();
            vert.Tangent = node[3].as<glm::vec3>();
            vert.Bitangent = node[4].as<glm::vec3>();

            // vert.m_BoneIDs = node[5].as<int*>();
            // vert.m_Weights = node[6].as<float*>();
            vert.EntityID = node[7].as<int>();
        }
    };

}