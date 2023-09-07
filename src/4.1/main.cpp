#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include <iostream>
#include "shader.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include"camera.h"
#include"model.h"
#include"imgui/imgui.h"
#include"imgui/imgui_impl_glfw.h"
#include"imgui/imgui_impl_opengl3.h"


// 窗口长宽
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
//按键输入
void processInput(GLFWwindow* window);
//自适应窗口大小
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
//鼠标
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);
//滚轮
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);


//
bool firstMouse = true;
//
float lastX, lastY;

float deltaTime = 0.0f; // 当前帧与上一帧的时间差
float lastFrame = 0.0f; // 上一帧的时间

Camera camera;

int main()
{
 
    glfwInit();          //初始化，使用glfw来打开一个窗口
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);// 设置窗口版本，“MAJOR”代表主版本号
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);// 设置窗口版本，“MAJOR”代表副版本号//OpenGL3.3
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);// GLFW_OPENGL_PROFILE用告诉窗口，这个版本是为了openGL 做准备的。
                                                                    // openGL用的版本用“CORE_PROFILE”来表示，指的是3.1以后的版本 新版的
    const char* glsl_version="#version 330";
    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);  //新建一个窗口
    if (window == NULL)//判断窗口输出是否成功，失败则给出一段提示
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();   //终止
        return -1;
    }
    glfwMakeContextCurrent(window);// 捕获当前窗口，准备对当前窗口进行画图
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window,mouse_callback);//鼠标移动
    glfwSetScrollCallback(window,scroll_callback);//鼠标滚轮
    // 加载所有 OpenGL 函数指针
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    
    //imgui上下文
     IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); 
    (void)io;
    //设置样式风格
    ImGui::StyleColorsDark();
    //设置平台和渲染器
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    //shader文件
    Shader ourShader("./src/4.1/ourShader.vs", "./src/4.1/ourShader.fs");//
    Shader lightShader("./src/4.1/light.vs","./src/4.1/light.fs");
    //开启z缓冲
    glEnable(GL_DEPTH_TEST);

    //将当前深度值和深度缓冲区中的深度值进行比较
    // glDepthFunc(GL_ALWAYS);
    glDepthMask(GL_TRUE);

    //加载模型
    Model ourModel = Model("D:/c++/opengl/model/nanosuit/nanosuit.obj");
    Model cubelight=Model("D:/code/model/light.obj");
    float f=0.0f;
    ImVec4 clear_color =ImVec4(0.1,0.1,0.1,1.0);
  
    //渲染循环
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        processInput(window);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("imgui");
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        ImGui::SliderFloat("float",&f,0.0f,1.0f);
        ImGui::ColorEdit3("clear color",(float*)&clear_color);
        ImGui::End();

   
        glClearColor(clear_color.x,clear_color.y,clear_color.z,clear_color.w);//清空屏幕所用颜色
        
       //清空屏幕和z缓冲
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      
      
       
        ourShader.use();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));	
        ourShader.setMat4("model", model);


        glm::vec3 lightpos(sin(glfwGetTime()), 1.0f, cos(glfwGetTime()));
        // point light 
        ourShader.setVec3("light.position", lightpos);
        ourShader.setVec3("light.ambient", glm::vec3(1.0f, 1.0f, 1.0f));
        ourShader.setVec3("light.diffuse", 1.0f, 1.0f, 1.0f);
        ourShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);
        ourShader.setFloat("light.constant", 1.0f);
        ourShader.setFloat("light.linear", 0.09f);
        ourShader.setFloat("light.quadratic", 0.032f);

        ourShader.setVec3("viewPos",camera.Position);

        ourModel.Draw(ourShader);

        lightShader.use();
        lightShader.setMat4("projection",projection);
        lightShader.setMat4("view",view);
        glm::mat4 models = glm::mat4(1.0f);
        models = glm::translate(models,lightpos);
        
        models = glm::scale(models, glm::vec3(1.0f, 1.0f, 1.0f));	

        lightShader.setMat4("model",models);
        cubelight.Draw(lightShader);

        //渲染gui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


    
        //glfw：交换缓冲区和轮询IO事件
        glfwSwapBuffers(window);//双缓冲
        glfwPollEvents();
    }

    //回收前面分配的GLFW相关资源
        glfwTerminate();
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        return 0;

    
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}


   

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; 

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

