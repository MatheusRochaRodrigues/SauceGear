#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb/stb_image.h>

#include<string>
#include<fstream>
#include<sstream>
#include<iostream>
#include<cerrno>
#include<vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Camera.h" 
#include "Model.h" 
#include "IBLMapGenerator.h"
#include "SkyboxRenderer.h"
#include "Lighting.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void RenderScene(Shader& pbrShader); 
void renderScene(Shader& shader);
unsigned int loadTexture(char const* path);

// settings
const unsigned int SCR_WIDTH = 1680;
const unsigned int SCR_HEIGHT = 820;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = 800.0f / 2.0;
float lastY = 600.0 / 2.0;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;


// lights
// ------
glm::vec3 lightPositions[] = {
    glm::vec3(-10.0f,  10.0f, 10.0f),
    glm::vec3(10.0f,  10.0f, 10.0f),
    glm::vec3(-10.0f, -10.0f, 10.0f),
    glm::vec3(10.0f, -10.0f, 10.0f),
};
glm::vec3 lightColors[] = {
    glm::vec3(300.0f, 300.0f, 300.0f),
    glm::vec3(300.0f, 300.0f, 300.0f),
    glm::vec3(300.0f, 300.0f, 300.0f),
    glm::vec3(300.0f, 300.0f, 300.0f)
};
int nrRows = 7;
int nrColumns = 7;
float spacing = 2.5;

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "GearSauce", NULL, NULL);
    glfwMakeContextCurrent(window);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    // set depth function to less than AND equal for skybox depth trick.
    glDepthFunc(GL_LEQUAL);
    // enable seamless cubemap sampling for lower mip levels in the pre-filter map.
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    // build and compile shaders
    // -------------------------

    //PBR 
    Shader pbrShader("PBR/pbr.vs", "PBR/pbr.fs");           //Shader pbrShader("pbr.vs", "pbrWithoutIBL.fs");
    //IBL - DIFFUSE
    Shader equirectangularToCubemapShader("cubemap.vs", "PBR/equirectangular_to_cubemap.fs");
    Shader irradianceShader("cubemap.vs", "PBR/irradiance_convolution.fs");
    //IBL - SPECULAR
    Shader prefilterShader("cubemap.vs", "PBR/prefilter.fs");
    Shader brdfShader("PBR/brdf.vs", "PBR/brdf.fs");  
    //Skybox hdr
    Shader backgroundShader("background.vs", "background.fs");

    //BASE COLOR TEST
    //Shader baseColor("BaseColor.vs", "BaseColor.fs"); 
    Shader baseColor("BlinnPhong/BaseShadow.vs", "BlinnPhong/BaseShadow.fs");   //Shader baseShadow("BaseShadow.vs", "BaseShadow.fs");


    //Shader shader("Debug/ShadowMap.vs", "Debug/ShadowMap.fs"); 
    //Shader debugDepthQuad("3.1.3.debug_quad.vs", "3.1.3.debug_quad_depth.fs"); 
    Shader screenShader("Debug/FrameBufferQuad.vs", "Debug/debug_quad_depth.fs");


    pbrShader.use();
    pbrShader.setInt("irradianceMap", 0);
    pbrShader.setInt("prefilterMap", 1);
    pbrShader.setInt("brdfLUT", 2);
    pbrShader.setVec3("albedo", 0.5f, 0.0f, 0.0f);
    pbrShader.setFloat("ao", 1.0f);


    // Inicializa o gerador de mapas IBL
    IBLMapGenerator iblGen(
        equirectangularToCubemapShader,
        irradianceShader,
        prefilterShader,
        brdfShader
    );
    // Gera os mapas a partir do HDR
    iblGen.Generate("resources/Textures/hdr/tst/rogland_moonlit_night_4k.hdr");
    // Obtém os mapas gerados
    GLuint envCubemap = iblGen.GetEnvironmentMap();
    GLuint irradianceMap = iblGen.GetIrradianceMap();
    GLuint prefilterMap = iblGen.GetPrefilterMap();
    GLuint brdfLUTTexture = iblGen.GetBrdfLUT();



    // initialize static shader uniforms before rendering
    // --------------------------------------------------
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    pbrShader.use();
    pbrShader.setMat4("projection", projection);

    SkyboxRenderer skybox(backgroundShader, projection);

    // then before rendering, configure the viewport to the original framebuffer's screen dimensions
    int scrWidth, scrHeight;
    glfwGetFramebufferSize(window, &scrWidth, &scrHeight);
    glViewport(0, 0, scrWidth, scrHeight);



    // load models
    // -----------
    Model ModelDebugAssimp("Resources/Models/backpack/backpack.obj");
    //Model ModelDebugAssimp("Resources/Models/Cerberus_by_Andrew_Maximov/Cerberus_LP.FBX");
    
    unsigned int woodTexture = loadTexture("Resources/Textures/ww.png");


    // configure depth map FBO
    // -----------------------
    const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

    Shader shader("Shadows/ShadowMapD.vs", "Shadows/ShadowMapD.fs");
    Shader shaderP("Shadows/ShadowMapP.vs", "Shadows/ShadowMapP.gs" , "Shadows/ShadowMapP.fs");
    Lighting lightManager(shader, shaderP, SHADOW_WIDTH, SHADOW_HEIGHT);

    /*lightManager.InstanceDirectionalLight(
        glm::vec3(-2.0f, 4.0f, -1.0f), glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));*/

    lightManager.InstancePointLight(
        glm::vec3(0.0f, 0.0f, 0.0f), glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));


    //Shader screenShaderCube("debug_Cube_depth.vs", "debug_Cube_depth.fs");
    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        //glEnable(GL_CULL_FACE);


        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        // render scene, supplying the convoluted irradiance map to the final shader.
        // ------------------------------------------------------------------------------------------
                   // pbrShader.use();
                   // glm::mat4 view = camera.GetViewMatrix();
                   // pbrShader.setMat4("view", view);
                   // pbrShader.setVec3("camPos", camera.Position);

                   // // bind pre-computed IBL data
                   // glActiveTexture(GL_TEXTURE0);
                   // glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
                   // glActiveTexture(GL_TEXTURE1);
                   // glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
                   //glActiveTexture(GL_TEXTURE2);
                   // glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);


                   // for (unsigned int i = 0; i < sizeof(lightPositions) / sizeof(lightPositions[0]); ++i)
                   // {
                   //     glm::vec3 newPos = lightPositions[i] + glm::vec3(sin(glfwGetTime() * 5.0) * 5.0, 0.0, 0.0);
                   //     newPos = lightPositions[i];
                   //     pbrShader.setVec3("lightPositions[" + std::to_string(i) + "]", newPos);
                   //     pbrShader.setVec3("lightColors[" + std::to_string(i) + "]", lightColors[i]);

                   //     glm::mat4 model = glm::mat4(1.0f);
                   //     model = glm::translate(model, newPos);
                   //     model = glm::scale(model, glm::vec3(0.5f));
                   //     pbrShader.setMat4("model", model);
                   //     pbrShader.setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
                   //     renderSphere();
                   // }

                   // RenderScene(pbrShader);

                   // //// render skybox (render as last to prevent overdraw) 
                   // //// (irradianceMap) display irradiance map  -Debug in Skybox
                   // //// (prefilterMap)  display prefilter map    -Debug in Skybox
                   // skybox.Draw(view, envCubemap);  

                   // //// render BRDF map to screen
                   // ////brdfShader.Use();
                   // ////renderQuad();
                   // // 


        ////light
        lightManager.UpdateLights(camera.Position); 
        glCullFace(GL_FRONT); 

        //lightManager.allLights[0].position.z = static_cast<float>(sin(glfwGetTime() * 0.5) * 3.0);
        // move light position over time 

        //Shader* sa = lightManager.DirectionalShader;
        Shader* sa = lightManager.PointShader;

        //// don't forget to enable shader before setting uniforms
        sa->use(); 
        //// view/projection transformations
        glm::mat4 projection1 = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view1 = camera.GetViewMatrix();
        //sa->setMat4("projection", projection1);
        //sa->setMat4("view", view1); 
        ////// render the loaded model
        glm::mat4 model1 = glm::mat4(1.0f);
        //model1 = glm::translate(model1, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
        //model1 = glm::scale(model1, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
        //model1 = glm::scale(model1, glm::vec3(0.1f, 0.1f, 0.1f));	// it's a bit too big for our scene, so scale it down
        //sa->setMat4("model", model1);
        //ModelDebugAssimp.Draw(*sa);
        renderScene(*sa);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glCullFace(GL_BACK); // don't forget to reset original culling face
        //end shadow



        //render scene real
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //// don't forget to enable shader before setting uniforms
        baseColor.use();
        baseColor.setInt("diffuseTexture", 0);
        baseColor.setInt("shadowMap", 2);
        baseColor.setInt("depthMap", 1);

        // view/projection transformations 
        baseColor.setMat4("projection", projection1);
        //baseColor.setMat4("view", view1);
        baseColor.setVec3("viewPos", camera.Position);
        baseColor.setMat4("view", view1);
            
        /// cuidadooooooooooooooooooooooooooooooooooooooooo
        baseColor.setFloat("far_plane", lightManager.far_plane);

        // render the loaded model
        baseColor.setMat4("model", model1);

        baseColor.setVec3("lightPos", lightManager.allLights[0].position);
        baseColor.setMat4("lightSpaceMatrix", lightManager.allLights[0].lightSpaceMatrices);

        //ModelDebugAssimp.Draw(baseColor);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, woodTexture); 

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, lightManager.allLights[0].depthMap);

        //glActiveTexture(GL_TEXTURE1);
        //glBindTexture(GL_TEXTURE_2D, lightManager.allLights[0].depthMap); 
        renderScene(baseColor);



        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
         
        /*screenShader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, lightManager.allLights[0].depthMap);
        renderQuad();

        /*screenShaderCube.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, lightManager.allLights[0].depthMap);
        screenShaderCube.setFloat("far_plane", lightManager.far_plane);
        sa->setMat4("projection", projection1);
        sa->setMat4("view", view1);
        renderCube()*/;


        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}


