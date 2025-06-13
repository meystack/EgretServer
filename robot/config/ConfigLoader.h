#pragma once

#define MAXFIRENDCOUNT 150

enum enDoWordProp
{
	dwpIdle = -1,
	dwpOnHook = 1,
	dwpQuesting = 2,
};

struct MonsterData
{
	WORD nMonsterId;
	ACTORNAME sName;
};

typedef struct tagMeetingActor
{
	int nActorId;
	ACTORNAME sName;
	ACTORNAME sCnName;
	EntityHandle hHandle;

}MEETINGACTOR, *PMEETINGACTOR;

class CConfigLoader:
	public CCustomLuaConfig
{
public:

	bool LoadAllConfig();

	inline CSceneConfigLoader& GetSceneConfig() { return m_SceneConfigLoader; }
	//inline CItemConfigLoader& GetItemConfig() { return m_itemConfigLoader; }

	//bool LoadItemConfig( LPCTSTR sFilePath );
	bool LoadSceneConfig(LPCTSTR);
	bool LoadAIConfig(LPCTSTR);

	CAITreeConfigLoader& GetAIBehavierTree(){ return m_AITreeConfigLoader; } 

	// inline size_t convertToACP(const char* lpMsg, const size_t nMsgLen, char *pBuffer, const size_t BufferLen)
	// {
	// 	wchar_t sWCBuf[4096];
	// 	DWORD cch = (DWORD)MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, lpMsg, (int)nMsgLen, sWCBuf, ArrayCount(sWCBuf)-1);
	// 	if ( cch > 0 )
	// 	{
	// 		cch = (DWORD)WideCharToMultiByte(CP_ACP, 0, sWCBuf, cch, pBuffer, int(BufferLen-1), NULL, NULL);
	// 		if ( cch > 0 )
	// 		{
	// 			pBuffer[cch] = 0;
	// 			return cch;
	// 		}
	// 	}
	// 	else 
	// 	{
	// 		cch = (DWORD)__min(BufferLen-1, nMsgLen);
	// 		memcpy(pBuffer, lpMsg, cch);
	// 		pBuffer[cch] = 0;
	// 		return cch;
	// 	}
	// 	return 0;
	// }

private:
	
	static const struct DataProviderLoadTerm
	{
	
		bool (CConfigLoader::*loadProc)(LPCTSTR);
		char * sFilePath;
		char * comment;
	}	sLoader[];

	CSceneConfigLoader			m_SceneConfigLoader; 
	CAITreeConfigLoader			m_AITreeConfigLoader;
	//CItemConfigLoader           m_itemConfigLoader; 
};