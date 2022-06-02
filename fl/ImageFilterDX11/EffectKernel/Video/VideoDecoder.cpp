#ifndef _WIN64
#include "VideoDecoder.h"
#include "libyuv.h"
#include "BaseDefine/Define.h"

#undef makeErrorStr
static char errorStr[AV_ERROR_MAX_STRING_SIZE];
#define makeErrorStr(errorCode) av_make_error_string(errorStr, AV_ERROR_MAX_STRING_SIZE, errorCode)

#ifndef SAFE_DELETE
#define SAFE_DELETE(p) if(p){delete (p); (p) = NULL;}
#endif

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) if(p){delete [] (p); (p) = NULL;}
#endif

static void my_ffmpeg_log_callback(void* ptr, int level, const char* fmt, va_list vl)
{
	char res[1024];
	vsprintf_s(res, fmt, vl);
	switch (level)
	{
	case AV_LOG_ERROR:
		break;
	default:
		//LOGI("%s", res);
		break;
	}
}


static int open_codec_context(int *stream_idx, AVFormatContext *fmt_ctx, AVCodecContext **dec_ctx, enum AVMediaType type)
{
	int ret;
	AVStream *st;
	AVCodec *dec = NULL;
	*stream_idx = -1;
	if ((ret = av_find_best_stream(fmt_ctx, type, -1, -1, NULL, 0)) < 0)
	{
		LOGE("Could not find %s stream !(%s)\n",
			av_get_media_type_string(type), makeErrorStr(ret));
		return ret;
	}
	else
	{
		*stream_idx = ret;
		st = fmt_ctx->streams[*stream_idx];
		/* find decoder for the stream */
		*dec_ctx = st->codec;
		dec = avcodec_find_decoder((*dec_ctx)->codec_id);
		if (!dec)
		{
			LOGE("Failed to find %s codec(%s)\n",
				av_get_media_type_string(type), makeErrorStr(ret));
			return ret;
		}
		if ((ret = avcodec_open2(*dec_ctx, dec, NULL)) < 0)
		{
			LOGE("Failed to open %s codec(%s)\n",
				av_get_media_type_string(type), makeErrorStr(ret));
			return ret;
		}
	}
	return 0;
}


CVideoDecoder::CVideoDecoder()
{
	m_pFormatContext = NULL;
	m_pVideoStream = NULL;
	m_pAudioStream = NULL;
	m_pVideoCodec = NULL;
	m_pAudioCodec = NULL;
	m_pSrcFrame = NULL;
	m_pRGBFrame = NULL;
	m_pRotateYuvFrame = NULL;
	rgb_sws_ctx = NULL;
	m_ReturnRefYUV = NULL;
}

CVideoDecoder::~CVideoDecoder()
{
	this->Close();
}

