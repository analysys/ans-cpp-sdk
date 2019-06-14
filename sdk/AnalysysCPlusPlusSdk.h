#ifndef _ANALYSYS_CPLUS_PLUS_SDK_
#define _ANALYSYS_CPLUS_PLUS_SDK_

#include "Json.h"
#include "AnalysysCppSdkCommType.h"

struct SdkData;
/// <summary>
/// 说明：字符串编码请使用UTF-8编码
/// </summary>
class AnalysysCPlusPlusSdk 
{
public:
    AnalysysCPlusPlusSdk(void);
    ~AnalysysCPlusPlusSdk(void);
    /// <summary>
    /// 初始化 SDK
    /// </summary>
    /// <param name="strServerUrl">网站获取的App key</param>
    /// <param name="strAppKey">AppKey</param>
    /// <returns></returns>
    bool init(const std::string& strServerUrl, const std::string& strAppKey);

    /// <summary>
    /// 设置Debug模式
    /// </summary>
    void setDebugMode(int eDebug);
    /// <summary>
    /// 实时上传
    /// </summary>
    /// <returns></returns>
    void syncCollecter();
    /// <summary>
    /// 批量上传
    /// </summary>
    /// <param name="batchNum">batchNum：批量发送数量，默认值：1条,最大30条</param>
    /// <param name="nBatchSec">nBatchSec：批量发送等待时间(秒)，默认值：10秒</param>
    /// <returns></returns>
    void batchCollecter(int nBatchNum=20, int nBatchSec = 10);
    /// <summary>
    /// 记录行为事件
    /// </summary>
    /// <param name="strDistinctId">用户 ID,长度大于 0 且小于 255字符</param>
    /// <param name="bIsLogin">用户 ID 是否是登录 ID</param>
    /// <param name="strEventName">事件名</param>
    /// <param name="jAttributes">事件属性,最多包含 100条,且 key 以字母或 $ 开头，可包含字母、数字、下划线和 $，字母不区分大小写，$ 开头为预置事件属性,
    /// 最大长度 125字符,不支持乱码和中文,value 类型约束(String/Number/boolean/list/数组)，若为字符串,最大长度255字符</param>
    /// <returns></returns>
    void track(const std::string& strDistinctId, bool bIsLogin, const std::string& strEventName, const JObject& jAttributes, const std::string& strPlatform);

    /// <summary>
    /// 用户关联
    /// 用户 ID 关联接口。将 strAliasId 和 strDistinctId 关联，计算时会认为是一个用户的行为。该接口是在 distinctId 发生变化的时候调用，来告诉 SDK distinctId 变化前后的 ID 对应关系。
    ///    该场景一般应用在用户注册/登录的过程中。比如：一个匿名用户浏览商品，系统为其分配的strDistinctId = "1234567890987654321"，随后该匿名用户进行注册，系统为其分配了新的注册 ID
    /// </summary>
    /// <param name="strAliasId">用户注册 ID，长度大于 0，且小于 255字符</param>
    /// <param name="strDistinctId">用户匿名ID，长度大于 0，且小于 255字符</param>
    /// <returns></returns>
    void alias(const std::string& strAliasId, const std::string& strDistinctId, const std::string& strPlatform);

    /// <summary>
    /// SDK提供以下接口供用户设置用户的属性，比如用户的年龄/性别等信息。
    /// 用户属性是一个标准的K-V结构，K和V均有相应的约束条件，如不符合则丢弃该次操作。 参数约束:
    /// 属性名称
    ///    以字母或$开头，可包含字母、数字、下划线和$，字母不区分大小写，$开头为预置事件属性,最大长度125字符,不支持乱码和中文
    ///    属性值
    ///    支持部分类型：string/number/boolean/集合/数组; 若为字符串,则最大长度255字符; 若为数组或集合,则最多包含100条,且key约束条件与属性名称一致,value最大长度255字符
    ///    设置单个或多个属性，如用户所在城市，用户昵称，用户头像信息等。如果之前存在，则覆盖，否则，新创建。
    /// </summary>
    /// <param name="strDistinctId">用户 ID,长度大于 0 且小于 255字符</param>
    /// <param name="bIsLogin">用户 ID 是否是登录 ID</param>
    /// <param name="AttributesNode">事件属性,最多包含 100条,且 key 以字母或 $ 开头，可包含字母、数字、下划线和 $，字母不区分大小写，$ 开头为预置事件属性,
    /// 最大长度 125字符,不支持乱码和中文,value 类型约束(String/Number/boolean/list/数组)，若为字符串,最大长度255字符</param>
    /// <returns></returns>
    void profileSet(const std::string& strDistinctId, bool bIsLogin, const JObject& jAttributes, const std::string& strPlatform);

