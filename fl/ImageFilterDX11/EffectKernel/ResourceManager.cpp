#include "ResourceManager.h"
#include "../Toolbox/stb_image.h"
#include "DDS/DDSTextureLoader.h"
#ifdef _WIN32
#include "windows.h"
#endif


ResourceManager::ResourceManager()
{

}

ResourceManager::~ResourceManager()
{
	SAFE_DELETE(m_texpool);
}

int ResourceManager::Init()
{
	if (!m_texpool)	m_texpool = new DXTexturePool;
	return 1;
}

long long ResourceManager::generateID()
{
	m_id++;
	return m_id;
}

long long ResourceManager::loadImage(ImageInfo& info) {
	Image* image = new Image;
	image->info = info;

	int width, height, channel;
	unsigned char* data = stbi_load((info.dir + std::string("/") + info.relative_filepath).c_str(), &width, &height, &channel, STBI_rgb_alpha);
	DX11Texture* tex = m_texpool->fetchDXTexture2D((char*)data, width, height);
	stbi_image_free(data);
	image->tex = tex;

	image->info.width = width;
	image->info.height = height;
	image->info.channel = channel;

	long long id = generateID();
	image->id = id;

	m_images[id] = image;
	return id;
}

long long ResourceManager::loadImageFromZip(ImageInfo& info, HZIP hZip)
{
	int index;
	ZIPENTRY ze;
	long long id = -1;
	if (FindZipItem(hZip, info.relative_filepath.c_str(), true, &index, &ze) == ZR_OK)
	{
		char *pDataBuffer = new char[ze.unc_size];
		ZRESULT res = UnzipItem(hZip, index, pDataBuffer, ze.unc_size);
		if (res == ZR_OK)
		{
			Image* image = new Image;
			image->info = info;

			const char *str = info.relative_filepath.c_str();
			if (strcmp(str + strlen(str) - 4, ".dds") == 0)
			{
				DX11Texture* tex = new DX11Texture();

				SDDSInfo ddsInfo;
				CreateDDSTextureFromBuffer(DevicePtr, (BYTE*)pDataBuffer, ze.unc_size, &tex->m_texShaderView, &ddsInfo);

				tex->m_nWidth = ddsInfo.iWidth;
				tex->m_nHeight = ddsInfo.iHeight;
				tex->m_bGenMipmap = ddsInfo.iMipCount > 1;

				image->info.width = ddsInfo.iWidth;
				image->info.height = ddsInfo.iHeight;
				image->info.channel = ddsInfo.iMipCount;

				id = generateID();
				image->id = id;

				m_images[id] = image;

				image->tex = tex;
			}
			else
			{
				int texW, texH, nChannel;
				//unsigned char *pImageDataSrc = ccLoadImageFromBuffer((unsigned char *)pDataBuffer, ze.unc_size, &texW, &texH, &nChannel, 0);
				unsigned char *pImageDataSrc = stbi_load_from_memory((unsigned char *)pDataBuffer, ze.unc_size, &texW, &texH, &nChannel, 0);
				if (pImageDataSrc != NULL && texW*texH > 0)
				{

					image->info.width = texW;
					image->info.height = texH;
					image->info.channel = nChannel;

					id = generateID();
					image->id = id;

					m_images[id] = image;
					//转换成4通道.
					if (nChannel == 3)
					{
						unsigned char* pImageDataDest = new unsigned char[texW*texH * 4];
						unsigned char *pImageSourceTemp = pImageDataSrc;
						unsigned char *pImageDestTemp = pImageDataDest;
						for (int r = 0; r < texH; ++r)
						{
							for (int c = 0; c < texW; ++c)
							{
								pImageDestTemp[0] = pImageSourceTemp[0];
								pImageDestTemp[1] = pImageSourceTemp[1];
								pImageDestTemp[2] = pImageSourceTemp[2];
								pImageDestTemp[3] = 255;

								pImageSourceTemp += 3;
								pImageDestTemp += 4;
							}
						}
						DX11Texture* tex = m_texpool->fetchDXTexture2D((char*)pImageDataDest, texW, texH);
						stbi_image_free(pImageDataSrc);
						delete[]pImageDataDest;

						image->tex = tex;
					}
					else
					{
						DX11Texture* tex = m_texpool->fetchDXTexture2D((char*)pImageDataSrc, texW, texH);
						stbi_image_free(pImageDataSrc);
						image->tex = tex;
					}
					
				}
			}
		}
		delete[]pDataBuffer;
		
	}
	return id;
}

