#version 430 core

layout(std430, binding=1) buffer Pos {
	vec4 pos[];
};

out gl_PerVertex {
    vec4 gl_Position;
};

out block {
     vec4 color;
     vec2 texCoord;
} Out;

void main() {
    // expand points to quads without using GS
    int particleID = gl_VertexID >> 2; // 4 vertices per particle
    vec4 particlePos = pos[particleID];

    Out.color = vec4(1.0, 1.0, 1.0, 1.0);

    //map vertex ID to quad vertex
    vec2 quadPos = vec2( ((gl_VertexID - 1) & 2) >> 1, (gl_VertexID & 2) >> 1);

    Out.texCoord = quadPos;
    gl_Position = particlePos;

}