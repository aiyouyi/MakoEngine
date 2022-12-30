#pragma once
#include <Runtime/Core/Public/CoreMinimal.h>
#include "core/inc.h"
#include <guiddef.h>

namespace va
{
    const std::string VIRTUAL_ACTOR_PIPE_NAME = "{79D3A489-AF91-467A-B929-2FD37D31B6B4}.VirtualActor";
    const std::string VIRTUAL_ACTOR_SERVER_REGISTER_INFO_NAME = "{2BB1E564-3D51-441D-82C2-5E540EBA7F6F}.ServerRegisterInfo";
    const std::string VIRTUAL_ACTOR_SWAPCHAIN_NAME = "{ECA172EE-541B-4A98-AE92-05CD924CFF57}";
    const std::string VIRTUAL_ACTOR_CANERA_SWAPCHAIN_NAME = "{B60C746D-6BEC-40A3-B262-5DB8B29F8D26}";

	enum class ClientType
	{
		None = -1,
		MobileZhuShou = 0,
		MobileGame = 1
	};

	enum class ScreenType
	{
		None = -1, //默认竖屏
		PortraitOriention = 0,//竖屏
		LandscapeOriention = 1 //横屏
	};

    uint32_t client_process_id();
    uint64_t client_session_id();
	ClientType getClientType();
	ScreenType getScreenType();
	bool isPhoneLive();
    const std::map<std::string, std::string> & arguments();
    std::string CCACTORLIB_API argument(std::string name);

    // 服务器注册信息，server 的进程 id
    struct server_register_desc
    {
        uint32_t client_process_id;
        uint32_t server_process_id;
    };

	enum class command : uint32_t
	{
		none = 0,
		// 使用内置采集设备
		select_device,
		// 更改背景图片
		set_background,
		set_showcamera,
		change_actor,//变换模型
		set_detective_point_type,//106,240,iphonx
		send_points106 = 6,
		apple_anchors = 7,
		send_pcmData,
		taf_data = 9,
		change_to_2D = 10,
		role_data = 11,
		show_again = 12,
		set_render_params = 13,
		sendback_points106 = 14,
		use_shared_tex_capture = 15,
		set_version = 16,
        report_data = 17, // 数据上报
    };

    enum class DriverType
    {
        Unkonwn = 0,
        iOSPoints106,
        iOSAppleAR,
        AndroidPoints106,

        WindowsPoints106,
        WindowsPoints240,
    };

    struct packet_header
    {
        command command;
        uint32_t nbytes;
        uint32_t param0;
        uint32_t param1;
        uint32_t param2;
        uint32_t param3;
    };

	enum Version
	{
		Version_20190313 = 1,
		Version_20190404 = 2,
	};

    class packet
    {
    public:
        packet(std::vector<byte_t> & buffer);
        packet(const std::vector<byte_t> & buffer);
        ~packet();

        template<typename T, typename = typename std::is_trivially_copyable<T>::type>
        size_t push(const T & val)
        {
            return push((const byte_t *)&val, sizeof(T));
        }
        size_t push(const byte_t * data, size_t length);

        template<typename T, typename = typename std::is_trivially_copyable<T>::type>
        size_t pop(T & val)
        {
            return pop((byte_t *)&val, sizeof(T));
        }
        size_t pop(byte_t * data, size_t length);

        template<typename T, typename = typename std::is_trivially_copyable<T>::type>
        T pop()
        {
            T val = {};
            pop((byte_t *)&val, sizeof(T));
            return val;
        }

    public:
        size_t push_string(const char * str, size_t length)
        {
            auto s = push((uint32_t)length);
            s += push((const byte_t *)str, (uint32_t)length);
            return s;
        }
        size_t push_string(const std::string & str)
        {
            auto s = push((uint32_t)str.length());
            s += push((const byte_t *)str.data(), str.length());
            return s;
        }

        std::string pop_string()
        {
            uint32_t length = 0;
            pop(length);
            std::string str(length, 0);
            pop((byte_t *)str.data(), length);
            return str;
        }
    private:
        std::vector<byte_t> & _buffer;
        size_t _pos = 0;
    };

    struct apple_anchors
    {
        // Left Eye
        float faceOrientation[3] = { 0, 0, 1 };
        float lookAt[3] = { 0, 0, 1 };

        float eyeBlinkLeft = 0.0f;
        float eyeLookDownLeft = 0.0f;
        float eyeLookInLeft = 0.0f;
        float eyeLookOutLeft = 0.0f;
        float eyeLookUpLeft = 0.0f;
        float eyeSquintLeft = 0.0f;
        float eyeWideLeft = 0.0f;

        // Right Eye
        float eyeBlinkRight = 0.0f;
        float eyeLookDownRight = 0.0f;
        float eyeLookInRight = 0.0f;
        float eyeLookOutRight = 0.0f;
        float eyeLookUpRight = 0.0f;
        float eyeSquintRight = 0.0f;
        float eyeWideRight = 0.0f;

        // Mouth and Jaw
        float jawForward = 0.0f;
        float jawLeft = 0.0f;
        float jawRight = 0.0f;
        float jawOpen = 0.0f;
        float mouthClose = 0.0f;
        float mouthFunnel = 0.0f;
        float mouthPucker = 0.0f;
        float mouthLeft = 0.0f;
        float mouthRight = 0.0f;
        float mouthSmileLeft = 0.0f;
        float mouthSmileRight = 0.0f;
        float mouthFrownLeft = 0.0f;
        float mouthFrownRight = 0.0f;
        float mouthDimpleLeft = 0.0f;
        float mouthDimpleRight = 0.0f;
        float mouthStretchLeft = 0.0f;
        float mouthStretchRight = 0.0f;
        float mouthRollLower = 0.0f;
        float mouthRollUpper = 0.0f;
        float mouthShrugLower = 0.0f;
        float mouthShrugUpper = 0.0f;
        float mouthPressLeft = 0.0f;
        float mouthPressRight = 0.0f;
        float mouthLowerDownLeft = 0.0f;
        float mouthLowerDownRight = 0.0f;
        float mouthUpperUpLeft = 0.0f;
        float mouthUpperUpRight = 0.0f;

        //Eyebrows, Cheeks, and Nose
        float browDownLeft = 0.0f;
        float browDownRight = 0.0f;
        float browInnerUp = 0.0f;
        float browOuterUpLeft = 0.0f;
        float browOuterUpRight = 0.0f;
        float cheekPuff = 0.0f;
        float cheekSquintLeft = 0.0f;
        float cheekSquintRight = 0.0f;
        float noseSneerLeft = 0.0f;
        float noseSneerRight = 0.0f;

        // Tongue
        float tongueOut = 0.0f;
    };
}
