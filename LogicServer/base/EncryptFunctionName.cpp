#include "StdAfx.h"
#include "EncryptFunctionName.h"
#include "Container.hpp"
char*		CEncryptFunctionName::m_ValStr = NULL;
int			CEncryptFunctionName::m_ValLen = 4096;

CEncryptFunctionName::CEncryptFunctionName()
{
	if(!m_ValStr)
	{
		m_ValStr = (char*)malloc(4096);
	}
}

CEncryptFunctionName::~CEncryptFunctionName()
{
	if(m_ValStr)
	{
		free(m_ValStr);
	}
}

char* CEncryptFunctionName::ExtractMessageBox(char* strVal,int nKey)
{
	if(!strVal) return "";
	while(m_ValLen < strlen(strVal)+100)
	{
		m_ValStr = (char*) realloc(m_ValStr,(m_ValLen + 1024));
		m_ValLen += 1024;
	}

	memset(m_ValStr,0, m_ValLen);

	int index=0;
	char* token = strtok(strVal, "/");
	while(token)
	{
		if(index > 0)
		{
			token = strtok(NULL, "/");
		}
		if(token)
		{
			if(!m_ValStr[0])
			{
				strcpy(m_ValStr,token);
			}
			else
			{
				strcat(m_ValStr,token);
			}
			token = strtok(NULL, "/");
			if(token)
			{
				strcat(m_ValStr,"/");
				char szTemp[256] = {0};
				EncryptFunctionName(token,szTemp,nKey%100);
				strcat(m_ValStr,szTemp);
			}
		}
		index++;
	}

	//OutputMsg(rmTip,_T("message Npc Talk result:%s"),m_ValStr);
	return m_ValStr;
}

char* CEncryptFunctionName::EncryptExtractString(LPCSTR strVal,int nKey)
{
	if(!strVal) return "";

	//OutputMsg(rmTip,_T("strVal Npc Talk result:%s"),strVal);

	while(m_ValLen < strlen(strVal)+100)
	{
		m_ValStr = (char*) realloc(m_ValStr,(m_ValLen + 1024));
		m_ValLen += 1024;
	}

	//OutputMsg(rmTip,_T("strResult Npc Talk result:%s"),strVal);
	memset(m_ValStr,0, m_ValLen);

	char* token = strtok((char*)strVal, "@");
	int index=0;
	while(token)
	{
		if(index > 0)
		{
			token = strtok( NULL, "@");
		}
		if(token)
		{
			if(!m_ValStr[0])
			{
				strcpy(m_ValStr,token);
			}
			else
			{
				strcat(m_ValStr,token);
			}

			token = strtok( NULL, ">");
			if(token)
			{
				strcat(m_ValStr,"@");
				char szTemp[256] = {0};
				EncryptFunctionName(token,szTemp,nKey%100);
				strcat(m_ValStr,szTemp);
				strcat(m_ValStr,">");
			}
		}
		index++;
	}

	//OutputMsg(rmTip,_T("encryp Npc Talk result:%s"),m_ValStr);

	return m_ValStr;
}

void CEncryptFunctionName::EncryptFunctionName(char* StrSrc,char* StrDest,int nKey)
{
	if(!StrSrc || !StrDest) return;

	if(IsNeedEncrypt(StrSrc))
	{
		strcpy(StrDest,StrSrc);
		return;
	}
	else
	{
		for(int i=0;i<strlen(StrSrc);i++)
		{
			if(StrSrc[i] != '@')
			{
				int nStr = StrSrc[i];
				nStr ^= nKey;
				char strTemp[4];
				sprintf(strTemp,"%03d",nStr);
				if(StrDest[0])
				{
					strcat(StrDest,strTemp);
				}
				else
				{
					strcpy(StrDest,strTemp);
				}
			}
			else
			{
				strcpy(StrDest,StrSrc);
				break;
			}
		}

	}
	//OutputMsg(rmTip,_T("StrSrc:%s"),StrSrc);
}

void CEncryptFunctionName::DecryptFunctionName(char* StrSrc,char* StrDest,int nKey)
{
	if(!StrSrc) return; 
	if(IsNeedEncrypt(StrSrc) == true)
	{
		strcpy(StrDest,StrSrc);
		return;
	}
	else
	{
		int nNum = nKey%100;

		int nIndex = 0;
		for(int i=0;i<strlen(StrSrc);)
		{
			if(StrSrc[i] != '@')
			{
				if(StrSrc[i] >= '0' && StrSrc[i] <= '9')
				{
					char strTemp[3];
					for(int j=0;j<3;j++)
					{
						strTemp[j] = StrSrc[j + i];
					}

					int nStr = atoi(strTemp);
					nStr ^= nNum;

					StrDest[nIndex] = nStr;
					nIndex++;

					i += 3;
				}
				else
				{
					strcpy(StrDest,StrSrc);
					return;
				}
			}
			else
			{
				strcpy(StrDest,StrSrc);
				return;
			}
		}

		StrDest[nIndex] = 0;
	}

	//OutputMsg(rmTip,_T("Decrypt StrSrc:%s"),StrSrc);
}

bool CEncryptFunctionName::IsNeedEncrypt(const char* strSrc)
{
	if(StringMatching(strSrc,"main") || StringMatching(strSrc,"exit") || StringMatching(strSrc,"showWin,")
		|| StringMatching(strSrc,"startPlay") || StringMatching(strSrc,"QuestAwardDesc,")
		|| StringMatching(strSrc,"AskExitToReloadMap,") || StringMatching(strSrc,"checkNewPlayerNumber,")
		|| StringMatching(strSrc,"GiveUpQuest,") || StringMatching(strSrc,"OnEnterFinalArea")
		|| StringMatching(strSrc,"ShowQuestTalk"))
	{
		return true;
	}

	for(int i = 0; i < m_FunctionName.count(); ++i)
	{
		FUNCTIONNAMES& tmpStr = m_FunctionName[i];
		if(tmpStr.sName && StringMatching(strSrc,tmpStr.sName))
		{
			return true;
		}
	}
	return false;
}

bool CEncryptFunctionName::StringMatching(const char* srcStr, const char *matchStr) 
{ 
	int n = (int)strlen(srcStr); 
	int m = (int)strlen(matchStr); 

	if(n < m || m == 0) 
	{ 
		return false; 
	} 

	for(int j = 0; j <= n-m; ++j) 
	{ 
		if (!memcmp(srcStr + j, matchStr, m))
		{
			return true;
		}
	} 

	return false;
} 
