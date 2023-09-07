#ifndef MODEL_H
#define MODEL_H
#include <glad/glad.h> 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "mesh.h"
#include "shader.h"
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
unsigned int TextureFromFile(const char* path, const string& directory, bool gamma = false);

class Model
{
public:
    // model data 
    vector<Texture> textures_loaded;	///�洢��ĿǰΪֹ���ص����������Ż���ȷ����������ض�Ρ�
    vector<Mesh> meshes;//ģ��mesh����
    string directory;//�ļ�Ŀ¼
    bool gammaCorrection;

    // ���캯�����ļ���ַ
    Model(string const& path, bool gamma = false) : gammaCorrection(gamma)
    {
        loadModel(path);
    }

    // ����ģ��
    void Draw(Shader& shader)
    {
        for (unsigned int i = 0; i < meshes.size(); i++)
            meshes[i].Draw(shader);
    }

private:
    // ���ļ��м���һ��֧��ASSIMP��չ��ģ�ͣ������������洢������������
    void loadModel(string const& path)
    {
        // ͨ��ASSIMP��ȡ�ļ�
        Assimp::Importer importer;
        //aiscene���ڵ�                                 //�Զ�ת��Ϊ������      //�Զ����ɶ��㷨����         //��תuv Y��        //�Զ�������������
        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
        // ������
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
            return;
        }
        // �����ļ�·����Ŀ¼·��
        directory = path.substr(0, path.find_last_of('/'));

        // �ݹ�ش���ASSIMP�ĸ��ڵ�
        processNode(scene->mRootNode, scene);


        //test����
        //����
        // cout<<scene->mRootNode->mNumMeshes<<endl;//0
        // cout<<scene->mMeshes[0]->mName.C_Str()<<endl;//visor
        // cout<<scene->mNumMeshes<<endl;//7
        // cout<<scene->mRootNode->mNumChildren<<endl;//7���ӽڵ�
        // cout<<scene->mRootNode->mChildren[0]->mNumMeshes<<endl;//�ӽڵ�0����1��mesh
        // cout<<scene->mRootNode->mChildren[0]->mMeshes[0]<<endl;//mesh����
        // cout<<scene->mRootNode->mChildren[0]->mNumChildren<<endl;
        // cout<<scene->mMeshes[0]->mNumVertices<<endl;//156
        // cout<<scene->mMeshes[0]->mVertices[0].x<<endl;
        // cout<<scene->mMeshes[0]->mVertices[0].y<<endl;
        // cout<<scene->mMeshes[0]->mVertices[0].z<<endl;
        // cout<<scene->mMeshes[0]->mNumFaces<<endl;
        //����
        // cout<<scene->mNumMaterials<<endl;//��������//7
        // for(int i=0;i<scene->mNumMaterials;i++){
        // cout<<scene->mMeshes[i]->mMaterialIndex;//��������
        // cout<<scene->mMaterials[scene->mMeshes[i]->mMaterialIndex]->GetName().C_Str()<<endl;//��������
        // }
        // cout<<scene->mMaterials[0]->GetName().C_Str()<<endl; //��0�Ų���Ϊ�ղ���
       

    }

    // �Եݹ鷽ʽ����ڵ㡣����λ�ڽڵ��ϵ�ÿ�����������񣬲������ӽڵ����ظ��˹���(����еĻ�)��
    void processNode(aiNode* node, const aiScene* scene)
    {
        // ����λ�ڵ�ǰ�ڵ��ÿ������
        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            //�ڵ����ֻ�������������������е�ʵ�ʶ���
            //�������������е����ݣ��ڵ�ֻ�Ǳ��ֶ�������֯(����ڵ�֮��Ĺ�ϵ)��
            //node->mMeshes[]��mesh������
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }
        
        // �����Ǵ�������������(����еĻ�)֮�����ǵݹ�ش���ÿ���ӽڵ�
        for (unsigned int i = 0; i < node->mNumChildren; i++)
        {
            processNode(node->mChildren[i], scene);
        }

    }
