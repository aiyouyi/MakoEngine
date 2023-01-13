#include "FaceGiftEffectPlayer.h"
#include "StickerInterface.h"
#include "EffectCInterface.h"
#include "Toolbox/zip/unzip.h"
#include "Toolbox/xmlParser.h"
#include <iostream>
#include <string>


using namespace std;

void GiftEffectCallback_Func(int event, const void *lpParam);

#define FGEPInst (FaceGiftEffectPlayer::getInst())
class FaceGiftEffectPlayer
{
public:
	FaceGiftEffectPlayer() 
	{
		m_handleSticker = NULL;
		m_handleEffect = NULL;
		callback = NULL;
	}
	~FaceGiftEffectPlayer() 
	{
		destoryEffect();
		if (m_handleEffect != NULL)
		{
			ccStickerDestory(m_handleSticker);
		}
		if (m_handleEffect != NULL)
		{
			ccEffectRelease(m_handleEffect);
		}
		
	}

	static FaceGiftEffectPlayer *getInst()
	{
		static FaceGiftEffectPlayer *s_inst = new FaceGiftEffectPlayer();
		return s_inst;
	}

	void SetGiftResourcePath(const char*path)
	{
		if (m_handleEffect == NULL)
		{
			m_handleEffect = ccEffectCreate();
		}
		ccEffectSetPath(m_handleEffect, path);
	}

	void SetBGRA(unsigned char*pBGRA)
	{
		if (m_handleEffect != NULL)
		{
			ccEffectSetBGRA(m_handleEffect, pBGRA);
		}
	}
	void setGiftEffect(const string &szGift, const string szGiftXML, const string giftId,GiftEffectCallback callback = NULL, bool bCache = true)
	{
		//string name_list = szGift.substr(szGift.size() - 7, szGift.size());
		auto flag = szGift.find("_AR", 0);
		if (flag != string::npos)
		{
			m_isUseNew = true;
			if (m_handleEffect == NULL)
			{
				m_handleEffect = ccEffectCreate();
			}

			if (ccEffectSetEffectZipAsyn(m_handleEffect, szGift.c_str(), giftId.c_str(), GiftEffectCallback_Func, this))
			{
				m_szGiftID = giftId;
				this->callback = callback;
			}
		}
		else
		{
			m_isUseNew = false;
			if (m_handleSticker == NULL)
			{
				m_handleSticker = ccStickerCreate();
			}

			if (ccStickerSetEffectZip_aysn(m_handleSticker, szGift.c_str(), szGiftXML.c_str(), GiftEffectCallback_Func, this))
			{
				m_szGiftID = giftId;
				this->callback = callback;
			}
		}

	}

	ID3D11ShaderResourceView *playEffect(ID3D11ShaderResourceView *pTexture, int width, int height, const ccFDFaceRes *faceRes)
	{
		if (m_isUseNew)
		{
			if (m_handleEffect == NULL)
			{
				m_handleEffect = ccEffectCreate();
			}
			return ccEffectProcess(m_handleEffect, pTexture, width, height, faceRes);
		}
		else
		{
			if (m_handleSticker == NULL)
			{
				m_handleSticker = ccStickerCreate();
			}
			return ccStickerProcessTexture(m_handleSticker, pTexture, width, height, faceRes);
		}
	
	}

	bool playEffectExt(ID3D11ShaderResourceView *pTexture, ID3D11Texture2D *pTargetTexture, int width, int height, const ccFDFaceRes *faceRes, void *pExtInfo)
	{
		if (m_isUseNew)
		{
			if (m_handleEffect == NULL)
			{
				m_handleEffect = ccEffectCreate();
			}
			if (pExtInfo!=NULL)
			{
				SMaskInfo *maskinfo = (SMaskInfo*)pExtInfo;
				ccEffectSetMask(m_handleEffect, maskinfo->pMask, maskinfo->maskW, maskinfo->maskH);
			}

			return ccEffectProcessTexture(m_handleEffect, pTexture, pTargetTexture, width, height, faceRes);
		}
		else
		{
			if (m_handleSticker == NULL)
			{
				m_handleSticker = ccStickerCreate();
			}
			return ccStickerProcessTextureExt(m_handleSticker, pTexture, pTargetTexture, width, height, faceRes, pExtInfo);
		}

	}

	void destoryEffect()
	{
		if (m_handleSticker != NULL)
		{
			ccStickerSetEffectZip_aysn(m_handleSticker, NULL, NULL, 0, 0);
			ccStickerSetEffectCacheSize(m_handleSticker, 0);
		}

		if (m_handleEffect != NULL)
		{
			ccEffectSetEffectZipSync(m_handleEffect, NULL, NULL, 0, 0);
		}
	}

