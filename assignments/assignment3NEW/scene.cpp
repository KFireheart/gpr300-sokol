#include "scene.h"

// batteries
#include "batteries/math.h"
#include "batteries/opengl.h"

// ew
#include "ew/procGen.h"

// imgui
#include "imgui/imgui.h"

constexpr int kFramebufferWidth = 800;
constexpr int kFramebufferHeight = 600;

constexpr float orbit_radius = 2.0f;
constexpr glm::vec4 light_orbit_radius = {2.0f, 4.0f, -4.0f, 1.0f};

struct
{
    glm::vec3 color = {0.145f, 0.265f, 0.417f};
    float intensity = 8.7f;
} light_control;

struct FullscreenQuad
{
    GLuint vao;
    GLuint vbo;

    void Initialize()
    {
        // clang-format off
        float quad_vertices[] = {
            // pos (x, y) texcoord (u, v)
            -1.0f,  1.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f,
            1.0f, -1.0f, 1.0f, 0.0f,

            -1.0f,  1.0f, 0.0f, 1.0f,
            1.0f, -1.0f, 1.0f, 0.0f,
            1.0f,  1.0f, 1.0f, 1.0f,
        };
        // clang-format on

        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), &quad_vertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(sizeof(float) * 2));

        glBindVertexArray(0);
    }
} fullscreen_quad;

struct Framebuffer
{
    GLuint fbo;
    GLuint position;
    GLuint normal;
    GLuint albedo;
    GLuint material;
    GLuint depth;

    void Initialize()
    {
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        // position attachment
        glGenTextures(1, &position);
        glBindTexture(GL_TEXTURE_2D, position);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, kFramebufferWidth, kFramebufferHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, position, 0);

        // normal attachment
        glGenTextures(1, &normal);
        glBindTexture(GL_TEXTURE_2D, normal);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, kFramebufferWidth, kFramebufferHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, normal, 0);

        // albedo attachment
        glGenTextures(1, &albedo);
        glBindTexture(GL_TEXTURE_2D, albedo);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, kFramebufferWidth, kFramebufferHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, albedo, 0);

        // material attachment
        glGenTextures(1, &material);
        glBindTexture(GL_TEXTURE_2D, material);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, kFramebufferWidth, kFramebufferHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, material, 0);

        // depth attachment
        glGenTextures(1, &depth);
        glBindTexture(GL_TEXTURE_2D, depth);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, kFramebufferWidth, kFramebufferHeight, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depth, 0);

        GLenum array[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
        glDrawBuffers(4, array);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            printf("G-buffer framebuffer not complete\n");
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
} framebuffer;

struct LighVolumebuffer
{
    GLuint fbo;
    GLuint color;
    GLuint depth;

    void Initialize()
    {
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        glGenTextures(1, &color);
        glBindTexture(GL_TEXTURE_2D, color);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, kFramebufferWidth, kFramebufferHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);

        glGenTextures(1, &depth);
        glBindTexture(GL_TEXTURE_2D, depth);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, kFramebufferWidth, kFramebufferHeight, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depth, 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            printf("Light volume framebuffer not complete\n");
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
} lightvolumebuffer;

struct Material
{
    float ambient = 0.167f;
    float diffuse = 0.292f;
    float specular = 0.187f;
    float shininess = 0.099f;
} material;

struct
{
    int width = 1;
    float light_radius = 2.5f;
    bool draw_light_volume = false;
} debug;

Scene::Scene()
{
    suzanne = std::make_unique<ew::Model>("assets/models/suzanne.obj");
    geometry = std::make_unique<ew::Shader>("assets/shaders/deferred/geometry.vs", "assets/shaders/deferred/geometry.fs");
    blinnphong = std::make_unique<ew::Shader>("assets/shaders/deferred/blinnphong.vs", "assets/shaders/deferred/blinnphong.fs");
    noprocess = std::make_unique<ew::Shader>("assets/shaders/fullscreen.vs", "assets/shaders/fullscreen.fs");
    lightsphere = std::make_unique<ew::Shader>("assets/shaders/deferred/light.vs", "assets/shaders/deferred/light.fs");

    sphere.load(ew::createSphere(1.0f, 8));

    ambient = {
        .intensity = 1.0f,
        .color = {0.5f, 0.5f, 0.5f},
    };

    framebuffer.Initialize();
    lightvolumebuffer.Initialize();
    fullscreen_quad.Initialize();

    InitializeInstanceData();
}

Scene::~Scene()
{
}

void Scene::InitializeInstanceData(void)
{
    auto width = debug.width;
    auto size = (width - (-width) + 1) * (width - (-width) + 1);
    model_instances.resize(size);
    light_instances.resize(size);

    auto i = 0;
    for (auto x = -debug.width; x <= debug.width; x++)
    {
        for (auto y = -debug.width; y <= debug.width; y++, i++)
        {
            const auto position = glm::vec3(x * 3.0f, 0, y * 3.0f);
            const auto orbit = batteries::random_point_on_sphere();

            light_instances[i] = {
                .color = batteries::random_color(),
                .position = glm::vec4(position, 1.0f) + orbit * orbit_radius,
            };

            model_instances[i] = batteries::random_model_matrix(position);
        }
    }
}

void Scene::Update(float dt)
{
    batteries::Scene::Update(dt);
}

