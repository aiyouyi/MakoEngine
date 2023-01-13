#ifndef __I_FURNACE_INTERFACE_H__
#define __I_FURNACE_INTERFACE_H__

#ifndef INPUT_FORMAT_DEFINE
#define INPUT_FORMAT_DEFINE
enum INPUT_FORMAT {
	RRGGBB = 0,
	BGR888 = 1,
	BGRA8888 = 2,
	RGB888 = 3,
	RGBA8888 = 4,
};
#endif

class IFurnaceInterface {
	struct NetResult
	{
		size_t n;
		size_t c;
		size_t h;
		size_t w;
		void* data;
	};
public:
	virtual int load_segment_model(const char* model_small_save_path, const char* model_large_save_path, int ori_input_w, int ori_input_h, int* save_height, int* save_width) = 0;
	//virtual unsigned char* detect_segment(size_t ori_width, size_t ori_height, unsigned char* ori_img, INPUT_FORMAT format, int frame_index) = 0;
	virtual unsigned char* detect_segment_image(size_t ori_width, size_t ori_height, unsigned char* ori_img, INPUT_FORMAT format, int frame_index, int* save_height, int* save_width) = 0;
	virtual unsigned char* detect_segment_screen(size_t ori_width, size_t ori_height, unsigned char* ori_img, INPUT_FORMAT format, int frame_index, int screen_number) = 0;
	virtual void set_frame_interval(int set_frame_interval) = 0;
	virtual void release() = 0;
};

#if defined(_WIN32) || defined(WIN32)
extern "C" __declspec(dllexport) IFurnaceInterface* CreateFurnaceInterface();
#else
extern "C" IFurnaceInterface* CreateFurnaceInterface();
#endif

#endif