#pragma once

/***************************************************************/
/*
/*					针对不同版本以及运营商而不同的宏定义配置读取类
/*
/*   在VSPDef.txt中定义针对不同游戏版本或游戏运营商而不同的配置值，使得整个
/* 游戏在保持版本数据一致的情况下可以针对配置文件中的定义而加载不同的配置文件、
/* 决定某些功能是否开启等等。
/*   VSP定义同时也作为游戏中的一种常量配置值提供对象。
/*
/***************************************************************/

class CVSPDefinition
{
public:
	/* 定义各种必需的常量名称 */
	static LPCTSTR SPID;		//运营商ID的定义名称
	static LPCTSTR LANGUAGE;	//语言种类的定义名称
	static LPCTSTR GAMENAME;	//游戏名称的定义名称
	static LPCTSTR BINDCOINNAME;//绑定货币的定义名称
	static LPCTSTR COINNAME;	//货币的定义名称
	static LPCTSTR BINDYBNAME;	//绑定元宝的定义名称
	static LPCTSTR YBNAME;		//元宝的定义名称

	/* 常量对象数据结构定义 */
	typedef struct Definition
	{
		LPTSTR sDefName;	//常量名称
		LPTSTR sDefValue;	//常量值
	}*PDEFINITION;

public:
	CVSPDefinition();
	~CVSPDefinition();

	/*
	* Comments: 通过名称获取定义值
	* Param LPCTSTR sDefName: 名称
	* @Return LPCTSTR: 如果存在指定名称的常量则返回常量定义值否则返回NULL
	*/
	LPCTSTR GetDefinition(LPCTSTR sDefName) const;
	/*
	* Comments: 将所有的定义作为宏增加到脚本预处理器中
	* Param CCustomLuaPreProcessor & preProcessor:
	* @Return VOID:
	*/
	VOID RegisteToPreprocessor(CCustomLuaPreProcessor &preProcessor);
	/*
	* Comments: 从文件加载常量定义
	* Param LPCTSTR sFilePath: 文件路径，可以是基于工作目录的相对路径也可以是绝对路径
	* @Return bool: 加载成功则返回true否则返回false。
	*/
	bool LoadDefinitions(LPCTSTR sFilePath);
	/*
	* Comments: 设置一个常量定义，可以添加、删除以及修改定义
	* Param LPCTSTR sDefName: 定义名称（不能为空）
	* Param LPCTSTR sDefValue:定义值，为空则表示删除此定义否则当定义存在则更新定义，不存在则添加定义
	* @Return VOID:
	*/
	VOID SetDefinition(LPCTSTR sDefName, LPCTSTR sDefValue);

protected:
	/*
	* Comments: 清空所有定义对象并释放内存
	* @Return VOID:
	*/
	VOID ClearDefinitions();

private:
	CBaseList<Definition> m_Defines;
};