#version 430 core
#extension GL_EXT_shader_io_blocks : enable

in block {
    vec4 color;
    vec2 texCoord;
} In;

layout(location=0) out vec4 fragColor;

void main() {
    // Quick fall-off computation
    //float r = 0.2;
    //float i = exp(-r*r);
    //if (i < 0.01) discard;

    fragColor = vec4(In.color.rgb, 1.0);
}