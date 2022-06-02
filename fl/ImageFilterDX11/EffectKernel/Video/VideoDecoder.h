#pragma once

#ifndef _WIN64
#ifdef __cplusplus
extern "C" {
#endif
#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavutil/channel_layout.h>
#include <libavutil/common.h>
#include <libavutil/imgutils.h>
#include <libavutil/mathematics.h>
#include <libavutil/samplefmt.h>
#include <libswresample/swresample.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/audio_fifo.h>
#ifdef __cplusplus
}
#endif

class CVideoDecoder
{
public:
	CVideoDecoder();
	~CVideoDecoder();
	int Open(const char* file);
	int OpenFromMemory(char* buffer, size_t buffer_size);
	int Start();
	int Width();
	int Height();
	int GetRotate();
	unsigned char* GetNextFrame();
	unsigned char* GetFrameByTimeStamp(double timeStamp);
	AVFrame* GetYuvFrame();
	double GetTimeStamp();
	double GetFps() noexcept;
	double GetDuration() noexcept;
	int Close();
private:
	AVFormatContext* m_pFormatContext;
	AVStream *m_pVideoStream;
	AVStream *m_pAudioStream;
	AVCodecContext *m_pVideoCodec;
	AVCodecContext *m_pAudioCodec;
	//src decode frame
	AVFrame* m_pSrcFrame;
	//
	AVFrame* m_pRotateYuvFrame;
	//for save rgba frame
	AVFrame* m_pRGBFrame;
	AVFrame* m_ReturnRefYUV;
	//for save frame.
	struct SwsContext* rgb_sws_ctx;
	//init from memory
	AVIOContext *avio_ctx = nullptr;
	uint8_t* avio_ctx_buffer = nullptr;
	//video stream index
	int m_video_stream_idx;
	//audio stream index
	int m_audio_stream_idx;
	int m_VideoWidth;
	int m_VideoHeight;
	//
	int m_DstVideoWidth;
	int m_DstVideoHeight;
	//rotate;
	int m_Rotate;
	double m_TimeStamp;
};

#endif