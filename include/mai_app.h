#include "mai_renderer.h"
#include "mai_vk_backend/vk_buffer.h"
#include "mai_vk_backend/vk_shader.h"

#pragma once

struct MaiApp {
  MAI::VKShader *vert = nullptr;
  MAI::VKShader *frag = nullptr;
  MAI::VKPipeline *pipeline = nullptr;
  MAI::VKbuffer *vertexBuffer = nullptr;
  MAI::VKbuffer *indexBuffer = nullptr;

  void run();
  MAI::MAIRenderer *renderer = nullptr;
  MaiApp() = default;
  ~MaiApp();

private:
  void init();
  void mainLoop();
};
