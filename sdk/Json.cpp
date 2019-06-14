#include "Json.h"


#include <vector>
#include <map>
#include <string>
#include <stdio.h> 
#include "AnalysysCppSdkCommType.h"

#define MAX_SHOW_LOG_LEN 512
#define MAX_SHOW_LOG_FORMAT_LEN 100

enum JType
{
    JBOOL,
    JINT,
    JSTRING,
    JDOUBLE,
    JLIST,
    JMAP,
    JTIME,
};

/// <summary>
/// Json处理类基类
/// 不支持乱码
/// </summary>
class JBase
{
public:
    /// <summary>
    /// 获取类型
    /// </summary>
    /// <returns></returns>
    virtual JType getType()=0;
    virtual ~JBase()
    {

    }
    /// <summary>
    /// 转Json字符串虚函数
    /// </summary>
    /// <returns></returns>
    virtual std::string toJson()=0;

    /// <summary>
    /// 克隆
    /// </summary>
    /// <returns></returns>
    virtual JBase* clone()=0;
};

/// <summary>
/// Jbool处理类
/// </summary>
class JBool : public JBase
{
public:
    JBool():m_bValue(false)
    {
    }
    virtual JType getType()
    {
        return JBOOL;
    }
    virtual std::string toJson()
    {
        if(m_bValue)
        {
            return "true";
        }    
        else
        {
            return "false";
        }
    }
    virtual JBase* clone()
    {
        return new JBool(*this);
    }

public:
    bool m_bValue;

};

/// <summary>
/// Jdouble处理类基类
/// </summary>
class JDouble : public JBase
{
public:
    JDouble():m_dValue(-1)
    {
    }
    virtual JType getType()
    {
        return JBOOL;
    }
    virtual std::string toJson()
    {
        return std::to_string((long double)m_dValue);
    
    }
    virtual JBase* clone()
    {
        return new JDouble(*this);
    }
public: 
    double m_dValue;

};


/// <summary>
/// JInt处理类
/// </summary>
class JInt : public JBase
{
public:
    JInt():m_lValue(-1)
    {
    }
    virtual JType getType()
    {
        return JINT;
    }
    virtual std::string toJson()
    {
        return std::to_string(m_lValue);
    }
    virtual JBase* clone()
    {
        return new JInt(*this);
    }
public: 
    long long m_lValue;

};


/// <summary>
/// JInt处理类
/// </summary>
class JTime : public JBase
{
public:
    JTime():m_nSeconds(0),m_nMilliseconds(0)
    {
    }
    virtual JType getType()
    {
        return JTIME;
    }
    /// <summary>
    /// Json只需要秒数，微秒暂时不支持
    /// </summary>
    virtual std::string toJson()
    {        
        return std::to_string(m_nSeconds);
    }
    virtual JBase* clone()
    {
        return new JTime(*this);
    }
public: 
    long long m_nSeconds;
    long long m_nMilliseconds;

};


/// <summary>
/// JString处理类
/// </summary>
class JString : public JBase
{
public:
    JString():m_strValue("")
    {
    }
    virtual JType getType()
    {
        return JSTRING;
    }
    virtual std::string toJson()
    {
        std::string strBuffer;
        DumpString(m_strValue, &strBuffer); 
        return strBuffer;
    }
    virtual JBase* clone()
    {
        return new JString(*this);
    }
public: 
    std::string m_strValue;
private:
    void DumpString(const std::string& strValue, std::string* strBuffer) 
    {
        *strBuffer += '"';
        for (int nIndex = 0; nIndex < (int)strValue.size(); nIndex++) {

            char c = strValue[nIndex];
            switch (c) {
            case '"':
                *strBuffer += "\\\"";
                break;
            case '\\':
                *strBuffer += "\\\\";
                break;
            case '\b':
                *strBuffer += "\\b";
                break;
            case '\f':
                *strBuffer += "\\f";
                break;
            case '\n':
                *strBuffer += "\\n";
                break;
            case '\r':
                *strBuffer += "\\r";
                break;
            case '\t':
                *strBuffer += "\\t";
                break;
            default:
                *strBuffer += c;
                break;
            }
        }
        *strBuffer += '"';
    }
};

