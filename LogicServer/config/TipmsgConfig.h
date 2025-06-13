#pragma once
#include<map>
//系统提示的ID。书写规则：配置文件里增加一条记录，必须在这里增加一个ID，否则后面将全部乱
//每个系统用一个配置文件

typedef struct tagTipmsgConfig
{
	CVector<char *> data;

	/*
	* Comments: 通过系统提示的ID获取系统提示的内容
	* Param INT_PTR nTipmsgID: 系统提示的ID
	* @Return char *: 系统提示的字符串的指针
	*/
	inline char *GetTipmsg(INT_PTR nTipmsgID)
	{
		if(nTipmsgID <0 || nTipmsgID >= data.count() ) 
			return NULL;
		else return data[nTipmsgID];		
	}
}TIPMSGCONFIG, *PTIPMSGCONFIG; 


typedef struct tagNewTipmsgConfig
{
	// CVector<char *> data;
	std::map<int, char[1024]> data;

	/*
	* Comments: 通过系统提示的ID获取系统提示的内容
	* Param INT_PTR nTipmsgID: 系统提示的ID
	* @Return char *: 系统提示的字符串的指针
	*/
	inline char *GetTipmsg(INT_PTR nTipmsgID)
	{
		// if(nTipmsgID <0 || nTipmsgID >= data.count() ) 
		// 	return NULL;
		// else return data[nTipmsgID];		
		if(data.find(nTipmsgID) != data.end())
			return data[nTipmsgID];
		return NULL;
	}
}NEWTIPMSGCONFIG, *PNEWTIPMSGCONFIG; 
