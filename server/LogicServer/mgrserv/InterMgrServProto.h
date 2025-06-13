#pragma once

/************************************************************************/
/* 
/*              与管理后台服务器的通信消息定义
/*
/************************************************************************/

 //后台发给逻辑服务器的
#define 	MSS_KICKPLAY				10000	//踢角色下线（数据段为编码后的角色名称）
#define 	MSS_KICKUSER				10001	//踢账号下线（数据段为编码后的账号字符串）
#define 	MSS_QUERYPLAYONLINE			10002	//查询角色是否在线（数据段为编码后的角色名称）
#define 	MSS_QUERYUSERONLINE			10003	//查询账号是否在线（数据段为编码后的账号字符串）
#define 	MSS_ADDNOTICE				10004	//添加公告
#define 	MSS_DELNOTICE				10005	//删除公告
#define 	MSS_DELAY_UPHOLE			10006	//进入倒计时维护状态(param倒计时秒数)
#define 	MSS_CANLCE_UPHOLE			10007	//取消倒计时维护状态
#define 	MSS_SHUTUP					10008	//禁言(Param  时间, 单位是分钟,数据段为编码后的角色名称)
#define 	MSS_RELEASESHUTUP			10009	//解禁言（数据段为编码后的角色名称）
#define		MSS_OPEN_COMMONSERVER		10010	//开启跨服
#define		MSS_CLOSE_COMMONSERVER		10011	//关闭跨服
#define		MSS_SET_CHATLEVEL			10012	//后台设置聊天等级
#define		MSS_DELAY_COMBINE			10013	//后台设置合服倒计时(param为时间（小时），最后跟开启时间,格式如：2013-01-01 10:0:0)
#define		MSS_SET_REFRESHCORSS		10014	//后台刷新跨服配置
#define		MSS_SET_COMMON_SRVID		10015	//设置跨服的服务器ID
#define		MSS_CLOSE_NOTICE			10016	//删除所有公告
#define		MSS_DELTE_NOTICEBYID		10017	//删除公告（根据Id） 字符串：公告id
#define		MSS_SET_SPEED_CHECK			10018   //设置加速外挂检测的参数  字符串:时间比率|开始检测的次数|加速检查值
#define		MSS_SETCHECK_SPEED			10019	//设置检测外挂的标志 字符串:状态(0关闭 1开启)|外挂处理方法(0踢下线，1小黑屋)
#define		MSS_SET_HOTUPDATE			10020	//热更新脚本
#define		MSS_KICK_ALLACTORS			10021	//踢掉本服所有的玩家
#define		MSS_SET_ACTORSTATUS			10022	//后台T人与禁用用户
#define		MSS_SET_SESSIONCMD			10023	//转发后台消息给会话服 字符串
#define		MSS_SEND_MAIL				10024	//后台发邮件(应该没用到)
#define		MSS_COMPENSATE				10025	//后台开启补偿 数据段为：（标记|补偿方案ID|补偿时间(分钟)）
#define		MSS_GET_ONLINEACTOR			10026	//获取在线人数
#define		MSS_OPEN_THELINEACTIVITY    10027   //平台链接活动
#define		MSS_ADD_FILTERWORDS			10028	//添加屏蔽字
#define		MSS_SET_EXP_RATE			10029	//设置多倍经验
#define		MSS_SET_REMOVEMONEY			10030	//后台删除金钱
#define		MSS_SET_REMOVEITEM			10031	//后台删除物品
#define		MSS_10032			10032
#define		MSS_ACT_TOTAL_CHARGE		10033	//累计充值活动
#define		MSS_ACT_DEL_GUILD			10034	//删除行会
#define		MSS_ACT_DEL_GUILD_MEMO		10035	//删除行会公告
#define		MSS_ACT_SET_GUILD_MEMO		10036	//设置行会公告
#define		MSS_SET_GUILD_LEADER		10037	//设置行会会长
#define		MSS_SET_GUILD_CERTIFICATION	10038	//设置官方认证（是否拉人）
#define     MSS_SHUTUP_BY_ID			10039   //用id禁言，解禁言(可离线操作，同时保留在线用名字禁言)
#define     MSS_BROAD_POP_NOTICE		10040   //广播弹框公告
#define     MSS_UPDATE_BROAD_POP_NOTICE		10041   //广播更新弹框公告
#define 	MSS_FCMKICKPLAY				10042	//防沉迷踢角色下线（数据段为编码后的角色名称）
#define 	MSS_DELTE_GUILD				10043	//删除工会
#define		MSS_SET_CHATRECHARGE		10044	//后台设置聊天充值金额
#define		MSS_SET_NEW_CDKEY_YXM       10045	//游戏猫的新CDKEY方式
#define		MSS_SET_TRADINGQUOTA		10046	//后台设置交易额度
#define		MSS_SET_NEW_37_BINDPHONE    10047	//37游戏绑定手机

#define 	MSS_KICK_USER_BY_ID		    10049	//角色ID封禁（数据段为编码后的角色名称） 
//#define 	MSS_KICK_USER_BY_NAME		10048	//角色账号封禁（数据段为编码后的角色名称）
//#define 	MSS_KICK_USER_BY_ACCOUNT	10049	//角色账号封（数据段为编码后的账号字符串）
// #define 	MSS_RELEA_KICK_USER_BY_NAME		10051	//角色账号解封（数据段为编码后的角色名称）
// #define 	MSS_RELEA_KICK_USER_BY_ACCOUNT	10052	//角色账号解封（数据段为编码后的账号字符串）
#define 	MSS_RELEA_KICK_USER_BY_ID   10050	//角色ID解封（数据段为编码后的角色名称）
#define 	MSS_CLEAR_MSG		        10051	//清屏
#define		MSS_ADD_CUSTOMTITLE         10052	//自定义称号 添加
#define		MSS_DEL_CUSTOMTITLE         10053	//自定义称号 删除
 