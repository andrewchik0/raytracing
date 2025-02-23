#include "rt.h"

int main()
{
  raytracing::init_options options;
  raytracing::rt app;

  options.width = 1200;
  options.height = 900;
  options.title = "Ray tracing app";
  options.scene_filename = "scenes/default.yaml";

  app.init(options);
  app.run();
}
