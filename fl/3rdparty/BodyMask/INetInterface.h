#ifndef __INET_INTERFACE_H__

#define __INET_INTERFACE_H__
#include <iostream>
#include <ImageProcess.hpp>
#include <Interpreter.hpp>
#include "utildef.h"

using namespace MNN;

#if defined(_WIN32) || defined(WIN32)
#else
#include "stddef.h"
#endif

struct NetResult {
	size_t n;
	size_t c;
	size_t h;
	size_t w;
	void* data;
};

//

class INetInterface;
#if defined(_WIN32) || defined(WIN32)
extern "C" __declspec(dllexport) INetInterface* CreateNetInterface();
#else
extern "C" INetInterface* CreateNetInterface();
#endif
//INetInterface* CreateNetInterface();
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

class INetInterface {

public:
	void setNumThreads(int num_threads);
	int init(const char* model_file);
	~INetInterface();
	void setModelInput(int input_h, int input_w, int channel);
	void setModelOutput();
	float* forward(float* input_data);
	void releaseInput();
	void releaseOutput();

	static INetInterface *instance();

	//
	void checkModelInput();
	void checkModelOutput();

private:
	Tensor* inputTensor;
	Tensor* nhwc_Tensor;
	Tensor* output_Tensor;
	Tensor* output_nhwc_Tensor;

	ScheduleConfig conf;
	Interpreter* interpreter = nullptr;
	Session* sess = nullptr;

	float* nhwc_data;
	int nhwc_size;

	float* output_data;
	int output_size;
};

#endif