	void setMaxCacheSize(int nSize)
	{
		if (m_handleSticker == 0)
		{
			m_handleSticker = ccStickerCreate();
		}

		ccStickerSetEffectCacheSize(m_handleSticker, nSize);
	}

	void proc_callback(int event)
	{
		if (event == EE_FINISHED)
		{
			if (callback != NULL)
			{
				(*callback)(m_szGiftID.c_str());
			}
		}
		else if (event == EE_START)
		{
			printf("start!\r\n");
		}
		else if(event == EE_LOOP)
		{
			printf("loop!\r\n");
		}
	}

	cc_handle_t GetHandle()
	{
		return m_handleEffect;
	}
private:
	cc_handle_t m_handleSticker;

	cc_handle_t m_handleEffect;

	GiftEffectCallback callback;
	string m_szGiftID;
	bool m_isUseNew = false;
};

void GiftEffectCallback_Func(int event, const void *lpParam)
{
	if (lpParam != NULL)
	{
		((FaceGiftEffectPlayer *)lpParam)->proc_callback(event);
	}
}

DX11IMAGEFILTER_EXPORTS_API void ccSetGiftResourcePath(const char *szPath)
{
	FGEPInst->SetGiftResourcePath(szPath);
}

DX11IMAGEFILTER_EXPORTS_API void ccStartGiftEffect(const char *szGift, const char *giftId,GiftEffectCallback callback, bool bCache)
{
	FGEPInst->setGiftEffect(szGift, "",giftId, callback, bCache);
}
DX11IMAGEFILTER_EXPORTS_API ID3D11ShaderResourceView *ccPlayGiftEffect(ID3D11ShaderResourceView *pTexture, int width, int height, const ccFDFaceRes *faceRes)
{
	return FGEPInst->playEffect(pTexture, width, height, faceRes);
}

DX11IMAGEFILTER_EXPORTS_API bool ccPlayGiftEffectExt(ID3D11ShaderResourceView *pTexture, ID3D11Texture2D *pTargetTexture, int width, int height, const ccFDFaceRes *faceRes)
{
	unsigned long time = GetTickCount();
	bool bRes = FGEPInst->playEffectExt(pTexture, pTargetTexture, width, height, faceRes, NULL);
	//Sleep(3000);
	time = GetTickCount() - time;
	if (time > 2000)
	{
		FILE *file = fopen("facelog.info", "at");
		if (file != NULL)
		{
			fprintf(file, "playeffect:%d\r\n", time);
			fclose(file);
		}
	}

	return bRes;
}

DX11IMAGEFILTER_EXPORTS_API bool ccPlayGiftEffectExt2(ID3D11ShaderResourceView *pTexture, ID3D11Texture2D *pTargetTexture, int width, int height, const ccFDFaceRes *faceRes, void *pExtInfo)
{
	unsigned long time = GetTickCount();
	bool bRes = FGEPInst->playEffectExt(pTexture, pTargetTexture, width, height, faceRes, pExtInfo);
	//Sleep(3000);
	time = GetTickCount() - time;
	if (time > 2000)
	{
		FILE *file = fopen("facelog.info", "at");
		if (file != NULL)
		{
			fprintf(file, "playeffect:%d\r\n", time);
			fclose(file);
		}
	}

	return bRes;
}

void ccSetBGRAGiftEffect(unsigned char * pBGRA)
{
	FGEPInst->SetBGRA(pBGRA);
}

DX11IMAGEFILTER_EXPORTS_API void ccDestoryGiftEffect()
{
	FGEPInst->destoryEffect();
}

DX11IMAGEFILTER_EXPORTS_API void ccSetGiftEffectCacheSize(int nSize)
{
	FGEPInst->setMaxCacheSize(nSize);
}

DX11IMAGEFILTER_EXPORTS_API void ccDestoryGiftInst()
{
	delete FGEPInst;
}

DX11IMAGEFILTER_EXPORTS_API void ccEffectSetGiftAnchor(unsigned char * data, int width, int height, AnchorType type)
{
	if (FGEPInst->GetHandle())
	{
		ccEffectSetMask(FGEPInst->GetHandle(),data, width, height,UNKNOW_EFFECT,type);
	}
}

DX11IMAGEFILTER_EXPORTS_API void ccEffectSetBodyPointOld(ccBodyRes* bodyRes)
{
	if (FGEPInst->GetHandle())
	{
		ccEffectSetBodyPoint(FGEPInst->GetHandle(), bodyRes);
	}
	
}

DX11IMAGEFILTER_EXPORTS_API void ccEffectSetSplitScreenOld(int SplitScreen, CCEffectType type)
{
	if (FGEPInst->GetHandle())
	{
		ccEffectSetSplitScreen(FGEPInst->GetHandle(), SplitScreen, type);
	}
}
