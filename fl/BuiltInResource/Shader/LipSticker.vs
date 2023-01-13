     attribute vec2 inputTextureCoordinate;
     varying vec2 textureCoordinate;
     varying vec2 textureCoordinate2;
     uniform vec4 LipRect;
     void main()
     {
         vec2 LipTex = inputTextureCoordinate*LipRect.zw+LipRect.xy;
         gl_Position = vec4(LipTex*2.0-1.0,0.5,1.0);
         textureCoordinate = inputTextureCoordinate;
         textureCoordinate2 = LipTex;
     }
     