
#include "HttpSender.h"
#include <iostream>
#include <curl/curl.h>

#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <locale>
#include <utility>
#include <sstream>
#include <stdio.h>
#include <iomanip>

#include <zlib.h>
#include <string.h>


#define SA_SDK_VERSION "1.0.1"
#define SA_SDK_NAME "CPP"
#define SA_SDK_FULL_NAME SA_SDK_NAME " " SA_SDK_VERSION


typedef std::map<std::string, std::string> HeaderFields;
/// <summary>
/// 网络应答
/// </summary>
typedef struct
{
    int m_nCode;
    std::string m_strbody;
    HeaderFields m_hHeaders;
} Response;

/// <summary>
/// Http 发送数据
/// </summary>
Response Post(const std::string& strUrl,
    const std::string& strCtype,
    const std::string& strData,
    int nTimeoutSecond=3,
    const std::vector<std::pair<std::string, std::string> >& vecHeaders
    = std::vector<std::pair<std::string, std::string> >());

/// <summary>
/// gzip压缩
/// </summary>
bool CompressString(const std::string& str, std::string* strOutString, int nCompressionLevel = Z_BEST_COMPRESSION)
{
    z_stream zs;  // z_stream is zlib's control structure
    memset(&zs, 0, sizeof(zs));


    if (deflateInit2(&zs, nCompressionLevel, Z_DEFLATED,
        15 | 16, 8, Z_DEFAULT_STRATEGY) != Z_OK) 
    {
            std::cerr << "deflateInit2 failed while compressing." << std::endl;
            return false;
    }

    zs.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(str.data()));
    zs.avail_in = static_cast<uInt>(str.size());  // set the z_stream's input

    int ret;
    char szOutBuffer[32768];

    // retrieve the compressed bytes blockwise
    do {
        zs.next_out = reinterpret_cast<Bytef*>(szOutBuffer);
        zs.avail_out = sizeof(szOutBuffer);

        ret = deflate(&zs, Z_FINISH);

        if (strOutString->size() < zs.total_out) 
        {
            // append the block to the output string
            strOutString->append(szOutBuffer, zs.total_out - strOutString->size());
        }
    } while (ret == Z_OK);

    deflateEnd(&zs);

    if (ret != Z_STREAM_END) {  // an error occurred that was not EOF
        std::cerr << "Exception during zlib compression: (" << ret << ") " << zs.msg << std::endl;
        return false;
    }

    return true;
}
const char kBase64Chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
/// <summary>
/// Base64编码
/// </summary>
std::string Base64Encode(const std::string& strData) 
{
    const unsigned char* bytes_to_encode = reinterpret_cast<const unsigned char*>(strData.data());
    int in_len = strData.length();
    std::string ret;
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    while (in_len-- > 0)
    {
        char_array_3[i++] = *(bytes_to_encode++);
        if (i == 3)
        {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (i = 0; (i < 4); i++)
                ret += kBase64Chars[char_array_4[i]];
            i = 0;
        }
    }

    if (i != 0) 
    {
        for (j = i; j < 3; j++)
        {
            char_array_3[j] = '\0';
        }
        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;
        for (j = 0; (j < i + 1); j++)
        {
            ret += kBase64Chars[char_array_4[j]];
        }
        while ((i++ < 3))
        {
            ret += '=';
        }
    }
    return ret;
}

/// <summary>
/// curl 写数据回调
/// </summary>
size_t WriteCallback(void* pData, size_t nSize, size_t nMemb, void* pUserData) 
{
    Response* rResponse;
    rResponse = reinterpret_cast<Response*>(pUserData);
    rResponse->m_strbody.append(reinterpret_cast<char*>(pData), nSize* nMemb);

    return (nSize * nMemb);
}

/// <summary>
/// curl 头数据回调
/// </summary>
size_t HeaderCallback(void* pData, size_t nSize, size_t nMemb, void* pUserData)
{
    Response* rResponse;
    rResponse = reinterpret_cast<Response*>(pUserData);
    std::string strHeader(reinterpret_cast<char*>(pData), nSize * nMemb);
    size_t nSeparator = strHeader.find_first_of(':');
    if (std::string::npos == nSeparator) 
    {
        rResponse->m_hHeaders[strHeader] = "present";
    } 
    else
    {
        std::string strKey = strHeader.substr(0, nSeparator);
        std::string strValue = strHeader.substr(nSeparator + 1);
        rResponse->m_hHeaders[strKey] = strValue;
    }

    return (nSize * nMemb);
}