void Scene::Render(void)
{
    const auto view_proj = camera.Projection() * camera.View();

   
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.fbo);
    {
        glViewport(0, 0, kFramebufferWidth, kFramebufferHeight);
        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        geometry->use();
        geometry->setMat4("view_proj", view_proj);
        geometry->setFloat("material.ambient", material.ambient);
        geometry->setFloat("material.diffuse", material.diffuse);
        geometry->setFloat("material.specular", material.specular);
        geometry->setFloat("material.shininess", material.shininess);

        auto i = 0;
        for (auto x = -debug.width; x <= debug.width; x++)
        {
            for (auto y = -debug.width; y <= debug.width; y++, i++)
            {
                geometry->setMat4("model", model_instances[i]);
                suzanne->draw();
            }
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    
    glBindFramebuffer(GL_FRAMEBUFFER, lightvolumebuffer.fbo);
    {
        glViewport(0, 0, kFramebufferWidth, kFramebufferHeight);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
        glDisable(GL_CULL_FACE);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Bind G-buffer textures
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, framebuffer.position);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, framebuffer.normal);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, framebuffer.albedo);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, framebuffer.material);

        blinnphong->use();
        blinnphong->setInt("g_position", 0);
        blinnphong->setInt("g_normal", 1);
        blinnphong->setInt("g_albedo", 2);
        blinnphong->setInt("g_material", 3);
        blinnphong->setVec3("camera_position", camera.position);

        const int MAX_LIGHTS = 300;
        int num_lights = std::min((int)light_instances.size(), MAX_LIGHTS);
        blinnphong->setInt("num_lights", num_lights);

        for (int i = 0; i < num_lights; ++i) {
            std::string base = "lights[" + std::to_string(i) + "]";
            blinnphong->setVec3(base + ".position", glm::vec3(light_instances[i].position));
            blinnphong->setVec3(base + ".color", light_control.color * light_control.intensity);
        }

        glBindVertexArray(fullscreen_quad.vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    {
        noprocess->use();
        noprocess->setInt("screen", 0);

        glDisable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindVertexArray(fullscreen_quad.vao);
        glActiveTexture(GL_TEXTURE0);

        glBindTexture(GL_TEXTURE_2D, lightvolumebuffer.color);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        
        glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer.fbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBlitFramebuffer(0, 0, kFramebufferWidth, kFramebufferHeight,
                          0, 0, kFramebufferWidth, kFramebufferHeight,
                          GL_DEPTH_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    {
        lightsphere->use();
        lightsphere->setMat4("view_proj", view_proj);

        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
        glDisable(GL_CULL_FACE);

        auto i = 0;
        for (auto x = -debug.width; x <= debug.width; x++)
        {
            for (auto y = -debug.width; y <= debug.width; y++, i++)
            {
                auto sphere_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(light_instances[i].position));
                sphere_matrix = glm::scale(sphere_matrix, glm::vec3(0.1f));
                lightsphere->setMat4("model", sphere_matrix);
                lightsphere->setVec3("light.position", glm::vec3(light_instances[i].position));
                lightsphere->setVec3("light.color", light_instances[i].color);
                sphere.draw();
            }
        }
    }
}

void Scene::Debug(void)
{
    cameracontroller.Debug();

    ImGui::Begin("Controlls", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::Checkbox("Paused", &time.paused);
    ImGui::SliderFloat("Time Factor", &time.factor, 0.0f, 10.0f);

    if (ImGui::SliderInt("Width", &debug.width, 1, 100))
    {
        InitializeInstanceData();
    }

    if (ImGui::CollapsingHeader("Lights"))
    {
        ImGui::Checkbox("Draw Volumes", &debug.draw_light_volume);
        ImGui::SliderFloat("Light Radisu", &debug.light_radius, 1.0f, 100.0f);

        ImGui::ColorEdit3("Light Color", &light_control.color.x, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);
        ImGui::SliderFloat("Intensity", &light_control.intensity, 0.0f, 10.0f);
    }

    if (ImGui::CollapsingHeader("Material"))
    {
        ImGui::SliderFloat("Ambient", &material.ambient, 0.0f, 1.0f);
        ImGui::SliderFloat("Diffuse", &material.diffuse, 0.0f, 1.0f);
        ImGui::SliderFloat("Specular", &material.specular, 0.0f, 1.0f);
        ImGui::SliderFloat("Shininess", &material.shininess, 0.0f, 1.0f);
    }

    if (ImGui::CollapsingHeader("Geometry Buffer"))
    {
        ImVec2 uv_min(0.0f, 1.0f);
        ImVec2 uv_max(1.0f, 0.0f);

        ImGui::Text("Lighting:");
        ImGui::Image((ImTextureID)(intptr_t)lightvolumebuffer.color, ImVec2(200, 150), uv_min, uv_max);

        ImGui::Text("Albedo:");
        ImGui::Image((ImTextureID)(intptr_t)framebuffer.albedo, ImVec2(200, 150), uv_min, uv_max);

        ImGui::Text("Material:");
        ImGui::Image((ImTextureID)(intptr_t)framebuffer.material, ImVec2(200, 150), uv_min, uv_max);

        ImGui::Text("Position:");
        ImGui::Image((ImTextureID)(intptr_t)framebuffer.position, ImVec2(200, 150), uv_min, uv_max);

        ImGui::Text("Normal:");
        ImGui::Image((ImTextureID)(intptr_t)framebuffer.normal, ImVec2(200, 150), uv_min, uv_max);

        ImGui::Text("Depth:");
        ImGui::Image((ImTextureID)(intptr_t)framebuffer.depth, ImVec2(200, 150), uv_min, uv_max);
    }
    ImGui::End();
}