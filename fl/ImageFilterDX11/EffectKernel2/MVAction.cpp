#include "MVAction.h"
#include "Toolbox/Helper.h"
#include <fstream>  
#include <string>
#include "Toolbox/DXUtils/DX11Context.h"

using namespace std;
#define VALUE_STEP 7
SLinearData::SLinearData()
{
	m_vValue = NULL;
	m_vTime = NULL;
	m_vDivide = NULL;
	m_fps = 30;
	m_nPts = 0;
}

bool SLinearData::load(const char *szAction)
{
	bool bRes = false;
	ifstream in(szAction);
	string line;
	if (in) // 有该文件  
	{
		try
		{
			int nKeys = 0;
			int nfps = 0;
			int w = 1920;
			int h = 1080;
			getline(in, line);
			sscanf(line.c_str(), "%d,%d,%d,%d", &nKeys, &nfps, &w, &h);

			if (nKeys <= 0)
			{
				in.close();
				return false;
			}

			int nLastDivideC = 0;
			int nLastDivideR = 0;

			int nDivideR = 0;
			vector<float> arrTime;
			vector<float> arrValue;
			int index = 0;
			while (getline(in, line)) // line中不包括每行的换行符  
			{
				if (line.length() == 0)
				{
					continue;
				}

				float frame;
				vec2 vTranslate;
				vec2 vScale;
				float fRotate;
				int nDivideC = nLastDivideC;
				int nDivideR = nLastDivideR;
				sscanf(line.c_str(), "%f,%f,%f,%f,%f,%f,%d,%d", &frame, &vTranslate.x, &vTranslate.y, &vScale.x, &vScale.y, &fRotate, &nDivideR, &nDivideC);
				if (nDivideC > 0 || nDivideR > 0)
				{
					nLastDivideC = nDivideC;
					nLastDivideR = nDivideR;
				}

				arrTime.push_back(frame);
				arrValue.push_back((vTranslate.x - w*0.5f) / min(w, h));
				arrValue.push_back((vTranslate.y - h*0.5f) / min(w, h));
				arrValue.push_back(vScale.x);
				arrValue.push_back(vScale.y);
				arrValue.push_back(fRotate);
				arrValue.push_back(nDivideR);
				arrValue.push_back(nDivideC);
				index++;
			}

			load(arrTime, arrValue, nKeys, nfps);
			in.close();
			bRes = true;
		}
		catch (...)
		{
			destory();
			bRes = false;
		}
	}
	return bRes;
}

bool SLinearData::loadFromBuffer(char *pDataBuffer, int nBufferSize)
{
	int nKeys = 0;
	int nfps = 0;
	int w = 1920;
	int h = 1080;

	int nDivideR = 0;
	vector<float> arrTime;
	vector<float> arrValue;
	int index = 0;

	char szLine[256];
	int off = 0;
	int rPos = 0;
	string strData = pDataBuffer;
	int iLine = 0;

	int nLastDivideC = 0;
	int nLastDivideR = 0;
	while (true)
	{
		rPos = strData.find("\r\n", off);
		if (rPos == strData.npos)
		{
			if (off >= nBufferSize)
			{
				break;
			}
			else
			{
				rPos = nBufferSize;
			}
		}
		

		char *pDataT = pDataBuffer + off;
		strncpy(szLine, pDataT, rPos - off);
		szLine[rPos - off] = 0;
		if (strlen(szLine) > 0)
		{
			printf("%s\r\n", szLine);
			if (iLine == 0)
			{
				sscanf(szLine, "%d,%d,%d,%d", &nKeys, &nfps, &w, &h);
			}
			else
			{
				int nDivideC = nLastDivideC;
				int nDivideR = nLastDivideR;
				float frame;
				vec2 vTranslate;
				vec2 vScale;
				float fRotate;
				sscanf(szLine, "%f,%f,%f,%f,%f,%f,%d,%d", &frame, &vTranslate.x, &vTranslate.y, &vScale.x, &vScale.y, &fRotate, &nDivideR, &nDivideC);
				if (nDivideC > 0 || nDivideR > 0)
				{
					nLastDivideC = nDivideC;
					nLastDivideR = nDivideR;
				}

				arrTime.push_back(frame);
				arrValue.push_back((vTranslate.x - w*0.5f) / min(w, h));
				arrValue.push_back((vTranslate.y - h*0.5f) / min(w, h));
				arrValue.push_back(vScale.x);
				arrValue.push_back(vScale.y);
				arrValue.push_back(fRotate);
				arrValue.push_back(nDivideR);
				arrValue.push_back(nDivideC);
				index++;
			}
			++iLine;
		}

		off = rPos + 2;
	}

	load(arrTime, arrValue, index, nfps);
	return true;
}

