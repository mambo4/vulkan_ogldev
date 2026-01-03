#version 460

layout(location = 0) out vec4 out_Color;
layout(location = 0) in vec2 uv;
layout(binding=2) uniform sampler2D textureSampler;
void main() {
  out_Color = texture(textureSampler,uv);
}