#pragma once
#include "CoreMinimal.h"
#include "win/shared_memory.h"

struct huyaSharedData;

class  CCheckData
{
public:
	CCheckData();
	virtual ~CCheckData();

	static CCheckData* GetInstance();
	static void Release();
	
	bool IsEnableSharedResource();
	void StartCheckSharedCopy();
	std::tuple<int64_t, int64_t, int64_t> GetDXOffset();

private:
	void ServerInit();
	void CreateCheckProcess(const wchar_t * cmd);

private:


	win32::shared_memory<huyaSharedData> m_sharedData;
	bool m_alreadyCheck = false;
};