Image* ResourceManager::getImage(long long image_id)
{
	auto iter = m_images.find(image_id);
	if (iter != m_images.end())
	{
		return iter->second;
	}
	return NULL;
}

void ResourceManager::freeImage(long long image_id)
{
	auto iter = m_images.find(image_id);
	if (iter != m_images.end())
	{
		Image* image = iter->second;
		if (NULL != image)
		{
			if (NULL != image->tex)
			{
				m_texpool->returnAndRemoveDXTexture2D(image->tex);
				delete image->tex;
			}
			delete image;
		}
		m_images.erase(iter);
	}
}

void ResourceManager::saveImage(long long image_id)
{
	//TODO
}

long long ResourceManager::loadAnim(AnimInfo& info)
{
	Anim* anim = new Anim;
	anim->info = info;

	for (auto relative_filename : info.relative_filename_list)
	{
		ImageInfo image_info;
		image_info.dir = info.dir;
		image_info.relative_filepath = relative_filename;
		long long img_id = loadImage(image_info);

		anim->images.push_back(img_id);
	}
	long long id = generateID();
	anim->id = id;
	m_anims[id] = anim;
	return anim->id;
}

long long ResourceManager::loadAnimFromZip(AnimInfo& info, HZIP hZip)
{
	Anim* anim = new Anim;
	anim->info = info;

	for (auto relative_filename : info.relative_filename_list)
	{
		ImageInfo image_info;
		image_info.dir = info.dir;
		image_info.relative_filepath = relative_filename;
		long long img_id = loadImageFromZip(image_info, hZip);

		anim->images.push_back(img_id);
	}
	long long id = generateID();
	anim->id = id;
	m_anims[id] = anim;
	return anim->id;
}

Anim* ResourceManager::getAnim(long long anim_id)
{
	auto anim_iter = m_anims.find(anim_id);
	if (anim_iter != m_anims.end())
	{
		Anim* anim = anim_iter->second;
		return anim;
	}
	else
	{
		return NULL;
	}
}

void ResourceManager::SetAnimFps(long long anim_id, float fps)
{
	auto anim_iter = m_anims.find(anim_id);
	if (anim_iter != m_anims.end())
	{
		Anim* anim = anim_iter->second;

		anim->info.fps = fps;
	}
}

bool ResourceManager::SetAnimCrop(long long anim_id)
{
	Image* img = getAnimFrame(anim_id, 0);
	if (img != NULL)
	{
		float ratio = (float)img->info.width / img->info.height;
		if (ratio == 930.0 / 1240)
		{
			return false;
		}
	}
	return true;
}

Image* ResourceManager::getAnimFrame(long long anim_id, int frame_index)
{
	auto anim_iter = m_anims.find(anim_id);
	if (anim_iter != m_anims.end())
	{
		Anim* anim = anim_iter->second;
		int total_frame_num = anim->images.size();
		if (frame_index < total_frame_num)
		{
			return getImage(anim->images[frame_index]);
		}
		else
		{
			return NULL;
		}
	}
	else
	{
		return NULL;
	}
}

Image* ResourceManager::getAnimFrame(long long anim_id, float time)
{
	auto anim_iter = m_anims.find(anim_id);
	if (anim_iter != m_anims.end())
	{
		Anim* anim = anim_iter->second;
		// image
		float during = 0.0;
		int frame_index = 0;
		// anim
		if (anim->info.fps > 0.00005)
		{
			during = 1000.0 / anim->info.fps;
			frame_index = int(time / during);
		}

		int total_frame_num = anim->images.size();
		return getImage(anim->images[frame_index%total_frame_num]);
	}
	else
	{
		return NULL;
	}
}

void ResourceManager::freeAnim(long long anim_id)
{
	auto anim_iter = m_anims.find(anim_id);
	if (anim_iter != m_anims.end())
	{
		Anim* anim = anim_iter->second;
		for (auto image : anim->images)
		{
			freeImage(image);
		}
		delete anim;
		m_anims.erase(anim_iter);
	}
}


long long ResourceManager::loadVideo(VideoInfo& info)
{

}