/// <summary>
/// 网络连接类
/// </summary>
class Connection 
{
public:
    typedef struct 
    {
        double m_dTotalTime;
        double m_dNameLookupTime;
        double m_dConnectTime;
        double m_dAppConnectTime;
        double m_dPreTransferTime;
        double m_dStartTransferTime;
        double m_dRedirectTime;
        int m_nRedirectCount;
    } RequestInfo;

    explicit Connection(const std::string& strBaseUrl)
        : m_rLastRequest(), m_hHeaderFields() 
    {
            this->m_hCurlHandle = curl_easy_init();
            if (!this->m_hCurlHandle) 
            {
                //throw std::runtime_error("Couldn't initialize curl handle");
            }
            this->m_strBaseUrl = strBaseUrl;
            this->m_nTimeout = 0;
            this->bFollowRedirects = false;
            this->m_nMaxRedirects = -1l;
            this->m_bNoSignal = false;
    }

    ~Connection()
    {
        if (this->m_hCurlHandle)
        {
            curl_easy_cleanup(this->m_hCurlHandle);
        }
    }

    void SetTimeout(int nSeconds)
    {
        this->m_nTimeout = nSeconds;
    }

    void AppendHeader(const std::string& strKey, const std::string& strValue)
    {
        this->m_hHeaderFields[strKey] = strValue;
    }

    Response Post(const std::string& strUrl, const std::string& strData)
    {
        /** Now specify we want to POST data */
        curl_easy_setopt(this->m_hCurlHandle, CURLOPT_POST, 1L);
        /** set post fields */
        curl_easy_setopt(this->m_hCurlHandle, CURLOPT_POSTFIELDS, strData.c_str());
        curl_easy_setopt(this->m_hCurlHandle, CURLOPT_POSTFIELDSIZE, strData.size());

        return this->PerformCurlRequest(strUrl);
    }

private:
    Response PerformCurlRequest(const std::string& strUri)
    {
        // init return type
        Response rRet = {};

        std::string strUrl = std::string(this->m_strBaseUrl + strUri);
        std::string strHeaderString;
        CURLcode strRes;
        curl_slist* pHeaderList = NULL;

        /** set query URL */
        curl_easy_setopt(this->m_hCurlHandle, CURLOPT_URL, strUrl.c_str());
        /** set callback function */
        curl_easy_setopt(this->m_hCurlHandle, CURLOPT_WRITEFUNCTION,
            WriteCallback);
        /** set data object to pass to callback function */
        curl_easy_setopt(this->m_hCurlHandle, CURLOPT_WRITEDATA, &rRet);
        /** set the header callback function */
        curl_easy_setopt(this->m_hCurlHandle, CURLOPT_HEADERFUNCTION,
            HeaderCallback);
        /** callback object for headers */
        curl_easy_setopt(this->m_hCurlHandle, CURLOPT_HEADERDATA, &rRet);
        /** set http headers */
        for (HeaderFields::const_iterator it = this->m_hHeaderFields.begin();
            it != this->m_hHeaderFields.end(); ++it)
        {
                strHeaderString = it->first;
                strHeaderString += ": ";
                strHeaderString += it->second;
                pHeaderList = curl_slist_append(pHeaderList, strHeaderString.c_str());
        }
        curl_easy_setopt(this->m_hCurlHandle, CURLOPT_HTTPHEADER,pHeaderList);
        /** set user agent */
        curl_easy_setopt(this->m_hCurlHandle, CURLOPT_USERAGENT,SA_SDK_FULL_NAME);

        // 若使用 HTTPS，有两种配置方式，选用其中一种即可：
        // 1. 使用 CA 证书（下载地址 http://curl.haxx.se/ca/cacert.pem ），去掉下面一行的注释，并指定证书路径，例如证书在当前目录下
        // curl_easy_setopt(this->m_hCurlHandle, CURLOPT_CAINFO, "cacert.pem");
        // 2. （不建议，仅测试时方便可以使用）不验证服务端证书，去掉下面两行的注释
        // curl_easy_setopt(this->m_hCurlHandle, CURLOPT_SSL_VERIFYHOST, 0L);
        // curl_easy_setopt(this->m_hCurlHandle, CURLOPT_SSL_VERIFYPEER, 0L);

        // set timeout
        if (this->m_nTimeout != 0) 
        {
            curl_easy_setopt(this->m_hCurlHandle, CURLOPT_TIMEOUT, this->m_nTimeout);
            // dont want to get a sig alarm on timeout
            curl_easy_setopt(this->m_hCurlHandle, CURLOPT_NOSIGNAL, 1);
        }
        // set follow redirect
        if (this->bFollowRedirects) 
        {
            curl_easy_setopt(this->m_hCurlHandle, CURLOPT_FOLLOWLOCATION, 1L);
            curl_easy_setopt(this->m_hCurlHandle, CURLOPT_MAXREDIRS,static_cast<long long>(this->m_nMaxRedirects));
        }

        if (this->m_bNoSignal) 
        {
            // multi-threaded and prevent entering foreign signal handler (e.g. JNI)
            curl_easy_setopt(this->m_hCurlHandle, CURLOPT_NOSIGNAL, 1);
        }

        strRes = curl_easy_perform(this->m_hCurlHandle);
        if (strRes != CURLE_OK) 
        {
            rRet.m_strbody = curl_easy_strerror(strRes);
            rRet.m_nCode = -1;
        } 
        else 
        {
            long long  lHttpCode = 0;
            curl_easy_getinfo(this->m_hCurlHandle, CURLINFO_RESPONSE_CODE, &lHttpCode);
            rRet.m_nCode = static_cast<int>(lHttpCode);
        }

        curl_easy_getinfo(this->m_hCurlHandle, CURLINFO_TOTAL_TIME,    &this->m_rLastRequest.m_dTotalTime);
        curl_easy_getinfo(this->m_hCurlHandle, CURLINFO_NAMELOOKUP_TIME,&this->m_rLastRequest.m_dNameLookupTime);
        curl_easy_getinfo(this->m_hCurlHandle, CURLINFO_CONNECT_TIME,&this->m_rLastRequest.m_dConnectTime);
        curl_easy_getinfo(this->m_hCurlHandle, CURLINFO_APPCONNECT_TIME,&this->m_rLastRequest.m_dAppConnectTime);
        curl_easy_getinfo(this->m_hCurlHandle, CURLINFO_PRETRANSFER_TIME,&this->m_rLastRequest.m_dPreTransferTime);
        curl_easy_getinfo(this->m_hCurlHandle, CURLINFO_STARTTRANSFER_TIME,&this->m_rLastRequest.m_dStartTransferTime);
        curl_easy_getinfo(this->m_hCurlHandle, CURLINFO_REDIRECT_TIME,&this->m_rLastRequest.m_dRedirectTime);
        curl_easy_getinfo(this->m_hCurlHandle, CURLINFO_REDIRECT_COUNT,    &this->m_rLastRequest.m_nRedirectCount);
        // free header list
        curl_slist_free_all(pHeaderList);
        // reset curl handle
        curl_easy_reset(this->m_hCurlHandle);
        return rRet;
    }

