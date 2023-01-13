     attribute vec2 inputTextureCoordinate;
     varying vec2 textureCoordinate;
     
     void main()
     {
         gl_Position = vec4(inputTextureCoordinate*2.0-1.0,0.5,1.0);
         textureCoordinate = inputTextureCoordinate;
     }
     