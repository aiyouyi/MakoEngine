#pragma once
#include<map>
#include<string>

namespace tinygltf
{
	class Model;
}

class GLBMeshData
{
public:

	GLBMeshData();
	~GLBMeshData();
	bool ReadFromFile(std::wstring FilePath);
	int MeshNum();
	int PrimitiveNum(int MeshIndex);
	
	void* GetIndiceBuffer(int MeshIndex, int PrimitiveIndex, int& Count, int& NumType, int& VectorType, int& ByteStride);
	void* GetAttrBuffer(std::string AttrName, int MeshIndex, int PrimitiveIndex, int& Count, int& NumType, int& VectorType, int& ByteStride);

	static int GetNumSize(int NumType);

	template<typename T>
	static bool ConvertBuffer(void* SrcBuffer, T* DesBuffer, int Count, int NumType, int VectorType, int ByteStride);

private:
	
	void* GetData(int AttributeIndex, int& Count, int& NumType, int& VectorType, int& ByteStride);

	tinygltf::Model* pModel = nullptr;
};

class GLBMeshPool
{
public:
	~GLBMeshPool();
	static GLBMeshPool* GetInstance()
	{
		static GLBMeshPool MeshPool;
		return &MeshPool;
	}

	std::map<std::wstring, GLBMeshData*> MeshDataPool;

	GLBMeshData* GetGLBMesh(std::wstring FilePath);
};



template<typename T>
bool GLBMeshData::ConvertBuffer(void* SrcBuffer, T* DesBuffer, int Count, int NumType, int VectorType, int ByteStride)
{
	int VecSize = VectorType % 32;
	int NumSize = GetNumSize(NumType);
	if (NumSize==sizeof(T))
	{
		for (int Index = 0; Index < Count; ++Index)
		{
			memcpy(DesBuffer + Index * VecSize * NumSize, SrcBuffer + Index * ByteStride, VecSize * NumSize);
		}
		return true;
	}
	return false;
}
