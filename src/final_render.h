#pragma once

namespace raytracing
{
  class rt;
  struct render_options
  {
    uint32_t width = 1920, height = 1080;
    uint32_t samples = 128;
    uint32_t bounces = 16;

    std::string filename = "render.png";

    bool sequence = true;

    std::string video_filename_base = "tmp/video";
    uint32_t framerate = 30;
    float duration = 10;
  };

  class final_render
  {
  public:

    final_render() = default;
    final_render(const final_render&) = delete;
    explicit final_render(rt* rt);
    ~final_render() = default;

    render_options mRenderOptions;

    void render_to_image();
    void render_to_video();

  private:

    rt* mRt = nullptr;
  };
}