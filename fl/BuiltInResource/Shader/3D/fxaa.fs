#version 300 es
#ifdef GL_ES
precision highp  float;
#else
#define highp
#define mediump
#define lowp
#endif
in vec4 variable_vertex;
in vec2 vertex_uv;
uniform vec4 resolution;
uniform sampler2D colorBuffer;

out vec4 output_color;

#define G3D_FXAA_PATCHES   1
//#define POST_PROCESS_OPAQUE 1
#define FXAA_GLSL_130 1
#define FXAA_GATHER4_ALPHA 1

#define MEDIUMP_FLT_MIN    0.00006103515625

#ifdef POST_PROCESS_OPAQUE
#   define FXAA_GREEN_AS_LUMA 0
#else
#   define FXAA_GREEN_AS_LUMA 1
#endif

struct PostProcessInputs
{
	vec4 color;
};

#define FxaaBool bool
#define FxaaDiscard discard
#define FxaaFloat float
#define FxaaFloat2 vec2
#define FxaaFloat3 vec3
#define FxaaFloat4 vec4
#define FxaaHalf float
#define FxaaHalf2 vec2
#define FxaaHalf3 vec3
#define FxaaHalf4 vec4
#define FxaaInt2 ivec2
#define FxaaSat(x) clamp(x, 0.0, 1.0)
#define FxaaTex sampler2D

// Requires "#version 130" or better
#define FxaaTexTop(t, p) textureLod(t, p, 0.0)
#define FxaaTexOff(t, p, o, r) textureLodOffset(t, p, 0.0, o)
#if (FXAA_GATHER4_ALPHA == 1)
    // use #extension gpu_shader5 : enable
    #define FxaaTexAlpha4(t, p) textureGather(t, p, 3)
    #define FxaaTexOffAlpha4(t, p, o) textureGatherOffset(t, p, o, 3)
    #define FxaaTexGreen4(t, p) textureGather(t, p, 1)
    #define FxaaTexOffGreen4(t, p, o) textureGatherOffset(t, p, o, 1)
#endif
/*--------------------------------------------------------------------------*/

#define FxaaLuma(rgba) (rgba).y


