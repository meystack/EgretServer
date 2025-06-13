#include "StdAfx.h"
namespace DataPack
{
	static CBufferAllocator	g_DataPackAllocator;

	void * allocPacket(void* pEntity,int nSystemID,int nCmdID)
	{
		
		if(pEntity == NULL || ((CEntity *)pEntity)->GetType() != enActor) return NULL ;
		CActorPacket &pack = ((CActor*)pEntity)->GetFastPacket();
		//if(pack.packet != NULL) return NULL;
		((CActor*)pEntity)->AllocPacket(pack); // 把网关协议头写进去

		/*
		保存指针以便在角色申请数据包后出现脚本错误从而无法flush数据包的时候回收数据包，
		如果脚本出错且数据包没有被回收，则会导致整个游戏引擎就此不能正常工作
		*/
		CLogicScript::ScriptActorPacket = &pack;

		pack.packet->operator << ((BYTE)nSystemID);
		pack.packet->operator << ((BYTE)nCmdID);
		return &pack;
	}

	void* allocPacketEx()
	{		
		CDataPacket *pPack = (CDataPacket *)g_DataPackAllocator.AllocBuffer(sizeof(CDataPacket));
		if (pPack)
			new (pPack)CDataPacket(CEnvirConfig::m_pAllocator);

		CActorPacket* pActorPack = (CActorPacket *)g_DataPackAllocator.AllocBuffer(sizeof(CActorPacket));
		pActorPack->packet = pPack;
		return pActorPack;
	}

	void freePacketEx(void* pack)
	{
		CActorPacket* pActorPacket = (CActorPacket *)pack;
		if (!pActorPacket) return;
		CDataPacket* pPack = pActorPacket->packet;
		if (pPack)
		{
			pPack->~CDataPacket();
			g_DataPackAllocator.FreeBuffer(pPack);
		}
		pActorPacket->packet = NULL;
		g_DataPackAllocator.FreeBuffer(pActorPacket);
	}
	
	void flush(void* pActorPacket)
	{
		if(pActorPacket ==NULL) return;
		((CActorPacket*)pActorPacket)->flush();
		((CActorPacket*)pActorPacket)->packet = NULL;
		//标记逻辑脚本申请的数据包已回收！
		CLogicScript::ScriptActorPacket = NULL;
	}

	void flushCs(void *packet)
	{
		if (packet == NULL) return;
		CLogicSSClient *pSSClient = GetLogicServer()->GetSessionClient();
		CDataPacket &data = pSSClient->allocProtoPacket(jxInterSrvComm::SessionServerProto::cSendGroupMessage);
		data << (WORD)jxInterSrvComm::SessionServerProto::fcScriptData;
		CActorPacket *pPacket = (CActorPacket *)packet;
		data.writeBuf(pPacket->packet->getMemoryPtr(), pPacket->packet->getLength());
		pSSClient->flushProtoPacket(data);
	}

	int broadcastScene(void* pActorPacket, unsigned int fbHandle, int nSceneId)
	{
		if(pActorPacket ==NULL) return 0;
		if(((CActorPacket*)pActorPacket)->packet == NULL) return 0;
		CDataPacket& data = *((CActorPacket*)pActorPacket)->packet;

		if (CFuBen *pFb = CFuBenManager::m_FuBenMgr->GetDataPtr(fbHandle))
		{
			if(CScene *pScene = pFb->GetScene(nSceneId))
				pScene->Broadcast(data.getMemoryPtr(),data.getPosition());
		}
	}

	void broadcasetWorld(void* packet, int nLevel, int nCircle)
	{
		if (packet)
		{
			CActorPacket* pPack = (CActorPacket *)packet;
			GetGlobalLogicEngine()->GetEntityMgr()->BroadCast(pPack->packet->getMemoryPtr(), pPack->packet->getPosition(), nLevel, nCircle);
		}
	}