// renders the 3D scene
// --------------------
void renderScene(Shader& shader)
{
    // room cube
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::scale(model, glm::vec3(5.0f));
    shader.setMat4("model", model);
    glDisable(GL_CULL_FACE); // note that we disable culling here since we render 'inside' the cube instead of the usual 'outside' which throws off the normal culling methods.
    shader.setInt("reverse_normals", 1); // A small little hack to invert normals when drawing cube from the inside so lighting still works.
    renderCube();
    shader.setInt("reverse_normals", 0); // and of course disable it
    glEnable(GL_CULL_FACE);
    // cubes
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(4.0f, -3.5f, 0.0));
    model = glm::scale(model, glm::vec3(0.5f));
    shader.setMat4("model", model);
    renderCube();
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(2.0f, 3.0f, 1.0));
    model = glm::scale(model, glm::vec3(0.75f));
    shader.setMat4("model", model);
    renderCube();
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-3.0f, -1.0f, 0.0));
    model = glm::scale(model, glm::vec3(0.5f));
    shader.setMat4("model", model);
    renderCube();
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-1.5f, 1.0f, 1.5));
    model = glm::scale(model, glm::vec3(0.5f));
    shader.setMat4("model", model);
    renderCube();
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-1.5f, 2.0f, -3.0));
    model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
    model = glm::scale(model, glm::vec3(0.75f));
    shader.setMat4("model", model);
    renderCube();
}

