#pragma once

class CLanguageTextProvider :
	protected CCustomLuaConfig,//不能使用CustomLogicLuaConfig!
	public CLangTextSection
{
public:
	typedef CLangTextSection Inherited; 
	typedef CCustomLuaConfig Inherited2;

public:
	CLanguageTextProvider();
	~CLanguageTextProvider();

	//从文件加载语言包
	bool LoadFromFile(LPCTSTR sFileName);

protected:
	void showError(LPCTSTR sError);

private:
	//将当前脚本表中的语言配置读取到Section中
	bool readLanguagePacket(CLangTextSection &Section);

	bool LoadOldLangConfig(TIPMSGCONFIG & dataConfig,LPCTSTR sTableName,LPCTSTR sLangName = "OldLang");
	bool LoadLangConfig(NEWTIPMSGCONFIG & dataConfig,LPCTSTR sTableName,LPCTSTR sLangName = "Lang");

private:
	CObjectAllocator<char>	m_TextAllocator;//语言包内存管理器
	CObjectAllocator<char>	m_LogicDataAllocator;//逻辑的(提示，喊话等)的内存管理器

};
