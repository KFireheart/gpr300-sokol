#include "scene.h"

// imgui
#include "imgui/imgui.h"
#include "imguizmo/ImGuizmo.h"

// glm
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

// batteries
#include "batteries/opengl.h"

#include "ew/procGen.h"


struct {
    float shininess = 10.0f;
    glm::vec3 diffuse = {0.5f, 0.5f, 0.5f};
    glm::vec3 specular = {1.0f, 1.0f, 1.0f};
    glm::vec3 ambient = {0.1f, 0.1f, 0.1f};
    float blur_strength = 0.063f;
    float shadow_bias = 0.02f;
} debug;

struct {
    GLuint fbo;
    GLuint depth;

} ShadowBuffer;


struct FullScreenQuad {
        GLuint vao;
        GLuint vbo;

        void initialize() {
            float vertices[] = {
                // pos (x, y),
                // texcoord (u, v)
                // triangle 1
                -1.0f, 1.0f, 0.0f, 1.0f,   
                -1.0f, -1.0f, 0.0f, 0.0f,
                1.0f, -1.0f, 1.0f, 0.0f,
    
                // triangle 2
                -1.0f, 1.0f, 0.0f, 1.0f,
                1.0f, -1.0f, 1.0f, 0.0f,
                1.0f, 1.0f, 1.0f, 1.0f,
            };

            glGenVertexArrays(1, &vao);
            glBindVertexArray(vao);

            glBindVertexArray(vao);
            glGenBuffers(1, &vbo);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(sizeof(float) * 2));

            //allways last
            glBindVertexArray(0);
        }
    } fullscreen_quad;


void Scene::createFrameBuffer()
{
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER,  fbo);
    {

        glGenTextures(1, &fbo_texture);
        glBindTexture(GL_TEXTURE_2D, fbo_texture);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 800, 600, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        //fbo color attachment      
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo_texture, 0);


        //fbo depth texture
        glGenTextures(1, &fbo_depth_texture);
        glBindTexture(GL_TEXTURE_2D, fbo_depth_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, 800, 600, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        //fbo depth attachment
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, fbo_depth_texture, 0);


        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            const char* msg = (status == GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT) ? "INCOMPLETE_ATTACHMENT" :
                (status == GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT) ? "MISSING_ATTACHMENT" :
                (status == GL_FRAMEBUFFER_UNSUPPORTED) ? "UNSUPPORTED (e.g. format)" : "OTHER";
            printf("ERROR: Main framebuffer not complete: %s (0x%x)\n", msg, (unsigned)status);
            return;
        }

        //cleanup textures
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Scene::createDepthBuffer()
{
    glGenFramebuffers(1, &shadow_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER,  shadow_fbo);
    {

        glGenTextures(1, &shadow_depth_texture);
        glBindTexture(GL_TEXTURE_2D, shadow_depth_texture);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, 800, 600, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        //depth attachment      
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadow_depth_texture, 0);

        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);

        GLenum depthStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (depthStatus != GL_FRAMEBUFFER_COMPLETE) {
            const char* msg = (depthStatus == GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT) ? "INCOMPLETE_ATTACHMENT" :
                (depthStatus == GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT) ? "MISSING_ATTACHMENT" :
                (depthStatus == GL_FRAMEBUFFER_UNSUPPORTED) ? "UNSUPPORTED" : "OTHER";
            printf("ERROR: Shadow/depth framebuffer not complete: %s (0x%x)\n", msg, (unsigned)depthStatus);
            return;
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Scene::Scene()
{


    suzanne = std::make_unique<ew::Model>("assets/models/suzanne.obj");
    toon = std::make_unique<ew::Shader>("assets/shaders/default.vs", "assets/shaders/toon.fs");
    shadow = std::make_unique<ew::Shader>("assets/shaders/default_shadowmap.vs", "assets/shaders/toon_shadowmap.fs");
    zatoon = std::make_unique<ew::Texture>("assets/textures/ZAtoon.png");
    depth = std::make_unique<ew::Shader>("assets/shaders/depth.vs", "assets/shaders/depth.fs");

    //post processing
    post_process = std::make_unique<ew::Shader>("assets/shaders/fullscreen.vs", "assets/shaders/postprocessing/blur.fs");

    plane.load(ew::createPlane(100.0f, 100.0f, 100));


    light = { 
        .brightness = 1.0f,
        .color = {1.0f, 0.0f, 1.0f},
        .position = {2.0f, 2.0f, 2.0f},
    };

    palette = {
        .color1 = {1.0f, 0.0f, 0.0f},
        .color2 = {0.0f, 1.0f, 0.0f},
    };



    fullscreen_quad.initialize();

    createFrameBuffer();
    createDepthBuffer();


}



Scene::~Scene()
{
}

void Scene::Update(float dt)
{
    batteries::Scene::Update(dt);

    /* body */
}


void Scene::Render(void)
{
    const auto view_proj = camera.Projection() * camera.View();
    const auto light_proj = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.1f, 10.0f);
    const auto light_view = glm::lookAt(light.position, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    const auto light_view_proj = light_proj * light_view;
    
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glBindFramebuffer(GL_FRAMEBUFFER, shadow_fbo);
    {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glEnable(GL_DEPTH_TEST);

        glViewport(0, 0, 800, 600);

        glClear(GL_DEPTH_BUFFER_BIT);

        depth->use();

        // scene matrices
        depth->setMat4("light_view_proj", light_view_proj);
        depth->setMat4("model", glm::mat4(1.0f));
        suzanne->draw();

        // also render the plane into the shadow map
        const auto plane_mat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -2.0f, 0.0f));
        depth->setMat4("model", plane_mat);
        plane.draw();

    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //Suzanne pipeline
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);


    auto index = 0;
    glActiveTexture(GL_TEXTURE0 + index);
    glBindTexture(GL_TEXTURE_2D, zatoon->getID());

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, shadow_depth_texture);


    shadow->use();

    // scene matrices
    shadow->setMat4("view_proj", view_proj);
    shadow->setMat4("light_view_proj", light_view_proj);

    shadow->setVec3("camera", camera.position);
    shadow->setVec3("light.position", light.position);
    shadow->setVec3("light.color", light.color);

    // set textures: ZAtoon gradient and shadow map
    shadow->setInt("zatoon", index);      // texture unit 0
    shadow->setInt("shadow_map", 1);      // texture unit 1

    shadow->setVec3("material.diffuse", debug.diffuse);
    shadow->setVec3("material.specular", debug.specular);
    shadow->setVec3("material.ambient", debug.ambient);
    shadow->setFloat("material.shininess", debug.shininess);

    shadow->setVec3("pal.color1", palette.color1);
    shadow->setVec3("pal.color2", palette.color2);

    // shadow bias to fight acne (tweak in UI)
    shadow->setFloat("bias", debug.shadow_bias);

    // draw suzanne
    shadow->setMat4("model", glm::mat4(1.0f));
    suzanne->draw();

    const auto plane_mat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -2.0f, 0.0f));
    shadow->setMat4("model", plane_mat);
    plane.draw();

    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    {//render the fullscreen quad
    post_process->use();
    post_process->setInt("screen", 0);

    post_process->setFloat("strength", debug.blur_strength);

    //set up the fullscreen pipeline
    glDisable(GL_DEPTH_TEST);

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //draw the fullscreen quad
    glBindVertexArray(fullscreen_quad.vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fbo_texture);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    
}

