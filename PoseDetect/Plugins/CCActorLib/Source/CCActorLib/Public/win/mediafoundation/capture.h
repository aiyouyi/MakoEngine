#pragma once

#include "core/inc.h"
#include "core/vec2.h"
#include "core/event.h"

struct IMFAttributes;

struct  video_capture_device_desc
{
	std::string name;
	std::string deviceid;
};

struct  video_capture_mode
{
	core::pixel_format formmat = core::pixel_format::none;
	core::vec2i size;
	core::vec2i framerate = { 1, 1 };
};

struct  audio_capture_mode
{
	core::sample_format format = core::sample_format::none;
	int32_t sample_rate = 0;
	int32_t channels = 0;
};

class  video_capture_source
{
public:
	virtual ~video_capture_source() {}
	virtual core::error_e start(const video_capture_mode & mode) = 0;
	virtual core::error_e stop() = 0;
	virtual const video_capture_mode& currentMode() = 0;

public:
	// 原始数据是从下往上的扫描线，data 指向最后一行（第一个扫描线）
	core::event<void(const byte_t * scanline0, int32_t stride, const video_capture_mode & mode)> sampled;
};

class  audio_capture_source
{
public:
	virtual ~audio_capture_source() {}
	virtual core::error_e start(const audio_capture_mode & mode) = 0;
	virtual core::error_e stop() = 0;
	virtual const audio_capture_mode & currentMode() = 0;

public:
	// 音频的 stride 就是该 sample 的总长度（字节）
	core::event<void(const byte_t * scanline0, int32_t stride, const audio_capture_mode & mode)> sampled;
};

std::vector<video_capture_device_desc>  video_devices();
std::vector<video_capture_mode>  video_device_modes(std::string deviceid);
std::shared_ptr<video_capture_source>  create_video_source(std::string deviceid);

std::vector<video_capture_device_desc>  audio_devices();
std::vector<audio_capture_mode>  audio_device_modes(std::string deviceid);
std::shared_ptr<audio_capture_source>  create_audio_source(std::string deviceid);

class  video_device
{
public:
	video_device() {}
	~video_device() {}

	static video_device& getInstance();

	void addCameraDeviceDesc(const video_capture_device_desc& desc);
	std::vector<video_capture_device_desc> devices() const;
	bool empty() const;
	void clear();
private:
	std::vector<video_capture_device_desc> _cameraList;
};