/// <summary>
/// JList处理类
/// </summary>
class JList : public JBase
{
public:
    JList():m_nIndex(-1)
    {
    }
    ~JList(void)
    {
        for(std::vector<JBase*>::const_iterator it = m_vecJObj.begin(); it!=m_vecJObj.end(); it++)
        {
            JBase* pJson = (*it);
            if(pJson != NULL)
            {
                delete pJson;
                pJson = NULL;
            }
        }
        m_vecJObj.clear();
    }
    virtual JType getType()
    {
        return JLIST;
    }
    virtual std::string toJson()
    {
        std::string str = "[";
        bool bFirst = true;
        for(std::vector<JBase*>::const_iterator it = m_vecJObj.begin(); it!=m_vecJObj.end(); it++)
        {
            if (bFirst) 
            {
                bFirst = false;
            } 
            else
            {
                str += ',';
            }

            JBase* pJson = (*it);
            if(pJson == NULL)
            {
                 str += "null";
            }
            else
            {
                str+=pJson->toJson();
            }
            
        }
        str += "]";
        return str;
    }

    virtual JBase* clone()
    {
        JList* pRet = new JList;
        pRet->m_nIndex = this->m_nIndex;
        for(std::vector<JBase*>::const_iterator it = m_vecJObj.begin(); it!=m_vecJObj.end(); it++)
        {
            JBase* pItem = *it;
            if(pItem != NULL)
            {
                JBase* pClone = pItem->clone();
                pRet->m_vecJObj.push_back(pClone);
            }
            else
            {
                pRet->m_vecJObj.push_back(NULL);
            }
            
        }
        return pRet;
    }
public: 
    std::vector<JBase*> m_vecJObj;
    int m_nIndex;

};

/// <summary>
/// JMap处理类
/// </summary>
class JMap : public JBase
{
public:
    ~JMap(void)
    {
        for(std::map<std::string, JBase*>::const_iterator it = m_mapJObj.begin(); it!=m_mapJObj.end(); it++)
        {
            JBase* pJson = (*it).second;
            if(pJson == NULL)
            {
                delete pJson;
                pJson = NULL;
            }
        }
        m_mapJObj.clear();

    }
    virtual JType getType()
    {
        return JMAP;
    }
    virtual std::string toJson()
    {
        
        if(m_strOperateKey != "")
        {
            JBase* pBase = m_mapJObj[m_strOperateKey];
            std::string str;
            if(pBase != NULL)
                str = pBase->toJson();
            
            m_strOperateKey = "";
            return str;
        }
        else
        {
            std::string str = "{";
            bool first = true;
            for(std::map<std::string, JBase*>::const_iterator it = m_mapJObj.begin(); it!=m_mapJObj.end(); it++)
            {
                if (first)
                    first = false;
                else
                    str += ", ";
                str += '"';
                str += (*it).first;
                str += '"';
                str += ": ";
                if((*it).second == NULL)
                {
                    str += "null";
                }
                else
                {
                    str += (*it).second->toJson();
                }

            }
            str += "}";
            return str;    
        }
        
    
    }
    virtual JBase* clone()
    {
        JMap* pRet = new JMap;
        pRet->m_strOperateKey = this->m_strOperateKey;
        for(std::map<std::string, JBase*>::const_iterator it = m_mapJObj.begin(); it!=m_mapJObj.end(); it++)
        {
            JBase* pItem = it->second;
            if(pItem != NULL)
            {
                JBase* pClone = pItem->clone();
                pRet->m_mapJObj[it->first] = pClone;
            }
            else
            {
                pRet->m_mapJObj[it->first] = NULL;
            }

        }
        return pRet;
    }
public:      
    std::map<std::string, JBase*> m_mapJObj;
    std::string m_strOperateKey;
};

JObject::JObject()
{
    m_pPriData = NULL;
}

JObject::~JObject()
{

    if(m_pPriData!=NULL)
    {
        delete (JBase*)m_pPriData; 
    }
}

JObject::JObject(const JObject& other)
{
        
    if(other.m_pPriData == NULL)
    {
        this->m_pPriData = NULL;
        return ;
    }
    else
    {
        JBase* pOther = (JBase*)other.m_pPriData;
        this->m_pPriData = pOther->clone();
    }
    
    
}

