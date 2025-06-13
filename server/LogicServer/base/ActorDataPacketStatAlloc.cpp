#include "StdAfx.h"

CActorDataPacketStatAlloc* CActorDataPacketStatAlloc::m_spADPSAllocator;

CActorDataPacketStatAlloc::CActorDataPacketStatAlloc() 
	: m_hdlMgr(_T("ActorDataPacketStatHdlMgr"))
{

}