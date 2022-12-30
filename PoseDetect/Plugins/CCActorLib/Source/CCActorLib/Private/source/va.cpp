#include "va/va.h"
#include "core/logger.h"

namespace va
{
    static std::map<std::string, std::string> __arguments;
    static std::once_flag __arguments_once_flag;

    static void __parse_arguments()
    {
        int argc = __argc;
        wchar_t ** wargs = __wargv;
        char ** aargs = __argv;

        core::inf() << __FUNCTION__ " " << argc << (wargs ? " wide " : (aargs ? " ansi " : "none")) << " arguments";
        for (int iarg = 0; iarg < argc; ++iarg)
        {
            if (wargs)
                core::inf() << __FUNCTION__ " wide argumnets[" << iarg << "]=" << wargs[iarg];
            if (aargs)
                core::inf() << __FUNCTION__ " ansi argumnets[" << iarg << "]=" << aargs[iarg];
        }

        for (int iarg = 0; iarg < argc; ++iarg)
        {
            const wchar_t * warg = wargs ? wargs[iarg] : nullptr;
            if (warg && warg[0] == L'-')
                ++warg;

            auto pwequal = warg ? wcsstr(warg, L"=") : nullptr;
            if (pwequal)
            {
                std::string name = core::ucs2_u8(reinterpret_cast<const wchar_t *>(warg), pwequal - warg);
                std::string value = core::ucs2_u8(pwequal + 1);
                __arguments[name] = value;
                continue;
            }

            const char * aarg = aargs ? aargs[iarg] : nullptr;
            if (aarg && aarg[0] == '-')
                ++aarg;

            auto paval = aarg ? std::strstr(aarg, "=") : nullptr;
            if (paval)
            {
                std::string name = std::string(reinterpret_cast<const char *>(aarg), paval - aarg);
                std::string value = std::string(paval + 1);
                __arguments[name] = value;
                continue;
            }
        }
    }

    const std::map<std::string, std::string> & arguments()
    {
        std::call_once(__arguments_once_flag, __parse_arguments);
        return __arguments;
    }

    std::string argument(std::string name)
    {
        auto & args = arguments();
        auto iter = args.find(name);
        if (iter == args.end())
            return std::string();
        return iter->second;
    }

    uint32_t client_process_id()
    {
        static uint32_t __client_process_id = 0;
        static std::once_flag __client_process_id_once_flag;
        std::call_once(__client_process_id_once_flag, []()
        {
            std::string str = argument("client_process_id");
            __client_process_id = std::atoi(str.c_str());
        });
        return __client_process_id;
    }

    uint64_t client_session_id()
    {
        static uint64_t __client_session_id = 0;
        static std::once_flag __client_session_id_once_flag;
        std::call_once(__client_session_id_once_flag, []()
        {
            std::string str = argument("client_session_id");
            __client_session_id = std::atoll(str.c_str());
        });
        return __client_session_id;
    }

	ClientType getClientType()
	{
		static uint64_t client_type = -1;
		static std::once_flag __is_phone_live_once_flag;
		std::call_once(__is_phone_live_once_flag, []()
		{
			std::string str = argument("client_type");
			if (str.length())
			{
				client_type = std::atoi(str.c_str());
			}
		});
		return ClientType(client_type);
	}

	va::ScreenType getScreenType()
	{
		static uint64_t screen_type = -1;
		static std::once_flag __is_screen_type_once_flag;
		std::call_once(__is_screen_type_once_flag, []()
		{
			std::string str = argument("screen_type");
			if (str.length())
			{
				screen_type = std::atoi(str.c_str());
			}
		});
		return ScreenType(screen_type);
	}

	bool isPhoneLive()
	{
		va::ClientType clientType = va::getClientType();
		return clientType == va::ClientType::MobileZhuShou || clientType == va::ClientType::MobileGame;
	}

    packet::packet(std::vector<byte_t> & buffer) :_buffer(buffer)
    {
    }

    packet::packet(const std::vector<byte_t> & buffer) : _buffer(const_cast<std::vector<byte_t> &>(buffer))
    {
    }

    packet::~packet()
    {

    }

    size_t packet::push(const byte_t * data, size_t length)
    {
        _buffer.resize(_pos + length);
        std::memcpy(_buffer.data() + _pos, data, length);
        _pos += length;
        return length;
    }

    size_t packet::pop(byte_t * data, size_t length)
    {
        if (length > _buffer.size() - _pos)
            return 0;

        std::memcpy(data, _buffer.data() + _pos, length);
        _pos += length;
        return length;
    }
}