long long ResourceManager::loadVideoFromZip(VideoInfo& info, HZIP hZip)
{
	int index;
	ZIPENTRY ze;
	long long video_id = -1;
	if (FindZipItem( hZip, info.relative_filepath.c_str(), true, &index, &ze ) == ZR_OK)
	{
		char *pDataBuffer = new char[ze.unc_size];
		ZRESULT res = UnzipItem(hZip, index, pDataBuffer, ze.unc_size);
		if (res == ZR_OK)
		{
			Video* vdo = new Video();
			

			std::shared_ptr<VideoCaptureMapper> video_mapper = std::make_shared<VideoCaptureFFmpeg>();
			if (video_mapper->OpenFromMemory(pDataBuffer, ze.unc_size) != -1)
			{
				vdo->video_mapper = video_mapper;
				video_id = generateID();
				vdo->id = video_id;
				m_videos[video_id] = vdo;
				info.width = video_mapper->Width();
				info.height = video_mapper->Height();
				info.fps = video_mapper->GetFps();
				info.duration = video_mapper->GetDuration();

				Image* curFrame = new Image();
				curFrame->tex = new DX11Texture();
				curFrame->tex->initTexture(DXGI_FORMAT_B8G8R8A8_UNORM, D3D11_BIND_SHADER_RESOURCE, info.width * 0.5, info.height);
				vdo->current_frame = curFrame;
				vdo->info = info;
				vdo->video_mapper->Start();
			}
		}

		SAFE_DELETE_ARRAY(pDataBuffer);
	}

	return video_id;
}

Image* ResourceManager::getVideoFrame(long long video_id, double time)
{
	auto video_itr = m_videos.find(video_id);
	if (video_itr != m_videos.end())
	{
		Video* vdo = video_itr->second;
		if (vdo->bRestart)
		{
			vdo->video_mapper->Start();
			vdo->bRestart = false;
			vdo->cur_frame_idx = 0;
			return vdo->current_frame;
		}

		float during = 0.0;
		int frame_index = 0;
		during = 1000.0 / vdo->info.fps;
		frame_index = floor((time / during) + 0.5f);
		frame_index = frame_index % (int)(vdo->info.fps * vdo->info.duration / 1000);

		bool ReStart = frame_index < vdo->cur_frame_idx;
		if (ReStart)
		{
			vdo->bRestart = ReStart;
		}
		unsigned char* pData = nullptr;
		if (time == 0.0)
		{
			pData = vdo->video_mapper->GetNextFrame();
		}
		else
		{
			for (int ni = vdo->cur_frame_idx; ni < frame_index; ni++)
			{
				pData = vdo->video_mapper->GetNextFrame();
			}
		}

		vdo->cur_frame_idx = frame_index;
		if (pData)
		{
			int width = vdo->video_mapper->Width();
			int height = vdo->video_mapper->Height();

			//ccSavePng("D:/abb.png", pData, width, height, 4);
			unsigned char* new_buffer = (unsigned char*)malloc(sizeof(unsigned char) * width * 0.5 * height * 4);
			int new_width = width * 0.5 * 4;
			int new_height = height;

			unsigned char* src_ptr = pData;
			unsigned char* src_half_ptr = pData + new_width;
			unsigned char* dest = new_buffer;

			for (int i = 0; i < height; i++)
			{
				for (int j = 0; j < new_width; j+=4)
				{
					*dest++ = *src_ptr++;
					*dest++ = *src_ptr++;
					*dest++ = *src_ptr++;
					*dest++ = *src_half_ptr;
					src_ptr++;
					src_half_ptr = src_half_ptr + 4;
	
				}
				src_half_ptr = src_half_ptr + new_width;
				src_ptr = src_ptr + new_width;
			}

			vdo->current_frame->tex->updateTextureInfo(new_buffer, new_width, height);
			delete[] new_buffer;
			new_buffer = nullptr;

			return vdo->current_frame;
		}
		else
		{
			return vdo->current_frame;
		}


	}
	return nullptr;
}

void ResourceManager::freeVideo(long long video_id)
{
	auto video_itr = m_videos.find(video_id);
	if (video_itr != m_videos.end())
	{
		auto vdo = video_itr->second;
		SAFE_DELETE(vdo->current_frame->tex);
		SAFE_DELETE(vdo->current_frame);
		SAFE_DELETE(video_itr->second);
		m_videos.erase(video_itr);
	}
}

Image* ResourceManager::getImageCommon(long long id, float time)
{
	Image* img_ret = nullptr;

	if ((img_ret = getImage(id)) != nullptr)
	{
		return img_ret;
	}

	if ((img_ret = getAnimFrame(id, time)) != nullptr)
	{
		return img_ret;
	}

	if ((img_ret = getVideoFrame(id, time)) != nullptr)
	{
		return img_ret;
	}

	return img_ret;
}

void ResourceManager::freeMaterial(long long id)
{
	freeImage(id);
	freeAnim(id);
	freeVideo(id);
}

DX11Texture* ResourceManager::GetTexFromFile(std::string abs_filename)
{
	return NULL;
}
