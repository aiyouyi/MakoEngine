// **********************************************************************
// WUP version 1.0.2 by WSRD Tencent.
// **********************************************************************

#ifndef _WUP_H_
#define _WUP_H_
#include <map>
#include <string>
#include <vector>
#include <sstream>
#include "jce/RequestF.h"

#ifdef __GNUC__
#   if __GNUC__ >3 || __GNUC_MINOR__ > 3
#       include <ext/pool_allocator.h>
#   endif
#endif


using namespace taf;

namespace wup
{

//���taf����ֵ��key
const std::string STATUS_RESULT_CODE = "STATUS_RESULT_CODE";
const std::string STATUS_RESULT_DESC = "STATUS_RESULT_DESC"; 

/////////////////////////////////////////////////////////////////////////////////
// ���Է�װ��

template<typename TWriter = BufferWriter, typename TReader = BufferReader,template<typename> class Alloc = std::allocator >
        //template<typename> class Alloc = __gnu_cxx::__pool_alloc >
class UniAttribute
{
    typedef std::vector<char,Alloc<char> > VECTOR_CHAR_TYPE;
    typedef std::map<std::string, VECTOR_CHAR_TYPE, std::less<std::string>,Alloc< std::pair<std::string,VECTOR_CHAR_TYPE > > > VECTOR_CHAR_IN_MAP_TYPE;
    typedef std::map<std::string, VECTOR_CHAR_IN_MAP_TYPE, std::less<std::string>,Alloc< std::pair<std::string,VECTOR_CHAR_IN_MAP_TYPE > > >   WUP_DATA_TYPE;

public:
    /**
     * �������ֵ
     * 
     * @param T:   ��������
     * @param name:��������
     * @param t:   ����ֵ
     */
    template<typename T> void put(const std::string& name, const T& t)
    {
        //JceOutputStream<TWriter> os;
		OutputStream.reset();

		OutputStream.write(t, 0);

        //_data[name][Class<T>::name()] = os.getByteBuffer();
        VECTOR_CHAR_TYPE & v = _data[name][Class<T>::name()];
        v.assign(OutputStream.getBuffer(), OutputStream.getBuffer() + OutputStream.getLength());
    }
    /**
     * ��ȡ����ֵ�����Բ��������׳��쳣
     * 
     * @throw runtime_error
     * @param T:   ��������
     * @param name:��������
     * @param t:   ����ֵ�������
     */
    template<typename T> void get(const std::string& name, T& t)
    {
        auto mit = _data.find(name);

        if (mit != _data.end())
        {
            std::string type = Class<T>::name();

			auto mmit = mit->second.find(type);

            if (mmit == mit->second.end() && mit->second.size() > 0)
            {
                std::string firstType = mit->second.begin()->first;
                
                if (firstType.find_first_of("?") != std::string::npos) 
                {
                    mmit = mit->second.begin();
                }
				else if (firstType.find_first_of("com.huya") != std::string::npos)
				{	// Java ���񷵻ص�className �� com.huya ��ͷ
					mmit = mit->second.begin();
				}
				
            }
            if (mmit != mit->second.end())
            {
                //JceInputStream<TReader> is;
				InputStream.reset();

				InputStream.setBuffer(mmit->second);

				InputStream.read(t, 0, true);

                return;
            }
            std::ostringstream os;

            os  << "UniAttribute type match fail,key:" << name << ",type:" << Class<T>::name() << ",";

            if (mit->second.size() > 0)
            {
                os << "may be:" << mit->second.begin()->first;
            }
            throw std::runtime_error(os.str());
        }
        throw std::runtime_error(std::string("UniAttribute not found key:") +  name + ",type:" + Class<T>::name());
    }
    /**
     * ��ȡ����ֵ�����Բ��������׳��쳣
     * 
     * @throw runtime_error
     * @param T:   ��������
     * @param name:��������
     * @return T:  ����ֵ
     */
    template<typename T> T get(const std::string& name)
    {
        T t;

        get<T>(name, t);

        return t;
    }
    /**
     * ��ȡ����ֵ�������쳣�������ڵ����Է���ȱʡֵ
     * 
     * @param T:   ��������
     * @param name:��������
     * @param t:  ����ֵ�������
     * @param def:     Ĭ��ֵ
     */
    template<typename T> void getByDefault(const std::string& name, T& t, const T& def)
    {
        try
        {
            get<T>(name, t);
        }
        catch (runtime_error& e)
        {
            t = def;
        }
    }
    /**
     * ��ȡ����ֵ(�����쳣��defΪȱʡֵ)
     * 
     * @param T:   ��������
     * @param name:��������
     * @param:     Ĭ��ֵ
     * @return T:  ����ֵ
     */
    template<typename T> T getByDefault(const std::string& name, const T& def)
    {
        T t;

        getByDefault<T>(name, t, def);

        return t;
    }

