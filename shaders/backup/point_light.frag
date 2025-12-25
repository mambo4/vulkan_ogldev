#version 450

layout(location = 0) in vec2 fragOffset;
layout(location=0) out vec4 outColor;

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

layout(push_constant) uniform PushConstants {
    vec4 position;
    vec4 color; 
    float radius;
} pushConstants;

const float PI = 3.14159265359;
void main() {
    float distance = sqrt(dot(fragOffset, fragOffset));
    if (distance >= 1.0) {
        discard;
    }
    float cosDis = 0.5* ( cos(distance * PI)+1.0);
    outColor = vec4(pushConstants.color.rgb+cosDis, cosDis);
}