void SLinearData::load(vector<float> &arrTime, vector<float> &arrValue, int nKeys, int fps)
{
	m_vValue = new vec4[nKeys];
	m_vTime = new float[nKeys];
	m_vDivide = new int[nKeys*2];
	m_nPts = nKeys;
	for (int i = 0; i < nKeys; ++i)
	{
		m_vValue[i] = vec4(arrValue[VALUE_STEP * i], arrValue[VALUE_STEP * i + 1], arrValue[VALUE_STEP * i + 2], arrValue[VALUE_STEP * i + 4]);
		m_vTime[i] = arrTime[i];
		m_vDivide[2*i] = arrValue[VALUE_STEP * i + 5];
		m_vDivide[2*i+1] = arrValue[VALUE_STEP * i + 6];
		//printf("m_vTime[%d]:%f\r\n", i, arrTime[i]);
	}

	m_fps = fps;
}

bool SLinearData::eval(float fTime, vec4 &vTrans, int &nDivideR, int &nDivideC)
{
	//printf("fTime: %f\r\n", fTime);
	nDivideR = 1;
	nDivideC = 1;

	if (m_vValue == NULL || m_nPts == 0)
	{
		vTrans = vec4(0, 0, 100, 0);
		return false;
	}

	float fDrawTime = fTime * m_fps;
	if (fDrawTime < m_vTime[0])
	{
		vTrans = m_vValue[0];
		return true;
	}
	if (fDrawTime > m_vTime[m_nPts - 1])
	{
		vTrans = m_vValue[m_nPts - 1];

		return fDrawTime < m_vTime[m_nPts - 1]+0.2;
	}

	vec4 vRes = m_vValue[0];
	for (int i = 0; i < m_nPts - 1; ++i)
	{
		if (fDrawTime <= m_vTime[i + 1] && fDrawTime >= m_vTime[i])
		{
			float fWeight = (fDrawTime - m_vTime[i]) / (m_vTime[i + 1] - m_vTime[i]);
			vRes = m_vValue[i] * (1 - fWeight) + m_vValue[i + 1] * (fWeight);
			
			nDivideR = m_vDivide[2*i];
			nDivideC = m_vDivide[2 * i + 1];
			break;
		}
	}

	vTrans = vRes;

	return true;
}

void SLinearData::destory()
{
	if (m_vValue != NULL) 
	{
		delete[]m_vValue; 
		m_vValue= NULL;
	}

	if (m_vTime != NULL)
	{
		delete[]m_vTime;
		m_vTime = NULL;
	}

	if (m_vDivide != NULL)
	{
		delete[]m_vDivide;
		m_vDivide = NULL;
	}

	m_fps = 0;
	m_nPts = 0;
}

extern DX11Texture *CreateFromZIP(HZIP hZip, const char *szImagePath, bool bGenMipmap);

