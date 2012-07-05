#version 120

uniform sampler2D texture; // the texture with the scene you want to blur
uniform float phase;

const float texSizeX = 1024.0;
const float texSizeY = 1024.0;

const float blurSizeX = 1.0/texSizeX;
const float blurSizeY = 1.0/texSizeX;
 
void main(void)
{
   vec4 vTexCoord = gl_TexCoord[0];
   vec4 sum = vec4(0.0, 1.0, 0.0, 1.0);
 
   float q = (0.6 - 0.4 * sin(vTexCoord.y * 5 - phase * 5));	// blurring waves
 
   vec4 n = vec4(0.0);
   vec4 p = vec4(0.0); 
   vec4 n0 = vec4(0.0);

   float dx = 0;
   float kx = 0;
   float ky = 0;
   
   if (phase > 2.0)
   {
   		float p2 = phase - 2.0;
   		
   		kx = 1 + exp(-p2 * p2 * 100) + (1 / (p2 * 123)) * sin(vTexCoord.y * 20 - p2 * 30);
   		ky = 1 + exp(-p2 * p2 * 100) + (1 / (p2 * 300)) * sin(vTexCoord.y * 60 - p2 * 80);
   		dx = (phase + (1 / (p2 * 500))) * blurSizeX;
	    
	    if (kx < 1.005)
	    {
	       kx = 1;
	       ky = 1;
	   	   dx = 0;
	   	}
   }	
 
   n0 = texture2D(texture, vec2(kx * vTexCoord.x + dx, ky * vTexCoord.y)) * 1;
   sum = n0;
   p += 1;
  
  
   n = texture2D(texture, vec2(vTexCoord.x + dx - 1.0 * blurSizeX, vTexCoord.y)) * 0.125 * q;
   if (n.x + n.y + n.z < n0.x + n0.y + n0.z) { sum += n; p += 0.125 * q; } 
   n = texture2D(texture, vec2(vTexCoord.x + dx + 1.0 * blurSizeX, vTexCoord.y)) * 0.125 * q;
   if (n.x + n.y + n.z < n0.x + n0.y + n0.z) { sum += n; p += 0.125 * q; } 
   n = texture2D(texture, vec2(vTexCoord.x + dx, vTexCoord.y - 1.0 * blurSizeY)) * 0.125 * q;
   if (n.x + n.y + n.z < n0.x + n0.y + n0.z) { sum += n; p += 0.125 * q; } 
   n = texture2D(texture, vec2(vTexCoord.x + dx, vTexCoord.y + 1.0 * blurSizeY)) * 0.125 * q;
   if (n.x + n.y + n.z < n0.x + n0.y + n0.z) { sum += n; p += 0.125 * q; } 
   
   n = texture2D(texture, vec2(vTexCoord.x + dx + 1.0 * blurSizeX, vTexCoord.y + 1.0 * blurSizeY)) * 0.0625 * q;
   if (n.x + n.y + n.z < n0.x + n0.y + n0.z) { sum += n; p += 0.0625 * q; } 
   n = texture2D(texture, vec2(vTexCoord.x + dx - 1.0 * blurSizeX, vTexCoord.y + 1.0 * blurSizeY)) * 0.0625 * q;
   if (n.x + n.y + n.z < n0.x + n0.y + n0.z) { sum += n; p += 0.0625 * q; } 
   n = texture2D(texture, vec2(vTexCoord.x + dx + 1.0 * blurSizeX, vTexCoord.y - 1.0 * blurSizeY)) * 0.0625 * q;
   if (n.x + n.y + n.z < n0.x + n0.y + n0.z) { sum += n; p += 0.0625 * q; } 
   n = texture2D(texture, vec2(vTexCoord.x + dx - 1.0 * blurSizeX, vTexCoord.y - 1.0 * blurSizeY)) * 0.0625 * q;
   if (n.x + n.y + n.z < n0.x + n0.y + n0.z) { sum += n; p += 0.0625 * q; } 

   sum /= p;
 
   sum *= sin((vTexCoord.y * 400 - phase) * 2 * 3.14 ) * 0.05 + 0.95;
 
   gl_FragColor = sum;
}