JObject& JObject::operator=(const JObject& other)
{
    
    if( this == &other)
    {
        return *this;
    }
        
    JBase* pData = (JBase*)m_pPriData;
    if(pData == NULL)
    {
        pData = (JBase*)other.m_pPriData;
        if(pData!=NULL)
            m_pPriData = (void*)pData->clone();
        return *this;
    }


    JType jType = pData->getType();
    if(jType == JLIST)
    {
        //通过nIndex 获取key对应的指针
        JList* pDataList = (JList*)pData;    
                
        //如果size为0时，表示拷贝构造，不为0小标操作
        JBase* pUpdateData = (JBase*)m_pPriData;
        if(pDataList->m_vecJObj.size() > 0)
        {
            pUpdateData = pDataList->m_vecJObj[pDataList->m_nIndex];
        }

        if((void*)pUpdateData == other.m_pPriData)
        {
            return *this;
        }

        if(pUpdateData != NULL)
        {
            delete pUpdateData;
        }
        JBase* pOther = (JBase*)other.m_pPriData;
        pUpdateData = (JBase*)pOther->clone();

            
        if(pDataList->m_vecJObj.size() > 0)
        {
            pDataList->m_vecJObj[pDataList->m_nIndex] = pUpdateData;
        }
        else
        {
            m_pPriData = (void*)pUpdateData;
        
        }

    }
    else if(jType == JMAP)
    {
        //获取key对应的指针
        JMap* pDataMap = (JMap*)pData;    
        JBase* pOther = (JBase*)other.m_pPriData;
        if(pOther == NULL)
        {
            delete (JBase*)m_pPriData;
            m_pPriData = NULL;
            return *this;
        }

        //如果size为0时，表示拷贝构造，不为0小标操作
        JBase* pUpdateData = (JBase*)m_pPriData;

        std::map<std::string, JBase*>::iterator itMap = pDataMap->m_mapJObj.find(pDataMap->m_strOperateKey);
        if(itMap!=pDataMap->m_mapJObj.end())
        {
            pUpdateData = pDataMap->m_mapJObj[pDataMap->m_strOperateKey];

        }

        if((void*)pUpdateData == other.m_pPriData)
        {
            return *this;
        }

        if(pUpdateData != NULL)
        {
            delete pUpdateData;
        }
        
        pUpdateData = (JBase*)pOther->clone();


        if(itMap!=pDataMap->m_mapJObj.end())
        {
            pDataMap->m_mapJObj[pDataMap->m_strOperateKey] = pUpdateData;
            
        }
        else
        {
            m_pPriData = (void*)pUpdateData;

        }
        pDataMap->m_strOperateKey = "";
    }
    else
    {

        JBase* pOther = (JBase*)other.m_pPriData;
        m_pPriData = (void*)pOther->clone();
    }
    
    return *this;

}

JObject& JObject::operator=(bool bValue)
{
    JBase* pData = (JBase*)m_pPriData;
    if(pData == NULL)
    {
        pData = new JBool;
        ((JBool*)pData)->m_bValue = bValue;
        m_pPriData = pData;
    }
    else
    {
        if(pData->getType() == JBOOL)
        {
            ((JBool*)pData)->m_bValue = bValue;
        }
        else if(pData->getType() == JMAP)
        {
            JMap* pJMap = (JMap*)pData;


            JBase* pUpdateData = pJMap->m_mapJObj[pJMap->m_strOperateKey];
            if(pUpdateData != NULL)
            {
                delete pUpdateData;
            }
            pUpdateData = new JBool;
            ((JBool*)pUpdateData)->m_bValue = bValue;
            
            if(pJMap->m_strOperateKey!="")
            {
                pJMap->m_mapJObj[pJMap->m_strOperateKey] = pUpdateData;
                pJMap->m_strOperateKey = "";
            }
            else
            {
                delete pJMap;
                this->m_pPriData = pUpdateData;
            }
            
        
        }
        else if(pData->getType() == JLIST)
        {
            JList* pJList = (JList*)pData;
            JBase* pUpdateData = pJList->m_vecJObj[pJList->m_nIndex];
            if(pUpdateData != NULL)
            {
                delete pUpdateData;
            }
            pUpdateData = new JBool;
            ((JBool*)pUpdateData)->m_bValue = bValue;
            pJList->m_vecJObj[pJList->m_nIndex] = pUpdateData;
            
        }
        else
        {
            delete pData;
            pData = new JBool;
            ((JBool*)pData)->m_bValue = bValue;
            m_pPriData = pData;
        }
        
    }
    return *this;

}

