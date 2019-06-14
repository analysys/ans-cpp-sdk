
#include "Json.h"
#include "AnalysysCPlusPlusSdk.h"
#include <string>
#include <iostream>
using namespace std;



//
#ifdef WIN32
#include <Windows.h>
void ANSItoUTF8(string &strAnsi)  
{
    //获取转换为宽字节后需要的缓冲区大小，创建宽字节缓冲区，936为简体中文GB2312代码页
    UINT nLen = MultiByteToWideChar(936,NULL,strAnsi.c_str(),-1,NULL,NULL);
    WCHAR *wszBuffer = new WCHAR[nLen+1];
    nLen = MultiByteToWideChar(936,NULL,strAnsi.c_str(),-1,wszBuffer,nLen);
    wszBuffer[nLen] = 0;
    //获取转为UTF8多字节后需要的缓冲区大小，创建多字节缓冲区
    nLen = WideCharToMultiByte(CP_UTF8,NULL,wszBuffer,-1,NULL,NULL,NULL,NULL);
    CHAR *szBuffer = new CHAR[nLen+1];
    nLen = WideCharToMultiByte(CP_UTF8,NULL,wszBuffer,-1,szBuffer,nLen,NULL,NULL);
    szBuffer[nLen] = 0;

    strAnsi = szBuffer;
    //内存清理
    delete []wszBuffer;
    delete []szBuffer;
}
#endif

int main(int argc, char* argv[])
{

    
    string APP_ID = "1234";
    string ANALYSYS_SERVICE_URL = "http://192.168.220.167:8089";

    AnalysysCPlusPlusSdk analysys;
    analysys.setDebugMode(2);
    analysys.init(ANALYSYS_SERVICE_URL, APP_ID);
    analysys.syncCollecter();
    string distinctId = "1234567890987654321";

    try
    {
    //浏览商品
    JObject trackPropertie;
    //trackPropertie["麻麻$ip"] = "112.112.112.112";
    //char szTemp[521];
    //memset(szTemp, (int)'b', 521);
    //trackPropertie["$ip"] = szTemp;//"112.112.112.112";

    JObject superPropertie;
    superPropertie["sex"]= "male"; 
    superPropertie["age"]= (long long)23; 
    analysys.registerSuperAttributes(superPropertie);


    JObject bookList;
    bookList[0] = "Thinking in Java";
    trackPropertie["productName"] = bookList;
    trackPropertie["productType"] = "Java book";


    trackPropertie["producePrice"] = (long long)80;
    trackPropertie["shop"] = "xx shop";
    analysys.track(distinctId, false, "ViewProduct", trackPropertie, "JS");


    JObject jPro;
    jPro["userPoint"]=(long long)20;
    analysys.profileIncrement(distinctId, true, jPro, "js");

    analysys.getSuperAttributes();
    //JObject j = analysys.getSuperProperty("555");

    //用户注册登录 
    string registerId = "ABCDEF123456789";
    analysys.alias(registerId, distinctId, "JS");



    //用户信息
    JObject profiles;
    profiles["$city"] = "beijin";
    profiles["$province"] = "beijin";
    profiles["nickName"] = "nickeName123";
    profiles["userLevel"] = (long long)0;
    profiles["userPoint"] = (long long)0;

    JObject interestList;
    interestList[0] = "sports";
    interestList[1] = "football";
    interestList[2] = "game";
    //cout<<interestList.toJson()<<endl;
    profiles["interest"] = interestList;
    analysys.profileSet(registerId, true, profiles, "JS");

    //用户注册时间
    JObject profile_age;
    profile_age["registerTime"] = "20180101101010";
    analysys.profileSetOnce(registerId, true, profile_age, "JS");

    //重新设置公共属性
    analysys.clearSuperAttributes();
    superPropertie = (long long)1;
    superPropertie["userLevel"]=(long long)0; //用户级别
    superPropertie["userPoint"]=(long long)0; ///用户积分
    analysys.registerSuperAttributes(superPropertie);

    //再次浏览商品
    trackPropertie = false;
    trackPropertie["$ip"]="112.112.112.112"; //IP地址
    
    JObject abookList;
    abookList[0]="Thinking in Java";
    trackPropertie["productName"]=bookList;  //商品列表

    string str = "Java书籍";
    //中文需要转码
    #ifdef WIN32
    ANSItoUTF8(str);
    #endif
    trackPropertie["productType"]=str;//商品类别
    trackPropertie["producePrice"]= (long long)80;          //商品价格
    trackPropertie["shop"]= "xx shop";     //店铺名称
    analysys.track(registerId, true, "ViewProduct", trackPropertie, "JS");

    //订单信息
    trackPropertie = (long long)1;
    trackPropertie["orderId"]= "ORDER_12345";
    trackPropertie["price"]=(long long)80;
    analysys.track(registerId, true, "Order", trackPropertie, "JS");

    //支付信息
    trackPropertie = (long long)1;
    trackPropertie["orderId"]= "ORDER_12345";
    trackPropertie["productName"]= "Thinking in Java";
    trackPropertie["productType"]= "Java book";
    trackPropertie["producePrice"]= (long long)80;
    trackPropertie["shop"]= "xx shop";
    trackPropertie["productNumber"]=(long long) 1;
    trackPropertie["price"]= (long long)80;
    trackPropertie["paymentMethod"]= "AliPay";
    analysys.track(registerId, true, "Payment", trackPropertie, "JSsdf");
    analysys.batchCollecter();
    analysys.track(registerId, true, "Payment", trackPropertie, "JSsdf");


    analysys.getSuperAttributes();
    analysys.getSuperProperty("userPoint");

    analysys.profileUnSet(distinctId,true,"abc","js");

    analysys.profileDelete(distinctId, true, "Android");



    analysys.flush();

    }
    catch (SDKException& e)
    {
        std::string strErr = e.what();
        cout << strErr << endl;
    }
    return 0;
}