FxaaFloat4 fxaa(
    //
    // Use noperspective interpolation here (turn off perspective interpolation).
    // {xy} = center of pixel
    highp FxaaFloat2 pos,
    //
    // Used only for FXAA Console, and not used on the 360 version.
    // Use noperspective interpolation here (turn off perspective interpolation).
    // {xy__} = upper left of pixel
    // {__zw} = lower right of pixel
    highp FxaaFloat4 fxaaConsolePosPos,
    //
    // Input color texture.
    // {rgb_} = color in linear or perceptual color space
    // if (FXAA_GREEN_AS_LUMA == 0)
    //     {___a} = luma in perceptual color space (not linear)
    FxaaTex tex,
    //
    // Only used on FXAA Console.
    // This must be from a constant/uniform.
    // This effects sub-pixel AA quality and inversely sharpness.
    //   Where N ranges between,
    //     N = 0.50 (default)
    //     N = 0.33 (sharper)
    // {__z_} =  N/screenWidthInPixels
    // {___w} =  N/screenHeightInPixels
    highp FxaaFloat2 fxaaConsoleRcpFrameOpt,
    //
    // Only used on FXAA Console.
    // Not used on 360, but used on PS3 and PC.
    // This must be from a constant/uniform.
    // {__z_} =  2.0/screenWidthInPixels
    // {___w} =  2.0/screenHeightInPixels
    highp FxaaFloat2 fxaaConsoleRcpFrameOpt2,
    //
    // Only used on FXAA Console.
    // This used to be the FXAA_CONSOLE__EDGE_SHARPNESS define.
    // It is here now to allow easier tuning.
    // This does not effect PS3, as this needs to be compiled in.
    //   Use FXAA_CONSOLE__PS3_EDGE_SHARPNESS for PS3.
    //   Due to the PS3 being ALU bound,
    //   there are only three safe values here: 2 and 4 and 8.
    //   These options use the shaders ability to a free *|/ by 2|4|8.
    // For all other platforms can be a non-power of two.
    //   8.0 is sharper (default!!!)
    //   4.0 is softer
    //   2.0 is really soft (good only for vector graphics inputs)
    FxaaFloat fxaaConsoleEdgeSharpness,
    //
    // Only used on FXAA Console.
    // This used to be the FXAA_CONSOLE__EDGE_THRESHOLD define.
    // It is here now to allow easier tuning.
    // This does not effect PS3, as this needs to be compiled in.
    //   Use FXAA_CONSOLE__PS3_EDGE_THRESHOLD for PS3.
    //   Due to the PS3 being ALU bound,
    //   there are only two safe values here: 1/4 and 1/8.
    //   These options use the shaders ability to a free *|/ by 2|4|8.
    // The console setting has a different mapping than the quality setting.
    // Other platforms can use other values.
    //   0.125 leaves less aliasing, but is softer (default!!!)
    //   0.25 leaves more aliasing, and is sharper
    FxaaFloat fxaaConsoleEdgeThreshold,
    //
    // Only used on FXAA Console.
    // This used to be the FXAA_CONSOLE__EDGE_THRESHOLD_MIN define.
    // It is here now to allow easier tuning.
    // Trims the algorithm from processing darks.
    // The console setting has a different mapping than the quality setting.
    // This only applies when FXAA_EARLY_EXIT is 1.
    // This does not apply to PS3,
    // PS3 was simplified to avoid more shader instructions.
    //   0.06 - faster but more aliasing in darks
    //   0.05 - default
    //   0.04 - slower and less aliasing in darks
    // Special notes when using FXAA_GREEN_AS_LUMA,
    //   Likely want to set this to zero.
    //   As colors that are mostly not-green
    //   will appear very dark in the green channel!
    //   Tune by looking at mostly non-green content,
    //   then start at zero and increase until aliasing is a problem.
    FxaaFloat fxaaConsoleEdgeThresholdMin
) {
    // Corners: average luma of four corners
    FxaaFloat lumaNw = FxaaLuma(FxaaTexTop(tex, fxaaConsolePosPos.xy));
    FxaaFloat lumaSw = FxaaLuma(FxaaTexTop(tex, fxaaConsolePosPos.xw));
    FxaaFloat lumaNe = FxaaLuma(FxaaTexTop(tex, fxaaConsolePosPos.zy));
    FxaaFloat lumaSe = FxaaLuma(FxaaTexTop(tex, fxaaConsolePosPos.zw));

    // Read the middle RGB and luma
    FxaaFloat4 rgbyM = FxaaTexTop(tex, pos.xy);
    FxaaFloat lumaM = FxaaLuma(rgbyM);

    // Find the largest and smallest luminance value of the four corners.
    FxaaFloat lumaMaxNwSw = max(lumaNw, lumaSw);
    // Lottes' original bias to avoid later divide by zero.  We bias
    // the actual term required below.  In each case, the bias should be
    // less than one intensity gradiation at 8-bit
#ifdef G3D_FXAA_PATCHES
        lumaNe += 1.0 / 384.0;
#endif
    FxaaFloat lumaMinNwSw = min(lumaNw, lumaSw);
    FxaaFloat lumaMaxNeSe = max(lumaNe, lumaSe);
    FxaaFloat lumaMinNeSe = min(lumaNe, lumaSe);
    FxaaFloat lumaMax = max(lumaMaxNeSe, lumaMaxNwSw);
    FxaaFloat lumaMin = min(lumaMinNeSe, lumaMinNwSw);

	//return vec4(lumaMin, 0.0, 0.0, 1.0);

    // The threshold for a luminance edge
    FxaaFloat lumaMaxScaled = lumaMax * fxaaConsoleEdgeThreshold;

    // Min, including the center
    FxaaFloat lumaMinM = min(lumaMin, lumaM);
    FxaaFloat lumaMaxScaledClamped = max(fxaaConsoleEdgeThresholdMin, lumaMaxScaled);  // lumaMaxScaledClamped 看起来是正常的

    // Max, including the center
    FxaaFloat lumaMaxM = max(lumaMax, lumaM);

    // Total range within a 3x3 neighborhood, for thresholding
    // whether it is worth appling AA
    FxaaFloat lumaMaxSubMinM = lumaMaxM - lumaMinM; // 这里lumaMaxM 和 lumaMinM 都是1.0

    // If the entire range is less than the edge threshold, apply no AA
    if (lumaMaxSubMinM < lumaMaxScaledClamped) { return rgbyM; }

    // Diagonal gradient (biased to avoid a later divide by zero)
    FxaaFloat dirSwMinusNe = lumaSw - lumaNe;

    // Other diagonal gradient
    FxaaFloat dirSeMinusNw = lumaSe - lumaNw;

    // Tangent to the edge
    FxaaFloat2 dir = FxaaFloat2(
        dirSwMinusNe + dirSeMinusNw, // == (SW + SE) - (NE + NW) ~= S - N
        dirSwMinusNe - dirSeMinusNw  // == (SW + NW) - (SE + NE) ~= W - E
    );

    // Avoid a division by 0 when normalizing dir
    // Should we instead do the following?
    //     FxaaFloat2 dir1 = dir.xy / (length(dir) + MEDIUMP_FLT_MIN);
    // Or even the following?
    //     FxaaFloat2 dir1 = dirLength < MEDIUMP_FLT_MIN ? FxaaFloat2(0.0) : dir.xy / dirLength
    FxaaFloat dirLength = length(dir.xy);
    if (dirLength < MEDIUMP_FLT_MIN) { return rgbyM; }

    FxaaFloat2 dir1 = dir.xy / dirLength;

    // Step one pixel-width along the tangent in both the positive and negative half
    // spaces and sample there
    FxaaFloat4 rgbyN1 = FxaaTexTop(tex, pos.xy - dir1 * fxaaConsoleRcpFrameOpt.xy);
    FxaaFloat4 rgbyP1 = FxaaTexTop(tex, pos.xy + dir1 * fxaaConsoleRcpFrameOpt.xy);

#ifdef G3D_FXAA_PATCHES
        // A second sample up to two pixels away along the tangent.  This is a "min" in the original.
        //  We increase the distance (i.e., amount of blur) as the luma contrast increases.
        FxaaFloat dirAbsMinTimesC = max(abs(dir1.x), abs(dir1.y)) * fxaaConsoleEdgeSharpness * 0.015;
        FxaaFloat2 dir2 = dir1.xy * min(lumaMaxSubMinM / dirAbsMinTimesC, 3.0);
#   else
        FxaaFloat dirAbsMinTimesC = min(abs(dir1.x), abs(dir1.y)) * fxaaConsoleEdgeSharpness;
        FxaaFloat2 dir2 = clamp(dir1.xy / dirAbsMinTimesC, vec2(-2.0), vec2(2.0));
#   endif
    FxaaFloat4 rgbyN2 = FxaaTexTop(tex, pos.xy - dir2 * fxaaConsoleRcpFrameOpt2.xy);
    FxaaFloat4 rgbyP2 = FxaaTexTop(tex, pos.xy + dir2 * fxaaConsoleRcpFrameOpt2.xy);

    // (twice the) average of adjacent pixels on the tangent directions
    FxaaFloat4 rgbyA = rgbyN1 + rgbyP1;

    // Average of four adjacent pixels on the tangent direction
    FxaaFloat4 rgbyB = ((rgbyN2 + rgbyP2) * 0.25) + (rgbyA * 0.25);

    // If the 4-tap average along the tangent directions is much different from
    // the range of the original, go back to the 2-tap average. 
    // Otherwise, use the 4-tap average.  Considering this threshold avoids some
    // ringing and blurring at object edges
    FxaaBool twoTap = (FxaaLuma(rgbyB) < lumaMin) || (FxaaLuma(rgbyB) > lumaMax);
    if (twoTap) { rgbyB.xyz = rgbyA.xyz * 0.5; }

#ifdef G3D_FXAA_PATCHES
        // Keep some of the original contribution to avoid thin lines degrading completely
        // and overblurring. This is an addition to the original Lottes algorithm
        // rgbyB = sqrt(lerp(rgbyB * rgbyB, rgbyM * rgbyM, 0.25));  // Luminance preserving
        rgbyB = mix(rgbyB, rgbyM, 0.25); // Faster  //TODO: lerp replaced by mix
#   endif
    return rgbyB; 
}

    void postProcess(inout PostProcessInputs postProcess) {
        // First, compute an exact upper bound for the area we need to sample from.
        // The render target may be larger than the viewport that was used for scene
        // rendering, so we cannot rely on the wrap mode alone.
        highp vec2 fboSize = vec2(textureSize(colorBuffer, 0));
        highp vec2 invSize = 1.0 / fboSize;
        highp vec2 halfTexel = 0.5 * invSize;
        highp vec2 viewportSize = resolution.xy;

        // The clamp needs to be over-aggressive by a half-texel due to bilinear sampling.
        highp vec2 excessSize = 0.5 + fboSize - viewportSize;
        highp vec2 upperBound = 1.0 - excessSize * invSize;

        // Next, compute the coordinates of the texel center and its bounding box.
        // There is no need to clamp the min corner since the wrap mode will do
        // it automatically.

        // variable_vertex is already interpolated to pixel center by the GPU
        highp vec2 texelCenter = min(variable_vertex.xy, upperBound);
        highp vec2 texelMaxCorner = min(variable_vertex.xy + halfTexel, upperBound);
        highp vec2 texelMinCorner = variable_vertex.xy - halfTexel;

        vec4 color = fxaa(
                texelCenter,
                vec4(texelMinCorner, texelMaxCorner),
                colorBuffer,
                invSize,             // FxaaFloat4 fxaaConsoleRcpFrameOpt,
                2.0 * invSize,       // FxaaFloat4 fxaaConsoleRcpFrameOpt2,
                8.0,                 // FxaaFloat fxaaConsoleEdgeSharpness,
#ifdef G3D_FXAA_PATCHES
                0.08,                // FxaaFloat fxaaConsoleEdgeThreshold,
#else
                0.125,               // FxaaFloat fxaaConsoleEdgeThreshold,
#endif
                0.04                 // FxaaFloat fxaaConsoleEdgeThresholdMin
        );
#ifdef POST_PROCESS_OPAQUE
        color.a = 1.0;
#endif
        postProcess.color = color;

    }

void main()
{
	PostProcessInputs inputs;

	postProcess(inputs);

	output_color = inputs.color;
}