// renders the 3D scene
// --------------------
//void renderScene(const Shader& shader)
//void renderScene(Shader& shader)
//{
//    // floor
//    glm::mat4 model = glm::mat4(1.0f);
//    shader.setMat4("model", model);
//    glBindVertexArray(planeVAO);
//    glDrawArrays(GL_TRIANGLES, 0, 6);
//    // cubes
//    model = glm::mat4(1.0f);
//    model = glm::translate(model, glm::vec3(0.0f, 1.5f, 0.0));
//    model = glm::scale(model, glm::vec3(0.5f));
//    shader.setMat4("model", model);
//    renderCube();
//    model = glm::mat4(1.0f);
//    model = glm::translate(model, glm::vec3(2.0f, 0.0f, 1.0));
//    model = glm::scale(model, glm::vec3(0.5f));
//    shader.setMat4("model", model);
//    renderCube  ();
//    model = glm::mat4(1.0f);
//    model = glm::translate(model, glm::vec3(-1.0f, 0.0f, 2.0));
//    model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
//    model = glm::scale(model, glm::vec3(0.25));
//    shader.setMat4("model", model);
//    renderCube();
//
//    model = glm::mat4(1.0f); 
//    shader.setMat4("model", model);
//    renderPlane();
//}


void RenderScene(Shader& pbrShader) {
    // render rows*column number of spheres with varying metallic/roughness values scaled by rows and columns respectively
    glm::mat4 model = glm::mat4(1.0f);
    for (int row = 0; row < nrRows; ++row)
    {
        pbrShader.setFloat("metallic", (float)row / (float)nrRows);
        for (int col = 0; col < nrColumns; ++col)
        {
            // we clamp the roughness to 0.025 - 1.0 as perfectly smooth surfaces (roughness of 0.0) tend to look a bit off
            // on direct lighting.
            pbrShader.setFloat("roughness", glm::clamp((float)col / (float)nrColumns, 0.05f, 1.0f));

            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(
                (float)(col - (nrColumns / 2)) * spacing,
                (float)(row - (nrRows / 2)) * spacing,
                -2.0f
            ));
            pbrShader.setMat4("model", model);
            pbrShader.setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
            renderSphere();
        }
    }


    // render light source (simply re-render sphere at light positions)
    // this looks a bit off as we use the same shader, but it'll make their positions obvious and 
    // keeps the codeprint small.
    for (unsigned int i = 0; i < sizeof(lightPositions) / sizeof(lightPositions[0]); ++i)
    {
        glm::vec3 newPos = lightPositions[i] + glm::vec3(sin(glfwGetTime() * 5.0) * 5.0, 0.0, 0.0);
        newPos = lightPositions[i];
        pbrShader.setVec3("lightPositions[" + std::to_string(i) + "]", newPos);
        pbrShader.setVec3("lightColors[" + std::to_string(i) + "]", lightColors[i]);

        model = glm::mat4(1.0f);
        model = glm::translate(model, newPos);
        model = glm::scale(model, glm::vec3(0.5f));
        pbrShader.setMat4("model", model);
        pbrShader.setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
        renderSphere();
    }
}

bool wireframe = false;
bool zPressedLastFrame = false;
// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
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


    bool zPressed = glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS;
    if (zPressed && !zPressedLastFrame) {
        wireframe = !wireframe;
        glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
    } 
    zPressedLastFrame = zPressed;

}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}


// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
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

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat 
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
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