#pragma once

#include "core/inc.h"
#include <Runtime/Core/Public/CoreMinimal.h>

#ifdef __cplusplus
extern "C" {
#else
#if defined(_MSC_VER) && !defined(inline)
#define inline __inline
#endif
#endif

enum hook_type {
	HOOKTYPE_FORWARD_OVERWRITE,
	HOOKTYPE_FORWARD_CHAIN,
	HOOKTYPE_REVERSE_CHAIN
};

struct func_hook {
	void                   *call_addr;

	uintptr_t              func_addr;       /* function being hooked to */
	uintptr_t              hook_addr;       /* hook function itself */
	void                   *bounce_addr;
	const char             *name;
	enum hook_type         type;
	bool                   is_64bit_jump;
	bool                   hooked;
	bool                   started;
	bool                   attempted_bounce;
	uint8_t                unhook_data[14];
	uint8_t                rehook_data[14];
};

extern void hook_init(struct func_hook *hook,
		void *func_addr, void *hook_addr, const char *name);
extern void hook_start(struct func_hook *hook);
extern void do_hook(struct func_hook *hook, bool force);
extern void unhook(struct func_hook *hook);

static inline void rehook(struct func_hook *hook)
{
	do_hook(hook, false);
}

static inline void force_rehook(struct func_hook *hook)
{
	do_hook(hook, true);
}

#ifdef __cplusplus
}

class CCACTORLIB_API ObsStudioHook
{
public:
	ObsStudioHook()
		: func(NULL)
		, hookFunc(NULL)
        , bHooked(false)
	{
		memset(&mHook, sizeof(func_hook),0);
	}
	~ObsStudioHook()
	{

	}

	inline bool Hook(void* funcIn, void* hookFuncIn)
	{
		if (bHooked)
		{
			if (funcIn == func)
			{
				if (hookFunc != hookFuncIn)
				{
					hookFunc = hookFuncIn;
					Rehook();
					return true;
				}
			}

			mHook.type = HOOKTYPE_FORWARD_OVERWRITE;//������Ҫ��������Ȼ��Ϸ���õ�ʱ������
			Unhook();
		}

		func = funcIn;
		hookFunc = hookFuncIn;

		hook_init(&mHook, func, hookFunc, "");
		return true;
	}

	inline void Rehook(bool bForce = false)
	{
		if ((!bForce && bHooked) || !func)
		{
			return;
		}

		rehook(&mHook);

		bHooked = true;
	}

	inline void Unhook()
	{
		if (!bHooked || !func)
			return;

		unhook(&mHook);

		bHooked = false;
	}

	void* GetCallAddress()
	{
		return mHook.call_addr;
	}


	void* func;//ԭ����
	void* hookFunc;//�ҵ�hook������ַ
	func_hook mHook;
	bool bHooked;
};


#endif