JObject& JObject::operator=(long long lValue)
{
    
    JBase* pData = (JBase*)m_pPriData;
    if(pData == NULL)
    {
        pData = new JBool;
        ((JInt*)pData)->m_lValue = lValue;
        m_pPriData = pData;
    }
    else
    {
        if(pData->getType() == JINT)
        {
            ((JInt*)pData)->m_lValue = lValue;
        }
        else if(pData->getType() == JMAP)
        {
            JMap* pJMap = (JMap*)pData;
            JBase* pUpdateData = pJMap->m_mapJObj[pJMap->m_strOperateKey];
            if(pUpdateData != NULL)
            {
                delete pUpdateData;
            }
            pUpdateData = new JInt;
            ((JInt*)pUpdateData)->m_lValue = lValue;
            

            if(pJMap->m_strOperateKey!="")
            {
                pJMap->m_mapJObj[pJMap->m_strOperateKey] = pUpdateData;
                pJMap->m_strOperateKey = "";
            }
            else
            {
                delete pJMap;
                this->m_pPriData = pUpdateData;
            }

        }
        else if(pData->getType() == JLIST)
        {
            JList* pJList = (JList*)pData;
            JBase* pUpdateData = pJList->m_vecJObj[pJList->m_nIndex];
            if(pUpdateData != NULL)
            {
                delete pUpdateData;
            }
            pUpdateData = new JInt;
            ((JInt*)pUpdateData)->m_lValue = lValue;
            pJList->m_vecJObj[pJList->m_nIndex] = pUpdateData;

        }
        else
        {
            delete pData;
            pData = new JInt;
            ((JInt*)pData)->m_lValue = lValue;
            m_pPriData = pData;
        }

    }
    return *this;
}

