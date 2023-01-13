#version 300 es

layout(location = 0) in vec3 position;

#define float2   vec2
#define float3   vec3
#define float4   vec4

out vec2 vertex_uv;
out vec4 variable_vertex;

uniform vec4 resolution;
uniform vec2 clipControl;

struct PostProcessVertexInputs {

    // We provide normalized and non-normalized texture coordinates to custom vertex shaders.
    // By default the non-normalized coordinates are passed through to the fragment shader.
    vec2 normalizedUV;
    vec2 texelCoords;

	vec4 vertex;
};

void initPostProcessMaterialVertex(out PostProcessVertexInputs inputs)
{
    inputs.vertex = vec4(0.0);
}

void postProcessVertex(inout PostProcessVertexInputs postProcess) 
{
    postProcess.vertex.xy = postProcess.normalizedUV;
}

void main()
{
	    // Initialize the vertex shader inputs to sensible default values.
    PostProcessVertexInputs inputs;
    initPostProcessMaterialVertex(inputs);

    inputs.normalizedUV = position.xy * 0.5 + 0.5;
    inputs.texelCoords = inputs.normalizedUV * resolution.xy;

// In Vulkan and Metal, texture coords are Y-down. In OpenGL, texture coords are Y-up.
#if defined(TARGET_METAL_ENVIRONMENT) || defined(TARGET_VULKAN_ENVIRONMENT)
    inputs.texelCoords.y = frameUniforms.resolution.y - inputs.texelCoords.y;
    inputs.normalizedUV.y = 1.0 - inputs.normalizedUV.y;
#endif

    gl_Position = vec4(position, 1.0);

    // Adjust clip-space
    gl_Position.z = dot(gl_Position.zw, clipControl.xy);

    // Invoke user code
    postProcessVertex(inputs);

    vertex_uv = inputs.texelCoords;

	variable_vertex = inputs.vertex;
}