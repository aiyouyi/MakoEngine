#version 300 es
layout(location = 0) in vec3 position;
out vec2 textureCoordinate;
void main() {
    gl_Position = vec4(position,1.0);
    textureCoordinate = position.xy * 0.5 + 0.5;
}