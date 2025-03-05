#pragma once

#include "pch.h"

#include <imgui.h>

#include "render/texture.h"

namespace ImGui
{
  inline void Image(const raytracing::texture& texture)
  {
    Image(texture.get_handle(), {float(texture.width()), float(texture.height())});
  }

  inline void Image(const raytracing::texture& texture, const ImVec2 center, const float angle)
  {
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    float cosA = cos(angle);
    float sinA = sin(angle);
    auto halfSize = ImVec2(texture.width() * 0.5f, texture.height() * 0.5f);

    ImVec2 pos[4] =
    {
      ImVec2(center.x - halfSize.x * cosA + halfSize.y * sinA, center.y - halfSize.x * sinA - halfSize.y * cosA),
      ImVec2(center.x + halfSize.x * cosA + halfSize.y * sinA, center.y + halfSize.x * sinA - halfSize.y * cosA),
      ImVec2(center.x + halfSize.x * cosA - halfSize.y * sinA, center.y + halfSize.x * sinA + halfSize.y * cosA),
      ImVec2(center.x - halfSize.x * cosA - halfSize.y * sinA, center.y - halfSize.x * sinA + halfSize.y * cosA),
    };

    ImVec2 uv[4] =
    {
      ImVec2(0.0f, 0.0f), // Top-left
      ImVec2(1.0f, 0.0f), // Top-right
      ImVec2(1.0f, 1.0f), // Bottom-right
      ImVec2(0.0f, 1.0f)  // Bottom-left
    };

    drawList->AddImageQuad(texture.get_handle(), pos[0], pos[1], pos[2], pos[3], uv[0], uv[1], uv[2], uv[3], IM_COL32_WHITE);
  }
}

namespace raytracing
{
  class gui
  {
  public:
    uint32_t mGuiWidth = 450;

    bool mIsViewPortInFocus = false;
    glm::ivec2 mViewportPosition;
    glm::ivec2 mViewportSize;

    gui() = default;
    gui(const gui&) = delete;
    ~gui();

    bool init();
    void update();
    void draw();
  private:

    ImFont* mFont = nullptr, *mFAFont = nullptr;

    texture mLoadingTexture;

    float mOldFontSize = 0.0;

    void general_tab();
    void scene_tab();
    void objects_section();
    void add_item_window();
    void materials_section();
    void textures_section();
    void render_tab();

    void push_font(float scale);
    void pop_font();

    static bool check(bool value);

    bool mAddItemOpened = false;
    bool mShowAbout = false;

    void setup_style();
  };
}