	void writeString(void * pActorPacket,const char *str)
	{
		if(pActorPacket ==NULL) return;
		const char* sResutl = str;
		if ( str == NULL )
		{
			sResutl = "";
		}
		*((CActorPacket*)pActorPacket)->packet << sResutl;
	}
	char* readString(void * pPack)
	{
		if(pPack ==NULL) return NULL;
		LPCSTR result;
		*((CDataPacket*)pPack) >> result;
		return (char*)result;
	}
	 unsigned char readByte(void * pPack)
	 {
		 unsigned char bResult = 0;
		 if(pPack ==NULL) return 0;
		 ((CDataPacket*)pPack)->operator >> (bResult);
		 return bResult;
	}


	char readChar(void * pPack)
	{
		char cResult = 0;
		if(pPack ==NULL) return 0;
		((CDataPacket*)pPack)->operator >> (cResult);
		return cResult;
	}

	 unsigned short  readWord(void * pPack)
	{
		unsigned short wValue = 0;
		if(pPack ==NULL) return wValue;
		((CDataPacket*)pPack)->operator >> (wValue);
		return wValue;
	}
	short readShort(void * pPack)
	{
		short wValue = 0;
		if(pPack ==NULL) return wValue;
		((CDataPacket*)pPack)->operator >> (wValue);
		return wValue;
	}
	 int readInt(void * pPack)
	{
		int nValue = 0;
		if(pPack ==NULL) return nValue;
		((CDataPacket*)pPack)->operator >> (nValue);
		return nValue;
	}
	 unsigned int readUInt(void * pPack)
	{
		unsigned int uValue = 0;
		if(pPack ==NULL) return uValue;
		((CDataPacket*)pPack)->operator >> (uValue);
		return uValue;
	}
	double readUint64(void * pPack)
	{
		unsigned long long lValue = 0;
		double d=0;
		if(pPack ==NULL) return d;
		((CDataPacket*)pPack)->operator >> (lValue);
		
		memcpy(&d,&lValue,sizeof(lValue));
		return d;
	 }
	double readInt64(void * pPack)
	{
		long long lValue = 0;
		double d=0;
		if(pPack ==NULL) return d;
		((CDataPacket*)pPack)->operator >> (lValue);
		memcpy(&d,&lValue,sizeof(lValue));
		return d;
	}

	int readData( lua_State *L )
	{
		CDataPacket* pack = (CDataPacket*)lua_touserdata(L,1);
		if (!pack) return 0;
		int nParamCount = (int)lua_tointeger(L,2);
		for (int i = 0,index=3; i < nParamCount; i++,index++)
		{
			int dt = (int)lua_tointeger(L,index);
			switch (dt)
			{
			case dtByte:
				{
					BYTE b = 0;
					(*pack) >> b;
					lua_pushinteger(L,b);
					break;
				}
			case dtChar:
				{
					char b = 0;
					(*pack) >> b;
					lua_pushinteger(L,b);
					break;
				}
			case dtWord:
				{
					WORD b = 0;
					(*pack) >> b;
					lua_pushinteger(L,b);
					break;
				}
			case dtShort:
				{
					short b = 0;
					(*pack) >> b;
					lua_pushinteger(L,b);
					break;
				}
			case dtInt:
				{
					int b = 0;
					(*pack) >> b;
					lua_pushinteger(L,b);
					break;
				}
			case dtUint:
				{
					unsigned int b = 0;
					(*pack) >> b;
					lua_pushinteger(L,b);
					break;
				}
			case dtString:
				{
					const char* b = NULL;
					(*pack) >> b;
					lua_pushstring(L,b);
					break;
				}
			case dtInt64:
				{
					long long b = 0;
					(*pack) >> b;
					double d = 0;
					memcpy(&d,&b,sizeof(b));
					lua_pushnumber(L,d);
					break;
				}
			case dtUint64:
				{
					unsigned long long b = 0;
					(*pack) >> b;
					double d = 0;
					memcpy(&d,&b,sizeof(b));
					lua_pushnumber(L,d);
					break;
				}
			}
		}
		return nParamCount;
	}

