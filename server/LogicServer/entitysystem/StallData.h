
#pragma once

class CStallData
{
public:
	static const int STALLNAMELEN = 30;
	static const int STALLADLEN = 60;
	static const int STALLMSGLEN = 120;

	enum enStallOpt{
		soEndStall = 0,
		soStartStall = 1,
	};
	enum enStallResult{
		srStallSucc = 0,
		srStallFail = 1,
	};

	typedef struct tagStallItem
	{
		byte nFlag;
		byte nCoin;
		int nPrice;
		int nItemId;
		WORD wNum;
		CUserItem::ItemSeries series;
		tagStallItem()
		{
			nFlag = -1;
			nItemId = 0;
			wNum = 0;
			nCoin = 0;
			nPrice = 0;
		}
	}STALLITEM, *PSTALLITEM;

	typedef struct tagStallMsg
	{
		byte	  nMsgType;
		int			nId;
		ACTORNAME sPlayerName;
		char	  sMsg[STALLMSGLEN];
	}STALLMSG, *PSTALLMSG;

	typedef struct tagStallInfo
	{
		char sName[STALLNAMELEN];
		byte nGridCount;
		CVector<STALLITEM> vItems;
		CVector<STALLMSG> vMsgs;

	}STALLINFO, *PSTALLINFO;
};