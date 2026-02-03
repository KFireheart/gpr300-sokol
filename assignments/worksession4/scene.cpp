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
}debug;

Scene::Scene()
{


    suzanne = std::make_unique<ew::Model>("assets/models/suzanne.obj");
    toon = std::make_unique<ew::Shader>("assets/shaders/default.vs", "assets/shaders/toon.fs");
    zatoon = std::make_unique<ew::Texture>("assets/textures/ZAtoon.png");


    light = { 
        .brightness = 1.0f,
        .color = {1.0f, 0.0f, 1.0f},
        .position = {2.0f, 2.0f, 2.0f},
    };

    palette = {
        .color1 = {1.0f, 0.0f, 0.0f},
        .color2 = {0.0f, 1.0f, 0.0f},
    };
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

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);
    // glDisable(GL_DEPTH_TEST);

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

    /* build debug ui here */

    ImGui::End();
}