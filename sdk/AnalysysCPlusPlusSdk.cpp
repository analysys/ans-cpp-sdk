// AnalysysCPlusSdk.cpp : 定义 DLL 应用程序的导出函数。
//

#include "AnalysysCPlusPlusSdk.h"
#include "Json.h"
#include "HttpSender.h"
#include <deque>
#include <memory>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <time.h>


#include "AnalysysCppSdkCommType.h"
#define MAX_SHOW_LOG_LEN 512
#define MAX_SHOW_LOG_FORMAT_LEN 100
#define FLUSH_ALL_BATCH_SIZE 30

#ifdef _WIN32
#include <ctime>
#include <Windows.h>
#else
#include <sys/time.h>
#endif 


#if defined(__linux__)
#define SA_SDK_LOCALTIME(seconds, now) localtime_r((time_t*)(seconds), (now))
#elif defined(__APPLE__)
#define SA_SDK_LOCALTIME(seconds, now) localtime_r((seconds), (now))
#elif defined(_WIN32)
#define SA_SDK_LOCALTIME(seconds, now) localtime_s((now), (seconds))
#endif


/// <summary>
/// SDK数据处理类
/// </summary>
class SDKCommDataHandler
{

public:
    /// <summary>
    /// 忽略大小写,比较字符串
    /// </summary>
    static bool sCmpIgnoCase(const std::string &s1, const std::string &s2)
    {
        int sl = s1.length();
        int tl = s2.length();
        std::string s3 = s1;
        std::string s4 = s2;
        for(int i = 0; i < sl; i++)
        {
            if(s3[i]>=65 &&s3[i]<=90)
                s3[i] += (int)('a'-'A');
        }

        for(int j=0; j<tl; j++)
        {
            if(s4[j]>=65 && s4[j]<=90)
                s4[j] += (int)('a'-'A');
        }
        return s3==s4;

    }
    /// <summary>
    /// 获取平台字符串
    /// </summary>
    static void getPlatformString(const std::string& strPlatform, std::string& strOutPlatformString, std::string& strOutErrInfo)
    {
        if(strPlatform == "")
            return ;
        std::string strCmp;
        bool bFind = false;
        char* szArrPlatformName[] = {"Java", "python", "JS", "Node", "PHP", "WeChat", "Android", "iOS"};
        int nArrPlatformNameNum = 8;
        strOutPlatformString = strPlatform;
        
        for(int nIndex = 0; nIndex < nArrPlatformNameNum; nIndex++)
        {
            
            strCmp = szArrPlatformName[nIndex];
            if(sCmpIgnoCase(strPlatform, strCmp))
            {
                bFind = true;
                break;
            }
        }
        if(!bFind)
        {

            char szBuf[MAX_SHOW_LOG_LEN];
            if(strOutPlatformString.size() > MAX_SHOW_LOG_LEN - MAX_SHOW_LOG_FORMAT_LEN)
            {
                strOutPlatformString = strOutPlatformString.substr(0, MAX_SHOW_LOG_LEN - MAX_SHOW_LOG_FORMAT_LEN-1);
            }
            sprintf(szBuf, "Warning: param platform:%s  Your input are not:iOS/Android/JS/WeChat.",strOutPlatformString.c_str());
            strOutErrInfo = szBuf;

        }
        else
        {
            strOutErrInfo = "";
            strOutPlatformString = strCmp;
        }
    }
    /// <summary>
    /// 获取当前时间
    /// </summary>
    static long long getDateTimeNow()
    {
        long long lTime = 0;
#ifdef _WIN32
        //struct tm t;
        lTime = time(NULL)*1000;
        SYSTEMTIME st;
        GetLocalTime(&st);
        lTime = lTime+(double)st.wMilliseconds;
        
#else
        struct timeval tmv;
        gettimeofday(&tmv, NULL);
        lTime = tmv.tv_sec*1000+(double)tmv.tv_usec*0.001;
#endif 
        return lTime;
    }

};

