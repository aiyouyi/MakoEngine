///-------------------------------------------------------------------------------------------------
/// @file	E:\Git\ARGit\RealTimeMakeup\RealtimeMakeup\3DObject\MSAAFramebuffer.h.
///
/// @brief	Declares the msaa framebuffer class.
///-------------------------------------------------------------------------------------------------

#ifndef _MSAA_FRAMEBUFFER_H_
#define _MSAA_FRAMEBUFFER_H_
/*
	MSAA 跨平台抗锯齿,所有接口都需要是GL线程
	调用顺序
	-> Initialize()
	-> AsFrameBuffer&AsFrameBufferTexture
	-> SetupSize
	-> Bind
	-> Unbind
 */

#include "BaseDefine/Define.h"
#include "openglutil.h"

 ///-------------------------------------------------------------------------------------------------
 /// @class	MSAAFramebuffer
 ///
 /// @brief	A msaa framebuffer.
 ///-------------------------------------------------------------------------------------------------

class MSAAFramebuffer
{
public:

	///-------------------------------------------------------------------------------------------------
	/// @fn	MSAAFramebuffer::MSAAFramebuffer();
	///
	/// @brief	Default constructor.
	///-------------------------------------------------------------------------------------------------

	MSAAFramebuffer();

	///-------------------------------------------------------------------------------------------------
	/// @fn	MSAAFramebuffer::~MSAAFramebuffer();
	///
	/// @brief	Destructor.
	///-------------------------------------------------------------------------------------------------

	~MSAAFramebuffer();

	/*初始化MSAA,用于获取OpenGL信息
	return: false表示初始化失败.
	 */

	 ///-------------------------------------------------------------------------------------------------
	 /// @fn	bool MSAAFramebuffer::Initialize();
	 ///
	 /// @brief	Initializes this object.
	 ///
	 /// @return	True if it succeeds, false if it fails.
	 ///-------------------------------------------------------------------------------------------------

	bool Initialize();

	///-------------------------------------------------------------------------------------------------
	/// @fn	void MSAAFramebuffer::AsFrameBuffer(GLuint framebuffer);
	///
	/// @brief	通过外部引用一个FrameBuffer.
	///
	/// @param	framebuffer	The framebuffer.
	///-------------------------------------------------------------------------------------------------

	void AsFrameBuffer(GLuint framebuffer);

	///-------------------------------------------------------------------------------------------------
	/// @fn	void MSAAFramebuffer::AsFrameBufferTexture(GLuint framebufferTexture);
	///
	/// @brief	通过外部引用一个FrameBufferTexture.
	///
	/// @param	framebufferTexture	The framebuffer texture.
	///-------------------------------------------------------------------------------------------------

	void AsFrameBufferTexture(GLuint framebufferTexture);

	///-------------------------------------------------------------------------------------------------
	/// @fn	bool MSAAFramebuffer::SetupSize(int width, int height);
	///
	/// @brief	设置离屏抗锯齿窗口大小.
	///
	/// @param	width 	The width.
	/// @param	height	The height.
	///
	/// @return	True if it succeeds, false if it fails.
	///-------------------------------------------------------------------------------------------------

	bool SetupSize(int width, int height);

	///-------------------------------------------------------------------------------------------------
	/// @fn	bool MSAAFramebuffer::Bind();
	///
	/// @brief	绑定MSAA-FBO.
	///
	/// @return	True if it succeeds, false if it fails.
	///-------------------------------------------------------------------------------------------------

	bool Bind();

	///-------------------------------------------------------------------------------------------------
	/// @fn	bool MSAAFramebuffer::Unbind();
	///
	/// @brief	解绑.
	///
	/// @return	True if it succeeds, false if it fails.
	///-------------------------------------------------------------------------------------------------

	bool Unbind(GLuint outputFbo = 0);

	///-------------------------------------------------------------------------------------------------
	/// @fn	void MSAAFramebuffer::Destory();
	///
	/// @brief	销毁.
	///-------------------------------------------------------------------------------------------------

	void Destory();

	void AsEmissTexture(GLuint tex)
	{
		m_AsEmissTexture = tex;
	}

	void SetIsNeedDrawBack(bool value) {
		m_isNeedDrawBack = value;
	}

	void ClearColor();
	void AttatchColorBuffers(int nCount);
	bool m_isIpad;
private:

	///-------------------------------------------------------------------------------------------------
	/// @fn	void MSAAFramebuffer::DrawBackGround();
	///
	/// @brief	Draw back ground.
	///-------------------------------------------------------------------------------------------------

	void DrawBackGround();

	/// @brief	The msaa framebuffer.
	GLuint m_MSAAFramebuffer;
	/// @brief	The msaa renderbuffer.
	GLuint m_MSAARenderbuffer;
	/// @brief	The msaa depth renderbuffer.
	GLuint m_MSAADepthRenderbuffer;


	/// @brief	The gles version.
	int m_GLESVersion;

	/// @brief	外部FrameBuffer.
	GLuint m_AsFrameBuffer;
	/// @brief	as frame buffer texture.
	GLuint m_AsFrameBufferTexture;
	/// @brief	内部framebuffer.
	GLuint m_SingleRenderBuffer;
	/// @brief	The single framebuffer.
	GLuint m_SingleFramebuffer;
	/// @brief	Width of the texture.
	int m_TextureWidth;
	/// @brief	Height of the texture.
	int m_TextureHeight;

	bool m_isNeedDrawBack = true;

	GLuint textureBufferMultiSample = 0;

	GLuint textureEmissMultiSample = 0;

	GLuint m_AsEmissTexture;

};

#endif
