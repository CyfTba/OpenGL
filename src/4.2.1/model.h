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
    vector<Texture> textures_loaded;	///存储到目前为止加载的所有纹理，优化以确保纹理不会加载多次。
    vector<Mesh> meshes;//模型mesh数据
    string directory;//文件目录
    bool gammaCorrection;

    // 构造函数，文件地址
    Model(string const& path, bool gamma = false) : gammaCorrection(gamma)
    {
        loadModel(path);
    }

    // 绘制模型
    void Draw(Shader& shader)
    {
        for (unsigned int i = 0; i < meshes.size(); i++)
            meshes[i].Draw(shader);
    }

private:
    // 从文件中加载一个支持ASSIMP扩展的模型，并将结果网格存储在网格向量中
    void loadModel(string const& path)
    {
        // 通过ASSIMP读取文件
        Assimp::Importer importer;
        //aiscene根节点                                 //自动转换为三角形      //自动生成顶点法向量         //翻转uv Y轴        //自动计算切线坐标
        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
        // 检查错误
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
            return;
        }
        // 检索文件路径的目录路径
        directory = path.substr(0, path.find_last_of('/'));

        // 递归地处理ASSIMP的根节点
        processNode(scene->mRootNode, scene);


        //test测试
        //网格
        // cout<<scene->mRootNode->mNumMeshes<<endl;//0
        // cout<<scene->mMeshes[0]->mName.C_Str()<<endl;//visor
        // cout<<scene->mNumMeshes<<endl;//7
        // cout<<scene->mRootNode->mNumChildren<<endl;//7个子节点
        // cout<<scene->mRootNode->mChildren[0]->mNumMeshes<<endl;//子节点0中有1个mesh
        // cout<<scene->mRootNode->mChildren[0]->mMeshes[0]<<endl;//mesh索引
        // cout<<scene->mRootNode->mChildren[0]->mNumChildren<<endl;
        // cout<<scene->mMeshes[0]->mNumVertices<<endl;//156
        // cout<<scene->mMeshes[0]->mVertices[0].x<<endl;
        // cout<<scene->mMeshes[0]->mVertices[0].y<<endl;
        // cout<<scene->mMeshes[0]->mVertices[0].z<<endl;
        // cout<<scene->mMeshes[0]->mNumFaces<<endl;
        //材质
        // cout<<scene->mNumMaterials<<endl;//材质数量//7
        // for(int i=0;i<scene->mNumMaterials;i++){
        // cout<<scene->mMeshes[i]->mMaterialIndex;//材质索引
        // cout<<scene->mMaterials[scene->mMeshes[i]->mMaterialIndex]->GetName().C_Str()<<endl;//材质名字
        // }
        // cout<<scene->mMaterials[0]->GetName().C_Str()<<endl; //第0号材质为空材质
       

    }

    // 以递归方式处理节点。处理位于节点上的每个单独的网格，并在其子节点上重复此过程(如果有的话)。
    void processNode(aiNode* node, const aiScene* scene)
    {
        // 处理位于当前节点的每个网格
        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            //节点对象只包含索引来索引场景中的实际对象。
            //场景包含了所有的数据，节点只是保持东西的组织(比如节点之间的关系)。
            //node->mMeshes[]是mesh的索引
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }
        
        // 在我们处理完所有网格(如果有的话)之后，我们递归地处理每个子节点
        for (unsigned int i = 0; i < node->mNumChildren; i++)
        {
            processNode(node->mChildren[i], scene);
        }

    }
//处理mesh
    Mesh processMesh(aiMesh* mesh, const aiScene* scene)
    {
        // 需要填写的数据
        vector<Vertex> vertices;//顶点数据
        vector<unsigned int> indices;//顶点索引
        vector<Texture> textures;//材质

        // 遍历每个网格的顶点
        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;
            glm::vec3 vector; //我们声明了一个占位符向量，因为assimp使用自己的vector类，不能直接转换为glm的vec3类，所以我们首先将数据传输到这个占位符glm::vec3
            // 位置
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.Position = vector;
            // 法向
            if (mesh->HasNormals())
            {
                vector.x = mesh->mNormals[i].x;
                vector.y = mesh->mNormals[i].y;
                vector.z = mesh->mNormals[i].z;
                vertex.Normal = vector;
            }
            // 纹理坐标
            if (mesh->mTextureCoords[0]) // 网格是否有纹理坐标？
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
        // 现在遍历每个网格的面(面是一个网格的三角形)并检索相应的顶点索引。
        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            // 检索面的所有索引并将它们存储在索引向量中
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }
        // 材质//该mesh对应的材质
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        // 我们假设在着色器中有一个采样器名称的约定。每个漫反射纹理都应该命名    
        // 如'texture diffuseN'，其中N是一个从1到MAX SAMPIER NTiMRER的连续数
        // 这同样适用于其他纹理，如下所示:
        // 漫反射: texture_diffuseN
        // 高光: texture_specularN
        // 法向: texture_normalN

        // 1. diffuse maps                                 //材质     //材质类型              //材质名称
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

        // 返回从提取的网格数据创建的网格对象
        return Mesh(vertices, indices, textures);
    }

    //检查给定类型的所有材质纹理，如果尚未加载，则加载纹理。
    // 所需的信息作为纹理结构返回。
    vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName)
    {
        vector<Texture> textures;         //检查储存在材质中贴图的数量
        for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString str;
            //获取每个纹理的文件位置,地址存储在aistring中,第二个参数获取纹理在指定类型中的索引//一个材质球可以有多个贴图
            mat->GetTexture(type, i, &str);
            // 检查之前是否加载了纹理，如果是，继续下一个迭代:跳过加载新的纹理
            bool skip = false;
            for (unsigned int j = 0; j < textures_loaded.size(); j++)
            {
                if (std::strcmp(textures_loaded[j].path.data, str.C_Str()) == 0)
                {
                    textures.push_back(textures_loaded[j]);
                    skip = true; // 具有相同文件路径的纹理已经被加载，继续下一个。(优化)
                    break;
                }
            }
            if (!skip)
            {   // 如果纹理还没有加载，就加载它
                Texture texture;
                texture.id = TextureFromFile(str.C_Str(), this->directory);
                texture.type = typeName;
                texture.path = str.C_Str();//贴图地址
                textures.push_back(texture);
                textures_loaded.push_back(texture);  // 将其存储为整个模型加载的纹理，以确保我们不会不必要地加载重复的纹理。
            }
        }
        return textures;
    }
};


unsigned int TextureFromFile(const char* path, const string& directory, bool gamma)
{
    string filename = string(path);
    //地址为模型所在文件+贴图名
    filename = directory + '/' + filename;

    unsigned int textureID;
    //要生成的纹理数量//指向存储生成的纹理名称的数组的第一个元素的指针
    glGenTextures(1, &textureID);
    //stbi_load 判断贴图长宽，颜色通道数
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
        //将id绑定
        glBindTexture(GL_TEXTURE_2D, textureID);
        //生成纹理
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        //生成多级渐变mipmap
        glGenerateMipmap(GL_TEXTURE_2D);
        //纹理过度方式
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