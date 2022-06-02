
#ifndef TINYGLTF_IMPLEMENTATION
#define TINYGLTF_IMPLEMENTATION
#endif

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#endif

#ifndef STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#endif
#include "CC3DUtils.h"


glm::mat4 CC3DUtils::QuaternionToMatrix(const Vector4 vec4InQuaternion)
{
	glm::fmat4 resMatrix = glm::fmat4();
	Vector4 tmp = vec4InQuaternion;
	CC3DUtils::QuaternionNormalize(tmp);
	float x = tmp.x;
	float y = tmp.y;
	float z = tmp.z;
	float w = tmp.w;

	resMatrix[0][0] = (1.0f) - (2.0f) * (y * y + z * z);
	resMatrix[1][0] = (2.0f) * (x * y - z * w);
	resMatrix[2][0] = (2.0f) * (x * z + y * w);
	resMatrix[3][0] = 0.0f;

	resMatrix[0][1] = (2.0f) * (x * y + z * w);
	resMatrix[1][1] = (1.0f) - (2.0f) * (x * x + z * z);
	resMatrix[2][1] = (2.0f) * (y * z - x * w);
	resMatrix[3][1] = 0.0f;
	
	resMatrix[0][2] = (2.0f) * (x * z - y * w);
	resMatrix[1][2] = (2.0f) * (y * z + x * w);
	resMatrix[2][2] = (1.0f) - (2.0f) * (x * x + y * y);
	resMatrix[3][2] = 0.0f;

	resMatrix[0][3] = 0.0f;
	resMatrix[1][3] = 0.0f;
	resMatrix[2][3] = 0.0f;
	resMatrix[3][3] = 1.0f;

	return glm::mat4(resMatrix);
}

void CC3DUtils::QuaternionInterpolate(Vector4 & vec4Out, const Vector4 & vec4Start, const Vector4 & vec4End, float fFactor)
{
	// calc cosine theta
	float cosom = vec4Start.x * vec4End.x + vec4Start.y * vec4End.y + vec4Start.z * vec4End.z + vec4Start.w * vec4End.w;

	// adjust signs (if necessary)
	Vector4  end = vec4End;
	if (cosom < 0.0f)
	{
		cosom = -cosom;
		end.x = -end.x;   // Reverse all signs
		end.y = -end.y;
		end.z = -end.z;
		end.w = -end.w;
	}

	// Calculate coefficients
	float sclp, sclq;
	if ((1.0f - cosom) > 0.0001f) // 0.0001 -> some epsillon
	{
		// Standard case (slerp)
		float omega, sinom;
		omega = (std::acos)(cosom); // extract theta from dot product's cos theta
		sinom = (std::sin)(omega);
		sclp = (std::sin)((1.0f - fFactor) * omega) / sinom;
		sclq = (std::sin)(fFactor * omega) / sinom;
	}
	else
	{
		// Very close, do linear interp (because it's faster)
		sclp = 1.0f - fFactor;
		sclq = fFactor;
	}

	vec4Out.x = sclp * vec4Start.x + sclq * end.x;
	vec4Out.y = sclp * vec4Start.y + sclq * end.y;
	vec4Out.z = sclp * vec4Start.z + sclq * end.z;
	vec4Out.w = sclp * vec4Start.w + sclq * end.w;
	return;
}

void CC3DUtils::QuaternionNormalize(Vector4 & vec4Out)
{
	if (&vec4Out == NULL)
		return;
	// compute the magnitude and divide through it
	const float mag = sqrt(vec4Out.x*vec4Out.x + vec4Out.y*vec4Out.y + vec4Out.z*vec4Out.z + vec4Out.w*vec4Out.w);
	if (mag)
	{
		const float invMag = 1.0f / mag;
		vec4Out.x *= invMag;
		vec4Out.y *= invMag;
		vec4Out.z *= invMag;
		vec4Out.w *= invMag;
	}
	return;
}

CCglTFModel::~CCglTFModel()
{
	for (int i = 0; i < m_pData.size(); i++)
	{
		SAFE_DELETE_ARRAY(m_pData[i]);
		m_pData[i] = NULL;
	}
}

void * CCglTFModel::Getdata(int attributeIndex, uint32 & nCount, int & CommpontType)
{
	const auto &indicesAccessor = m_Model->accessors[attributeIndex];
	const auto &bufferView = m_Model->bufferViews[indicesAccessor.bufferView];
	const auto &buffer = m_Model->buffers[bufferView.buffer];
	const auto dataAddress = buffer.data.data() + bufferView.byteOffset +
		indicesAccessor.byteOffset;
	const auto byteStride = indicesAccessor.ByteStride(bufferView);
	nCount = indicesAccessor.count;
	CommpontType = indicesAccessor.componentType;


	int type = indicesAccessor.type;
	int nStep = 0;
	if (type == TINYGLTF_TYPE_SCALAR) {
		nStep = 1;
	}
	else if (type == TINYGLTF_TYPE_VEC2) {
		nStep = 2;
	}
	else if (type == TINYGLTF_TYPE_VEC3) {

		nStep = 3;
	}
	else if (type == TINYGLTF_TYPE_VEC4) {

		nStep = 4;
	}
	int OneSize = 0;

	if (CommpontType == 5122 || CommpontType == 5123) {
		OneSize = sizeof(uint16);
	}
	else if (CommpontType == 5124 || CommpontType == 5125) {
		OneSize = sizeof(uint32);
	}
	else if (CommpontType == 5126) {

		OneSize = sizeof(float);
	}
	else if (CommpontType == 5120 || CommpontType == 5121) {
		OneSize = sizeof(uint8);
	}

	if (nStep == 0 || OneSize == 0 || nStep*OneSize == byteStride)
	{
		return (void *)dataAddress;
	}
	else
	{
		uint8 *pData = new uint8[nStep * OneSize *nCount];
		uint8 *pSrc = (uint8 *)dataAddress;
		for (int i=0;i<nCount;i++)
		{

			memcpy(pData + i * OneSize *nStep, pSrc + i * byteStride, OneSize*nStep);
		}
		m_pData.push_back(pData);
		return (void *)pData;
	}

}


unsigned char *ccLoadImage(char const *filename, int *x, int *y, int *comp, int req_comp)
{
	return  stbi_load(filename, x, y, comp, req_comp);
}


unsigned char *ccLoadImageFromBuffer(unsigned char *pBuffer, int nLen, int *x, int *y, int *comp, int req_comp)
{
	return  stbi_load_from_memory(pBuffer, nLen, x, y, comp, req_comp);
}

float *ccLoadImagef(char const *filename, int *x, int *y, int *comp, int req_comp)
{
	//stbi_set_flip_vertically_on_load(true);
	float*pData =   stbi_loadf(filename, x, y, comp, req_comp);

    //stbi_set_flip_vertically_on_load(false);
	return pData;
}

float *ccLoadImagefFromBuffer(unsigned char *pBuffer, int nLen, int *x, int *y, int *comp, int req_comp)
{
	return  stbi_loadf_from_memory(pBuffer, nLen, x, y, comp, req_comp);
}

void ccSavePng(const char* pFile, unsigned char * pBuffer, int x, int y, int comp)
{
	stbi_write_png(pFile, x, y, comp, pBuffer,x*comp);
}

void ccSaveJpg(const char* pFile, unsigned char * pBuffer, int x, int y, int comp)
{
	stbi_write_jpg(pFile, x, y, comp, pBuffer, 0);
}


std::string GetFilePathExtension(const std::string &FileName) {
	if (FileName.find_last_of(".") != std::string::npos)
		return FileName.substr(FileName.find_last_of(".") + 1);
	return "";
}