JObject& JObject::operator=(const char* szValue)
{
    if(szValue == NULL)
    {
        return *this;
    }

    JBase* pData = (JBase*)m_pPriData;
    if(pData == NULL)
    {
        pData = new JString;
        ((JString*)pData)->m_strValue = szValue;
        m_pPriData = pData;
    }
    else
    {
        if(pData->getType() == JSTRING)
        {
            ((JString*)pData)->m_strValue = szValue;
        }
        else if(pData->getType() == JMAP)
        {
            JMap* pJMap = (JMap*)pData;
            JBase* pUpdateData = pJMap->m_mapJObj[pJMap->m_strOperateKey];
            if(pUpdateData != NULL)
            {
                delete pUpdateData;
            }
            pUpdateData = new JString;
            ((JString*)pUpdateData)->m_strValue = szValue;
            

            if(pJMap->m_strOperateKey!="")
            {
                pJMap->m_mapJObj[pJMap->m_strOperateKey] = pUpdateData;
                pJMap->m_strOperateKey = "";
            }
            else
            {
                delete pJMap;
                this->m_pPriData = pUpdateData;
            }



        }
        else if(pData->getType() == JLIST)
        {
            JList* pJList = (JList*)pData;
            JBase* pUpdateData = pJList->m_vecJObj[pJList->m_nIndex];
            if(pUpdateData != NULL)
            {
                delete pUpdateData;
            }
            pUpdateData = new JString;
            ((JString*)pUpdateData)->m_strValue = szValue;
            pJList->m_vecJObj[pJList->m_nIndex] = pUpdateData;

        }
        else
        {
            delete pData;
            pData = new JString;
            ((JString*)pData)->m_strValue = szValue;
            m_pPriData = pData;
        }

    }
    return *this;

}
JObject& JObject::operator=(const std::string& strValue)
{

    JBase* pData = (JBase*)m_pPriData;
    if(pData == NULL)
    {
        pData = new JString;
        ((JString*)pData)->m_strValue = strValue;
        m_pPriData = pData;
    }
    else
    {
        if(pData->getType() == JSTRING)
        {
            ((JString*)pData)->m_strValue = strValue;
        }
        else if(pData->getType() == JMAP)
        {
            JMap* pJMap = (JMap*)pData;
            JBase* pUpdateData = pJMap->m_mapJObj[pJMap->m_strOperateKey];
            if(pUpdateData != NULL)
            {
                delete pUpdateData;
            }
            pUpdateData = new JString;
            ((JString*)pUpdateData)->m_strValue = strValue;
            

            if(pJMap->m_strOperateKey!="")
            {
                pJMap->m_mapJObj[pJMap->m_strOperateKey] = pUpdateData;
                pJMap->m_strOperateKey = "";
            }
            else
            {
                delete pJMap;
                this->m_pPriData = pUpdateData;
            }

        }
        else if(pData->getType() == JLIST)
        {
            JList* pJList = (JList*)pData;
            JBase* pUpdateData = pJList->m_vecJObj[pJList->m_nIndex];
            if(pUpdateData != NULL)
            {
                delete pUpdateData;
            }
            pUpdateData = new JString;
            ((JString*)pUpdateData)->m_strValue = strValue;
            pJList->m_vecJObj[pJList->m_nIndex] = pUpdateData;

        }
        else
        {
            delete pData;
            pData = new JString;
            ((JString*)pData)->m_strValue = strValue;
            m_pPriData = pData;
        }

    }
    return *this;

}
JObject& JObject::operator[](const std::string& strKey)
{
    
    return (*this)[strKey.c_str()];;
}
/// <summary>
/// 键值对处理,JMap对象处理
/// </summary>
JObject& JObject::operator[](const char* szKey)
{
    if(szKey == NULL)
    {
        return *this;
    }
    
    std::string strKey = szKey;
    JBase* pData = (JBase*)m_pPriData;
    if(pData == NULL)
    {
        pData = new JMap;    
        m_pPriData = pData;
    }
    else
    {
        //m_pPriData不是JMap对象，则要释放原有内存，分配新内存
        if(pData->getType() != JMAP)
        {
            delete pData;
            pData = new JMap;
            m_pPriData = pData;
        }

    }

    
    //KEY不存在则添加key,并存下当前操作的key，value赋值由=操作符重载处理
    JMap* pJMap = (JMap*)pData;
    std::map<std::string, JBase*>::iterator it = pJMap->m_mapJObj.find(strKey);
    if(it == pJMap->m_mapJObj.end())
    {
        
        pJMap->m_mapJObj[strKey] =     NULL;
        

        
    }
    pJMap->m_strOperateKey = strKey;
    return *this;    
}

JObject& JObject::operator[](int nIndex)
{
    if(nIndex < 0)
    {
        return *this;
    }
    
    JBase* pData = (JBase*)m_pPriData;
    if(pData == NULL)
    {
        pData = new JList;    
        m_pPriData = pData;
    }
    else
    {
        //m_pPriData不是JList对象，则要释放原有内存，分配新内存
        if(pData->getType() != JLIST)
        {
            delete pData;
            pData = new JList;
            m_pPriData = pData;
        }

    }

    //判断nIndex值是否大于原有大小，如果大于则由重新分配大小
    JList* pJList = (JList*)pData;
    int nSize = pJList->m_vecJObj.size();
    for(int i = 0; i < nIndex-nSize+1;i++)
    {
    
        pJList->m_vecJObj.push_back(NULL);
    }
    pJList->m_nIndex = nIndex; 
    return *this;
        
}

JObject& JObject::operator+(const JObject &other)
{

    JBase* pBase = (JBase*)m_pPriData;
    JBase* pBaseOther = (JBase*)other.m_pPriData;
    if(pBase==NULL || pBaseOther == NULL)
        return *this;

    if(pBaseOther->getType() == pBase->getType() && pBase->getType() == JMAP)
    {
        JMap* pMap = (JMap*)pBase;
        JMap* pMapOther = (JMap*)pBaseOther;
        for(std::map<std::string, JBase*>::iterator it = pMapOther->m_mapJObj.begin(); it != pMapOther->m_mapJObj.end();it++)
        {
            //if(pMap->m_mapJObj.find(it->first) == pMap->m_mapJObj.end())
            //{
            //    pMap->m_mapJObj[it->first] = it->second;
            //}
            pMap->m_mapJObj[it->first] = it->second;
        }
        
    }
    else if(pBase->getType() == JLIST)
    {
        JList* pJList = (JList*)pBase;
        JBase* pAdd = pBaseOther->clone();
        pJList->m_vecJObj.push_back(pAdd);
    }
    return *this;
}