	 void writeByte(void * pActorPacket, unsigned char btValue)
	{
		if(pActorPacket ==NULL) return;
		((CActorPacket*)pActorPacket)->packet->operator << (btValue);
	}
	 void writeChar(void * pActorPacket , char cValue)
	{
		if(pActorPacket ==NULL) return;
		((CActorPacket*)pActorPacket)->packet->operator << (cValue);
	}
	 void writeWord(void * pActorPacket,unsigned short wValue)
	{
		if(pActorPacket ==NULL) return;
		((CActorPacket*)pActorPacket)->packet->operator << (wValue);
	 }
	 void writeShort(void * pActorPacket, short wValue)
	{
		if(pActorPacket ==NULL) return;
		((CActorPacket*)pActorPacket)->packet->operator << (wValue);
	}
	 void writeInt(void * pActorPacket,int  nValue)
	{
		if(pActorPacket ==NULL) return;
		((CActorPacket*)pActorPacket)->packet->operator << (nValue);
	}
	 void writeUInt(void * pActorPacket,unsigned int  uValue)
	{
		if(pActorPacket ==NULL) return;
		((CActorPacket*)pActorPacket)->packet->operator << (uValue);
	}
	 void writeUint64(void * pActorPacket,double d)
	{
		if(pActorPacket ==NULL) return;
		unsigned long long lValue = 0;
		// lValue = static_cast<unsigned long long>(d);
		memcpy(&lValue,&d,sizeof(d));
		((CActorPacket*)pActorPacket)->packet->operator << (lValue);
	}
	 void writeInt64(void * pActorPacket,double d )
	{
		if(pActorPacket ==NULL) return;
		long long lValue = 0;
		memcpy(&lValue,&d,sizeof(d));
		// lValue = static_cast<long long>(d);
		((CActorPacket*)pActorPacket)->packet->operator << (lValue);
	}

	 int writeData( lua_State *L )
	 {
		 CActorPacket* packet = (CActorPacket*)lua_touserdata(L,1);
		 if (!packet) return 0;
		 int nParamCount = (int)lua_tointeger(L,2);
		 for (int i = 0,index=3; i < nParamCount; i++,index+=2)
		 {
			 int dt = (int)lua_tointeger(L,index);
			 switch (dt)
			 {
			 case dtByte:
				 {
					 int b = (int)lua_tointeger(L,index+1);
					 (*packet) << (BYTE)b;
					 break;
				 }
			 case dtChar:
				 {
					 int b = (int)lua_tointeger(L,index+1);
					 (*packet) << (char)b;
					 break;
				 }
			 case dtWord:
				 {
					 int b = (int)lua_tointeger(L,index+1);
					 (*packet) << (WORD)b;
					 break;
				 }
			 case dtShort:
				 {
					 int b = (int)lua_tointeger(L,index+1);
					 (*packet) << (short)b;
					 break;
				 }
			 case dtInt:
				 {
					 int b = (int)lua_tointeger(L,index+1);
					 (*packet) << (int)b;
					 break;
				 }
			 case dtUint:
				 {
					 int b = (int)lua_tointeger(L,index+1);
					 (*packet) << (unsigned int)b;
					 break;
				 }
			 case dtString:
				 {
					 const char* b = lua_tostring(L,index+1);
					 packet->packet->writeString(b);
					 break;
				 }
			 case dtInt64:
				 {
					 double d = (double)lua_tonumber(L,index+1);
					 long long b = 0;
					 memcpy(&b,&d,sizeof(d));
					 (*packet) << (long long)b;
					 break;
				 }
			 case dtUint64:
				 {
					 double d = (double)lua_tonumber(L,index+1);
					 long long b = 0;
					 memcpy(&b,&d,sizeof(d));
					 (*packet) << (unsigned long long)b;
					 break;
				 }
			 }
		 }
		 return 0;
	 }

	void broadGuild(void* packet, int nGuildId)
	{
		if (packet)
		{
			CActorPacket* pPack = (CActorPacket *)packet;
			CGuild* pGuild = GetGlobalLogicEngine()->GetGuildMgr().FindGuild(nGuildId);
			if (pGuild)
			{
				pGuild->SendData(pPack->packet->getMemoryPtr(), pPack->packet->getPosition());
			}
		}
	}
};
