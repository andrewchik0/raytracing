
#include "rt.h"

int main()
{
  raytracing::init_options options;
  raytracing::rt app;

  options.title = "Ray tracing app";

  app.run(options);
}