//����mesh
    Mesh processMesh(aiMesh* mesh, const aiScene* scene)
    {
        // ��Ҫ��д������
        vector<Vertex> vertices;//��������
        vector<unsigned int> indices;//��������
        vector<Texture> textures;//����

        // ����ÿ������Ķ���
        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;
            glm::vec3 vector; //����������һ��ռλ����������Ϊassimpʹ���Լ���vector�࣬����ֱ��ת��Ϊglm��vec3�࣬�����������Ƚ����ݴ��䵽���ռλ��glm::vec3
            // λ��
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.Position = vector;
            // ����
            if (mesh->HasNormals())
            {
                vector.x = mesh->mNormals[i].x;
                vector.y = mesh->mNormals[i].y;
                vector.z = mesh->mNormals[i].z;
                vertex.Normal = vector;
            }
            // ��������
            if (mesh->mTextureCoords[0]) // �����Ƿ����������ꣿ
            {
                glm::vec2 vec;
                vec.x = mesh->mTextureCoords[0][i].x;
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.TexCoords = vec;
            }
            else
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);

            vertices.push_back(vertex);
        }
        // ���ڱ���ÿ���������(����һ�������������)��������Ӧ�Ķ���������
        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            // ����������������������Ǵ洢������������
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }
        // ����//��mesh��Ӧ�Ĳ���
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        // ���Ǽ�������ɫ������һ�����������Ƶ�Լ����ÿ������������Ӧ������    
        // ��'texture diffuseN'������N��һ����1��MAX SAMPIER NTiMRER��������
        // ��ͬ����������������������ʾ:
        // ������: texture_diffuseN
        // �߹�: texture_specularN
        // ����: texture_normalN

        // 1. diffuse maps                                 //����     //��������              //��������
        vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        // 2. specular maps
        vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        // 3. normal maps
        std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
        // 4. height maps
        std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
        textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

        // ���ش���ȡ���������ݴ������������
        return Mesh(vertices, indices, textures);
    }

    //���������͵����в������������δ���أ����������
    // �������Ϣ��Ϊ����ṹ���ء�
    vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName)
    {
        vector<Texture> textures;         //��鴢���ڲ�������ͼ������
        for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString str;
            //��ȡÿ��������ļ�λ��,��ַ�洢��aistring��,�ڶ���������ȡ������ָ�������е�����//һ������������ж����ͼ
            mat->GetTexture(type, i, &str);
            // ���֮ǰ�Ƿ��������������ǣ�������һ������:���������µ�����
            bool skip = false;
            for (unsigned int j = 0; j < textures_loaded.size(); j++)
            {
                if (std::strcmp(textures_loaded[j].path.data, str.C_Str()) == 0)
                {
                    textures.push_back(textures_loaded[j]);
                    skip = true; // ������ͬ�ļ�·���������Ѿ������أ�������һ����(�Ż�)
                    break;
                }
            }
            if (!skip)
            {   // �������û�м��أ��ͼ�����
                Texture texture;
                texture.id = TextureFromFile(str.C_Str(), this->directory);
                texture.type = typeName;
                texture.path = str.C_Str();//��ͼ��ַ
                textures.push_back(texture);
                textures_loaded.push_back(texture);  // ����洢Ϊ����ģ�ͼ��ص�������ȷ�����ǲ��᲻��Ҫ�ؼ����ظ�������
            }
        }
        return textures;
    }
};


unsigned int TextureFromFile(const char* path, const string& directory, bool gamma)
{
    string filename = string(path);
    //��ַΪģ�������ļ�+��ͼ��
    filename = directory + '/' + filename;

    unsigned int textureID;
    //Ҫ���ɵ���������//ָ��洢���ɵ��������Ƶ�����ĵ�һ��Ԫ�ص�ָ��
    glGenTextures(1, &textureID);
    //stbi_load �ж���ͼ������ɫͨ����
    int width, height, nrComponents;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;
        //��id��
        glBindTexture(GL_TEXTURE_2D, textureID);
        //��������
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        //���ɶ༶����mipmap
        glGenerateMipmap(GL_TEXTURE_2D);
        //������ȷ�ʽ
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
#endif