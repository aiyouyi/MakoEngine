#ifndef __I_FURNACE_INTERFACE_H__
#define __I_FURNACE_INTERFACE_H__

#define FURNACE_SEGMENT 1

class IFurnaceInterface;

#if defined(_WIN32) || defined(WIN32)
extern "C" __declspec(dllexport) IFurnaceInterface* CreateFurnaceInterface();
#else
extern "C" IFurnaceInterface* CreateFurnaceInterface();
#endif

#ifndef INPUT_FORMAT_DEFINE
#define INPUT_FORMAT_DEFINE
enum INPUT_FORMAT {
	RRGGBB = 0,
	BGR888 = 1,
	BGRA8888 = 2,
	RGB888 = 3,
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
	virtual int load_segment_model(const char* model_small_save_path, const char* model_large_save_path, int ori_input_w, int ori_input_h, int* save_height, int* save_width, int compute_mode = 0) = 0;
	virtual unsigned char* detect_segment(size_t ori_width, size_t ori_height, unsigned char* ori_img, INPUT_FORMAT format, int frame_index) = 0;
	virtual void release() = 0;
};

#endif