JObject& JObject::operator-(const std::string& strKey)
{
    JBase* pBase = (JBase*)m_pPriData;
    if(pBase==NULL)
        return *this;

    if( pBase->getType() == JMAP)
    {
        JMap* pMap = (JMap*)pBase;
        pMap->m_mapJObj.erase(strKey);

    }
    return *this;
}
JObject& JObject::operator-(const int nListIndex)
{
    JBase* pBase = (JBase*)m_pPriData;
    if(pBase==NULL)
        return *this;

    if( pBase->getType() == JLIST)
    {
        JList* pJList = (JList*)pBase;
        if(nListIndex < pJList->m_vecJObj.size())
        {
            std::vector<JBase*>::iterator it = pJList->m_vecJObj.begin();
            for(int nIndex = 0; nIndex <= nListIndex; nIndex++)
                it++;
            pJList->m_vecJObj.erase(it);
        }
        

    }
    return *this;
}
std::string JObject::toJson()
{
    JBase* pBase = (JBase*)m_pPriData;
    if(pBase == NULL)
        return "";
    return pBase->toJson();
}

int JObject::getSize()
{
    int nSize = 0;
    JBase* pBase = (JBase*)m_pPriData;
    if(pBase != NULL && pBase->getType() == JLIST)
    {
        nSize = ((JList*)pBase)->m_vecJObj.size();
    }
    else if(pBase != NULL && pBase->getType() == JMAP)
    {
        nSize = ((JMap*)pBase)->m_mapJObj.size();
    }
    return nSize;
}


class MyRegex
{
#define  MYREGEX_HEADER_SIZE 7
public:
    static bool checkKeyPattern(const std::string& strKey, int nMaxCount)
    {
        bool bRes = true;
        //int nMaxCount = 99;
        int nCount = 0;
        const char* szHeaders[MYREGEX_HEADER_SIZE] = {"xwhat", "xwhen", "xwho", "appid", "xcontext", "\\lib",  "\\$lib_version"};
        std::string strLowChars="abcdefghijkmlnopqrstuvwxyz";
        std::string strHighChars="ABCDEFGHIJKMLNOPQRSTUVWXYZ";
        std::string strNumberChars="$0123456789_";

        for(int nIndex = 0; nIndex < strKey.size(); nIndex++)
        {
            //以$ 字母开头
            if(nIndex == 0)
            {
                if(strKey[0] != '$' && findCharPos(strLowChars, strKey[0]) == std::string::npos && findCharPos(strHighChars, strKey[0]) == std::string::npos)
                {
                    return false;
                }
            }

            if(strKey[nIndex] == '\\')
            {
                int nPos1 = strKey.find("\\lib", nIndex);
                int nPos2 = strKey.find("\\$lib_version", nIndex);
                if(nPos1!=nIndex && nPos2!=nIndex)
                {
                    return false;
                }

            }
            else
            {
                int nPosLC = findCharPos(strLowChars, strKey[nIndex]);
                int nPosHC = findCharPos(strHighChars, strKey[nIndex]);
                int nPosNumber = findCharPos(strNumberChars, strKey[nIndex]);
                if(nPosLC == std::string::npos && nPosHC == std::string::npos && nPosNumber == std::string::npos)
                {
                    return false;
                }
            }
        }
        for(int nIndex = 0; nIndex < MYREGEX_HEADER_SIZE; nIndex++)
        {
            int nCounter = 0;
            int nFindPos = 0;
            while(std::string::npos != nFindPos)
            {

                nFindPos = strKey.find(szHeaders[nIndex], nFindPos);
                if(nFindPos != std::string::npos)
                {
                    std::string strTemp = szHeaders[nIndex];
                    nFindPos+=strTemp.size();
                    nCounter++;
                }
                else
                {
                    break;
                }

            }

            if(nCounter > nMaxCount)
            {
                return false;
            }

        }
        return true;
    }

    static bool checkkeyPatternContext(const std::string& strkeyContext, int nMaxCount)
    {
        //125
        return checkKeyPattern(strkeyContext, nMaxCount);
    }

