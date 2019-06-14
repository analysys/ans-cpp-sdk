
#ifndef   _JSON_
#define  _JSON_
#include <string>

/// <summary>
/// Json处理类,
/// 传入string，char*时请使用utf-8编码
/// 单独[]操作，未做其它操作符时如 JObject j; 不支持j["123"]; 支持j["123"] = 1;支持j["123"].toJson; 
/// </summary>
class JObject
{
public: 
    JObject();
    ~JObject();

    /// <summary>
    /// 获取Json字符串
    /// </summary>
    /// <returns>返回Json字符串</returns>
    std::string toJson();
    /// <summary>
    /// 拷贝构造
    /// </summary>
    JObject(const JObject& other); 
    /// <summary>
    /// 拷贝赋值
    /// </summary>
    JObject& operator=(const JObject &other);
    /// <summary>
    /// 如果是列表或者MAP，返回元素个数
    /// </summary>
    int getSize();
    /// <summary>
    /// 运算符重载
    /// </summary>
    JObject& operator=(bool bValue);
    JObject& operator=(long long lValue);
    JObject& operator=(double dValue);
    JObject& operator=(const char* szValue);
    JObject& operator=(const std::string& strValue);
    JObject& operator[](const char* szKey);
    JObject& operator[](const std::string& strKey);
    JObject& operator[](int nIndex);
    JObject& operator+(const JObject &other);
    JObject& operator-(const std::string& strKey);
    JObject& operator-(const int nListIndex);
    /// <summary>
    /// JS参数非法值判断
    /// </summary>
    /// <param name="strEventName">事件名</param>
    /// <param name="jMapProp">键值对</param>
    /// <returns>如果验算非法,将抛出SDKException异常</returns>
    static void checkParam(const std::string&  strEventName, const JObject& jMapProp);
    /// <summary>
    /// JS属性非法值判断
    /// </summary>
    /// <param name="strDistinctId">用户标识</param>
    /// <param name="strEventName">事件名称</param>
    /// <param name="jProperties">属性</param>
    /// <param name="nCommProLen">公共属性长度</param>
    /// <returns>如果验算非法,将抛出SDKException异常</returns>
    static void checkProperty(const std::string& strDistinctId, const std::string& strEventName, const JObject& jProperties, int nCommProLen);
public:
    void* m_pPriData;

};


#endif //_JSON_

