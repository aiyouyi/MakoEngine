
#pragma once

namespace mt3dface {

	///-------------------------------------------------------------------------------------------------
	/// @struct	MT2DFaceInfo
	///
	/// @brief	Information about the mt 2D face.(人脸点与人脸框和yawangle信息)
	///
	/// 
	/// @author	Alop
	/// @date	19/1/22
	///-------------------------------------------------------------------------------------------------

	struct MTFace2DMesh
	{
		/// @brief	The reconstruct vertexs
		float *pVertexs;//[nVertex*3]

		/// @brief	The texture coordinates
		float *pTextureCoordinates;//[nVertex*2]

		/// @brief	The vertex num
		int nVertex;

		/// @brief	Zero-based index of the triangle
		unsigned short *pTriangleIndex;//[nTriangle*3]

		/// @brief	The triangle num
		int nTriangle;
	};


    enum eReconstruct2DMode
	{
		MT_FACE_25D =0,        //2.5D重建
		MT_FACE_2D_BACKGROUND,  //2D带背景网格
		MT_FACE_2D_MUITIBACKGROUND //2D多层背景网格
	};
	class SimpleMesh;
	class  MTFace2DInterFace
	{
	public:
		MTFace2DInterFace();
		~MTFace2DInterFace();

		///-------------------------------------------------------------------------------------------------
		/// @fn	MTFace2DMesh MTFace2DInterFace::*Get2DMesh(MT2DFaceInfo &faceInfo, int nWidth, int nHeight);
		///
		/// @brief	Gets 2D mesh.
		///
		/// @author	Alop
		/// @date	19/1/22
		///
		/// @param      	pFacePoint106	Face Point 106
		/// @param 		   	nWidth  	    The img width.
		/// @param 		   	nHeight 	    The img height.
		/// @param 		   	pitchAngle     face pitch angle .
		///
		/// @return	Null if it fails, else the 2D mesh.
		///-------------------------------------------------------------------------------------------------

		MTFace2DMesh *Get2DMesh(float *pFacePoint106, int nWidth, int nHeight,float pitchAngle,eReconstruct2DMode Reconstruct2DMode = MT_FACE_25D);

		///-------------------------------------------------------------------------------------------------
		/// @fn	float MTFace2DInterFace::*GetStandVerts();
		///
		/// @brief	Gets stand vertices. 点数与MTFace2DMesh.nVertex相同,这个点主要用于一些mask图的绘制，因为mask
		/// 		肯定是画在标准图上的
		///
		/// @author	Alop
		/// @date	19/2/12
		///
		/// @return	Null if it fails, else the stand vertices.
		///-------------------------------------------------------------------------------------------------

		float *GetStandVerts();
	private:
		SimpleMesh *m_p2DFaceReconstruct;
		MTFace2DMesh *m_pFace2DMesh;
		float *m_pStandPoint;

		SimpleMesh *m_p2DFaceBackReconstruct;
		SimpleMesh *m_p2DMultiFaceBackReconstruct;
	};

}