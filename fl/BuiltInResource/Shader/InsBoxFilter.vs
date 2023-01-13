     attribute vec2 inputTextureCoordinate;
     uniform  float texelWidthOffset;
     uniform  float texelHeightOffset;
     varying vec2 textureCoordinate;
     varying vec4 textureShift_1;
     varying vec4 textureShift_2;
     varying vec4 textureShift_3;
     varying vec4 textureShift_4;
     
     void main()
     {
         gl_Position = vec4(inputTextureCoordinate*2.0-1.0,0.5,1.0);
         vec2 singleStepOffset = vec2(texelWidthOffset, texelHeightOffset);
         textureCoordinate = inputTextureCoordinate;
         textureShift_1 = vec4(inputTextureCoordinate - singleStepOffset, inputTextureCoordinate + singleStepOffset);
         textureShift_2 = vec4(inputTextureCoordinate - 2.0 * singleStepOffset, inputTextureCoordinate + 2.0 * singleStepOffset);
         textureShift_3 = vec4(inputTextureCoordinate - 3.0 * singleStepOffset, inputTextureCoordinate + 3.0 * singleStepOffset);
         textureShift_4 = vec4(inputTextureCoordinate - 4.0 * singleStepOffset, inputTextureCoordinate + 4.0 * singleStepOffset);
     }
     