    /**
     *���ȫ������ֵ
     */
    void clear() 
    { 
        _data.clear(); 
    }

    /** ����
     * 
     * @param buff�� �������������
     */
    void encode(std::string& buff)
    {
        //JceOutputStream<TWriter> os;
		OutputStream.reset();

		OutputStream.write(_data, 0);

        buff.assign(OutputStream.getBuffer(), OutputStream.getLength());
    }

    /** ����
     * 
     * @param buff�� �������������
     */
    void encode(std::vector<char>& buff)
    {
        //JceOutputStream<TWriter> os;
		OutputStream.reset();

		OutputStream.write(_data, 0);

        //os.swap(buff);
        buff.assign(OutputStream.getBuffer(), OutputStream.getBuffer() + OutputStream.getLength());
    }

    /** ����
     * 
     * @throw runtime_error
     * @param buff�������ű�������bufferָ��
     * @param len�� ����buff���ȣ��������������
     */
    void encode(char* buff, size_t & len)
    {   
        //JceOutputStream<TWriter> os;
		OutputStream.reset();

		OutputStream.write(_data, 0);

        if(len < OutputStream.getLength()) throw runtime_error("encode error, buffer length too short");
        memcpy(buff, OutputStream.getBuffer(), OutputStream.getLength());
        len = OutputStream.getLength();
    }

    /** ����
     * 
     * @throw runtime_error
     * @param buff���������ֽ�����bufferָ��
     * @param len�� �������ֽ����ĳ���
     */
    void decode(const char* buff, size_t len)
    {
        //JceInputStream<TReader> is;
		InputStream.reset();

		InputStream.setBuffer(buff, len);

        _data.clear();

		InputStream.read(_data, 0, true);
    }
    /**
     * ����
     * 
     * @throw runtime_error
     * @param buff�� ��������ֽ���
     */
    void decode(const std::vector<char>& buff)
    {
        //JceInputStream<TReader> is;
		InputStream.reset();

		InputStream.setBuffer(buff);

        _data.clear();

		InputStream.read(_data, 0, true);
    }
    /**
     * ��ȡ���е�����
     * 
     * @return const map<std::string,map<std::string,std::vector<char>>>& : ����map
     */
    const std::map<std::string, std::map<std::string, std::vector<char> > >& getData() const
    {
        return _data;
    }

    /**
     * �ж����Լ����Ƿ�Ϊ��
     * 
     * @return bool:�����Ƿ�Ϊ��
     */
    bool isEmpty()
    {
        return _data.empty();
    }

    /**
     * ��ȡ���Լ��ϴ�С
     * 
     * @return size_t:  ���ϴ�С
     */
    size_t size()
    {
        return _data.size();
    }

//    /**
//     * �ж������Ƿ����
//     * 
//     * @param key:��������
//     * @return bool:�Ƿ����
//     */
	bool containsKey(const std::string & key)
	{
		return _data.find(key) != _data.end();
	}

protected:
    //WUP_DATA_TYPE _data;

public:
    JceInputStream<TReader>     InputStream;
    JceOutputStream<TWriter>    OutputStream;
    std::map<std::string, std::map<std::string, std::vector<char> > > _data;
};



/////////////////////////////////////////////////////////////////////////////////
// ���󡢻�Ӧ����װ��

template<typename TWriter = BufferWriter, typename TReader = BufferReader,template<typename> class Alloc = std::allocator >
struct UniPacket : protected RequestPacket, public UniAttribute<TWriter, TReader, Alloc>
{
public:
    /**
     * ���캯��
     */
    UniPacket() 
    {
        iVersion = 2; cPacketType = 0; 

        iMessageType = 0; iRequestId = 0; 

        sServantName = ""; sFuncName = ""; 

        iTimeout = 0; sBuffer.clear(); 

        context.clear(); status.clear(); 

        UniAttribute<TWriter, TReader,Alloc>::_data.clear();
    }

    /**
     * ��������
     * @param wup
     */
    UniPacket(const UniPacket &wup)  { *this = wup;}