int CVideoDecoder::Open(const char* file)
{
	int ret = -1;
	av_register_all();
	avcodec_register_all();
	av_log_set_callback(my_ffmpeg_log_callback);

	if (m_pFormatContext)
	{
		avformat_close_input(&m_pFormatContext);
		m_pFormatContext = NULL;
	}


	if ((ret = avformat_open_input(&m_pFormatContext, file, NULL, NULL)) < 0)
	{
		LOGE("Error: Could not open %s (%s)\n", file, makeErrorStr(ret));
		avformat_close_input(&m_pFormatContext);
		m_pFormatContext = NULL;
		return -1;
	}

	/* 获取音视频流信息 */
	if ((ret = avformat_find_stream_info(m_pFormatContext, NULL)) < 0)
	{
		LOGE("Could not find stream information (%s)\n", makeErrorStr(ret));
		avformat_close_input(&m_pFormatContext);
		m_pFormatContext = NULL;
		return -1;
	}

	//open codec context.
	if ((ret = open_codec_context(&m_video_stream_idx, m_pFormatContext, &m_pVideoCodec, AVMEDIA_TYPE_VIDEO)) < 0)
	{
		LOGE("No exit video.\n");
		avformat_close_input(&m_pFormatContext);
		m_pFormatContext = NULL;
		return -2;
	}

	// 	if (m_pVideoCodec->pix_fmt != AV_PIX_FMT_YUV420P && m_pVideoCodec->pix_fmt != AV_PIX_FMT_YUVJ420P)
	// 	{
	// 		LOGE("Unsupport format(%d).\n", m_pVideoCodec->pix_fmt);
	// 		return -3;
	// 	}

	if (m_video_stream_idx >= 0)
	{
		m_pVideoStream = m_pFormatContext->streams[m_video_stream_idx];
		if (m_pVideoCodec->pix_fmt == AV_PIX_FMT_NONE)
		{
			LOGE("set video format. \n");
			m_pVideoCodec->pix_fmt = AV_PIX_FMT_YUV420P;
		}
	}

	AVDictionaryEntry *tag = NULL;
	tag = av_dict_get(m_pVideoStream->metadata, "rotate", tag, 0);
	if (tag == NULL)
	{
		m_Rotate = 0;
	}
	else
	{
		int angle = atoi(tag->value);
		angle %= 360;
		m_Rotate = angle;
	}


	m_DstVideoWidth = m_VideoWidth = m_pVideoCodec->width;
	m_DstVideoHeight = m_VideoHeight = m_pVideoCodec->height;

	if (m_Rotate == 90 || m_Rotate == 270)
	{
		m_DstVideoWidth = m_VideoHeight;
		m_DstVideoHeight = m_VideoWidth;
	}

	rgb_sws_ctx = sws_getContext(
		m_DstVideoWidth,
		m_DstVideoHeight,
		m_pVideoCodec->pix_fmt,
		m_DstVideoWidth,
		m_DstVideoHeight,
		AV_PIX_FMT_BGRA,
		SWS_AREA, NULL, NULL, NULL);


	if (m_pSrcFrame)
	{
		av_frame_free(&m_pSrcFrame);
		m_pSrcFrame = NULL;
	}
	m_pSrcFrame = av_frame_alloc();

	m_pRGBFrame = av_frame_alloc();
	{
		m_pRGBFrame->format = AV_PIX_FMT_BGRA;
		int numBytes = av_image_get_buffer_size(AV_PIX_FMT_BGRA, m_DstVideoWidth, m_DstVideoHeight, 1);
		uint8_t *buffer = (uint8_t *)av_malloc_array(1, numBytes);
		int len = av_image_fill_arrays(m_pRGBFrame->data, m_pRGBFrame->linesize, buffer, AV_PIX_FMT_BGRA, m_DstVideoWidth, m_DstVideoHeight, 1);
		m_pRGBFrame->width = m_DstVideoWidth;
		m_pRGBFrame->height = m_DstVideoHeight;
	}
	m_pRotateYuvFrame = av_frame_alloc();
	{
		m_pRotateYuvFrame->format = AV_PIX_FMT_YUV420P;
		int numBytes = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, m_DstVideoWidth, m_DstVideoHeight, 1);
		uint8_t *buffer = (uint8_t *)av_malloc_array(1, numBytes);
		int len = av_image_fill_arrays(m_pRotateYuvFrame->data, m_pRotateYuvFrame->linesize, buffer, AV_PIX_FMT_YUV420P, m_DstVideoWidth, m_DstVideoHeight, 1);
		m_pRotateYuvFrame->width = m_DstVideoWidth;
		m_pRotateYuvFrame->height = m_DstVideoHeight;
	}

	av_dump_format(m_pFormatContext, 0, file, 0);
	return ret;
}

