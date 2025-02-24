#include "rt.h"

int main()
{
  raytracing::init_options options;
  raytracing::rt app;

  options.title = "Ray tracing app";
  options.scene_filename = "scenes/monkey.yaml";

  app.init(options);
  app.run();
}
