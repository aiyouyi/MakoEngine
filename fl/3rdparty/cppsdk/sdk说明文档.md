## 更新日志

#### v0.4.0

加入单手手势识别，更新了手检测器，优化了动态模糊下识别性能

#### v0.3.0

优化重构代码，提升运行效率，增加了更多接口方便使用者调试性能

#### v0.2.4

将手检测器频率改为每帧检测，调低了手存在性判断阈值，修复了同一只手重叠两次的bug

#### v0.2.3

修复了左右手类型预测bug

#### v0.2.1,v0.2.2

更新调参优化后的模型

#### v0.2.0

使用基于RLE的RepVGG，移除了模型内的图片预处理操作

#### v0.1.0

仅用于3d姿态估计接口对接，深度估计存在bug，准确度较低

## 接口说明

接口初始化时需要传入模型文件所在文件夹路径，然后对每帧图片进行检测，返回结果，最后释放。

### 1. 接口初始化

输入参数为模型文件所在目录，若初始化成功返回true

```c++
ccHGInit("../models");
```

设置最大检测数量，限制屏幕中进行检测的手部数量最大值

```c++
ccHGSetMaxHandNum(2);
```

参数重置，完成初始化

```c++
ccHGReset();
```

设置手检测器检测频率，默认每1帧检测一次

```c++
ccHGSetDetFrequency(1);
```

设置手势识别松弛系数，调整手势判定的严格程度，数值越大表示判定越松，更容易识别但也更容易出现误检，默认值5.0

```c++
ccHGSetGestureLooseness(5.0); // 0.0~10.0之间取值
```

### 2. 检测

参数类型从左至右分别是(图像数据所在内存位置，图像宽，图像高，图像每行数据所占位数，图像格式)
本接口采用的图像格式为BGR

```c++
m_pBuffer = (unsigned char*)frame.ptr(); // opencv输入图像

ccHGDetect(m_pBuffer, frame.cols, frame.rows, frame.cols * frame.channels(), IM_BGR);
```

### 3. 获取检测结果

```c++
ccHGHandRes res; 
ccHGGetHandRes(&res);
```

结果类结构如下：

```c++
// 每个关键点的坐标结构
typedef struct ccHGKeypoint_t {
	float x;
	float y;
}ccHGKeypoint;

// 每只手的信息结构
typedef struct ccHGHandInfo_t {
	ccHGRect handRect;	// 手包围框
	ccHGKeypoint keyPointArr[HANDKEYPOINTNUM]; // 手部21个关键点位置
	int handType; // 0右手，1左手
	int gestureType; // 手势识别类型
}ccHGHandInfo;


typedef struct ccHGHandRes_t {
	ccHGHandInfo handInfo[CC_MAX_HAND_COUNT]; // 每只手的信息
	int numHand; // 当前结果中手的数量
}ccHGHandRes;
```

结果关键点坐标解析示例：

```c++
for (int j = 0; j < 21; j++) {
    ccHGKeyPoint pt = res.handInfo[i].keyPointArr[j];
    int x = round(pt.x);
    int y = round(pt.y);
    int z = round(pt.z);
}
```

### 4. 坐标说明

x，y坐标为输入图片中的坐标，图片左上角为0点
z坐标为相对深度关系，以手腕点（20号点）为根节点，摄像头到手腕点的方向为坐标值正方向，即，离摄像头越远z坐标越大

21个关键点的z坐标示例：
![handpose.png](E:\project\pl-handpose3d\pysdk\screenshot_0.jpg)

```c++
z0      57
z1      40
z2      38
z3      30
z4      16
z5      18
z6      21
z7      28
z8      6
z9      3
z10     1
z11     7
z12     -8
z13     -12
z14     -14
z15     -9
z16     -16
z17     -23
z18     -26
z19     -18
z20     0     // 手腕点
```