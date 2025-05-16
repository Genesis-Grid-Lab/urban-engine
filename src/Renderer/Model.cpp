#include "uepch.h"
#include "Renderer/Model.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <stb_image.h>
#include "Renderer/AssimpToGlm.h"

//temp
#include <glad/glad.h>
using namespace std;
namespace UE {

    unsigned int TextureFromFile(const char *path, const string &directory)
    {
        string filename = string(path);
        filename = directory + '/' + filename;

        unsigned int textureID;
        glGenTextures(1, &textureID);

        int width, height, nrComponents;
        stbi_set_flip_vertically_on_load(1);
        unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
        if (data)
        {
            GLenum format;
            if (nrComponents == 1)
                format = GL_RED;
            else if (nrComponents == 3)
                format = GL_RGB;
            else if (nrComponents == 4)
                format = GL_RGBA;

            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            stbi_image_free(data);
        }
        else
        {
            std::cout << "Texture failed to load at path: " << path << std::endl;
            stbi_image_free(data);
        }

        return textureID;
    }

    Model::Model(const std::string& path, bool gamma):m_GammaCorrection(gamma),m_Path(path){
        LoadModel(path);
    }

    void Model::Draw(const Ref<Shader>& shader){
        for(auto& mesh : m_Meshes)
            mesh->Draw(shader);        
    }

    void Model::LoadModel(const std::string& path){
        // read file via ASSIMP
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
        // error checking
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {            
            UE_CORE_ERROR("Assimp Error: {0}", importer.GetErrorString());
            return;
        }
        // retrieve the directory path of the filepath
        m_Directory = path.substr(0, path.find_last_of('/'));        
        // process ASSIMP's root node recursively
        ProcessNode(scene->mRootNode, scene);
    }

    void Model::ProcessNode(aiNode* node, const aiScene* scene){
        for(unsigned int i = 0; i < node->mNumMeshes; i++){
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            m_Meshes.push_back(ProcessMesh(mesh, scene));
        }

        for(unsigned int i = 0; i < node->mNumChildren; i++){
            ProcessNode(node->mChildren[i], scene);
        }
    }

    void Model::SetVertexBoneDataToDefault(Vertex& vertex){
        for(int i = 0; i < MAX_BONE_INFLUENCE; i++){
            vertex.m_BoneIDs[i] = -1;
            vertex.m_Weights[i] = 0.0f;
        }
    }