int CVideoDecoder::OpenFromMemory(char* buffer, size_t buffer_size)
{
	int ret = -1;
	av_register_all();
	avcodec_register_all();
	av_log_set_callback(my_ffmpeg_log_callback);

	if (m_pFormatContext)
	{
		avformat_close_input(&m_pFormatContext);
		m_pFormatContext = NULL;
	}

	if (!(m_pFormatContext = avformat_alloc_context())) {
		//ret = AVERROR(ENOMEM);
		return -1;
	}

	size_t avio_ctx_buffer_size = buffer_size;

	avio_ctx_buffer = (uint8_t*)av_malloc(avio_ctx_buffer_size);

	memcpy(avio_ctx_buffer, buffer, buffer_size);

	avio_ctx = avio_alloc_context((unsigned char*)avio_ctx_buffer, avio_ctx_buffer_size, 0, nullptr, nullptr, nullptr, nullptr);
	if (avio_ctx == nullptr)
	{
		LOGE("Use Buffer To Init Failed!");
		av_freep(avio_ctx_buffer);
		return -1;
	}

	m_pFormatContext->pb = avio_ctx;


	if ((ret = avformat_open_input(&m_pFormatContext, nullptr, NULL, NULL)) < 0)
	{
		//LOGE("Error: Could not open %s (%s)\n", file, makeErrorStr(ret));
		avformat_close_input(&m_pFormatContext);
		m_pFormatContext = NULL;
		return -1;
	}

	
	/* 获取音视频流信息 */
	if ((ret = avformat_find_stream_info(m_pFormatContext, NULL)) < 0)
	{
		LOGE("Could not find stream information (%s)\n", makeErrorStr(ret));
		avformat_close_input(&m_pFormatContext);
		m_pFormatContext = NULL;
		return -1;
	}

	//open codec context.
	if ((ret = open_codec_context(&m_video_stream_idx, m_pFormatContext, &m_pVideoCodec, AVMEDIA_TYPE_VIDEO)) < 0)
	{
		LOGE("No exit video.\n");
		avformat_close_input(&m_pFormatContext);
		m_pFormatContext = NULL;
		return -2;
	}

	// 	if (m_pVideoCodec->pix_fmt != AV_PIX_FMT_YUV420P && m_pVideoCodec->pix_fmt != AV_PIX_FMT_YUVJ420P)
	// 	{
	// 		LOGE("Unsupport format(%d).\n", m_pVideoCodec->pix_fmt);
	// 		return -3;
	// 	}

	if (m_video_stream_idx >= 0)
	{
		m_pVideoStream = m_pFormatContext->streams[m_video_stream_idx];
		if (m_pVideoCodec->pix_fmt == AV_PIX_FMT_NONE)
		{
			LOGE("set video format. \n");
			m_pVideoCodec->pix_fmt = AV_PIX_FMT_YUV420P;
		}
	}

	AVDictionaryEntry *tag = NULL;
	tag = av_dict_get(m_pVideoStream->metadata, "rotate", tag, 0);
	if (tag == NULL)
	{
		m_Rotate = 0;
	}
	else
	{
		int angle = atoi(tag->value);
		angle %= 360;
		m_Rotate = angle;
	}


	m_DstVideoWidth = m_VideoWidth = m_pVideoCodec->width;
	m_DstVideoHeight = m_VideoHeight = m_pVideoCodec->height;

	if (m_Rotate == 90 || m_Rotate == 270)
	{
		m_DstVideoWidth = m_VideoHeight;
		m_DstVideoHeight = m_VideoWidth;
	}

	rgb_sws_ctx = sws_getContext(
		m_DstVideoWidth,
		m_DstVideoHeight,
		m_pVideoCodec->pix_fmt,
		m_DstVideoWidth,
		m_DstVideoHeight,
		AV_PIX_FMT_BGRA,
		SWS_AREA, NULL, NULL, NULL);


	if (m_pSrcFrame)
	{
		av_frame_free(&m_pSrcFrame);
		m_pSrcFrame = NULL;
	}
	m_pSrcFrame = av_frame_alloc();

	m_pRGBFrame = av_frame_alloc();
	{
		m_pRGBFrame->format = AV_PIX_FMT_BGRA;
		int numBytes = av_image_get_buffer_size(AV_PIX_FMT_BGRA, m_DstVideoWidth, m_DstVideoHeight, 1);
		uint8_t *buffer = (uint8_t *)av_malloc_array(1, numBytes);
		int len = av_image_fill_arrays(m_pRGBFrame->data, m_pRGBFrame->linesize, buffer, AV_PIX_FMT_BGRA, m_DstVideoWidth, m_DstVideoHeight, 1);
		m_pRGBFrame->width = m_DstVideoWidth;
		m_pRGBFrame->height = m_DstVideoHeight;
	}
	m_pRotateYuvFrame = av_frame_alloc();
	{
		m_pRotateYuvFrame->format = AV_PIX_FMT_YUV420P;
		int numBytes = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, m_DstVideoWidth, m_DstVideoHeight, 1);
		uint8_t *buffer = (uint8_t *)av_malloc_array(1, numBytes);
		int len = av_image_fill_arrays(m_pRotateYuvFrame->data, m_pRotateYuvFrame->linesize, buffer, AV_PIX_FMT_YUV420P, m_DstVideoWidth, m_DstVideoHeight, 1);
		m_pRotateYuvFrame->width = m_DstVideoWidth;
		m_pRotateYuvFrame->height = m_DstVideoHeight;
	}

	av_dump_format(m_pFormatContext, 0, nullptr, 0);
	return ret;
}

int CVideoDecoder::Start()
{
	if (m_pFormatContext == NULL)
	{
		LOGE("No any video.");
		return -1;
	}
	int64_t seekTime = 0;
	int ret = avformat_seek_file(m_pFormatContext, -1, INT64_MIN, seekTime, INT64_MAX, 0);
	avcodec_flush_buffers(m_pVideoCodec);
	return ret;
}

