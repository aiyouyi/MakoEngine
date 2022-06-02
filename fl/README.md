#CCFilter 
>CCFilter  主要包含人脸特效渲染，虚拟形象，驱动等相关算法。
>需要依赖人脸检测与对齐，DX11。
>
```
目录说明

1.3rdparty/   ----         一些第三方库，比如Eigen，GLM等等

2.BuiltInResource/ ----    内置资源文件夹，包含shader文件，模型文件

3.FacialToolsQT-DX11/ ---- QTdemo框架，包含调用接口方式，DX环境，资源等处理

4.ImageFilterDX11/ -----   核心代码，主要有两个模块，人脸特效渲染系统 和虚拟直播渲染系统