void Scene::Debug(void)
{
    ImGuizmo::BeginFrame();
    ImGuizmo::SetDrawlist(ImGui::GetBackgroundDrawList());
    ImGuizmo::SetRect(0, 0, ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y);

    glm::mat4 m{1.0f};
    auto *view = glm::value_ptr(camera.View());
    auto *proj = glm::value_ptr(camera.Projection());
    
    ImGuizmo::DrawGrid(view, proj, glm::value_ptr(m), 100.0f);

    auto matrix = glm::translate(glm::mat4(1.0f), light.position);
    ImGuizmo::Manipulate(
        view,
        proj,
        ImGuizmo::TRANSLATE,
        ImGuizmo::WORLD,
        glm::value_ptr(matrix)
    );

    if (ImGuizmo::IsUsing())
    {
        light.position = glm::vec3(matrix[3]);
    }

    cameracontroller.Debug();

    ImGui::Begin("Controlls", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::Checkbox("Paused", &time.paused);
    ImGui::SliderFloat("Time Factor", &time.factor, 0.0f, 10.0f);
    ImGui::ColorEdit3("Light Color", &light.color[0]);
    ImGui::SliderFloat("Shininess", &debug.shininess, 0.0f, 100.0f);
    ImGui::DragFloat("Material Diffuse", &debug.diffuse.x, 0.01f, 0.0f, 1.0f);
    ImGui::DragFloat("Material Specular", &debug.specular.x, 0.01f, 0.0f, 1.0f);
    ImGui::DragFloat("Material Ambient", &debug.ambient.x, 0.01f, 0.0f, 1.0f);
    ImGui::SliderFloat("Shadow Bias", &debug.shadow_bias, 0.0f, 0.01f);

    ImGui::SeparatorText("Palette");
    ImGui::ColorEdit3("Color 1", &palette.color1[0]);
    ImGui::ColorEdit3("Color 2", &palette.color2[0]);

    ImGui::SeparatorText("post processing");
    ImGui::SliderFloat("blur strength", &debug.blur_strength, 0.0f, 20.0f);

    ImGui::Image(
        (void*)(intptr_t)fbo_texture,
        ImVec2(400, 300),
        ImVec2(0, 1), ImVec2(1, 0));

        //fbo depth texture
        ImGui::Image(
            (void*)(intptr_t)fbo_depth_texture,
            ImVec2(400, 300),
            ImVec2(0, 1), ImVec2(1, 0));

        ImGui::Image(
            (void*)(intptr_t)shadow_depth_texture,
            ImVec2(400, 300),
            ImVec2(0, 1), ImVec2(1, 0));

    /* build debug ui here */

    ImGui::End();
}