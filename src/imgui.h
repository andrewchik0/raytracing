#pragma once

namespace raytracing
{
  class imgui
  {
  public:
    bool init();
    void update();
  private:

    void scene_window();
    void add_item_window();
    void materials_window();

    bool mMaterialsOpened = false;
    bool mSceneOpened = false;
    bool mAddItemOpened = false;
  };
}