int CVideoDecoder::Close()
{
	if (m_pVideoStream && m_pVideoStream->codec)
	{
		avcodec_close(m_pVideoStream->codec);
		m_pVideoCodec = NULL;
		m_pVideoStream = NULL;
	}

	if (m_pAudioStream && m_pAudioStream->codec)
	{
		avcodec_close(m_pAudioStream->codec);
		m_pAudioCodec = NULL;
		m_pAudioStream = NULL;
	}

	if (m_pFormatContext)
	{
		avformat_close_input(&m_pFormatContext);
		m_pFormatContext = NULL;
	}
	if (m_pSrcFrame)
	{
		av_frame_free(&m_pSrcFrame);
		m_pSrcFrame = NULL;
	}
	if (m_pRGBFrame)
	{
		for (int i = 0; i < AV_NUM_DATA_POINTERS; ++i)
		{
			uint8_t * pDataMem = m_pRGBFrame->data[i];
			if (pDataMem != NULL)
			{
				av_free(pDataMem);
				m_pRGBFrame->data[i] = NULL;
			}
		}
		av_frame_free(&m_pRGBFrame);
		m_pRGBFrame = NULL;
	}

	if (m_pRotateYuvFrame)
	{
		for (int i = 0; i < 1; ++i)
		{
			uint8_t * pDataMem = m_pRotateYuvFrame->data[i];
			if (pDataMem != NULL)
			{
				av_free(pDataMem);
				m_pRotateYuvFrame->data[i] = NULL;
			}
		}
		av_frame_free(&m_pRotateYuvFrame);
		m_pRotateYuvFrame = NULL;
	}

	if (rgb_sws_ctx)
	{
		sws_freeContext(rgb_sws_ctx);
		rgb_sws_ctx = NULL;
	}

	if (avio_ctx)
	{
		av_freep(&avio_ctx->buffer);
	}
	av_free(&avio_ctx);
	//if (avio_ctx)
	//{
	//	avio_context_free(&avio_ctx);
	//}

	m_video_stream_idx = -1;
	m_audio_stream_idx = -1;

	return 0;
}

unsigned char* CVideoDecoder::GetNextFrame()
{
	if (m_pFormatContext == NULL)
	{
		LOGE("No any video opened.");
		return NULL;
	}
	int ret = -1;
	int frameFinished = 0;
	AVPacket packet = { 0 };
	av_init_packet(&packet);
	while (av_read_frame(m_pFormatContext, &packet) >= 0)
	{
		if (packet.stream_index == m_video_stream_idx)
		{
			ret = avcodec_decode_video2(m_pVideoCodec, m_pSrcFrame, &frameFinished, &packet);
			if (frameFinished)
			{
				int64_t nPTS = av_frame_get_best_effort_timestamp(m_pSrcFrame);
				double time_stemp = nPTS * av_q2d(m_pVideoStream->time_base);
				//LOGD("time_stemp = %.2f s", time_stemp);
				m_TimeStamp = time_stemp;
				AVFrame* pDstFrame = m_pSrcFrame;

				if (m_Rotate)
				{
					//libyuv::I420Rotate(m_pSrcFrame->data[0], m_pSrcFrame->linesize[0],
					//	m_pSrcFrame->data[1], m_pSrcFrame->linesize[1],
					//	m_pSrcFrame->data[2], m_pSrcFrame->linesize[2],
					//	m_pRotateYuvFrame->data[0], m_pRotateYuvFrame->linesize[0],
					//	m_pRotateYuvFrame->data[1], m_pRotateYuvFrame->linesize[1],
					//	m_pRotateYuvFrame->data[2], m_pRotateYuvFrame->linesize[2],
					//	m_pSrcFrame->width, m_pSrcFrame->height, (libyuv::RotationMode)m_Rotate);
					//pDstFrame = m_pRotateYuvFrame;
					//byte* pRotateData = new byte[]
				}
				m_ReturnRefYUV = pDstFrame;
				ret = sws_scale(rgb_sws_ctx, pDstFrame->data, pDstFrame->linesize, 0, pDstFrame->height, m_pRGBFrame->data, m_pRGBFrame->linesize);

				{
					//SwsContext* swsContext = swsContext = sws_getContext(m_pRGBFrame->width, m_pRGBFrame->height, AV_PIX_FMT_BGRA, m_pRGBFrame->width, m_pRGBFrame->height, AV_PIX_FMT_RGBA,
					//	NULL, NULL, NULL, NULL);

					//int linesize[8] = { m_pRGBFrame->linesize[0] * 3 };
					//int num_bytes = av_image_get_buffer_size(AV_PIX_FMT_RGBA, m_pRGBFrame->width, m_pRGBFrame->height, 1);
					//uint8_t* p_global_bgr_buffer = (uint8_t*)malloc(num_bytes * sizeof(uint8_t));
					//uint8_t* bgr_buffer[8] = { p_global_bgr_buffer };

					//sws_scale(swsContext, m_pRGBFrame->data, m_pRGBFrame->linesize, 0, m_pRGBFrame->height, bgr_buffer, linesize);

					//sws_freeContext(swsContext);
					//swsContext = nullptr;

					//free(p_global_bgr_buffer);
				}
				av_packet_unref(&packet);

				if (ret >= 0)
				{
					return m_pRGBFrame->data[0];
				}
				else
				{
					return NULL;
				}

			}
		}
		av_packet_unref(&packet);
		av_init_packet(&packet);
	}
	av_packet_unref(&packet);
	return NULL;
}

