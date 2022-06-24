#include "GLBMeshData.h"

#ifndef TINYGLTF_IMPLEMENTATION
#define TINYGLTF_IMPLEMENTATION
#endif

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#endif

#ifndef STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#endif

#include"common\tiny_gltf.h""
#include<filesystem>
#include<fstream>

static std::wstring GetFilePathExtension(const std::wstring& FileName) {
	if (FileName.find_last_of(L".") != std::string::npos)
		return FileName.substr(FileName.find_last_of(L".") + 1);
	return L"";
}

GLBMeshPool::~GLBMeshPool()
{
	for (auto& Iter : MeshDataPool)
	{
		delete Iter.second;
	}
	MeshDataPool.clear();
}

GLBMeshData* GLBMeshPool::GetGLBMesh(std::wstring FilePath)
{
	std::filesystem::path Path = FilePath;
	auto AbsPath = std::filesystem::absolute(Path);
	auto MeshIter = MeshDataPool.find(AbsPath.wstring());
	if (MeshIter == MeshDataPool.end())
	{
		GLBMeshData* pGLBMeshData = new GLBMeshData();
		pGLBMeshData->ReadFromFile(AbsPath.wstring());
		MeshDataPool.insert(std::pair(AbsPath.wstring(), pGLBMeshData));
		return pGLBMeshData;
	}
	return MeshIter->second;
}

GLBMeshData::GLBMeshData()
{
	pModel = new tinygltf::Model();
}

GLBMeshData::~GLBMeshData()
{
	delete pModel;
	pModel = nullptr;
}

bool GLBMeshData::ReadFromFile(std::wstring FilePath)
{
	std::ifstream File(FilePath, std::ios::in | std::ios::binary);
	const auto FileSize = std::filesystem::file_size(FilePath);
	std::string Result(FileSize, '\0');
	File.read(Result.data(), FileSize);

	tinygltf::TinyGLTF Gltf_ctx;
	std::string Err;
	std::string Warn;
	std::wstring Ext = GetFilePathExtension(FilePath);
	Gltf_ctx.SetStoreOriginalJSONForExtrasAndExtensions(false);

	bool Ret = false;
	if (Ext.compare(L"glb") == 0)
	{
		Ret = Gltf_ctx.LoadBinaryFromMemory(pModel, &Err, &Warn, (unsigned char*)Result.data(), FileSize);
	}
	return Ret;
}

int GLBMeshData::MeshNum()
{
	return pModel->meshes.size();
}

int GLBMeshData::PrimitiveNum(int MeshIndex)
{
	if (pModel->meshes.size()> MeshIndex)
	{
		return pModel->meshes[MeshIndex].primitives.size();
	}
	return 0;
}

void* GLBMeshData::GetIndiceBuffer(int MeshIndex, int PrimitiveIndex, int& Count, int& NumType, int& VectorType, int& ByteStride)
{
	auto& MeshPrimitive = pModel->meshes[MeshIndex].primitives[PrimitiveIndex];
	return GetData(MeshPrimitive.indices, Count, NumType, VectorType, ByteStride);
}

void* GLBMeshData::GetAttrBuffer(std::string AttrName, int MeshIndex, int PrimitiveIndex, int& Count, int& NumType, int& VectorType, int& ByteStride)
{
	auto& MeshPrimitive = pModel->meshes[MeshIndex].primitives[PrimitiveIndex];
	for (const auto& Attribute : MeshPrimitive.attributes)
	{
		if (Attribute.first== AttrName)
		{
			return GetData(Attribute.second, Count, NumType, VectorType, ByteStride);
		}
	}
	return nullptr;
}

int GLBMeshData::GetNumSize(int NumType)
{
	switch (NumType)
	{
	case TINYGLTF_COMPONENT_TYPE_BYTE:
	case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
		return sizeof(char);
	case TINYGLTF_COMPONENT_TYPE_SHORT:
	case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
		return sizeof(short);
	case TINYGLTF_COMPONENT_TYPE_INT:
	case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
		return sizeof(int);
	case TINYGLTF_COMPONENT_TYPE_FLOAT:
		return sizeof(float);
	case TINYGLTF_COMPONENT_TYPE_DOUBLE:
		return sizeof(double);
	}
	return 0;
}

void* GLBMeshData::GetData(int AttributeIndex, int& Count, int& NumType, int& VectorType, int& ByteStride)
{
	void* Result = nullptr;
	if (pModel)
	{
		const auto& IndicesAccessor = pModel->accessors[AttributeIndex];
		const auto& BufferView = pModel->bufferViews[IndicesAccessor.bufferView];
		const auto& Buffer = pModel->buffers[BufferView.buffer];
		const auto DataAddress = Buffer.data.data() + BufferView.byteOffset +IndicesAccessor.byteOffset;
		ByteStride = IndicesAccessor.ByteStride(BufferView);
		Count = IndicesAccessor.count;
		NumType = IndicesAccessor.componentType;
		VectorType = IndicesAccessor.type;
		Result = (void*)DataAddress;
	}
	return Result;	
}