    CURL* m_hCurlHandle;
    std::string m_strBaseUrl;
    HeaderFields m_hHeaderFields;
    int m_nTimeout;
    bool bFollowRedirects;
    int m_nMaxRedirects;
    bool m_bNoSignal;
    RequestInfo m_rLastRequest;
};

Response Post(const std::string& strUrl,
    const std::string& strCtype,
    const std::string& strData,
    int nTimeoutSecond,
    const std::vector<std::pair<std::string, std::string> >& vecHeaders)
{
        Response ret;
        //std::unique_ptr<Connection> conn;
        //try
        //{
        //    conn.reset(new Connection(""));
        //} catch (std::runtime_error& e)
        //{
        //    std::cerr << e.what() << std::endl;
        //    Response response;
        //    response.m_nCode = -1;
        //    response.m_strbody = e.what();
        //    return response;
        //}
        Connection* conn = new Connection("");
        conn->SetTimeout(nTimeoutSecond);
        if (strCtype.length() > 0) {
            conn->AppendHeader("Content-Type", strCtype);
        }
        for (std::vector<std::pair<std::string, std::string> >::const_iterator header = vecHeaders.begin(); header != vecHeaders.end(); header++) {
            conn->AppendHeader(header->first, header->second);
        }
        ret = conn->Post(strUrl, strData);
        if(conn != NULL)
            delete conn;
        return ret;
}

HttpSender::HttpSender():m_nRequestTimeoutSecond(0)
{
}
bool HttpSender::send(const std::string& strData, const std::string& strUrl)
{
    std::string strCompressedData = "";
    if (!CompressString(strData, &strCompressedData)) 
    {
        return false;
    }
    const std::string strBase64EncodedData = Base64Encode(strCompressedData);
    Response rResponse = Post(strUrl, "", strBase64EncodedData, m_nRequestTimeoutSecond, m_vecHttpHeaders);
    if (rResponse.m_nCode != 200)
    {
        std::cerr << "SensorsAnalytics SDK send failed: " << rResponse.m_strbody << std::endl;
        return false;
    }
    return true;
}

