#include "rt.h"

void init_scene()
{
  {
    SphereObject object {};
    object.radius = 0.8f;
    object.center = glm::vec4(0, 0, 0, 0);
    raytracing::rt::get()->add_sphere(object);
  }

  {
    SphereObject object {};
    object.radius = 4.f;
    object.center = glm::vec4(0, 3, 10.0, 0.0);
    raytracing::rt::get()->add_sphere(object);
  }

  {
    SphereObject object {};
    object.radius = 0.5f;
    object.center = glm::vec4(1.0, 0, 0, 0.0);
    raytracing::rt::get()->add_sphere(object);
  }

  {
    SphereObject object {};
    object.radius = 0.8f;
    object.center = glm::vec4(0, 0.5, 0.5, 0.0);
    raytracing::rt::get()->add_sphere(object);
  }

  {
    SphereObject object {};
    object.radius = 1.f;
    object.center = glm::vec4(2, -0.3, -0.5, 0.0);
    raytracing::rt::get()->add_sphere(object);
  }

  {
    SphereObject object {};
    object.radius = 0.8f;
    object.center = glm::vec4(0, 0, 0, 0.0);
    raytracing::rt::get()->add_sphere(object);
  }

  {
    PlaneObject object {};
    object.distance = 0.1f;
    object.normal = glm::vec4(0, .5, 0, 0.0);
    raytracing::rt::get()->add_plane(object);
  }
}

int main()
{
  raytracing::init_options options;
  raytracing::rt app;

  options.width = 1200;
  options.height = 900;
  options.title = "Ray tracing app";

  app.init(options);

  init_scene();

  app.run();
}
