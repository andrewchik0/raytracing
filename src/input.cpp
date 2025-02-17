#include "input.h"

#include <imgui.h>

#include "rt.h"

namespace raytracing
{
  bool input::key(sf::Keyboard::Key key)
  {
    return rt::get()->mInput.mKeyPressed[static_cast<int>(key)];
  }

  void input::handle(const std::optional<sf::Event>& event)
  {
    ImGuiIO& io = ImGui::GetIO();
    if (event->is<sf::Event::KeyPressed>())
    {
      mKeyPressed[static_cast<int>(event->getIf<sf::Event::KeyPressed>()->code)] = true;
    }
    if (event->is<sf::Event::KeyReleased>())
    {
      mKeyPressed[static_cast<int>(event->getIf<sf::Event::KeyReleased>()->code)] = false;
    }
    if (event->is<sf::Event::MouseButtonPressed>() && !io.WantCaptureMouse)
    {
      mMousePressed[static_cast<int>(event->getIf<sf::Event::MouseButtonPressed>()->button)] = true;
    }
    if (event->is<sf::Event::MouseButtonReleased>() && !io.WantCaptureMouse)
    {
      mMousePressed[static_cast<int>(event->getIf<sf::Event::MouseButtonReleased>()->button)] = false;
    }
    if (event->is<sf::Event::MouseMoved>() && !io.WantCaptureMouse)
    {
      mMouseX = event->getIf<sf::Event::MouseMoved>()->position.x;
      mMouseY = event->getIf<sf::Event::MouseMoved>()->position.y;
      mMouseDeltaX = mMouseX - mMouseXOld;
      mMouseDeltaY = mMouseY - mMouseYOld;
      mMouseXOld = mMouseX;
      mMouseYOld = mMouseY;
    }
  }

  void input::clear()
  {
    mMouseDeltaX = 0;
    mMouseDeltaY = 0;
  }
}
