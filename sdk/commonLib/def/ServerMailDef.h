#pragma once
#pragma  pack(4)
struct ServerMailDef
{
    int			nServerMailId;		//全服邮件id
	unsigned int nActorId;			//角色id
	char			sTitle[200];			//邮件标题
	char			sContent[600];		//邮件内容
	int			nMinVipLevel;			//最低vip等级
	int			nMaxVipLevel;			//最高vip等级
	int			nMinLevel;			//最低人物等级
	int			nMaxLevel;			//最高人物等级
	unsigned int	nMinRegisterTime;		//最早注册时间
	unsigned int	nMaxRegisterTime;		//最晚注册时间	
	unsigned int	nMinLoginTime	;		//最早登录时间
	unsigned int	nMaxLoginTime;		//最晚登录时间
	int			btOnline;			//在线	0全部，1在线，2不在线
	int			btSex;               //性别 -1全部，0男， 1女
	int    nWay;				//用途标识,0系统正常邮件，1后台福利
};
#pragma  pack()