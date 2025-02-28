#pragma once

#include "pch.h"

#include <imgui.h>

namespace raytracing
{
  class gui
  {
  public:
    uint32_t mGuiWidth = 450;

    bool mIsViewPortInFocus = false;
    glm::ivec2 mViewportPosition;
    glm::ivec2 mViewportSize;

    bool init();
    void update();
  private:

    ImFont* mFont = nullptr, *mFAFont = nullptr;

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