/// <summary>
/// 消费者数据处理类
/// </summary>
class DefaultConsumer
{

public:
    DefaultConsumer() :
        m_nBatchNum(10),
        m_nBatchSec(20),
        m_lTimeNow(0),
        m_bSyncSend(true),
        m_bIsDebug(false)
    {

    }
    ~DefaultConsumer()
    {

    }
    /// <summary>
    /// 获取时间格式字符串
    /// </summary>
    void getTimeLog(std::string& strTime)
    {
        long long lTicks = SDKCommDataHandler::getDateTimeNow();
        time_t lSeconds = lTicks*0.001;
        struct tm tm = {};
        SA_SDK_LOCALTIME(&lSeconds, &tm);
        char buff[64];
        sprintf(buff,  "\"time：%04d-%02d-%02d %02d:%02d:%02d.%03d\"",
            tm.tm_year + 1900,
            tm.tm_mon + 1,
            tm.tm_mday,
            tm.tm_hour,
            tm.tm_min,
            tm.tm_sec,
            (int)lTicks%1000);
        strTime = buff;
    }
    /// <summary>
    /// 发送
    /// </summary>
    bool send(JObject& jRecord)
    {

        bool bRes = true;
        if(m_lTimeNow <= 0)
        {
            m_lTimeNow = SDKCommDataHandler::getDateTimeNow();
        }
        std::string strJsonRecord = jRecord.toJson();
        m_dequeRecords.emplace_back(strJsonRecord);

        char szInfo[MAX_SHOW_LOG_LEN];
        std::string strJsonLog = strJsonRecord;
        std::string strLog;
        if(strJsonLog.size() > MAX_SHOW_LOG_LEN - MAX_SHOW_LOG_FORMAT_LEN)
        {
            strJsonLog = strJsonLog.substr(0, MAX_SHOW_LOG_LEN - MAX_SHOW_LOG_FORMAT_LEN-1);
        }
        if(m_bIsDebug)
        {
            sprintf(szInfo, "Send message to server: %s \ndata: %s", m_strUrl.c_str(), strJsonLog.c_str());
            std::cout << szInfo << std::endl;
        }

        //如果实时上传，则直接上传数据
        if(m_bSyncSend)
        {
            bRes = flush();
            if(m_bIsDebug)
            {
                getTimeLog(strLog);
                if(bRes)
                    sprintf(szInfo, "sync send message to server SUCCESS!");
                else
                    sprintf(szInfo, "sync send message to server FAIL!");
                strLog+=szInfo;
                std::cout << strLog << std::endl;
            }
        }
        else
        {
            if(m_nBatchNum > 0 && m_nBatchSec > 0)
            {
                long long lTimeSpan = (SDKCommDataHandler::getDateTimeNow() - m_lTimeNow)*0.001;

                if(m_dequeRecords.size() >= m_nBatchNum || lTimeSpan > m_nBatchSec)
                {
                    bRes = flush();
                    m_lTimeNow = SDKCommDataHandler::getDateTimeNow();
                    if(m_bIsDebug)
                    {
                        if(bRes)
                            sprintf(szInfo, "batch send message to server SUCCESS!");
                        else
                            sprintf(szInfo, "batch send message to server FAIL!");
                        strLog+=szInfo;
                        std::cout << strLog << std::endl;
                    }
                }
            }
            else
            {
                bRes = flush();
                if(m_bIsDebug)
                {
                    getTimeLog(strLog);
                    if(bRes)
                        sprintf(szInfo, "sync send message to server SUCCESS!");
                    else
                        sprintf(szInfo, "sync send message to server FAIL!");
                    strLog+=szInfo;
                    std::cout << strLog << std::endl;
                }
            }
        }
        return bRes;
    }
    /// <summary>
    /// 触发发送,发送最多 kFlushAllBatchSize(30) 条数据
    /// </summary>
    /// <param name="part_size">发送数量</param>
    /// <returns></returns>
    bool flushPart(size_t nPartSize)
    {
        std::vector<std::string> vecSendingRecords;    
        size_t nFlushSize = nPartSize < m_dequeRecords.size() ? nPartSize : m_dequeRecords.size();
        if (nPartSize == 0)
        {
            return true;
        }

        {
            auto iterEnd = m_dequeRecords.begin() + nFlushSize;
            vecSendingRecords.assign(m_dequeRecords.begin(), iterEnd);
            m_dequeRecords.erase(m_dequeRecords.begin(), iterEnd);
        }


        std::stringstream ssBuffer;
        ssBuffer << '[';
        for (std::vector<std::string>::iterator iter = vecSendingRecords.begin(); iter != vecSendingRecords.end(); ++iter) 
        {
            if (iter != vecSendingRecords.begin())
            {
                ssBuffer << ',';
            }
            std::string& strValue = *iter;
            ssBuffer << strValue;
        }
        ssBuffer << ']';

        bool bSendResult = m_httpSender.send(ssBuffer.str(), m_strUrl);
        return bSendResult;
    }

    /// <summary>
    /// 发送当前所有数据，如果发送中断（失败）返回 false，全部发送成功返回 true
    /// </summary>
    bool flush()
    {
        while (true) 
        {
            {
                if (m_dequeRecords.empty()) 
                {
                    break;
                }
            }

            bool bFlushResult = flushPart(FLUSH_ALL_BATCH_SIZE);
            if (!bFlushResult) 
            {
                return false;
            }
        }
        return true;
    }

