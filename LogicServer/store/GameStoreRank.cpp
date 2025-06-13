#include "StdAfx.h"
#include "GameStoreRank.h"
#include "../base/Container.hpp"
using namespace wylib::stream;
using namespace FDOP;

CGameStoreRank::~CGameStoreRank()
{
	m_SaleRankList.empty();
}

bool CGameStoreRank::LoadFromFile(LPCTSTR sFilePath)
{
	if ( !FileExists(sFilePath) )
		return false;
	//以共享读方式打开数据文件
	CFileStream fs(sFilePath, CFileStream::faShareRead);
	fs.setPosition(0);
	return LoadFromStream(fs);
}

bool CGameStoreRank::SaveToFile(LPCTSTR sFilePath) 
{
	if(!m_hasDataChange) return true; //没有修改就不需要存盘 
	TCHAR sSavePath[1024];
	
	//获取保存文件的目录路径，如果目录路径字符长度超出缓存长度则报错
	if ( ExtractFileDirectory(sFilePath, sSavePath, ArrayCount(sSavePath)) >= ArrayCount(sSavePath) )
	{
		OutputMsg(rmError, _T("unable to save Game Store Rank Data to %s, path to long"), sFilePath);
		return false;
	}
	//逐层判断目录是否存在，如果不存在则创建
	if ( !DeepCreateDirectory(sSavePath) )
	{
		OutputError(GetLastError(), _T("unable to create directory %s "), sSavePath);
		return false;
	}
	//打开文件并向其中写入销量排行数据
	CFileStream fs(sFilePath, CFileStream::faWrite|CFileStream::faShareRead,CFileStream::AlwaysCreate);
	m_hasDataChange =false; //存盘了就没有改变了
	return SaveToStream(fs);
}

INT __cdecl CGameStoreRank::CompareMerchandiseSaleData(CGameStoreRank::SaleData &r1, CGameStoreRank::SaleData &r2)
{
	if ( r1.uCount > r2.uCount )
		return -1;
	if ( r1.uCount < r2.uCount )
		return 1;
	return 0;
}

bool CGameStoreRank::LoadFromStream(CBaseStream &stream)
{
	GameStoreRankFileHeader hdr;
	LONG64 dwDataSize;

	//读取文件头	
	if ( stream.read(&hdr, sizeof(hdr)) < sizeof(hdr) )
	{
		OutputError(GetLastError(), _T(""));
		OutputMsg(rmError, _T("invalid Game Store Rank stream"));
		return false;
	}
	//检测文件标志
	if ( hdr.uIdent != GameStoreRankFileIdent )
	{
		OutputMsg(rmError, _T("invalid Game Store Rank FileIdent"));
		return false;
	}
	//检查文件版本号
	if ( hdr.uVersion != GameStoreRankFileVersion )
	{
		OutputMsg(rmError, _T("invalid Game Store Rank version %X"), hdr.uVersion);
		return false;
	}
	//判断数据流大小是否满足数据读取需要
	dwDataSize = sizeof(m_SaleRankList[0]) * hdr.uRecCount;
	if ( stream.getSize() - stream.getPosition() < dwDataSize )
	{
		OutputMsg(rmError, _T("out of stream size during read Game Store Rank data"));
		return false;
	}
	m_SaleRankList.reserve(hdr.uRecCount);
	m_SaleRankList.trunc(hdr.uRecCount);
	//读取销量排行数据
	if (dwDataSize > 0)
	{
		if ( dwDataSize != stream.read(&m_SaleRankList[0], dwDataSize) )
		{
			OutputMsg(rmError, _T("stream read error during read Game Store Rank Darta"));
			m_SaleRankList.trunc(0);
			return false;
		}
	}
	
	//对销量排行进行排序
	if (hdr.uRecCount > 0)
	{
		qsort(&m_SaleRankList[0], hdr.uRecCount, sizeof(m_SaleRankList[0]), 
			(int (__cdecl *)(const void *,const void *))CompareMerchandiseSaleData);
	}
	return true;
}

bool CGameStoreRank::SaveToStream(wylib::stream::CBaseStream &stream) const
{
	GameStoreRankFileHeader hdr;

	ZeroMemory(&hdr, sizeof(hdr));
	hdr.uIdent = GameStoreRankFileIdent;
	hdr.uVersion = GameStoreRankFileVersion;
	hdr.uRecCount = (UINT)m_SaleRankList.count();
	//写入文件头
	if ( stream.write(&hdr, sizeof(hdr)) != sizeof(hdr) )
	{
		OutputMsg(rmError, _T("failure write Game Store Rank file header"));
		return false;
	}
	//写入销量排行数据
	SIZE_T dwDataSize = sizeof(m_SaleRankList[0]) * hdr.uRecCount;
	if (dwDataSize > 0)
	{
		if ( dwDataSize != stream.write(&m_SaleRankList[0], dwDataSize) )
		{
			OutputMsg(rmError, _T("failure write Game Store Rank data"));
			return false;
		}
	}
	return true;
}

