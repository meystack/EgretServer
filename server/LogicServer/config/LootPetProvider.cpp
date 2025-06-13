#include "StdAfx.h"
#include "LootPetProvider.h"

CLootPetProvider::CLootPetProvider()
	: Inherited(),m_DataAllocator(_T("LootPetAlloc"))
{
}


CLootPetProvider::~CLootPetProvider()
{
}

bool CLootPetProvider::Load(LPCTSTR sFilePath)
{
	bool Result = false;
	CMemoryStream ms;
	CCustomLuaPreProcessor pp;

	try
	{
		//从文件加载配置脚本
		if ( ms.loadFromFile(sFilePath) <= 0 )
			showErrorFormat(_T("unable to load from %s"), sFilePath);
		//对配置脚本进行预处理
		GetLogicServer()->GetVSPDefine().RegisteToPreprocessor(pp);
		LPCTSTR sText = pp.parse((LPCTSTR)ms.getMemory(), sFilePath);
		//设置脚本内容
		if ( !setScript(sText) )
			showError(_T("syntax error on LootPet config"));
		Result = ReadAllConfig();
	}
	catch (RefString &s)
	{
		OutputMsg(rmError, _T("load LootPet config error: %s"), s.rawStr());
	}
	catch (...)
	{
		OutputMsg(rmError, _T("unexpected error on load LootPet config"));
	}
	//销毁脚本虚拟机
	setScript(NULL);
	return Result;
}

bool CLootPetProvider::ReadAllConfig()
{
	if ( !openGlobalTable("lootPetConfig") )
		return false;
	CDataAllocator dataAllocator;
	size_t nConfigCount = lua_objlen(m_pLua,-1);
	if (enumTableFirst())
	{
		bool boDefault = false;
		int nDefInt = 0;
		BOOL isNameValid = false;
		do 
		{
            if (enumTableFirst())
	        {
                do
                {
                    ONELOOTPETCONFIG item;
                    item.nLootPetId = getFieldInt("id");
                    item.nLootPetType = getFieldInt("type");
                    item.nContinueTime = getFieldInt("PetTime");
                    item.bAutoUse = getFieldBoolean("autouse",&boDefault);
                    item.nLootPetDistance = getFieldInt("nDropPetLootDistance",&nDefInt);
					strncpy(item.sLootPetName, getFieldString("PetName","PetName",&isNameValid), ArrayCount(item.sLootPetName));
                    if(feildTableExists("attr") && openFieldTable("attr"))
                    {
                        int  nCount = (int)lua_objlen(m_pLua,-1);
                        PGAMEATTR pAttr = NULL;
                        item.attri.nCount = nCount;
                        if(nCount > 0)
                        {
                            item.attri.pAttrs = (PGAMEATTR)m_DataAllocator.allocObjects(sizeof(GAMEATTR) * nCount);
                            ZeroMemory(item.attri.pAttrs, sizeof(GAMEATTR) * nCount);
                            pAttr = item.attri.pAttrs;
                        }
                        if(enumTableFirst())
                        {
                            if(!LoadAttri(pAttr))
                            {
                                return false;
                            }
                            pAttr++;
                        }
                        closeTable();
                    }
                    m_mLootPetConfs[item.nLootPetId] = item;
                    /* code */
                } while (enumTableNext());
            }
		} while (enumTableNext());
	}
	closeTable();
	// m_DataAllocator.~CObjectAllocator();
	// m_DataAllocator = dataAllocator;
	// ZeroMemory(&dataAllocator, sizeof(dataAllocator));
	
	return true;
}


ONELOOTPETCONFIG* CLootPetProvider::GetLootPetConfig(WORD wId)
{
	if(m_mLootPetConfs.find(wId) != m_mLootPetConfs.end())
        return &m_mLootPetConfs[wId];
	return NULL;
}
