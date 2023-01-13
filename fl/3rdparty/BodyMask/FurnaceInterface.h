#ifndef __FURNACE_INTERFACE_H_
#define __FURNACE_INTERFACE_H_

#include "IFurnaceInterface.h"
#include "utildef.h"
//#include "humanseg.h"
#include "myhumanseg.h"

class FurnaceInterface : public IFurnaceInterface
{
public:
	FurnaceInterface();
	~FurnaceInterface();
	static FurnaceInterface *instance();

public:
	virtual int					load_segment_model(const char* model_small_save_path, const char* model_large_save_path, int ori_input_w, int ori_input_h, int* save_height_large, int* save_width_large);
	virtual unsigned char*		detect_segment(size_t ori_width, size_t ori_height, unsigned char* ori_img, INPUT_FORMAT format, int frame_index);
	virtual unsigned char*		detect_segment_image(size_t ori_width, size_t ori_height, unsigned char* ori_img, INPUT_FORMAT format, int frame_index, int* save_height_large, int* save_width_large);
	virtual unsigned char*		detect_segment_screen(size_t ori_width, size_t ori_height, unsigned char* ori_img, INPUT_FORMAT format, int frame_index, int screen_number);
	virtual void				set_frame_interval(int set_frame_interval);
	virtual void				release();

private:
	myhumanseg::HumanSeg* _human_seg;
	int					_compute_mode;
	int					_framework_mode;
	int					_required_w;
	int					_required_h;
	int					_required_d;
};

#endif