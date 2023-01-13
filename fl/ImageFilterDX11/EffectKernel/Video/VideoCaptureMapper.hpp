#ifndef _VIDEO_CAPTURE_MAPPER_H_
#define _VIDEO_CAPTURE_MAPPER_H_

#include <memory>
#include "EffectKernel/Video/VideoDecoder.h"


class VideoCaptureMapper
{
public:
	VideoCaptureMapper() {  }
	virtual ~VideoCaptureMapper() {}

	virtual int Open(const char* file) = 0;
	virtual int OpenFromMemory(char* buffer, size_t buffer_size) = 0;
	virtual int Start() = 0;
	virtual int Width() = 0;
	virtual int Height() = 0;
	virtual unsigned char* GetNextFrame() = 0;
	virtual unsigned char* GetFrameByTimeStamp(double timeStamp) = 0;
	virtual double GetTimeStamp() = 0;
	virtual double GetFps() = 0;
	virtual double GetDuration() = 0;
	virtual int Close() = 0;
};


#ifndef _WIN64

class VideoCaptureFFmpeg : public VideoCaptureMapper
{
public:
	VideoCaptureFFmpeg() : VideoCaptureMapper()
	{
		m_decoder.reset(new CVideoDecoder());
	}
	virtual ~VideoCaptureFFmpeg()
	{
		Close();
	}

	virtual int Open(const char* file) override { return m_decoder->Open(file); }
	virtual int OpenFromMemory(char* buffer, size_t buffer_size) override { return m_decoder->OpenFromMemory(buffer, buffer_size); }
	virtual int Start() override { return m_decoder->Start(); }
	virtual int Width() override { return m_decoder->Width(); }
	virtual int Height() override { return m_decoder->Height(); }
	virtual unsigned char* GetNextFrame() override { return m_decoder->GetNextFrame(); }
	virtual unsigned char* GetFrameByTimeStamp(double timeStamp) override { return m_decoder->GetFrameByTimeStamp(timeStamp); }
	virtual double GetTimeStamp() override { return m_decoder->GetTimeStamp(); }
	virtual double GetFps() override { return m_decoder->GetFps(); }
	virtual double GetDuration() override { return m_decoder->GetDuration(); }
	virtual int Close() override { return m_decoder->Close(); }

protected:
	std::shared_ptr<CVideoDecoder> m_decoder;
};

#else
class VideoCaptureFFmpeg : public VideoCaptureMapper
{
public:
	VideoCaptureFFmpeg() : VideoCaptureMapper()
	{
	}
	virtual ~VideoCaptureFFmpeg()
	{
		Close();
	}

	virtual int Open(const char* file) override { return 0; }
	virtual int OpenFromMemory(char* buffer, size_t buffer_size) override { return 0; }
	virtual int Start() override { return 0; }
	virtual int Width() override { return 0; }
	virtual int Height() override { return 0; }
	virtual unsigned char* GetNextFrame() override { return NULL; }
	virtual unsigned char* GetFrameByTimeStamp(double timeStamp) override { return NULL; }
	virtual double GetTimeStamp() override { return 0; }
	virtual double GetFps() override { return 0; }
	virtual double GetDuration() override { return 0; }
	virtual int Close() override { return 0; }

protected:
};


#endif

#endif