    /// <summary>
    /// 清空发送队列
    /// </summary>
    void clear() 
    {
        m_dequeRecords.clear();
    }
    /// <summary>
    /// 批量发送
    /// </summary>
    void batchUpload(int nBatchNum, int nBatchSec)
    {
        m_bSyncSend = false;
        m_nBatchNum = nBatchNum;
        m_nBatchSec = nBatchSec;
    }
public:
    std::deque<std::string> m_dequeRecords;
    HttpSender m_httpSender;
    std::string m_strUrl;
    int m_nBatchSec;
    int m_nBatchNum;
    bool m_bSyncSend;
    long long m_lTimeNow;
    bool m_bIsDebug;
    
};


/// <summary>
/// SDK数据
/// </summary>
typedef struct SdkData
{
    std::string m_strAppKey;
    JObject m_jXcontextSuperProperties;
    ENUM_ANALISYS_DEBUG m_eDebugType;
    DefaultConsumer* m_pDefaultConsumer;
    SdkData():m_eDebugType(CLOSE),
    m_pDefaultConsumer(new DefaultConsumer)
    {

    }
    ~SdkData()
    {
        if(m_pDefaultConsumer != NULL)
        {
            delete m_pDefaultConsumer;
            m_pDefaultConsumer = NULL;
        }

    }


}_SdkData;

AnalysysCPlusPlusSdk::AnalysysCPlusPlusSdk()
{
    m_pPrivateData = new SdkData;
}

AnalysysCPlusPlusSdk::~AnalysysCPlusPlusSdk()
{
    if(m_pPrivateData!=NULL)
    {
        delete m_pPrivateData;
    }
}

bool AnalysysCPlusPlusSdk::init(const std::string& strServerUrl, const std::string& strAppKey)
{

    m_pPrivateData->m_pDefaultConsumer->m_strUrl = strServerUrl + "/up";
    m_pPrivateData->m_pDefaultConsumer->clear();
    m_pPrivateData->m_strAppKey = strAppKey;
    return true;
}

void AnalysysCPlusPlusSdk::batchCollecter(int nBatchNum, int nBatchSec)
{
    m_pPrivateData->m_pDefaultConsumer->batchUpload(nBatchNum, nBatchSec);
}

void AnalysysCPlusPlusSdk::syncCollecter()
{
    m_pPrivateData->m_pDefaultConsumer->m_bSyncSend = true;
}

void AnalysysCPlusPlusSdk::setDebugMode(int eDebug)
{
    m_pPrivateData->m_eDebugType = (ENUM_ANALISYS_DEBUG)eDebug;
    m_pPrivateData->m_pDefaultConsumer->m_bIsDebug = (ENUM_ANALISYS_DEBUG)eDebug != CLOSE;
}

void AnalysysCPlusPlusSdk::upload(const std::string& strDistinctId, bool bIsLogin, const std::string& strEventName, const JObject& jAttributes, const std::string& strPlatform)
{
    
    JObject::checkProperty(strDistinctId, strEventName, jAttributes, m_pPrivateData->m_jXcontextSuperProperties.getSize());
    JObject::checkParam("sss", m_pPrivateData->m_jXcontextSuperProperties);


    JObject::checkProperty(strDistinctId, strEventName, jAttributes, m_pPrivateData->m_jXcontextSuperProperties.getSize());

    JObject jObj;
    jObj["xwho"] = strDistinctId;  
    jObj["xwhen"] = SDKCommDataHandler::getDateTimeNow();
    jObj["xwhat"] = strEventName;
    jObj["appid"] = m_pPrivateData->m_strAppKey;

    JObject jNewProperties;
    std::string strProfile = "$profile";
    std::string strAlias = "$alias";
    int nPosProfile = strEventName.find(strProfile);
    int nPosAlias = strEventName.find(strAlias);

    if(nPosAlias != 0 && nPosProfile != 0)
    {
        jNewProperties = m_pPrivateData->m_jXcontextSuperProperties;

    }

    jNewProperties["$is_login"] = bIsLogin;
    std::string strPlatformOut;
    std::string strErrInfo;
    SDKCommDataHandler::getPlatformString(strPlatform, strPlatformOut, strErrInfo);
    if(strErrInfo != "" && m_pPrivateData->m_eDebugType != CLOSE)
    {
        std::cout << strErrInfo << std::endl;
    }
    if(strPlatformOut != "")
        jNewProperties["$platform"] = strPlatformOut;

    jNewProperties["$debug"] = (long long)m_pPrivateData->m_eDebugType;
    jNewProperties["$lib"] = SDK_ANALYSYS_LIB_TYPE;
    jNewProperties["$lib_version"] = SDK_ANALYSYS_LIB_VERSIONS;

    jNewProperties = jNewProperties + jAttributes;
    jObj["xcontext"] = jNewProperties;
    
    

    bool bRes = m_pPrivateData->m_pDefaultConsumer->send(jObj);
    if(m_pPrivateData->m_eDebugType != CLOSE && bRes)
    {
        std::string strOut = strEventName;
        strOut += "  success";
        std::cout << strOut << std::endl;
    }
}