    /**
     * ����������ɻ�Ӧ�������ṹ������ؼ���������Ϣ
     * 
     * @return UniPacket�� ��Ӧ��
     */
    UniPacket createResponse()
    {
        UniPacket respPacket;

        respPacket.sServantName = sServantName;
        respPacket.sFuncName    = sFuncName;
        respPacket.iRequestId   = iRequestId;

        return respPacket;
    }

    /**
     * ���룬����İ�ͷ4���ֽ�Ϊ�������ĳ��ȣ������ֽ���
     * 
     * @throw runtime_error
     * @param buff�� �������������
     */
    void encode(std::string& buff)
    {
        JceOutputStream<TWriter> &os = UniAttribute<TWriter, TReader,Alloc>::OutputStream;

        os.reset();
        
        doEncode(os);

        taf::Int32 iHeaderLen = htonl(sizeof(taf::Int32) + os.getLength());
        buff.assign((const char*)&iHeaderLen, sizeof(taf::Int32));

        buff.append(os.getBuffer(), os.getLength());
    }

    /**
     * ���룬����İ�ͷ4���ֽ�Ϊ�������ĳ��ȣ������ֽ���
     * 
     * @throw runtime_error
     * @param buff�� �������������
     */
    void encode(std::vector<char>& buff)
    {
        JceOutputStream<TWriter> & os = UniAttribute<TWriter, TReader,Alloc>::OutputStream;

        os.reset();

        doEncode(os);

        taf::Int32 iHeaderLen = htonl(sizeof(taf::Int32) + os.getLength());

        buff.resize(sizeof(taf::Int32) + os.getLength());
        memcpy(&buff[0], &iHeaderLen, sizeof(taf::Int32));
        memcpy(&buff[sizeof(taf::Int32)], os.getBuffer(), os.getLength());

    }

    /**
     * ���룬����İ�ͷ4���ֽ�Ϊ�������ĳ��ȣ������ֽ���
     * @throw runtime_error
     * @param buff����ű�������bufferָ��
     * @param len�� ����buff���ȣ��������������
     */
    void encode(char* buff, size_t & len)
    {
        JceOutputStream<TWriter> &os = UniAttribute<TWriter, TReader,Alloc>::OutputStream;

        os.reset();

        doEncode(os);

        taf::Int32 iHeaderLen = htonl(sizeof(taf::Int32) + os.getLength());
        if(len < sizeof(taf::Int32) + os.getLength()) throw runtime_error("encode error, buffer length too short");

        memcpy(buff, &iHeaderLen, sizeof(taf::Int32));
        memcpy(buff + sizeof(taf::Int32), os.getBuffer(), os.getLength());

        len = sizeof(taf::Int32) + os.getLength();
    }

    /** ����
     * 
     * @throw runtime_error
     * @param buff���������ֽ�����bufferָ��
     * @param len�� �������ֽ����ĳ���
     */

    void decode(const char* buff, size_t len)
    {
        if(len < sizeof(taf::Int32)) throw std::runtime_error("packet length too short");
    
        JceInputStream<TReader> &is = UniAttribute<TWriter, TReader,Alloc>::InputStream;

        is.reset();

        is.setBuffer(buff + sizeof(taf::Int32), len - sizeof(taf::Int32));

        readFrom(is);

        is.reset();

        is.setBuffer(sBuffer);

        UniAttribute<TWriter, TReader,Alloc>::_data.clear();

        is.read(UniAttribute<TWriter, TReader,Alloc>::_data, 0, true);
    }
public:
    /**
     * ��ȡ��ϢID
     * @return taf::Int32
     */
    taf::Int32 getRequestId() const { return iRequestId; }
    /**
     * ��������ID
     * @param value
     */
    void setRequestId(taf::Int32 value) { iRequestId = value; }
    /**
     * ��ȡ��������
     * @return const std::string&
     */
    const std::string& getServantName() const { return sServantName; }
    /**
     * ���ö�������
     * @param value
     */
    void setServantName(const std::string& value) { sServantName = value; }
    /**
     * ��ȡ������
     * @return const std::string&
     */
    const std::string& getFuncName() const { return sFuncName; }
    /**
     * ���÷�����
     * @param value
     */
    void setFuncName(const std::string& value) { sFuncName = value; }

protected:
    /**
     * �ڲ�����
     */
    void doEncode(JceOutputStream<TWriter>& os)
    {
        //ServantName��FuncName����Ϊ��
        if(sServantName.empty()) throw std::runtime_error("ServantName must not be empty");
        if(sFuncName.empty())    throw std::runtime_error("FuncName must not be empty");

        os.reset();

        os.write(UniAttribute<TWriter, TReader,Alloc>::_data, 0);

        //sBuffer = os.getByteBuffer();

        sBuffer.assign(os.getBuffer(), os.getBuffer() + os.getLength());

        os.reset();

        writeTo(os);
    }
};

/////////////////////////////////////////////////////////////////////////////////
// ����TAF�ķ���ʱʹ�õ���

template<typename TWriter = BufferWriter, typename TReader = BufferReader,template<typename> class Alloc = std::allocator>
struct TafUniPacket: public UniPacket<TWriter, TReader,Alloc>
{
public:
    TafUniPacket(){};
    TafUniPacket(const UniPacket<TWriter, TReader,Alloc> &wup) 
    : UniPacket<TWriter, TReader,Alloc>(wup) {};

