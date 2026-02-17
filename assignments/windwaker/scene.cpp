#include "scene.h"
#include "ew/procGen.h"

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
    glm::vec3 water_color = {0.0f, 0.4f, 0.7f};

}debug;

Scene::Scene()
{
    water = std::make_unique<ew::Shader>("assets/shaders/water.vs", "assets/shaders/water.fs");

    water128 = std::make_unique<ew::Texture>("assets/textures/windwaker/water128.png");
    water64 = std::make_unique<ew::Texture>("assets/textures/windwaker/water64.png");
    water32 = std::make_unique<ew::Texture>("assets/textures/windwaker/water32.png");
    water16 = std::make_unique<ew::Texture>("assets/textures/windwaker/water16.png");
    water8 = std::make_unique<ew::Texture>("assets/textures/windwaker/water8.png");

    plane.load(ew::createPlane(100.0f, 100.0f, 100));
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

    auto index = 0;
    glActiveTexture(GL_TEXTURE0 + index);
    glBindTexture(GL_TEXTURE_2D, water128->getID());

    water->use();

    // scene matrices
    water->setMat4("view_proj", view_proj);

    water->setVec3("camera_position", camera.position);
    water->setVec3("water_color", debug.water_color);
    water->setInt("texture0", 0);

    // draw suzanne
    plane.draw();
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

    ImGui::Checkbox("Paused", &time.paused);
    ImGui::SliderFloat("Time Factor", &time.factor, 0.0f, 10.0f);
    ImGui::ColorEdit3("Water Color", &debug.water_color[0]);
    ImGui::SliderFloat("Shininess", &debug.shininess, 0.0f, 100.0f);
    ImGui::DragFloat("Material Diffuse", &debug.diffuse.x, 0.01f, 0.0f, 1.0f);
    ImGui::DragFloat("Material Specular", &debug.specular.x, 0.01f, 0.0f, 1.0f);
    ImGui::DragFloat("Material Ambient", &debug.ambient.x, 0.01f, 0.0f, 1.0f);

    /* build debug ui here */

    ImGui::End();
}