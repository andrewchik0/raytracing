const float PI = 3.141592653589;

float atan2(float y, float x)
{
  bool s = bool(abs(x) > abs(y));
  return mix(PI / 2.0 - atan(x, y), atan(y, x), s);
}

vec3 gaussian_blur(sampler2D tex, vec2 windowSize, vec2 texCoords, float radius)
{
  float radiusSquared = radius * radius;
  float pixelSizeX = 1.0 / windowSize.x;
  float pixelSizeY = 1.0 / windowSize.y;
  vec2 center = vec2(texCoords.x, 1.0 - texCoords.y);
  float baseWeight = 1 / (2.0 * PI * radiusSquared);
  vec3 resultColor = vec3(0.0);

  for (float x = center.x - pixelSizeX * radius; x <= center.x + pixelSizeX * radius; x += pixelSizeX)
  {
    for (float y = center.y - pixelSizeY * radius; y <= center.y + pixelSizeY * radius; y += pixelSizeY)
    {
      float weight = baseWeight * exp(-(x * x + y * y) / (2.0 * radiusSquared));
      resultColor += texture(tex, vec2(x, y)).rgb * weight;
    }
  }

  return resultColor;
}

// A single iteration of Bob Jenkins' One-At-A-Time hashing algorithm.
uint hash( uint x ) {
  x += ( x << 10u );
  x ^= ( x >>  6u );
  x += ( x <<  3u );
  x ^= ( x >> 11u );
  x += ( x << 15u );
  return x;
}

// Compound versions of the hashing algorithm I whipped together.
uint hash( uvec2 v ) { return hash( v.x ^ hash(v.y)                         ); }
uint hash( uvec3 v ) { return hash( v.x ^ hash(v.y) ^ hash(v.z)             ); }
uint hash( uvec4 v ) { return hash( v.x ^ hash(v.y) ^ hash(v.z) ^ hash(v.w) ); }



// Construct a float with half-open range [0:1] using low 23 bits.
// All zeroes yields 0.0, all ones yields the next smallest representable value below 1.0.
float floatConstruct( uint m ) {
  const uint ieeeMantissa = 0x007FFFFFu; // binary32 mantissa bitmask
  const uint ieeeOne      = 0x3F800000u; // 1.0 in IEEE binary32

  m &= ieeeMantissa;                     // Keep only mantissa bits (fractional part)
  m |= ieeeOne;                          // Add fractional part to 1.0

  float  f = uintBitsToFloat( m );       // Range [1:2]
  return f - 1.0;                        // Range [0:1]
}



// Pseudo-random value in half-open range [0:1].
float random( float x ) { return floatConstruct(hash(floatBitsToUint(x))); }
float random( vec2  v ) { return floatConstruct(hash(floatBitsToUint(v))); }
float random( vec3  v ) { return floatConstruct(hash(floatBitsToUint(v))); }
float random( vec4  v ) { return floatConstruct(hash(floatBitsToUint(v))); }

vec3 randomOnSphere(vec3 seed)
{
  vec3 rand = vec3(random(vec2(u_time, seed.x)), random(vec2(u_time, seed.y)), random(vec2(u_time, seed.z)));
  float theta = rand.x * 2.0 * 3.14159265;
  float v = rand.y;
  float phi = acos(2.0 * v - 1.0);
  float r = pow(rand.z, 1.0 / 3.0);
  float x = r * sin(phi) * cos(theta);
  float y = r * sin(phi) * sin(theta);
  float z = r * cos(phi);
  return vec3(x, y, z);
}

vec3 randomHemisphereDirection(vec3 normal, vec3 rand)
{
  float rand1 = random(rand.xy + u_time);
  float rand2 = random(rand.yz + u_time);
  float theta = acos(rand1);
  float phi = 2.0 * 3.14159265359 * rand2;

  float x = cos(phi) * sin(theta);
  float y = sin(phi) * sin(theta);
  float z = cos(theta);

  vec3 randomDir = vec3(x, y, z);

  vec3 up = abs(normal.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
  vec3 tangent = normalize(cross(up, normal));
  vec3 bitangent = cross(normal, tangent);

  return normalize(tangent * randomDir.x + bitangent * randomDir.y + normal * randomDir.z);
}

mat4 skewMatrix(float sxy, float sxz, float syx, float syz, float szx, float szy)
{
  return mat4(
    1.0, sxy, sxz, 0.0,
    syx, 1.0, syz, 0.0,
    szx, szy, 1.0, 0.0,
    0.0, 0.0, 0.0, 1.0
  );
}

mat4 windSkewMatrix(float sx, float sz, float time)
{
  float swayX = sx * sin(time);
  float swayZ = sz * cos(time);
  return skewMatrix(swayX, swayZ, 0.0, 0.0, 0.0, 0.0);
}