unsigned char* CVideoDecoder::GetFrameByTimeStamp(double ms)
{
	if (m_pFormatContext == NULL)
	{
		LOGE("No any video opened.");
		return NULL;
	}
	int ret = -1;
	int frameFinished = 0;
	AVPacket packet = { 0 };
	av_init_packet(&packet);
	long long pos = (int64_t)(ms / (double)1000 / av_q2d(m_pVideoStream->time_base));

	float duration = 1000.0 / 30.0;
	int framNum = ms / duration;

	ret = av_seek_frame(m_pFormatContext, 0, framNum , AVSEEK_FLAG_FRAME);
	avcodec_flush_buffers(m_pFormatContext->streams[m_video_stream_idx]->codec);
	while (av_read_frame(m_pFormatContext, &packet) >= 0)
	{
		if (packet.stream_index == m_video_stream_idx)
		{
			ret = avcodec_decode_video2(m_pVideoCodec, m_pSrcFrame, &frameFinished, &packet);
			if (frameFinished)
			{
				int64_t nPTS = av_frame_get_best_effort_timestamp(m_pSrcFrame);
				double time_stemp = nPTS * av_q2d(m_pVideoStream->time_base);
				//LOGD("time_stemp = %.2f s", time_stemp);
				m_TimeStamp = time_stemp;
				AVFrame* pDstFrame = m_pSrcFrame;

				if (m_Rotate)
				{
					//libyuv::I420Rotate(m_pSrcFrame->data[0], m_pSrcFrame->linesize[0],
					//	m_pSrcFrame->data[1], m_pSrcFrame->linesize[1],
					//	m_pSrcFrame->data[2], m_pSrcFrame->linesize[2],
					//	m_pRotateYuvFrame->data[0], m_pRotateYuvFrame->linesize[0],
					//	m_pRotateYuvFrame->data[1], m_pRotateYuvFrame->linesize[1],
					//	m_pRotateYuvFrame->data[2], m_pRotateYuvFrame->linesize[2],
					//	m_pSrcFrame->width, m_pSrcFrame->height, (libyuv::RotationMode)m_Rotate);
					//pDstFrame = m_pRotateYuvFrame;
					//byte* pRotateData = new byte[]
				}
				m_ReturnRefYUV = pDstFrame;
				ret = sws_scale(rgb_sws_ctx, pDstFrame->data, pDstFrame->linesize, 0, pDstFrame->height, m_pRGBFrame->data, m_pRGBFrame->linesize);
				av_packet_unref(&packet);

				if (ret >= 0)
				{
					return m_pRGBFrame->data[0];
				}
				else
				{
					return NULL;
				}

			}
		}
		av_packet_unref(&packet);
		av_init_packet(&packet);
	}
	av_packet_unref(&packet);
	return NULL;
}

AVFrame* CVideoDecoder::GetYuvFrame()
{
	return m_ReturnRefYUV;
}

int CVideoDecoder::GetRotate()
{
	return m_Rotate;
}

int CVideoDecoder::Width()
{
	return m_DstVideoWidth;
}

int CVideoDecoder::Height()
{
	return m_DstVideoHeight;
}

double CVideoDecoder::GetTimeStamp()
{
	return m_TimeStamp;
}

double CVideoDecoder::GetFps() noexcept
{
	if (m_pVideoStream)
	{
		return m_pVideoStream->avg_frame_rate.num;
	}
	return 0;
}

double CVideoDecoder::GetDuration() noexcept
{
	if (m_pFormatContext)
	{
		return m_pFormatContext->duration * 0.001;
	}
	return 0.f;
}

#endif