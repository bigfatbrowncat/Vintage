#version 120

uniform sampler2D RTScene; // the texture with the scene you want to blur
uniform float phase;

const float texSizeX = 1024.0;
const float texSizeY = 1024.0;

const float blurSizeX = 1.0/texSizeX;
const float blurSizeY = 1.0/texSizeX;
 
void main(void)
{
   vec4 vTexCoord = gl_TexCoord[0];
   vec4 sum = vec4(0.0, 1.0, 0.0, 1.0);
 
   float q = (0.7 - 0.3 * sin(vTexCoord.y * 5 - phase * 5));	// blurring waves
 
   vec4 n = vec4(0.0);
   vec4 p = vec4(0.0); 
   vec4 n0 = vec4(0.0);
 
   n0 = texture2D(RTScene, vec2(vTexCoord.x, vTexCoord.y)) * 1;
   sum = n0;
   p += 1;
  
   n = texture2D(RTScene, vec2(vTexCoord.x - 1.0 * blurSizeX, vTexCoord.y)) * 0.125 * q;
   if (n.x + n.y + n.z < n0.x + n0.y + n0.z) { sum += n; p += 0.125 * q; } 
   n = texture2D(RTScene, vec2(vTexCoord.x + 1.0 * blurSizeX, vTexCoord.y)) * 0.125 * q;
   if (n.x + n.y + n.z < n0.x + n0.y + n0.z) { sum += n; p += 0.125 * q; } 
   n = texture2D(RTScene, vec2(vTexCoord.x, vTexCoord.y - 1.0 * blurSizeY)) * 0.125 * q;
   if (n.x + n.y + n.z < n0.x + n0.y + n0.z) { sum += n; p += 0.125 * q; } 
   n = texture2D(RTScene, vec2(vTexCoord.x, vTexCoord.y + 1.0 * blurSizeY)) * 0.125 * q;
   if (n.x + n.y + n.z < n0.x + n0.y + n0.z) { sum += n; p += 0.125 * q; } 
   
   n = texture2D(RTScene, vec2(vTexCoord.x + 1.0 * blurSizeX, vTexCoord.y + 1.0 * blurSizeY)) * 0.0625 * q;
   if (n.x + n.y + n.z < n0.x + n0.y + n0.z) { sum += n; p += 0.0625 * q; } 
   n = texture2D(RTScene, vec2(vTexCoord.x - 1.0 * blurSizeX, vTexCoord.y + 1.0 * blurSizeY)) * 0.0625 * q;
   if (n.x + n.y + n.z < n0.x + n0.y + n0.z) { sum += n; p += 0.0625 * q; } 
   n = texture2D(RTScene, vec2(vTexCoord.x + 1.0 * blurSizeX, vTexCoord.y - 1.0 * blurSizeY)) * 0.0625 * q;
   if (n.x + n.y + n.z < n0.x + n0.y + n0.z) { sum += n; p += 0.0625 * q; } 
   n = texture2D(RTScene, vec2(vTexCoord.x - 1.0 * blurSizeX, vTexCoord.y - 1.0 * blurSizeY)) * 0.0625 * q;
   if (n.x + n.y + n.z < n0.x + n0.y + n0.z) { sum += n; p += 0.0625 * q; } 

   sum /= p;
 
   sum *= sin((vTexCoord.y * 400 - phase) * 2 * 3.14 ) * 0.05 + 0.95;
 
   gl_FragColor = sum;
}
