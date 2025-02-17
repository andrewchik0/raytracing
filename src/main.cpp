#include "rt.h"

void init_scene()
{
  {
    SphereObject object;
    object.albedo = glm::vec4(0.0, 1.0, 1.0, 1.0);
    object.radius = 0.8f;
    object.center = glm::vec3(0, 0, 0);
    raytracing::rt::get()->add_sphere(std::move(object));
  }

  {
    PlaneObject object;
    object.albedo = glm::vec4(1.0, 1.0, 1.0, 1.0);
    object.distance = 0.8f;
    object.normal = glm::vec3(0, 1.0, 0);
    raytracing::rt::get()->add_plane(std::move(object));

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
