#ifndef   _ANALYSYS_CPP_SDK_COMMTYPE_
#define  _ANALYSYS_CPP_SDK_COMMTYPE_
#include <string>

/// <summary>
/// SDK版本信息
/// </summary>
#define SDK_ANALYSYS_LIB_VERSIONS "4.0.8"
#define SDK_ANALYSYS_LIB_TYPE "CPP"

/// <summary>
/// EventName 
/// </summary>
#define ANALISYS_P_SET "$profile_set" 
#define ANALISYS_P_SET_ONE "$profile_set_once"
#define ANALISYS_P_IN "$profile_increment"
#define ANALISYS_P_APP "$profile_append" 
#define ANALISYS_P_UN "$profile_unset"
#define ANALISYS_P_DEL "$profile_delete" 
#define ANALISYS_ALIAS "$alias"

/// <summary>
/// PlatForm
/// </summary>
#define ANALISYS_JAVA "Java" 
#define ANALISYS_PYTHON "python"
#define ANALISYS_JS "JS"
#define ANALISYS_NODE "Node"
#define ANALISYS_PHP "PHP" 
#define ANALISYS_WECHAT "WeChat" 
#define ANALISYS_ANDROID "Android"
#define ANALISYS_IOS "iOS"

/// <summary>
/// DEBUG 模式
/// </summary>
enum ENUM_ANALISYS_DEBUG 
{
    CLOSE,
    OPENNOSAVE,
    OPENANDSAVE,
};


/// <summary>
/// SDK异常类
/// </summary>
class SDKException
{
public:
    explicit SDKException(const std::string& strMessage)
    : m_strErrInfo(strMessage)
    {
    }
    explicit SDKException(const char *szMessage)
    : m_strErrInfo(szMessage)
    {
    }
    virtual const char * what() const 
    {
        return m_strErrInfo.c_str();
    }
private: 
    std::string m_strErrInfo;
};
#endif //_ANALYSYS_CPP_SDK_COMMTYPE_