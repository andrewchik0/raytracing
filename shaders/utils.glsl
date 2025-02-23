float random(vec2 st)
{
  return fract(sin(dot(st.xy, vec2(12.9898,78.233))) * 43758.5453123);
}

vec3 rand3(vec3 seed)
{
  return vec3(
    random(vec2(time, seed.x)) - 0.5,
    random(vec2(time, seed.y)) - 0.5,
    random(vec2(time, seed.z)) - 0.5
  );
}

const float PI = 3.141592653589;
float atan2(float y, float x)
{
  bool s = (abs(x) > abs(y));
  return mix(PI / 2.0 - atan(x, y), atan(y, x), s);
}