DWORD WINAPI MVThreadFun(LPVOID pM)
{
	SMVResourceAsyn *pResourceAsyn = (SMVResourceAsyn *)pM;

	const char *szPath = pResourceAsyn->szPath.c_str();
	const char *szXMLFile = pResourceAsyn->szXMLFile.c_str();

	char szFullFile[256];
	HZIP hZip = OpenZip(pResourceAsyn->szPath.c_str(), NULL);
	if (hZip != NULL)
	{
		int index;
		ZIPENTRY ze;
		if (ZR_OK == FindZipItem(hZip, pResourceAsyn->szXMLFile.c_str(), true, &index, &ze))
		{
			char *pDataBuffer = new char[ze.unc_size];
			ZRESULT res = UnzipItem(hZip, index, pDataBuffer, ze.unc_size);
			if (res != ZR_OK)
			{
				delete[]pDataBuffer;
				CloseZip(hZip);

				pResourceAsyn->status = SES_READY;
				return 0;
			}

			//解析xml
			XMLResults xResults;
			XMLNode nodeMV = XMLNode::parseBuffer(pDataBuffer, ze.unc_size, "MVEffect", &xResults);
			delete[]pDataBuffer;

			char szFullFile[256];
			sprintf(szFullFile, "%s/%s", szPath, szXMLFile);

			typedef map<string, Drawable *> DrawableMap_;
			DrawableMap_ m_mapImage;
			//载入动画drawable
			int i = -1;
			XMLNode nodeDrawable = nodeMV.getChildNode("anidrawable", ++i);
			while (!nodeDrawable.isEmpty())
			{
				const char *szDrawableName = nodeDrawable.getAttribute("name");
				AnimationDrawable *drawable = NULL;

				const char *szTarget = nodeDrawable.getAttribute("ref");
				if (szTarget != NULL)
				{
					map<string, Drawable *>::iterator it = m_mapImage.find(szTarget);
					if (it != m_mapImage.end())
					{
						AnimationDrawable *targetDrawable = (AnimationDrawable *)(it->second);
						drawable = (AnimationDrawable *)targetDrawable->Clone();
					}
				}
				if (drawable == NULL)
				{
					drawable = new AnimationDrawable();
				}

				const char *szOffset = nodeDrawable.getAttribute("offset");
				long offset = atol(szOffset);
				drawable->setOffset(offset);

				const char *szLoopMode = nodeDrawable.getAttribute("loopMode");
				if (szLoopMode != NULL && strcmp(szLoopMode, "oneShot") == 0)
				{
					drawable->setLoopMode(ELM_ONESHOT);
				}
				else
				{
					drawable->setLoopMode(ELM_REPEAT);
				}

				const char *szGenMipmap = nodeDrawable.getAttribute("genMipmaps");
				bool bGenMipmap = false;
				if (szGenMipmap != NULL && (strcmp(szGenMipmap, "yes") || strcmp(szGenMipmap, "YES")))
				{
					bGenMipmap = true;
				}

				const char *szItems = nodeDrawable.getAttribute("items");
				const char *szItemInfo = nodeDrawable.getAttribute("iteminfo");
				if (szItems != NULL && szItemInfo != NULL)
				{
					int iStart = 0;
					int nCount = 0;
					int nDuring = 0;
					char szImagePath[128];
					sscanf(szItemInfo, "%d,%d,%d", &iStart, &nCount, &nDuring);
					if (nCount > 0 && nDuring > 0)
					{
						for (; iStart <= nCount; iStart += 2)
						{
							sprintf(szImagePath, szItems, iStart);

							sprintf(szFullFile, "%s/%s", pResourceAsyn->szPath.c_str(), szImagePath);

							std::shared_ptr< CC3DTextureRHI> TexRHI = GetDynamicRHI()->FetchTexture(szFullFile, bGenMipmap);
							if (TexRHI == nullptr)
							{
								TexRHI = GetDynamicRHI()->CreateTextureFromZip(hZip, szImagePath, bGenMipmap);
								GetDynamicRHI()->RecoredTexture(szFullFile, TexRHI);
							}

							long during = nDuring;

							drawable->appandTex(during, TexRHI);
						}
					}
				}

				//解析动画信息
				int j = -1;
				XMLNode nodeItem = nodeDrawable.getChildNode("item", ++j);
				while (!nodeItem.isEmpty())
				{
					const char *szImagePath = nodeItem.getAttribute("image");
					sprintf(szFullFile, "%s/%s", pResourceAsyn->szPath.c_str(), szImagePath);

					std::shared_ptr< CC3DTextureRHI> TexRHI = GetDynamicRHI()->FetchTexture(szFullFile, bGenMipmap);
					if (TexRHI == nullptr)
					{
						TexRHI = GetDynamicRHI()->CreateTextureFromZip(hZip, szImagePath, bGenMipmap);
						GetDynamicRHI()->RecoredTexture(szFullFile, TexRHI);
					}

					const char *szDuring = nodeItem.getAttribute("duration");
					long during = atol(szDuring);

					drawable->appandTex(during, TexRHI);

					nodeItem = nodeDrawable.getChildNode("item", ++j);
				}

				m_mapImage.insert(std::make_pair(szDrawableName, drawable));

				nodeDrawable = nodeMV.getChildNode("anidrawable", ++i);
			}


			//解析加载2d矩形绘图
			i = -1;
			XMLNode nodeRect = nodeMV.getChildNode("rect", ++i);
			while (!nodeRect.isEmpty())
			{
				/*<rect name="cao" drawable="cao" x="100" y="100" width="100" height="100" alignType="EAPT_LT">
				</rect>
				*/
				Effect2DRect rectEffect;
				const char *szX = nodeRect.getAttribute("x");
				const char *szY = nodeRect.getAttribute("y");
				const char *szWidth = nodeRect.getAttribute("width");
				const char *szHeight = nodeRect.getAttribute("height");
				const char *szAlignType = nodeRect.getAttribute("alignType");

				int x = atoi(szX);
				int y = atoi(szY);
				int width = atoi(szWidth);
				int height = atoi(szHeight);
				int alignType = EAPT_CC;
				if (szAlignType != NULL)
				{
					std::string szArrAlignType[] = { "EAPT_LT", "EAPT_LB", "EAPT_RT", "EAPT_RB", "EAPT_CT", "EAPT_CB", "EAPT_LC", "EAPT_RC", "EAPT_CC" };
					for (int index = 0; index<EAPT_MAX; ++index)
					{
						if (strcmp(szAlignType, szArrAlignType[index].c_str()) == 0)
						{
							alignType = index;
							break;
						}
					}
				}

				rectEffect.setRect(x, y, width, height, alignType);

				const char *szDrawable = nodeRect.getAttribute("drawable");
				if (szDrawable != NULL && strlen(szDrawable) > 0)
				{
					map<std::string, Drawable *>::iterator it = m_mapImage.find(szDrawable);
					if (it != m_mapImage.end())
					{
						rectEffect.m_drawable = it->second->Clone();
					}
					else
					{
						rectEffect.m_drawable = NULL;
					}
				}

				pResourceAsyn->m_v2DEffectModel.push_back(rectEffect);
				nodeRect = nodeMV.getChildNode("rect", ++i);
			}

			DrawableMap_::iterator it = m_mapImage.begin();
			{
				while (it != m_mapImage.end())
				{
					if (it->second != NULL)
					{
						delete it->second;
					}
					++it;
				}
			}
			m_mapImage.clear();
		}
	}

	CloseZip(hZip);

	pResourceAsyn->status = SES_RUN;
	return 1;
}

