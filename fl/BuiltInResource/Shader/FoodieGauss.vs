     attribute vec2 inputTextureCoordinate;
     uniform  float texelWidthOffset;
     uniform  float texelHeightOffset;
     varying vec2 blurCoordinates[5];
     
     void main()
     {
         gl_Position = vec4(inputTextureCoordinate*2.0-1.0,0.5,1.0);
         vec2 singleStepOffset = vec2(texelWidthOffset, texelHeightOffset);
         blurCoordinates[0] = inputTextureCoordinate;
         blurCoordinates[1] = inputTextureCoordinate + singleStepOffset * 1.182425;
         blurCoordinates[2] = inputTextureCoordinate - singleStepOffset * 1.182425;
         blurCoordinates[3] = inputTextureCoordinate + singleStepOffset * 3.029312;
         blurCoordinates[4] = inputTextureCoordinate - singleStepOffset * 3.029312;
     }
     