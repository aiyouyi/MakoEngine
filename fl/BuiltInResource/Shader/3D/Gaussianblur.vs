#version 300 es
layout(location = 0) in vec3 position;
out vec2 v2f_TexCoords;
void main() {
    gl_Position = vec4(position,1.0);
    v2f_TexCoords = position.xy * 0.5 + 0.5;
}