void AnalysysCPlusPlusSdk::track(const std::string& strDistinctId, bool bIsLogin, const std::string& strEventName, const JObject& jAttributes, const std::string& strPlatform)
{
    upload(strDistinctId, bIsLogin, strEventName, jAttributes, strPlatform);
    

}

void AnalysysCPlusPlusSdk::alias(const std::string& strAliasId, const std::string& strDistinctId, const std::string& strPlatform)
{
    //"$alias"
    JObject jObj;
    jObj["$original_id"]= strDistinctId;
    upload(strAliasId, true, "$alias",jObj,strPlatform);
}

void AnalysysCPlusPlusSdk::profileSet(const std::string& strDistinctId, bool bIsLogin, const JObject& jAttributes, const std::string& strPlatform)
{
    upload(strDistinctId, bIsLogin, "$profile_set",jAttributes,strPlatform);    
}

void AnalysysCPlusPlusSdk::profileSetOnce(const std::string& strDistinctId, bool bIsLogin, const JObject& jAttributes, const std::string& strPlatform)
{
    upload(strDistinctId, bIsLogin, "$profile_set_once",jAttributes,strPlatform);    
}

void AnalysysCPlusPlusSdk::profileIncrement(const std::string& strDistinctId, bool bIsLogin, const JObject& proAttributes, const std::string& strPlatform)
{
    upload(strDistinctId, bIsLogin, "$profile_increment",proAttributes,strPlatform);    
}

void AnalysysCPlusPlusSdk::profileAppend(const std::string& strDistinctId, bool bIsLogin, const JObject& proAttributes, const std::string& strPlatform)
{
    upload(strDistinctId, bIsLogin, "$profile_append",proAttributes,strPlatform);    
}

void AnalysysCPlusPlusSdk::profileUnSet(const std::string& strDistinctId, bool bIsLogin, const std::string& strProperty, const std::string& strPlatform)
{
    JObject j;
    j[strProperty] = "";
    upload(strDistinctId, bIsLogin, "$profile_unset",j,strPlatform);    
}

void AnalysysCPlusPlusSdk::profileDelete(const std::string& strDistinctId,  bool bIsLogin, const std::string& strPlatform)
{
    JObject j;
    upload(strDistinctId, bIsLogin, "$profile_delete",j,strPlatform);
}

void AnalysysCPlusPlusSdk::registerSuperAttributes(const JObject& attrAttributes)
{

    int nNum = 100;
    JObject jTemp = attrAttributes;
    if(jTemp.getSize() > nNum)
    {
        throw SDKException("Too many super properties. max number is 100.");
    }
    this->m_pPrivateData->m_jXcontextSuperProperties = (long long)1;
    this->m_pPrivateData->m_jXcontextSuperProperties = attrAttributes;
    if(m_pPrivateData->m_eDebugType != CLOSE)
    {
        std::cout << "registerSuperProperties success" << std::endl;
    }
}

void AnalysysCPlusPlusSdk::unRegisterSuperProperty(const std::string& strKey)
{
    this->m_pPrivateData->m_jXcontextSuperProperties = this->m_pPrivateData->m_jXcontextSuperProperties - strKey;
}

void AnalysysCPlusPlusSdk::clearSuperAttributes()
{
    //delete (m_pPrivateData->m_jXcontextSuperProperties).m_pPriData;
    //m_pPrivateData->m_jXcontextSuperProperties.m_pPriData = NULL;
    JObject j;
    m_pPrivateData->m_jXcontextSuperProperties = j;
    m_pPrivateData->m_jXcontextSuperProperties = j;
    if(this->m_pPrivateData->m_eDebugType != CLOSE)
    {
        std::cout << "clearSuperProperties success" << std::endl;
    }

}

JObject AnalysysCPlusPlusSdk::getSuperProperty(const std::string& strKey)
{
    std::cout <<  strKey << ":" <<m_pPrivateData->m_jXcontextSuperProperties[strKey].toJson()<<std::endl;
    return this->m_pPrivateData->m_jXcontextSuperProperties;
}

const JObject& AnalysysCPlusPlusSdk::getSuperAttributes()
{
    std::cout << m_pPrivateData->m_jXcontextSuperProperties.toJson()<<std::endl;
    return m_pPrivateData->m_jXcontextSuperProperties;
}

bool AnalysysCPlusPlusSdk::flush()
{
    return m_pPrivateData->m_pDefaultConsumer->flush();
}