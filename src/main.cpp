#include "rt.h"

int main()
{
  raytracing::rt app;

  raytracing::init_options options =
  {
    .title = "Ray tracing app",
    .scene_filename = "scenes/koenigsegg.yaml",
  };

  app.init(options);
  app.run();
}
