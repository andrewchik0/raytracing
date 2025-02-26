#pragma once

#include "pch.h"

#include <imgui.h>

namespace raytracing
{
  class gui
  {
  public:
    uint32_t mGuiWidth = 450;

    bool init();
    void update();
  private:

    ImFont* mFont = nullptr;

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

    void setup_style(bool bStyleDark, float alpha_);
  };
}
