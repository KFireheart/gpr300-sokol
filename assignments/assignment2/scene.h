#pragma once

#ifdef __APPLE__
#  define GL_SILENCE_DEPRECATION
#endif
#include "batteries/opengl.h"

// batteries
#include "batteries/scene.h"
#include "batteries/lights.h"

// ew
#include "ew/model.h"
#include "ew/shader.h"
#include "ew/texture.h"

class Scene final : public batteries::Scene
{
  public:
    Scene();
    virtual ~Scene();

    void Update(float dt);
    void Render(void);
    void Debug(void);

  private:

    void createFrameBuffer();
    void createDepthBuffer();

    std::unique_ptr<ew::Model> suzanne;
    std::unique_ptr<ew::Shader> toon;
    //std::unique_ptr<ew::Texture> bricks;
    std::unique_ptr<ew::Texture> zatoon;
    std::unique_ptr<ew::Shader> depth;
    std::unique_ptr<ew::Shader> shadow;

    //post processing
    std::unique_ptr<ew::Shader> post_process;

    batteries::light_t light;

    ew::Mesh plane;

    struct {
      glm::vec3 color1;
      glm::vec3 color2;
    } palette;

    GLuint fbo;
    GLuint fbo_texture;
    GLuint fbo_depth_texture;

    //depth buffer
    GLuint shadow_fbo;
    GLuint shadow_depth_texture;
};
