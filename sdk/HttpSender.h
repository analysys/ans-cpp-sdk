#ifndef   _DEFAULT_CONSUMER_
#define  _DEFAULT_CONSUMER_
#include <string>
#include <vector>

/// <summary>
/// Http处理类
/// </summary>
class HttpSender
{
public:
    explicit HttpSender();
    /// <summary>
    /// 发送接口
    /// </summary>
    /// <param name="data">发送的字符串数据</param>
    /// <returns></returns>
    bool send(const std::string& strData, const std::string& strUrl);
public:
    int m_nRequestTimeoutSecond ;
    std::vector<std::pair<std::string, std::string> > m_vecHttpHeaders;

};
#endif