    /**
     * ����Э��汾
     * @param value
     */
    void setTafVersion(taf::Short value) { this->iVersion = value; }

    /**
     * ���õ�������
     * @param value
     */
    void setTafPacketType(taf::Char value) { this->cPacketType = value; }

    /**
     * ������Ϣ����
     * @param value
     */
    void setTafMessageType(taf::Int32 value) { this->iMessageType = value; }

    /**
     * ���ó�ʱʱ��
     * @param value
     */
    void setTafTimeout(taf::Int32 value) { this->iTimeout = value; }

    /**
     * ���ò�����������
     * @param value
     */
    void setTafBuffer(const std::vector<taf::Char>& value) { this->sBuffer = value; }

    /**
     * ����������
     * @param value
     */
    void setTafContext(const std::map<std::string, std::string>& value) { this->context = value; }

    /**
     * ����������Ϣ��״ֵ̬
     * @param value
     */
    void setTafStatus(const std::map<std::string, std::string>& value) { this->status = value; }

    /**
     * ��ȡЭ��汾
     * @return taf::Short
     */
    taf::Short getTafVersion() const { return this->iVersion; }

    /**
     * ��ȡ��������
     * @return taf::Char
     */
    taf::Char getTafPacketType() const { return this->cPacketType; }

    /**
     * ��ȡ��Ϣ����
     * @return taf::Int32
     */
    taf::Int32 getTafMessageType() const { return this->iMessageType; }

    /**
     * ��ȡ��ʱʱ��
     * @return taf::Int32
     */
    taf::Int32 getTafTimeout() const { return this->iTimeout; }

    /**
     * ��ȡ�������������
     * @return const std::vector<taf::Char>&
     */
    const std::vector<taf::Char>& getTafBuffer() const { return this->sBuffer; }

    /**
     * ��ȡ��������Ϣ
     * @return const map<std::string,std::string>&
     */
    const std::map<std::string, std::string>& getTafContext() const { return this->context; }

    /**
     * ��ȡ������Ϣ��״ֵ̬
     * @return const map<std::string,std::string>&
     */
    const std::map<std::string, std::string>& getTafStatus() const { return this->status; }

    /**
     * ��ȡ����taf�ķ���ֵ
     * 
     * @retrun taf::Int32
     */
    taf::Int32 getTafResultCode() const
    {
        std::map<std::string, std::string>::const_iterator it;
        if((it = this->status.find(STATUS_RESULT_CODE)) == this->status.end())
        {
            return 0;
        }
        else
        {
            return atoi(it->second.c_str());
        }
    }

    /**
     * ��ȡ����taf�ķ�������
     * 
     * @retrun std::string
     */
    std::string getTafResultDesc() const
    {
        map<std::string, std::string>::const_iterator it;
        if((it = this->status.find(STATUS_RESULT_DESC)) == this->status.end())
        {
            return "";
        }
        else
        {
            return it->second;
        }
    }

};

#ifdef __GNUC__
#   if __GNUC__ >3 || __GNUC_MINOR__ > 3
        typedef UniAttribute<BufferWriter,BufferReader, __gnu_cxx::__pool_alloc> UniAttrPoolAlloc;
        typedef UniPacket<BufferWriter,BufferReader, __gnu_cxx::__pool_alloc> UniPacketPoolAlloc;
        typedef TafUniPacket<BufferWriter,BufferReader, __gnu_cxx::__pool_alloc> TafUniPacketPoolAlloc;        
#   endif
#endif




}
////////////////////////////////////////////////////////////////////////////////////////////////
#endif
