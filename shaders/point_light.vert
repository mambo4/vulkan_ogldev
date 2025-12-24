#version 450

layout (location = 0) out vec2 fragOffset;

const vec2 OFFSETS[6] = vec2[](
    vec2(-5.0, -5.0),
    vec2( 5.0, -5.0),
    vec2(-5.0,  5.0),
    vec2( 5.0,  5.0),
    vec2( 5.0,  -5.0),
    vec2( -5.0, 5.0)
);

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


void main() {
    fragOffset = OFFSETS[gl_VertexIndex];
    vec3 cameraRightWorld = vec3(ubo.view[0][0], ubo.view[1][0], ubo.view[2][0]);
    vec3 cameraUpWorld = vec3(ubo.view[0][1], ubo.view[1][1], ubo.view[2][1]);
    
    vec3 positionWorld = pushConstants.position.xyz 
    + pushConstants.radius * cameraRightWorld * fragOffset.x 
    + pushConstants.radius * cameraUpWorld * fragOffset.y;

    gl_Position = ubo.projection * ubo.view * vec4(positionWorld, 1.0);
}