#include "types.glsl"

float mandelbulbSDF(vec3 p)
{
  const int ITERATIONS = 8;
  const float POWER = 8.0;

  vec3 z = p;
  float dr = 1.0;
  float r = length(z);

  for (int i = 0; i < ITERATIONS; i++)
  {
    if (r > 2.0) break;

    // Convert to spherical coordinates
    float theta = acos(z.z / r);
    float phi = atan(z.y, z.x);

    // Scale and rotate
    float zr = pow(r, POWER - 1.0);
    dr = POWER * zr * dr + 1.0;
    zr *= r;

    theta *= POWER;
    phi *= POWER;

    // Convert back to Cartesian coordinates
    z = zr * vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
    z += p;

    r = length(z);
  }

  return 0.5 * log(r) * r / dr;
}

HitData mandelbulbIntersect(Ray ray)
{
  HitData result;
  result.distance = FAR_PLANE;
  vec3 currentOrigin = ray.origin;
  float totalDistance = 0.0;
  const int maxSteps = 256;
  const float epsilon = 1e-6;
  const float maxDistance = 100.0;

  vec3 bulbPosition = vec3(3, 2, 0);

  for (int i = 0; i < maxSteps; i++)
  {
    float dist = mandelbulbSDF(currentOrigin - u_mandelbulb.position);

    // If close enough, hit detected
    if (dist < epsilon && totalDistance < result.distance)
    {
      result.distance = totalDistance;
      result.normal = normalize(currentOrigin);
      result.materialIndex = 0;
      break;
    }

    // If too far, terminate
    if (totalDistance > maxDistance)
      break;

    // Move forward
    currentOrigin += dist * ray.direction;
    totalDistance += dist;
  }
  return result;
}