#version 450

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragPosWorld;
layout (location = 2) in vec3 fragNormalWorld;
layout (location = 3) in vec2 fragUV;

layout (location = 0) out vec4 outColor;

layout(push_constant) uniform PushConstants {
    mat4 modelMatrix; 
    mat4 normalMatrix;
} push;

struct PointLight {
    vec4 position;
    vec4 color;
};

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projection;
    mat4 view;
    mat4 invView;
    vec4 ambientLightColor;
    PointLight pointLights[10]; //must update with m4_frame_info.hpp MAX_LIGHTS
    int numLights;
} ubo;

layout(set = 0, binding = 1) uniform sampler2D texSampler;

const int SPEC_POWER=512;

void main() {

  vec3 diffuseLight=ubo.ambientLightColor.rgb*ubo.ambientLightColor.a;
  vec3 specularLight=vec3(0.0);
  vec3 surfaceNormal=normalize(fragNormalWorld);

  vec3 cameraPosWorlds=ubo.invView[3].xyz;
  vec3 viewDirection=normalize(cameraPosWorlds-fragPosWorld);

  for (int i=0;i<ubo.numLights;i++){
    PointLight light=ubo.pointLights[i];
    vec3 directionToLight=light.position.xyz-fragPosWorld;
    float attenuation=1.0/dot(directionToLight,directionToLight);//distance squared
    directionToLight=normalize(directionToLight);

    float cosAngleincidence=max(dot(surfaceNormal,directionToLight),0.0);
    vec3 intensity=light.color.rgb*light.color.a*attenuation;
    diffuseLight+=intensity*cosAngleincidence;

    //specular component
    vec3 halfangle=normalize(directionToLight+viewDirection);
    float blinnTerm=dot(surfaceNormal,halfangle);
    blinnTerm =clamp(blinnTerm,0.0,1.0);
    blinnTerm=pow(blinnTerm, SPEC_POWER);//shininess hardcoded
    specularLight+=intensity*blinnTerm;

  }

  vec3 texColor = texture(texSampler, fragUV).rgb;

  outColor = vec4( (diffuseLight*fragColor + specularLight *fragColor)*texColor, 1.0);
}