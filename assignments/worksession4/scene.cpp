#include "scene.h"

// imgui
#include "imgui/imgui.h"
#include "imguizmo/imguizmo.h"

// glm
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

// batteries
#include "batteries/opengl.h"



struct {
    float shininess = 10.0f;
    glm::vec3 diffuse = {0.5f, 0.5f, 0.5f};
    glm::vec3 specular = {1.0f, 1.0f, 1.0f};
    glm::vec3 ambient = {0.1f, 0.1f, 0.1f};
    float strength = 1.0f;
} debug;

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

Scene::Scene()
{


    suzanne = std::make_unique<ew::Model>("assets/models/suzanne.obj");
    toon = std::make_unique<ew::Shader>("assets/shaders/default.vs", "assets/shaders/toon.fs");
    zatoon = std::make_unique<ew::Texture>("assets/textures/ZAtoon.png");

    //post processing
    post_process = std::make_unique<ew::Shader>("assets/shaders/fullscreen.vs", "assets/shaders/blur.fs");


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

    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER,  fbo);
    {

        glGenTextures(1, &fbo_texture);
        glBindTexture(GL_TEXTURE_2D, fbo_texture);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 800, 600, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
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


        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            printf("ERROR: Framebuffer is not complete\n");
            return;
        }

        //cleanup textures
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Scene::~Scene()
{
    glDeleteFramebuffers(1, &fbo);
}

void Scene::Update(float dt)
{
    batteries::Scene::Update(dt);

    /* body */
}


void Scene::Render(void)
{
    const auto view_proj = camera.Projection() * camera.View();


    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

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


    toon->use();

    // scene matrices
    toon->setMat4("model", glm::mat4(1.0f));
    toon->setMat4("view_proj", view_proj);

    toon->setVec3("camera", camera.position);
    toon->setVec3("light.position", light.position);
    toon->setVec3("light.color", light.color);

    //set the tune texture
    toon->setInt("zatoon", index);


    toon->setVec3("material.diffuse", debug.diffuse);
    toon->setVec3("material.specular", debug.specular);
    toon->setVec3("material.ambient", debug.ambient);
    toon->setFloat("material.shininess", debug.shininess);

    toon->setVec3("pal.color1", palette.color1);
    toon->setVec3("pal.color2", palette.color2);



    // draw suzanne
    suzanne->draw();

    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    {//render the fullscreen quad
    post_process->use();
    post_process->setInt("the_screen", 0);

    post_process->setFloat("strength", debug.strength);

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

    ImGui::SeparatorText("Palette");
    ImGui::ColorEdit3("Color 1", &palette.color1[0]);
    ImGui::ColorEdit3("Color 2", &palette.color2[0]);

    ImGui::SeparatorText("post processing");
    ImGui::SliderFloat("blur strength", &debug.strength, 0.0f, 10.0f);

    ImGui::Image(
        (void*)(intptr_t)fbo_texture,
        ImVec2(400, 300),
        ImVec2(0, 1), ImVec2(1, 0));

        //fbo depth texture
        ImGui::Image(
            (void*)(intptr_t)fbo_depth_texture,
            ImVec2(400, 300),
            ImVec2(0, 1), ImVec2(1, 0));

    /* build debug ui here */

    ImGui::End();
}