    static int findCharPos(const std::string& strChars, const char& c)
    {
        return strChars.find(c);
    }

};

void JObject::checkParam(const std::string& strEventName, const JObject& jProperties)
{
    int nIdLength = 255;
    int nKeyLength = 125;
    int nValueListLen = 100;
    std::string strPiEventName = "$profile_increment";
    std::string strPaEventName = "$profile_append";
    
    if(jProperties.m_pPriData == NULL)
        return;

    //只校验传入属性是map的情况
    JBase* pBase = (JBase*)jProperties.m_pPriData;
    if(pBase->getType() != JMAP)
    {
        return ;
    }

    JMap* pMap = (JMap*)pBase;
    const std::string strJson = pMap->toJson();

    //日志buf
    char szErr[MAX_SHOW_LOG_LEN];
    for(std::map<std::string, JBase*>::iterator itMap = pMap->m_mapJObj.begin(); itMap != pMap->m_mapJObj.end(); itMap++)
    {
        if(itMap->second == NULL )
            continue;
        JBase* pMapItem = itMap->second;
        std::string strShowKey = itMap->first;
        if(strShowKey.size() > MAX_SHOW_LOG_LEN - MAX_SHOW_LOG_FORMAT_LEN)
        {
            strShowKey = strShowKey.substr(0, MAX_SHOW_LOG_LEN - MAX_SHOW_LOG_FORMAT_LEN-1);
        }

        if (itMap->first.length() > nKeyLength) 
        {

            sprintf(szErr, "The property key %s is too long, max length is %d.", strShowKey.c_str(), nKeyLength);
            throw SDKException(szErr);
        }

        
        if (!MyRegex::checkkeyPatternContext(itMap->first, 125))
        {
            sprintf(szErr, "The property key %s is invalid.",strShowKey.c_str());
            throw SDKException(szErr);

        }

        if (pMapItem->getType() == JMAP ) 
        {
            sprintf(szErr, "he property %s is not Number, String, Boolean, List<String>.", strShowKey.c_str());
            throw SDKException(szErr);

        }
        if (pMapItem->getType() == JLIST ) 
        {
            JList* pList = (JList*)pMapItem;
            //数组集合约束 数组或集合内最多包含100条,若为字符串数组或集合,每条最大长度255个字符
            if(pList->m_vecJObj.size() > nValueListLen)
            {
                sprintf(szErr,"The property %s max number should be %d.", strShowKey.c_str(), nValueListLen);
                throw SDKException(szErr);
            }
            //数组item 只可能是JSTRING
            for(int nIndex = 0; nIndex < pList->m_vecJObj.size(); nIndex++)
            {
                JBase* pItem = pList->m_vecJObj[nIndex];
                if (pItem->getType() != JSTRING) 
                {
                    sprintf(szErr, "The property %s should be a list of String.", strShowKey.c_str());
                    throw SDKException(szErr);
                }

                if (pItem->toJson().size() > nIdLength)
                {
                    sprintf(szErr, "The property %s some value is too long, max length is %d.", strShowKey.c_str(), nIdLength);
                    throw SDKException(szErr);
                }
            }
        }

        std::string strMapItemJson = pMapItem->toJson();
        if (strMapItemJson.length() > nIdLength)
        {
            sprintf(szErr, "The property key's(%s) value String is too long, max length is %d.", strShowKey.c_str(), nIdLength);
            throw SDKException(szErr);
        }

        if (strEventName == strPiEventName && !(pMapItem->getType() == JINT))
        {
            sprintf(szErr, "The property value of %s should be a Number.", strShowKey.c_str());
            throw SDKException(szErr);
        }
        if (strPaEventName==strEventName) 
        {
            if (pMapItem->getType() != JLIST )
            {
                sprintf(szErr, "The property key of %s should be a List<String>.", strShowKey.c_str());
                throw SDKException(szErr);
            }
            else
            {
                JList* pList = (JList*)pMapItem;
                for(int nIndex = 0; nIndex < pList->m_vecJObj.size(); nIndex++)
                {
                    JBase* pItem = pList->m_vecJObj[nIndex];
                    if (pItem->getType() != JSTRING) 
                    {
                        sprintf(szErr, "The property key %s should be a list of String.", strShowKey.c_str());
                        throw SDKException(szErr);
                    }
                }
            }
        }


    }


}