/*
bNeedRank: false,仅记录数据，用参与排行榜排名
*/
UINT CGameStoreRank::UpdateSelaCount(INT nMerchandiseId, INT nSellCount,unsigned int nActorId, bool bNeedRank)
{
	INT_PTR i, nIdx = -1, nNewId;
	INT uResult = nSellCount;
	SaleData sd;

	for (i=m_SaleRankList.count()-1; i>-1; --i)
	{
		if (m_SaleRankList[i].nId == nMerchandiseId )
		{
			if(m_SaleRankList[i].nActorId ==nActorId || nActorId ==0)
			{
				nIdx = i;
				uResult += m_SaleRankList[i].uCount;	
				if(uResult <0) uResult =0;
				m_SaleRankList[i].uCount = (UINT)uResult;
				break;
			}
			
		}
	}
	
	//如果销量记录不存在则添加新记录
	if ( nIdx < 0 )
	{
		sd.nId = nMerchandiseId;
		sd.nActorId = nActorId;
		if(nSellCount >0)
		{
			sd.uCount = nSellCount;
		}
		else
		{
			sd.uCount =0;
		}
		
		nIdx = m_SaleRankList.add(sd);
	}
	
	if(bNeedRank)
	{
		//尝试将销量提升排行
		nNewId = nIdx - 1;
		while (nNewId > -1)
		{
			if ( (INT)m_SaleRankList[nNewId].uCount >uResult)
				break;
			nNewId--;
		}
		nNewId++;
		//如果新的排行索引不等于当前排行索引则改变在列表中的位置
		if ( nNewId != nIdx )
		{
			sd = m_SaleRankList[nIdx];
			m_SaleRankList.remove(nIdx);
			m_SaleRankList.insert(nNewId, sd);
		}
	}
	m_hasDataChange =true;
	if( uResult >=0)
	{
		return uResult;
	}
	else
	{
		return 0;
	}
}

void CGameStoreRank::LoadTodayConsumeRankFile()
{
	CRankingMgr &RankingMgr = GetGlobalLogicEngine()->GetRankingMgr();
	CRanking *pRanking = RankingMgr.GetRanking(enRankTodayConsumeYuBao);

	if(!pRanking)
	{
		pRanking = RankingMgr.Add(enRankTodayConsumeYuBao,g_szTodayConsumeYuBao, MAX_GUILDTITLENUM, 1);
		if (pRanking)
		{
			if (!pRanking->Load(g_szTodayConsumeYuBaoRanking))
			{
				char strColumnName[32];
				sprintf_s(strColumnName, sizeof(strColumnName),"name");
				pRanking->AddColumn(strColumnName, -1);
			}
		}
	}
}

void CGameStoreRank::SaveTodayConsumeRankToFile()
{
	CRankingMgr &RankingMgr = GetGlobalLogicEngine()->GetRankingMgr();
	CRanking *pRanking = RankingMgr.GetRanking(enRankTodayConsumeYuBao);

	if(pRanking)
	{
		pRanking->Save(g_szTodayConsumeYuBaoRanking);
	}
}

void CGameStoreRank::LoadYesterdayConsumeRank()
{
	CRankingMgr &RankingMgr = GetGlobalLogicEngine()->GetRankingMgr();
	CRanking *pRanking = RankingMgr.GetRanking(enRankYesterdayConsumeYuBao);

	if(!pRanking)
	{
		pRanking = RankingMgr.Add(enRankYesterdayConsumeYuBao,g_szYesterdayConsumeYuBao, MAX_GUILDTITLENUM, 1);
		if (pRanking)
		{
			if (!pRanking->Load(g_szYesterdayConsumeYuBaoRanking))
			{
				char strColumnName[32];
				sprintf_s(strColumnName, sizeof(strColumnName),"name");
				pRanking->AddColumn(strColumnName, -1);
			}
		}
	}
}

void CGameStoreRank::SaveYesterdayConsumeRankToFile()
{
	CRankingMgr &RankingMgr = GetGlobalLogicEngine()->GetRankingMgr();
	CRanking *pRanking = RankingMgr.GetRanking(enRankYesterdayConsumeYuBao);

	if(pRanking)
	{
		pRanking->Save(g_szYesterdayConsumeYuBaoRanking);
	}
}

void CGameStoreRank::RfeshConsumeRank()
{  
	CRankingMgr &RankingMgr = GetGlobalLogicEngine()->GetRankingMgr();
	CRanking *pRanking = RankingMgr.GetRanking(enRankYesterdayConsumeYuBao);
	
	if(pRanking)
	{
		pRanking->clear();

		CRanking *nRanking = RankingMgr.GetRanking(enRankTodayConsumeYuBao);

		if(nRanking)
		{
			int nCount = nRanking->GetItemCount();

			for(int i=0;i<nCount;i++)
			{
				CRankingItem* mItem = nRanking->GetItem(i);

				if(mItem)
				{
					unsigned int nActorId = mItem->GetId();
					int nPoint = mItem->GetPoint();

					GetLogicServer()->GetLogClient()->SendConsumeLog((WORD)GameLog::clActorTotalConsume, 
						(unsigned int)nActorId, mItem->GetSubData(0), mItem->GetSubData(0), (BYTE)mtYuanbao,
						(int)nPoint, (int)(nPoint));

					CRankingItem* pItem = pRanking->AddItem(nActorId,nPoint);
					
					if(pItem)
					{
						pItem->SetSub(0,mItem->GetSubData(0));
					}
				}
			}

			nRanking->clear();
		}
	}

}
