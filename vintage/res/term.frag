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

   float dx = 0, dy = 0;
   float kx = 0;
   float ky = 0;

   // Applying turn-on formula   
   if (phase > 0.0)
   {
   		float phx = 13 * (phase - 0.0);
   		float phy = 13 * (phase - 0.15);
   		if (phy < 0) phy = 0;

   		kx = 1 / (1 - exp(-2 * phx) + (phx + 0.2) * exp(-phx) * sin(1.0 / (phx*phx*phx + 0.1) + 0.1) * sin(5 * phx * phx - 0.1));
   		ky = 1 / (1 - exp(-2 * phy) + (phy + 0.2) * exp(-phy) * sin(1.0 / (phy*phy*phy + 0.1) + 0.1) * sin(5 * phy * phy - 0.1));
   		
   		dx = 0.5 - 0.5 * kx;
   	    dy = 0.5 - 0.5 * ky;
   }	

   if (kx * vTexCoord.x + dx < 1 && kx * vTexCoord.x + dx > 0 && 
       ky * vTexCoord.y + dy < 1 && ky * vTexCoord.y + dy > 0)
   {
   
   
	   n0 = texture2D(texture, vec2(kx * vTexCoord.x + dx, ky * vTexCoord.y + dy)) * 1;
	   sum = n0;
	   p += 1;
	  
	  
	   n = texture2D(texture, vec2(kx * vTexCoord.x + dx - 1.0 * blurSizeX, ky * vTexCoord.y + dy)) * 0.125 * q;
	   if (n.x + n.y + n.z < n0.x + n0.y + n0.z) { sum += n; p += 0.125 * q; } 
	   n = texture2D(texture, vec2(kx * vTexCoord.x + dx + 1.0 * blurSizeX, ky * vTexCoord.y + dy)) * 0.125 * q;
	   if (n.x + n.y + n.z < n0.x + n0.y + n0.z) { sum += n; p += 0.125 * q; } 
	   n = texture2D(texture, vec2(kx * vTexCoord.x + dx, vTexCoord.y + dy - 1.0 * blurSizeY)) * 0.125 * q;
	   if (n.x + n.y + n.z < n0.x + n0.y + n0.z) { sum += n; p += 0.125 * q; } 
	   n = texture2D(texture, vec2(kx * vTexCoord.x + dx, vTexCoord.y + dy + 1.0 * blurSizeY)) * 0.125 * q;
	   if (n.x + n.y + n.z < n0.x + n0.y + n0.z) { sum += n; p += 0.125 * q; } 
	   
	   n = texture2D(texture, vec2(kx * vTexCoord.x + dx + 1.0 * blurSizeX, ky * vTexCoord.y + dy + 1.0 * blurSizeY)) * 0.0625 * q;
	   if (n.x + n.y + n.z < n0.x + n0.y + n0.z) { sum += n; p += 0.0625 * q; } 
	   n = texture2D(texture, vec2(kx * vTexCoord.x + dx - 1.0 * blurSizeX, ky * vTexCoord.y + dy + 1.0 * blurSizeY)) * 0.0625 * q;
	   if (n.x + n.y + n.z < n0.x + n0.y + n0.z) { sum += n; p += 0.0625 * q; } 
	   n = texture2D(texture, vec2(kx * vTexCoord.x + dx + 1.0 * blurSizeX, ky * vTexCoord.y + dy - 1.0 * blurSizeY)) * 0.0625 * q;
	   if (n.x + n.y + n.z < n0.x + n0.y + n0.z) { sum += n; p += 0.0625 * q; } 
	   n = texture2D(texture, vec2(kx * vTexCoord.x + dx - 1.0 * blurSizeX, ky * vTexCoord.y + dy - 1.0 * blurSizeY)) * 0.0625 * q;
	   if (n.x + n.y + n.z < n0.x + n0.y + n0.z) { sum += n; p += 0.0625 * q; } 
	
	   sum /= p;
	 
	   sum *= sin((vTexCoord.y * 400 - phase) * 2 * 3.14 ) * 0.05 + 0.95;
	
	   gl_FragColor += sum * kx * ky;
	}
	else
	{
	   gl_FragColor = vec4(0.0);
	}
}