void JObject::checkProperty(const std::string& strDistinctId, const std::string& strEventName, const JObject& jProperties, int nCommProLen)
{

    
    std::string strAliasEventName = "$alias";
    std::string strProfileEventName = "$profile";
    std::string strOriginalId = "$original_id";
    int nEventNameLen = 99;
    int nConnonParamLen = 5;
    int nIdLength = 255;
    int nTotalParamLen = 300;

    int nMapSize = 0;
    JBase* pBase = (JBase*)jProperties.m_pPriData;

    char szErr[MAX_SHOW_LOG_LEN];
    if (strDistinctId.length() == 0) 
    {
        sprintf(szErr, "strDistinctId is empty.");
        throw SDKException(szErr);

    }

    std::string strOrgTempId = "";
    if(pBase!=NULL&&pBase->getType()==JMAP)
    {
        JMap* pMap = (JMap*)pBase;
        auto iter = pMap->m_mapJObj.find(strOriginalId);
        if(iter != pMap->m_mapJObj.end())
        {
            strOrgTempId = iter->second->toJson();
        }
        nMapSize = pMap->m_mapJObj.size();

    }
    else if(pBase!=NULL&&pBase->getType()==JLIST)
    {
        JList* pList = (JList*)pBase;
        nMapSize = pList->m_vecJObj.size();

    }


    if(strOrgTempId.length() > MAX_SHOW_LOG_LEN - MAX_SHOW_LOG_FORMAT_LEN)
    {
        strOrgTempId = strOrgTempId.substr(0, MAX_SHOW_LOG_LEN - MAX_SHOW_LOG_FORMAT_LEN-1);
    }

    if (strDistinctId.length() > nIdLength) 
    {

        sprintf(szErr, "strDistinctId %s is too long, max length is %d.", strDistinctId.c_str(), nIdLength);
        throw SDKException(szErr);
    }

    if (strEventName == strAliasEventName) 
    {
        if (strOrgTempId.length() == 0) 
        {
            sprintf(szErr, "original_id %s is empty.", strOrgTempId.c_str());
            throw SDKException(szErr);

        }
        if (strOrgTempId.length() > nIdLength) 
        {
            sprintf(szErr, "original_id %s is too long, max length is %d.", strOrgTempId.c_str(), nIdLength);
            throw SDKException(szErr);
            
        }


    }

    if (strEventName.length() == 0) 
    {
        sprintf(szErr, "EventName is empty.");
        throw SDKException(szErr);

    }
    std::string strEventNameShow = strEventName;
    if(strEventNameShow.length() > MAX_SHOW_LOG_LEN - MAX_SHOW_LOG_FORMAT_LEN)
    {
        strEventNameShow = strEventNameShow.substr(0, MAX_SHOW_LOG_LEN - MAX_SHOW_LOG_FORMAT_LEN-1);
    }

    if (strEventName.length() > nEventNameLen)
    {
        sprintf(szErr, "EventName %s is too long, max length is %d.", strEventNameShow.c_str(), nEventNameLen);
        throw SDKException(szErr);

    }

    if (!MyRegex::checkkeyPatternContext(strEventName, 99))
    {
        sprintf(szErr, "EventName %s is invalid.", strEventNameShow.c_str());
        throw SDKException(szErr);

    }

    int nPosProfile = strEventName.find(strProfileEventName);
    int nPosAlias = strEventName.find(strAliasEventName);


    //xcontext属性值不大于300个
    if(nPosAlias != 0 && nPosProfile != 0)
    {
        if(nMapSize + nCommProLen + nConnonParamLen > nTotalParamLen)
        {
            sprintf(szErr, "Too many attributes. max number is %d.", (nTotalParamLen - nCommProLen - nConnonParamLen));
            throw SDKException(szErr);
        }
    } 
    else 
    {
        if(nMapSize + nConnonParamLen > nTotalParamLen)
        {
            sprintf(szErr, "Too many attributes. max number is %d.", (nTotalParamLen - nConnonParamLen));
            throw SDKException(szErr);
        }
    }

    if(jProperties.m_pPriData != NULL)
        checkParam(strEventName, jProperties);

}
