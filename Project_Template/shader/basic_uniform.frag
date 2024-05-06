#version 460

// Variables from Vertex Shader
in vec3 Position;
in vec3 Normal;
//in vec2 TexCoord;
//
//// Texture mapping and fragment colour
//layout (binding = 0) uniform sampler2D Tex1;    
layout (location = 0) out vec4 FragColor;
layout (binding = 0) uniform sampler2D RenderTex; 

uniform float EdgeThreshold;
uniform int Pass;
const vec3 lum = vec3(0.3, 0.3, 0.722);

// Light structure info 
uniform struct Lightinfo{
    vec4 Position; 
    vec3 La;
    vec3 L;
}Light;

// Material structure with material properties
uniform struct MaterialInfo{
    vec3 Kd;
    vec3 Ka;
    vec3 Ks;
    float Shininess;
}Material;

// Fog structure with fog parameters
uniform struct FogInfo{
    float MaxDist;
    float MinDist;
    vec3 Color;
}Fog;

// Blinn-Phong model
vec3 blinnPhong(vec3 position, vec3 n)
{
    vec3 diffuse = vec3(0), spec = vec3(0);
    //vec3 texColor = texture(Tex1, TexCoord).rgb;
    vec3 ambient = Light.La * Material.Ka;

    vec3 s = normalize(Light.Position.xyz - position); 
    float sDotN = max(dot(s, n), 0.0);
    diffuse = Material.Kd * sDotN;
    if (sDotN > 0.0)
    {
        vec3 v = normalize (-position.xyz);
        vec3 h = normalize(v + s); // Correct normalization
        spec = Material.Ks * pow(max(dot(h, n), 0.0), Material.Shininess);
    }    
    return ambient + (diffuse + spec) * Light.L;

}

float luminance(vec3 color)
{
    return dot(lum, color);
}


vec4 pass1()
{    
      return vec4(blinnPhong(Position,normalize(Normal)),1.0);

}

       vec4 pass2()
{
     ivec2 pix = ivec2(gl_FragCoord.xy);
     float s00 = luminance (texelFetchOffset(RenderTex, pix, 0, ivec2(-1,1)).rgb);
     float s10 = luminance (texelFetchOffset(RenderTex, pix, 0, ivec2(-1,0)).rgb);
     float s20 = luminance (texelFetchOffset(RenderTex, pix, 0, ivec2(-1,-1)).rgb);
     float s01 = luminance (texelFetchOffset(RenderTex, pix, 0, ivec2(0,1)).rgb);
     float s21 = luminance (texelFetchOffset(RenderTex, pix, 0, ivec2(0,-1)).rgb);
     float s02 = luminance (texelFetchOffset(RenderTex, pix, 0, ivec2(1,1)).rgb);
     float s12 = luminance (texelFetchOffset(RenderTex, pix, 0, ivec2(1,0)).rgb);
     float s22 = luminance (texelFetchOffset(RenderTex, pix, 0, ivec2(1,-1)).rgb);

     float sx = s00+2*s10+s20-(s02+2*s12+s22);
     float sy = s00+2*s01+s02-(s20+2*s21+s22);
     float g  = sx*sx+sy*sy;
     if (g >EdgeThreshold)
        return vec4(0.1);
     else
        return texelFetch(RenderTex,pix, 0); // vec(0,0,0,1);
}

// Main function for calculating fragment colour
void main() 
{
    if (Pass == 1) FragColor = pass1();
  else if (Pass == 2) FragColor += pass2();

}