MVAction::MVAction()
{
	m_mvResource.reset();

	m_pFBO = NULL;
	m_rectDraw = NULL;

	m_pTargetTexture = NULL;

	m_fStartTime = -1;

	m_hLoadThread = 0;
}

MVAction::~MVAction()
{
	destory();
}

bool MVAction::loadAction(const char *szAction)
{
	if (m_mvResource.status == SES_LOADING)return false;
	reset();
	destoryResource();

	//load mv.txt运镜数据
	if (szAction == NULL && strlen(szAction) <= 4)
	{
		return false;
	}

	bool bRes = false;
	if (strcmp(szAction + strlen(szAction) - 4, ".zip") == 0)
	{
		HZIP hZip = OpenZip(szAction, NULL);
		if (hZip != NULL)
		{
			int index;
			ZIPENTRY ze;
			if (ZR_OK == FindZipItem(hZip, "mv.txt", true, &index, &ze))
			{
				char *pDataBuffer = new char[ze.unc_size];
				
				ZRESULT res = UnzipItem(hZip, index, pDataBuffer, ze.unc_size);
				if (res == ZR_OK)
				{
					bRes = m_rotateCurve.loadFromBuffer(pDataBuffer, ze.unc_size);
				}

				delete[]pDataBuffer;
			}

			CloseZip(hZip);
		}

		//异步线程load蒙层效果
		m_mvResource.destory();
		m_mvResource.status = SES_LOADING;
		m_mvResource.szPath = szAction;
		m_mvResource.szXMLFile = "maskEffect.xml";
		m_hLoadThread = CreateThread(NULL, 0, MVThreadFun, &m_mvResource, 0/*CREATE_SUSPENDED*/, NULL);
	}
	else if(strcmp(szAction + strlen(szAction) - 4, ".txt") == 0)
	{
		bRes = m_rotateCurve.load(szAction);
	}

	return bRes;
}
#include "Toolbox/DXUtils/DX11Context.h"
bool MVAction::renderEffectToTexture(ID3D11ShaderResourceView *pInputTexture, ID3D11Texture2D *pTargetTexture, int width, int height)
{
	if (m_pFBO == NULL || m_pFBO->width() != width || m_pFBO->height() != height || m_pTargetTexture != pTargetTexture)
	{
		if (m_pFBO != NULL)
		{
			delete m_pFBO;
		}

		m_pFBO = new DX11FBO();
		m_pFBO->initWithTexture(width, height, false, pTargetTexture);

		m_pTargetTexture = pTargetTexture;
	}
	if (m_rectDraw == NULL)
	{
		m_rectDraw = new RectDraw();
		m_rectDraw->init(1, 1);
	}

	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)width;
	vp.Height = (FLOAT)height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	DeviceContextPtr->RSSetViewports(1, &vp);

	if (m_fStartTime <= 0)
	{
		m_fStartTime = GetTickCount()/1000.0f;
	}

	int nDivideR = 1;
	int nDivideC = 1;
	vec4 vRes;
	bool bRes = m_rotateCurve.eval(GetTickCount() / 1000.0f - m_fStartTime, vRes, nDivideR, nDivideC);

	ID3D11DepthStencilState *a = ContextInst->fetchDepthStencilState(false, false);
	ID3D11BlendState *b= ContextInst->fetchBlendState(false, false, true);
	float blendFactor[] = { 0.f,0.f,0.f,0.f };
	DeviceContextPtr->OMSetBlendState(b, blendFactor, 0xffffffff);
	DeviceContextPtr->OMSetDepthStencilState(a, 0);
	ContextInst->setCullMode(D3D11_CULL_NONE);
	m_pFBO->bind();
	//渲染背景
	m_rectDraw->setShaderTextureView(pInputTexture);
	//m_rectDraw->render();
	if (nDivideR*nDivideC <= 1)
	{
		m_rectDraw->render(vec2(vRes.x / width*height, -vRes.y), vec2(vRes.z / 100, vRes.z / 100), -vRes.w, width, height);
		if (m_mvResource.status == SES_RUN)
		{
			for (int i = 0; i < m_mvResource.m_v2DEffectModel.size(); ++i)
			{
				m_mvResource.m_v2DEffectModel[i].render(width, height, (GetTickCount() - m_fStartTime*1000));
			}
		}
	}
	else
	{
		float xStep = 1.0f*width / nDivideC;
		float yStep = 1.0f*height / nDivideR;
		for (int r = 0; r < nDivideR; ++r)
		{
			for (int c = 0; c < nDivideC; ++c)
			{
				D3D11_VIEWPORT vp;
				vp.Width = xStep;
				vp.Height = yStep;
				vp.MinDepth = 0.0f;
				vp.MaxDepth = 1.0f;
				vp.TopLeftX = xStep*c;
				vp.TopLeftY = yStep*r;
				DeviceContextPtr->RSSetViewports(1, &vp);
				m_rectDraw->render(vec2(vRes.x / width*height, -vRes.y), vec2(vRes.z / 100, vRes.z / 100), -vRes.w, width, height);

				if (m_mvResource.status == SES_RUN)
				{
					for (int i = 0; i < m_mvResource.m_v2DEffectModel.size(); ++i)
					{
						m_mvResource.m_v2DEffectModel[i].render(width, height, (GetTickCount() - m_fStartTime * 1000));
					}
				}
			}
		}

		
		D3D11_VIEWPORT vp;
		vp.Width = width;
		vp.Height = height;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		DeviceContextPtr->RSSetViewports(1, &vp);
	}
	
	return bRes;
}

void MVAction::reset()
{
	m_fStartTime = -1;
}

void MVAction::destory()
{
	if (m_hLoadThread != NULL)
	{
		WaitForSingleObject(m_hLoadThread, 5000);
		::CloseHandle(m_hLoadThread);
		m_hLoadThread = 0;
	}

	destoryResource();

	SAFEDEL(m_pFBO);
	SAFEDEL(m_rectDraw);

	m_pTargetTexture = NULL;
}

void MVAction::destoryResource()
{
	m_rotateCurve.destory();
	m_mvResource.destory();
}