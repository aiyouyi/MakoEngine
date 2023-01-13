#version 300 es
#ifdef GL_ES
precision highp  float;
#else
#define highp
#define mediump
#define lowp
#endif
in vec3 WorldPos;

out vec4 out_FragColor;

uniform samplerCube environmentMap;

void main()
{
    vec3 envColor = texture(environmentMap, normalize(WorldPos)).rgb;
    
    // HDR tonemap and gamma correct
  //  envColor = envColor / (envColor + vec3(1.0));
  //  envColor = pow(envColor, vec3(1.0/2.2)); 
    
    out_FragColor = vec4(envColor, 1.0);
   //gl_FragColor = vec4((WorldPos+vec3(1.0,1.0,1.0))*0.5, 1.0);

  // vec2 uv = SampleSphericalMap(normalize(WorldPos));
  // vec3 color = texture(equirectangularMap, uv).rgb;
  // gl_FragColor = vec4(color, 1.0);

}
