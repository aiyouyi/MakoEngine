attribute vec2 atbCoord2d;        

attribute vec4 offest;  

varying vec4 xyzw;

void main(void)
{
    xyzw = offest;
    gl_Position = vec4(atbCoord2d*2.0-1.0,0.5,1.0);
}