    /// <summary>
    /// 只在首次设置时有效的属性。如：用户的注册时间。如果被设置的用户属性已存在，则这条记录会被忽略而不会覆盖已有数据，如果属性不存在则会自动创建。
    /// </summary>
    /// <param name="strDistinctId">用户 ID,长度大于 0 且小于 255字符</param>
    /// <param name="bIsLogin">用户 ID 是否是登录 ID</param>
    /// <param name="AttributesNode">事件属性,最多包含 100条,且 key 以字母或 $ 开头，可包含字母、数字、下划线和 $，字母不区分大小写，$ 开头为预置事件属性,
    /// 最大长度 125字符,不支持乱码和中文,value 类型约束(String/Number/boolean/list/数组)，若为字符串,最大长度255字符</param>
    /// <returns></returns>
    void profileSetOnce(const std::string& strDistinctId, bool bIsLogin, const JObject& jAttributes, const std::string& strPlatform);

    /// <summary>
    /// 设置用户属性相对变化值    设置用户属性的单个相对变化值(相对增加,减少)，只能对数值型属性进行操作，如果这个Profile之前不存在,则初始值为0。
    /// </summary>
    /// <param name="strDistinctId">用户 ID,长度大于 0 且小于 255字符</param>
    /// <param name="bIsLogin">用户 ID 是否是登录 ID</param>
    /// <param name="AttributesNode">事件属性,最多包含 100条,且 key 以字母或 $ 开头，可包含字母、数字、下划线和 $，字母不区分大小写，$ 开头为预置事件属性,
    /// 最大长度 125字符,不支持乱码和中文,value 类型约束(String/Number/boolean/list/数组)，若为字符串,最大长度255字符</param>
    /// <returns></returns>
    void profileIncrement(const std::string& strDistinctId, bool bIsLogin, const JObject& proAttributes, const std::string& strPlatform);

    /// <summary>
    /// 增加列表类型的属性
    /// 为列表类型的属性增加一个或多个元素，如：用户新增兴趣爱好
    ///    示例：用户初始填写的兴趣爱好为["户外活动"，"足球赛事"，"游戏"]，调用该接口追加["学习"，"健身"]，则用户的爱好变为["户外活动"，"足球赛事"，"游戏"，"学习"，"健身"]
    /// </summary>
    /// <param name="strDistinctId">用户 ID,长度大于 0 且小于 255字符</param>
    /// <param name="bIsLogin">用户 ID 是否是登录 ID</param>
    /// <param name="strProperty">事件属性,最多包含 100条,且 key 以字母或 $ 开头，可包含字母、数字、下划线和 $，字母不区分大小写，$ 开头为预置事件属性,
    /// 最大长度 125字符,不支持乱码和中文,value 类型约束(String/Number/boolean/list/数组)，若为字符串,最大长度255字符</param>
    /// <returns></returns>
    void profileAppend(const std::string& strDistinctId, bool bIsLogin, const JObject& proAttributes, const std::string& strPlatform);

    /// <summary>
    /// 删除单个用户属性
    /// </summary>
    /// <param name="strDistinctId">用户 ID,长度大于 0 且小于 255字符</param>
    /// <param name="bIsLogin">用户ID是否是登录 ID</param>
    /// <param name="property">事件属性</param>
    /// <returns></returns>        
    void profileUnSet(const std::string& strDistinctId, bool bIsLogin, const std::string& strProperty, const std::string& strPlatform);

    /// <summary>
    /// 清除已经设置的所有用户属性
    /// </summary>
    /// <param name="strDistinctId">用户 ID,长度大于 0 且小于 255字符</param>
    /// <param name="bIsLogin">用户 ID 是否是登录 ID</param>
    /// <param name="AttributesNode">事件属性</param>
    /// <returns></returns>
    void profileDelete(const std::string& strDistinctId,  bool bIsLogin, const std::string& strPlatform) ;

    /// <summary>
    /// 注册通用属性
    /// </summary>
    /// <param name="Attributes">设置多个属性</param>
    /// <returns></returns>    
    void registerSuperAttributes(const JObject& attrAttributes);

    /// <summary>
    /// 删除某个通用属性
    /// </summary>
    /// <param name="key">属性名</param>
    /// <returns></returns>
    void unRegisterSuperProperty(const std::string& strKey);

    /// <summary>
    /// 删除全部的通用属性
    /// </summary>
    /// <returns></returns>
    void clearSuperAttributes();

    /// <summary>
    /// 由属性名称查询获取单条通用属性
    /// </summary>
    /// <param name="key">属性名</param>
    /// <returns></returns>
    JObject getSuperProperty(const std::string& strKey);

    /// <summary>
    /// 获取全部的通用属性
    /// </summary>
    /// <returns></returns>
    const JObject& getSuperAttributes();

    /// <summary>
    /// 将所有本地数据发送到服务端
    /// </summary>
    /// <returns></returns>
    bool flush();
private:
    void upload(const std::string& strDistinctId, bool bIsLogin, const std::string& strEventName, const JObject& jAttributes, const std::string& strPlatform);
    SdkData* m_pPrivateData;
};
#endif