    Ref<Mesh> Model::ProcessMesh(aiMesh* mesh, const aiScene* scene){
        Ref<Mesh> tempMesh;
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        std::vector<TextureMesh> textures;

        for(unsigned int i = 0; i < mesh->mNumVertices; i++){
            Vertex ivertex;
            SetVertexBoneDataToDefault(ivertex);
            ivertex.Position = AssimpGLMHelpers::GetGLMVec(mesh->mVertices[i]);
            ivertex.Normal = mesh->HasNormals() ? AssimpGLMHelpers::GetGLMVec(mesh->mNormals[i]) : glm::vec3(0.0f);
            ivertex.TexCoords = mesh->mTextureCoords[0] ? glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y) : glm::vec2(0.0f);
            ivertex.Tangent = mesh->mTangents ? glm::vec3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z) : glm::vec3(0.0f);
            ivertex.Bitangent = mesh->mBitangents ? glm::vec3(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z) : glm::vec3(0.0f);
            vertices.push_back(ivertex);
        }

        for(unsigned int i = 0; i < mesh->mNumFaces; i++){
            aiFace face = mesh->mFaces[i];
            for(unsigned int j = 0; j < face.mNumIndices; j++){
                indices.push_back(face.mIndices[j]);
            }
        }

        //process materials
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        // we assume a convention for sampler names in the shaders. Each diffuse texture should be named
        // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
        // Same applies to other texture as the following list summarizes:
        // diffuse: texture_diffuseN
        // specular: texture_specularN
        // normal: texture_normalN

        // 1. diffuse maps
        std::vector<TextureMesh> diffuseMaps = LoadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        // 2. specular maps
        std::vector<TextureMesh> specularMaps = LoadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        // 3. normal maps
        std::vector<TextureMesh> normalMaps = LoadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
        // 4. height maps
        std::vector<TextureMesh> heightMaps = LoadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
        textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

        UE_CORE_WARN("Mesh with {0} vertices and {1} indices", vertices.size(), indices.size());

        ExtractBoneWeightForVertices(vertices,mesh,scene);

        tempMesh = CreateRef<Mesh>(vertices, indices, textures);
        return tempMesh;
    }

    void Model::SetVertexBoneData(Vertex& vertex, int boneID, float weight){
        for(int i = 0; i < MAX_BONE_INFLUENCE; i++){
            if(vertex.m_BoneIDs[i] < 0){
                vertex.m_Weights[i] = weight;
				vertex.m_BoneIDs[i] = boneID;
				break;
            }
        }
    }

    void Model::ExtractBoneWeightForVertices(std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene)
	{
		auto& boneInfoMap = m_BoneInfoMap;
		int& boneCount = m_BoneCounter;

		for (int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
		{
			int boneID = -1;
			std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();
            
			if (boneInfoMap.find(boneName) == boneInfoMap.end())
			{
				BoneInfo newBoneInfo;
				newBoneInfo.id = boneCount;
				newBoneInfo.offset = AssimpGLMHelpers::ConvertMatrixToGLMFormat(mesh->mBones[boneIndex]->mOffsetMatrix);
				boneInfoMap[boneName] = newBoneInfo;
				boneID = boneCount;
				boneCount++;
			}
			else
			{
				boneID = boneInfoMap[boneName].id;
			}
			assert(boneID != -1);
			auto weights = mesh->mBones[boneIndex]->mWeights;
			int numWeights = mesh->mBones[boneIndex]->mNumWeights;

			for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex)
			{
				int vertexId = weights[weightIndex].mVertexId;
				float weight = weights[weightIndex].mWeight;                
                if (vertexId >= vertices.size()) {
                    std::cerr << "Invalid vertex ID in bone weights: " << vertexId << " >= " << vertices.size() << "\n";
                    continue; // Or throw an error
                }
				assert(vertexId <= vertices.size());
				SetVertexBoneData(vertices[vertexId], boneID, weight);
			}
		}
	}

    std::vector<TextureMesh> Model::LoadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName){        
        std::vector<TextureMesh> textures;
        for(unsigned int i = 0; i < mat->GetTextureCount(type); i++){
            aiString str;
            mat->GetTexture(type, i, &str);
            if(str.length == 0 || str.C_Str() == nullptr) {
                std::cerr << "Invalid texture path!" << std::endl;
                continue;
            }
            // check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
            bool skip = false;
            for(unsigned int j = 0; j < m_TexturesLoaded.size(); j++){
                if(std::strcmp(m_TexturesLoaded[j].path.data(), str.C_Str()) == 0){
                    textures.push_back(m_TexturesLoaded[j]);
                    skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
                    break;
                }
            }

            if(!skip){
                // if texture hasn't been loaded already, load it
                std::string tempPath = str.C_Str();                
                // Texture2D texture = Texture2D((m_Directory + "/" + tempPath)); 
                TextureMesh texture;
                texture.id = TextureFromFile(str.C_Str(), m_Directory);                         
                texture.type = typeName;
                texture.path = str.C_Str();
                textures.push_back(texture);
                m_TexturesLoaded.push_back(texture);
                UE_CORE_INFO("Loaded tex type {0} path {1}", texture.type, str.C_Str());
                // if(texture.IsLoaded()){
                //     textures.push_back(texture);
                //     m_TexturesLoaded.push_back(texture);
                //     UE_CORE_INFO("Texture Loaded {0} with type {1}", texture.GetRendererID(), texture.type);
                // }
                // else{
                //     // textures.push_back(whiteTex);
                //     // m_TexturesLoaded.push_back(whiteTex);
                //     UE_CORE_INFO("Texture not Loaded {0} with type {1}", texture.GetRendererID(), texture.type);
                // }
            }
        }

        // if(!textures.empty())
            return textures;        
        
    }
    
}