#include "StdAfx.h"
#include "Container.hpp"
const TCHAR* const CRanking::m_sClassName = _T("CRanking");


int CRankingItem::GetIndex()
{
	if(pParent ==NULL || !IsValid()) return -10000;
	return pParent->GetNodeIndex(this);
}


//设置某列的数据
void CRankingItem::SetSub(INT_PTR nIndex, LPCSTR sData)
{
	CHECK_RANK_ITEM_VALID()
	if (sCd && nIndex >=0 && nIndex < nColCount && sData)
	{
		size_t nLen = strlen(sData);
		if (!sCd[nIndex])
		{
			sCd[nIndex] = (char*)CRankingMgr::m_pAllocator->AllocBuffer(nLen+1);
		}else if (strlen(sCd[nIndex]) < nLen)
		{
			CRankingMgr::m_pAllocator->FreeBuffer(sCd[nIndex]);
			sCd[nIndex] = (char*)CRankingMgr::m_pAllocator->AllocBuffer(nLen+1);
		}
		strcpy(sCd[nIndex],sData);
		sCd[nIndex][nLen] = 0;

		if(pParent && pParent->IsValid())
		{
			pParent->SetModify(true);
		}
	}
}

void CRankingItem::SetSub(INT_PTR nIndex, unsigned int nValue){
	char buf[20];
	sprintf_s(buf,sizeof(buf),"%u",nValue);
	SetSub(nIndex, buf);
}

void CRankingItem::SetId(unsigned int id) 
{
	CHECK_RANK_ITEM_VALID()  
	
	nId = id;
	if(pParent && pParent->IsValid())
		pParent->SetModify(true);
}

void CRankingItem::SetPoint(int point)
{
	CHECK_RANK_ITEM_VALID() 
	nPoint = point; 
	if(pParent && pParent->IsValid())
		pParent->SetModify(true);
}


void CRankingItem::SetParam(int param)
{
	CHECK_RANK_ITEM_VALID() 
	nParam = param; 
	if(pParent && pParent->IsValid())
		pParent->SetModify(true);
}


void CRankingItem::SetParam1(int param1)
{
	CHECK_RANK_ITEM_VALID() 
	nParam1 = param1; 
	if(pParent && pParent->IsValid())
		pParent->SetModify(true);
}

void CRankingItem::RemoveColumn(INT_PTR nIndex)
{
	CHECK_RANK_ITEM_VALID()
	if (sCd && nIndex >=0 && nIndex < nColCount && nColCount > 0)
	{
		if (sCd[nIndex])
		{
			CRankingMgr::m_pAllocator->FreeBuffer(sCd[nIndex]);
		}
		memcpy(sCd+nIndex,sCd+nIndex+1,sizeof(char*)* (nColCount - nIndex - 1));//后面的数据往前移
		nColCount--;
		if (nColCount > 0)
		{
			sCd[nColCount-1] = NULL;
		}
		/*
		if(pParent && pParent->IsValid())
		{
			pParent->SetModify();
		}
		*/

	}
}



void CRankingItem::AddColumn(LPCSTR sData,INT_PTR nIndex, bool bUnique)
{
		CHECK_RANK_ITEM_VALID()
		if (bUnique)
		{
			for (INT_PTR i = 0; i < nColCount; i ++)
			{
				if (0 == strcmp(sCd[i],sData))
					return;
			}
		}
		if (nIndex == -1)
		{
			SetColCount(nColCount+1);
			SetSub(nColCount-1,sData);
		}
		else if (nIndex >= 0 && nIndex < nColCount)
		{
			SetColCount(nColCount+1);
			memmove(sCd+(nIndex+1),sCd+nIndex,sizeof(char*)*(nColCount-nIndex));//列往后移
			sCd[nIndex] = NULL;
			SetSub(nIndex,sData);
		}
		/*
		if(pParent && pParent->IsValid())
		{
			pParent->SetModify(true);
		}
		*/

}

void CRankingItem::SetColumnTitle(LPCSTR sData, INT_PTR nIndex)
{
		CHECK_RANK_ITEM_VALID()
		if (nIndex >= 0 && nIndex < nColCount)
		{
			SetSub(nIndex, sData);
		}
		else
		{
			OutputMsg(rmError, "SetColumnTitle Error, Index wrong!");
		}
		/*
		if(pParent && pParent->IsValid())
		{
			pParent->SetModify(true);
		}
		*/

}





////////////////////////////////////////////////    CRanking        /////////////////////////////////
#pragma warning(push)
#pragma warning(disable:4355) 
CRanking::CRanking( LPCSTR sFileName ,INT_PTR nMax,bool boDisp,INT_PTR nMaxBroadCount):m_Title(this)
#pragma warning(pop)

{
	if (sFileName)
	{
		_asncpytA(m_szName,sFileName);
		if(strcmp(m_szName + strlen(m_szName) - 4, ".Rank"))
		{
			sprintf(m_szFile,"%s.Rank",m_szName);
		}
		else
		{
			_asncpytA(m_szFile, m_szName);
		}
	}else
	{
		m_szName[0];
		m_szFile[0] = 0;
	}
	m_szCnName[0] = 0;
	m_idTitle[0] = 0;
	m_pointTitle[0] = 0;

	m_RankMax				= (int)nMax;
	m_nMaxBroadCount		= (int)nMaxBroadCount;
	m_rankVisibleType		= enVisible_All;	
	ZeroMemory(&m_rankVisibleParam, sizeof(m_rankVisibleParam));
	m_Display				= boDisp;
	ZeroMemory(m_ColDisplay,sizeof(m_ColDisplay));
	m_boModify = false;
	m_bRmptySave =	false;
	m_columnCount =0;
	m_isRanked =true;
	m_nCheckSum = RANKING_CHECUSUM;
	m_saveTick = _getTickCount() + s_nDur;
	AtachRankProp(sFileName);	//初始化属性排行榜配置
	m_canSave = true;
}

CRanking::~CRanking()
{
	if (!IsValid())
	{
		return;
	}
	
	//保存数据并释放内存
	if(m_boModify)
	{
		Save(NULL,true);
	}

	for (INT_PTR i = 0; i < m_ItemList.count(); i++)
	{
		CRankingItem* item = m_ItemList[i];
		if(item && item->IsValid())
		{
			item->~CRankingItem();
			CRankingMgr::m_pAllocator->FreeBuffer(item);
		}
	}
	m_Title.Destroy();
	m_ItemList.empty();
	m_nCheckSum = 0;
	DetachhRankProp(m_szName);	
	m_boModify = false;
}

void CRanking::destroy()
{	
	//GetGlobalLogicEngine()->GetRankingMgr().Remove(m_szName);
}

void CRanking::clear()
{
	CHECK_RANK_VALID()
	for (INT_PTR i = 0; i < m_ItemList.count(); i++)
	{
		CRankingItem* item = m_ItemList[i];
		UpdateRankProp(item,i,true);
		if(item && item->IsValid())
		{
			item->~CRankingItem();
			CRankingMgr::m_pAllocator->FreeBuffer(item);
		}
	}

	m_ItemList.clear();
	SetEmptySave(true);
	m_boModify = true;

	Save(NULL, true);
}

bool CRanking::Load(LPCSTR sFile)
{
   	DECLARE_TIME_PROF("CRanking::Load");
	char sPath[256];
	sprintf_s(sPath,sizeof(sPath),"./data/runtime/rank/");
	if (!FDOP::FileExists(sPath))
	{
		FDOP::DeepCreateDirectory(sPath);
	}
	if(!sFile || !strlen(sFile)){
		sFile = m_szFile;
	}
	strcat(sPath,sFile);
	if(strcmp(sPath + strlen(sPath) - 5, ".Rank"))strcat(sPath,".Rank");
	FILE* fp = fopen(sPath,"r");
	if (!fp)
	{
		return false;
	}
	if (sFile != m_szFile)
	{
		_asncpytA(m_szFile,sFile);//保存的文件名
	}
	/*
	if(strcmp(sFile,"wishValue.txt") ==0)
	{
		OutputMsg(rmTip,"LOAD");
	}
	*/

	//纯文本，tab隔开各个字段，每行一个记录,第一行是标题
	INT_PTR row = 0;
	const INT_PTR size = 128;
	char sBuf[size];//缓冲区,保存每列的字符串，128应该够了
	INT_PTR bufpos = 0;
	INT_PTR col = 0;
	m_Title.Destroy(); // 为了支持重新从文件加载，先清空标题栏的所有列，否则标题出现双倍列数，导致排行榜加载失败
	CRankingItem* pItem = &m_Title;
	pItem->SetParent(this);
	char ch = (char)fgetc(fp);
	while (!feof(fp))
	{
		if (ch == '\n')//
		{
			sBuf[bufpos] = 0;
			if(bufpos ==0 && row ==0)
			{
				OutputMsg(rmError,"[1]排行榜文件%s,添加列失败,行=%d!",sPath,(int)row);
				goto lb_load_fail;
			}
			pItem->AddColumn(sBuf,-1);//这是列的数据

			bufpos = 0;	
			col = 0;
			if (row > 0)
			{
				if(pItem->GetColCount() != m_Title.GetColCount())
				{
					OutputMsg(rmError,"[2]排行榜文件%s 第%d行出现列数错误,标题的列数为%d,实际为%d，清查看数据",sPath,(int)row,(int)m_Title.GetColCount(),(int)pItem->GetColCount());
					goto lb_load_fail;
					//Assert(0);
				}
				//m_ItemList.add(pItem);			
				//INT_PTR nPos=GetInsertPos(pItem->nPoint); //获取应该插入的位置

				//m_ItemList.insert(nPos,pItem);
				m_ItemList.add(pItem);

				pItem = NULL;
			}
			
			pItem = (CRankingItem*)CRankingMgr::m_pAllocator->AllocBuffer(sizeof(CRankingItem));//下一行的数据
			new (pItem)CRankingItem(this);
			//pItem->SetParent(this);
			row++;
		}
		else if (ch == '\t')
		{
			sBuf[bufpos] = 0;
			if (col < 2)
			{
				//前两个是数字
				unsigned int value = (unsigned int)atof(sBuf);
				if (col == 0) pItem->nId = value;//id和分值的数据
				else pItem->nPoint = value;
			}
			else
			{
				if(sBuf[0] ==0 && row ==0)
				{
					OutputMsg(rmError,"[3]排行榜文件%s,添加列失败,行=%d,行=0表示是标题!",sPath,(int)row);
					//Assert(0);
					goto lb_load_fail;
				}
				pItem->AddColumn(sBuf,-1);//这是列的数据
			}
			bufpos = 0;
			col++;
		}
		else
		{
			if (bufpos < (size-1))
			{
				sBuf[bufpos++] = ch;
			}
		}
		ch = (char)fgetc(fp);
	}
	fclose(fp);

	if(pItem)//还有数据没加到队列中
	{
		sBuf[bufpos] = 0;
		if(sBuf[0] ==0 && row ==0)
		{
			OutputMsg(rmError,"[4]排行榜文件%s,添加列失败,行=%d,行=0表示是标题!",sPath,(int)row);
			//Assert(0);
			goto lb_load_fail;
		}
		pItem->AddColumn(sBuf,-1);//这是列的数据
		bufpos = 0;
		col = 0;
		if (row > 0)
		{
			if(pItem->GetColCount() != m_Title.GetColCount())
			{
				OutputMsg(rmError,"排行榜文件%s 第%行出现列数错误,标题的列数为%d,实际为%d，清查看数据",sPath,(int)row,(int)m_Title.GetColCount(),(int)pItem->GetColCount());
				//Assert(0);
				goto lb_load_fail;
			}		
			//INT_PTR nPos=GetInsertPos(pItem->nPoint); //获取应该插入的位置
			//m_ItemList.insert(nPos,pItem);
			m_ItemList.add(pItem);
		}
		else
		{
			
		}
		row++;
	}
	//刚装载完认为没有修改
	SetModify(false);

	return true;

lb_load_fail:
	//增加了容错处理
	
	for (INT_PTR i = 0; i < m_ItemList.count(); i++)
	{
		CRankingItem* item = m_ItemList[i];

		if(item && item->IsValid())
		{
			item->~CRankingItem();
			CRankingMgr::m_pAllocator->FreeBuffer(item);
		}
	}
	//清掉这些列
	m_ItemList.clear();
	m_Title.Destroy();

	return false;


}

void CRanking::ForceSave(LPCSTR sFile)
{
	CHECK_RANK_VALID()
   	DECLARE_TIME_PROF("CRanking::ForceSave");
	FILE* fp =NULL;
	CStringBuff &buff =GetGlobalLogicEngine()->GetStringBuff();
#ifdef _USE_TRY_CATCH
#ifdef WIN32
	__try
#endif
#endif 
	{	
		// 没有项，不需要保存 且，空数据不保存文件
		if (!m_bRmptySave && GetItemCount() <= 0)
		{  
			return;
		}

		char sPath[256];
		sprintf_s(sPath,sizeof(sPath),"./data/runtime/rank/"/*, GetLogicServer()->getStrFilePath()*/);
		if (!FDOP::FileExists(sPath))
		{
			FDOP::DeepCreateDirectory(sPath);
		}

		LPCSTR szRealFile = sFile? sFile : m_szFile;
		if (szRealFile == NULL || szRealFile[0] == 0)
		{
			szRealFile = (m_szName);		
		}
		strcat(sPath,szRealFile);
		if(strcmp(sPath + strlen(sPath) - 4, ".txt"))strcat(sPath,".txt");// 末尾不是".txt"就添加
		fp = fopen(sPath,"w");
		if (!fp)
		{
			OutputMsg(rmError,"打开文件失败 %s",m_szFile);
			return;
		}

		if(buff.IsLock())
		{
			OutputMsg(rmError," Force Save rank %s, buff is lock",m_szFile);
			fclose(fp);
			return;
		}
		OutputMsg(rmTip,"Force Save rank%s",sPath);

		buff.Reset();
		buff.SetLock(true);

		SaveItem(&m_Title, buff);//保存标题先

		INT_PTR nCount = m_ItemList.count();
		if (nCount > 0)
		{
			//fputc('\n',fp);
			buff <<('\n');
		}
		for (INT_PTR i = 0; i < nCount; i++)
		{
			CRankingItem* pItem = m_ItemList[i];

			SaveItem(pItem,buff);
			if (i != (nCount-1))
			{
				buff <<('\n');
			}
		}

		fwrite(buff.Get(),strlen(buff.Get()),1,fp); //把存盘改下

		//fputs(buff.Get(),fp);
		fclose(fp);
		buff.SetLock(false);
	}

#ifdef _USE_TRY_CATCH
#ifdef WIN32
	__except(CustomExceptHandler(GetExceptionInformation()))
	{
		if(buff.IsLock())
		{
			buff.SetLock(false);
		}
		if(fp)
		{
			fclose(fp);
		}
	}
#endif 
#endif 
}

void CRanking::Save(LPCSTR sFile,bool forceSave)
{
	if (!m_canSave)
	{
		return;
	}
	
	CHECK_RANK_VALID()
	if(!m_boModify) return; //没有改变
	
	//这里可能是不需要存盘
	if( !forceSave && m_saveTick > GetGlobalLogicEngine()->getTickCount() )
	{
		return;
	}
	DECLARE_TIME_PROF("CRanking::Save");

	FILE* fp =NULL;
	CStringBuff &buff =GetGlobalLogicEngine()->GetStringBuff();
#ifdef _USE_TRY_CATCH
#ifdef WIN32
	__try
#endif 
#endif
	{	
		// 没有项，不需要保存 且，空数据不保存文件
		//注意：m_bRmptySave还是要确保为true，否则榜一旦清空了会不保存，导致磁盘上的数据还在
		if (!m_bRmptySave && GetItemCount() <= 0)
		{  
			return;
		}
		
		char sPath[256];
		sprintf_s(sPath,sizeof(sPath),"./data/runtime/rank/"/*, GetLogicServer()->getStrFilePath()*/);
		if (!FDOP::FileExists(sPath))
		{
			FDOP::DeepCreateDirectory(sPath);
		}

		LPCSTR szRealFile = sFile? sFile : m_szFile;
		if (szRealFile == NULL || szRealFile[0] == 0)
		{
			szRealFile = (m_szName);		
		}
		strcat(sPath,szRealFile);
		if(strcmp(sPath + strlen(sPath) - 5, ".Rank"))strcat(sPath,".Rank");// 末尾不是".Rank"就添加
		fp = fopen(sPath,"w");
		if (!fp)
		{
			OutputMsg(rmError,"打开文件失败 %s",m_szFile);
			return;
		}

		if(buff.IsLock())
		{
			OutputMsg(rmError,"Save rank %s, buff is lock",m_szFile);
			fclose(fp);
			return;
		}
		OutputMsg(rmTip,"Save rank%s",sPath);

		buff.Reset();
		buff.SetLock(true);

		SaveItem(&m_Title, buff);//保存标题先

		INT_PTR nCount = m_ItemList.count();
		if (nCount > 0)
		{
			//fputc('\n',fp);
			buff <<('\n');
		}
		for (INT_PTR i = 0; i < nCount; i++)
		{
			CRankingItem* pItem = m_ItemList[i];
		
			SaveItem(pItem,buff);
			if (i != (nCount-1))
			{
				buff <<('\n');
			}
		}
		
		fwrite(buff.Get(),strlen(buff.Get()),1,fp); //把存盘改下

		//fputs(buff.Get(),fp);
		fclose(fp);
		m_boModify = false;
		m_saveTick =  GetGlobalLogicEngine()->getTickCount() + s_nDur; //存盘的间隔

		buff.SetLock(false);
}

#ifdef _USE_TRY_CATCH
#ifdef WIN32
	__except(CustomExceptHandler(GetExceptionInformation()))
	{
		if(buff.IsLock())
		{
			buff.SetLock(false);
		}
		if(fp)
		{
			fclose(fp);
		}
	}
#endif 
#endif
}

void CRanking::AddColumn( LPCSTR sTitle, INT_PTR nIndex, bool bUnique )
{
	CHECK_RANK_VALID()
	if(sTitle == NULL || sTitle[0] ==0)
	{
		OutputMsg(rmError,"AddColumn error, rank=%s, title is NULL",m_szFile);
		return;
	}
	m_Title.AddColumn(sTitle,nIndex,bUnique);
	INT_PTR nCount = m_ItemList.count();
	for (INT_PTR i = 0; i < nCount; i++)//所有排行榜项都要增加
	{
		CRankingItem* item = m_ItemList[i];
		item->AddColumn("-",nIndex,bUnique);//空的数据
	}
	m_boModify = true;
}

void CRanking::SetColumnTitle(LPCSTR sTitle, INT_PTR nIndex)
{
	CHECK_RANK_VALID()
	m_Title.SetColumnTitle(sTitle, nIndex);	//设置列标题
}

void CRanking::RemoveColumn( INT_PTR nIndex )
{
	CHECK_RANK_VALID()
	m_Title.RemoveColumn(nIndex);
	INT_PTR nCount = m_ItemList.count();
	for (INT_PTR i = 0; i < nCount; i++)//所有排行榜项都要增加
	{
		CRankingItem* item = m_ItemList[i];
		item->RemoveColumn(nIndex);
	}
	m_boModify = true;
}



CRankingItem* CRanking::Update( INT_PTR nId, INT_PTR nPoint )
{
	if(!IsValid())return NULL;
	INT_PTR index = GetIndexFromId(nId);
	if ((index >= 0) && (index < m_ItemList.count()))
	{
		CRankingItem* item = m_ItemList[index];
		if ( item && nPoint != 0)
		{
			INT_PTR newPoint = item->nPoint + nPoint; //新的分数

			item->SetPoint((int)newPoint);
			

			INT_PTR nNewPos= GetInsertPos((int)newPoint);

			//如果让插入到最后的话，在后面的分数都一样的情况下，加个判断，防止自己删除又加上去的无用的操作
			if(nNewPos != index && nNewPos >=0 && nNewPos < m_ItemList.count())
			{
				m_ItemList.remove(index); //先删除这个项

				nNewPos= GetInsertPos((int)newPoint);
				m_ItemList.insert(nNewPos,item);
				UpdateRankProp(item,nNewPos);
			}
				
			m_boModify = true;
	
		}
		return item;
	}
	else
	{
		return NULL;
	}
}


CRankingItem* CRanking::Set( INT_PTR nId, INT_PTR nPoint )
{
	if(!IsValid())return NULL;
	INT_PTR index = GetIndexFromId(nId);
	if ((index >= 0) && (index < m_ItemList.count()))
	{
		CRankingItem* item = m_ItemList[index];
		if ( item && item->nPoint != nPoint)
		{
			//先删除原来的，再找个新位置添加进去就可以了
			item->SetPoint((int)nPoint);

			if(m_isRanked)
			{
				INT_PTR nNewPos= GetInsertPos((int)nPoint);
				if(nNewPos != index)
				{
					m_ItemList.remove(index); //先删除这个项

					nNewPos= GetInsertPos((int)nPoint);
					m_ItemList.insert(nNewPos,item);
					UpdateRankProp(item,nNewPos);
				}
			}
			
			m_boModify = true;
		}
		return item;
	}
	else
	{
		return NULL;
	}
}

void CRanking::RemoveId( INT_PTR nId )
{
	CHECK_RANK_VALID()
	INT_PTR index = GetIndexFromId(nId);
	if ((index >= 0) && (index < m_ItemList.count()))
	{
		CRankingItem* pItem = m_ItemList[index];
		if (pItem &&pItem->IsValid())
		{
			UpdateRankProp(pItem,index,true);
			m_ItemList.remove(index);
			pItem->~CRankingItem();
			CRankingMgr::m_pAllocator->FreeBuffer(pItem);
		}
		m_boModify = true;
	}
}

CRankingItem* CRanking::GetPtrFromId( INT_PTR nId ,INT_PTR& nIndex)
{
	if(!IsValid())return NULL;
	nIndex= GetIndexFromId(nId);
	if (nIndex < 0 || nIndex >= m_ItemList.count()) return NULL;
	return m_ItemList[nIndex];
}


CRankingItem*  CRanking::GetPtrFromId(UINT_PTR nId)
{
	if(!IsValid())return NULL;
	INT_PTR nIndex= GetIndexFromId(nId);
	if (nIndex < 0 || nIndex >= m_ItemList.count()) return NULL;
	return m_ItemList[nIndex];
}




INT_PTR CRanking::GetIndexFromId( UINT_PTR nId )
{
	if(!IsValid())return -1;
	if(nId ==0) return -1;
	for(INT_PTR i=m_ItemList.count() -1; i >=0 ; i-- )
	{
		if(m_ItemList[i]->nId == nId)
		{
			return i;
		}
	}
	return -1;
}

bool CRanking::SwapItem(INT_PTR nPos1,INT_PTR nPos2 )
{
	if(!IsValid())return false;

	/*
	if(m_isRanked)
	{
		OutputMsg(rmError,"Rank %sis Raned,can not swapitem",m_szFile);
		return false;
	}
	*/

	if(nPos1 == nPos2 ) return false;
	if(nPos1 <0 || nPos1 >= m_ItemList.count() ||  nPos2 <0 || nPos2 >= m_ItemList.count() )
	{
		return false;
	}
	CRankingItem *pItem1= m_ItemList[nPos1];
	CRankingItem *pItem2= m_ItemList[nPos2];
	if(pItem1->IsValid() && pItem2->IsValid())
	{
		m_ItemList[nPos1]=pItem2;
		m_ItemList[nPos2]=pItem1;
		m_boModify =true;
		m_isRanked =false;
		return true;
	}
	else
	{
		return false;
	}
}
bool CRanking::JumpQueue(CRankingItem *pItem,INT_PTR nNewPos)
{
	if(!IsValid())return false;

	INT_PTR nPos = pItem->GetIndex();
	if(nPos <0 || nPos >= m_ItemList.count()) return false;
	
	//已经排序好的不需要重新排列
	if(isRanked())
	{
		OutputMsg(rmError,"Ranked rank %scan not jump queue",m_szFile);
		return false;
	}
	if(nPos ==nNewPos )return false;
	if(nNewPos <0 || nNewPos >= m_ItemList.count()) return false;

	m_ItemList.remove(nPos);
	m_ItemList.insert(nNewPos,pItem); //插队
	m_boModify =true;
	return true;
}
//降序去查找一个位置
INT_PTR CRanking::GetInsertPos(int nPoint)
{
	if(!IsValid())
	{
		return 0;
	}
	if(m_ItemList.count() <=0) return 0;

	for(INT_PTR i=0 ; i< m_ItemList.count() ;i ++)
	{
		int nComp = m_ItemList[i]->nPoint;
		if(nPoint > nComp) //如果分数大于这个点了
		{
			return i;
		}
	}
	return  m_ItemList.count() ;
}

CRankingItem* CRanking::AddItem( UINT_PTR nId,INT_PTR nPoints/* = 0*/,bool bRankFlag)
{
	if(!IsValid())
		return NULL;
	INT_PTR nIdIndex = GetIndexFromId(nId);

	if ( nIdIndex>= 0)
	{
		return NULL;
	}
	
	CRankingItem* item = (CRankingItem*)CRankingMgr::m_pAllocator->AllocBuffer(sizeof(CRankingItem));
	new (item)CRankingItem(this);
	item->SetPoint((int)nPoints);
	item->SetId((unsigned int)nId);	// 
	item->SetColCount(m_Title.GetColCount());
	//item->SetParent(this);

	//如果需要排序的话就排序下，否则直接放后面
	if(bRankFlag)
	{
		INT_PTR nPos = GetInsertPos((int)nPoints); //获取需要插入的位置
		
		m_ItemList.insert(nPos,item);
		UpdateRankProp(item,nPos);		
	}
	else
	{
		m_ItemList.add(item);  
		UpdateRankProp(item,m_ItemList.count() -1);
		SetRanked(false); //这已经不是排序好的表了
	}
	

	m_boModify = true;

	return item;
}

CRankingItem* CRanking::GetItem( INT_PTR nIndex )
{
	if(!IsValid())return NULL;
	if (nIndex >= 0 && nIndex < m_ItemList.count())
	{
		return m_ItemList[nIndex];
	}
	return NULL;
}


/*
CRankingItem* CRanking::FindMaxItem()
{
	if(!IsValid())return NULL;

	INT_PTR nCount = m_ItemList.count();
	if ( nCount <=0 )
	{
		return NULL;
	}
	if(isRanked()) //如果是排序好的话，直接取第1个
	{
		return m_ItemList[0];
	}
	else  //没有排序好，需要便利查找
	{
		int nPoint =-1;
		int nIndex =-1;
		for(INT_PTR i=0;i < m_ItemList.count(); i++)
		{
			CRankingItem *pItem = m_ItemList[i];
			if(pItem->IsValid() && pItem->GetPoint() > nPoint)
			{
				nIndex = i;
				nPoint = pItem->GetPoint();
			}
		}
		//找个最大的索引出去
		if(nIndex >=0)
		{
			return m_ItemList[nIndex];
		}
		
	}
	return NULL;
}
*/




void CRanking::SaveItem( CRankingItem* pItem,  CStringBuff &buff )
{
	CHECK_RANK_VALID()
	if(pItem ==NULL || pItem->IsValid() ==false) return;
	//DECLARE_FUN_TIME_PROF()
	char sBuf[2048];   //注意title列数过多
	sprintf_s(sBuf,sizeof(sBuf),"%u\t%d",pItem->nId,pItem->nPoint);
	//fputs(sBuf,fp);
	//buff.Append(sBuf);
	
	for (INT_PTR i = 0; i < pItem->GetColCount(); i++)
	{
		//fputc('\t',fp);
		//buff << ('\t');
		strcat(sBuf,"\t");

		LPCSTR sData = pItem->GetSubData(i);
		//buff.Append(sData?sData:"-");
		strcat(sBuf,sData);
		//fputs(sData?sData:"-",fp);
	}
	buff.Append(sBuf);
}

void CRanking::SetColDisplay( INT_PTR nIndex, bool disp )
{
	if (nIndex >= 0 && nIndex < m_Title.GetColCount())
	{
		m_ColDisplay[nIndex] = !disp;//1是不显示
	}
}

bool CRanking::isColDisplay( INT_PTR nIndex )
{
	if (nIndex >= 0 && nIndex < m_Title.GetColCount())
	{
		return !m_ColDisplay[nIndex];
	}
	return false;
}


void CRanking::SetRankPropConfig(int nRankPropIndex)
{
	DataList<RANKPROP>& rankProps = GetLogicServer()->GetDataProvider()->GetMiscConfig().GetRankProps();

	if (nRankPropIndex < 0 || nRankPropIndex >= rankProps.count)
	{
		OutputMsg(rmError, "Set Rank Prop Error! config index=%d, config count=%d", nRankPropIndex,rankProps.count);
		return;
	}

	RANKPROP *pProps = rankProps.pData + nRankPropIndex;
	if (pProps == NULL) return;

	m_nRankPropIndex = nRankPropIndex;			//从0算起
	m_nRankPropMax = (int)pProps->props.count;	//保存数量

}

void CRanking::UpdateRankProp(CRankingItem *pItem,INT_PTR nIndex, bool bClear)
{
	if (m_nRankPropIndex < 0) return;
	DECLARE_TIME_PROF("CRanking::UpdateRankProp");
	
	if(pItem ==NULL ) return;
	CActor *pActor = GetGlobalLogicEngine()->GetEntityMgr()->GetEntityPtrByActorID(pItem->nId);
	if (pActor == NULL) return;
	
	int nRankIndex = -1;
	//如果清空榜单操作则绝对删除
	if (!bClear)	
	{
		if (nIndex < m_nRankPropMax && pItem->nPoint >= m_nRankPropValue)
		{
			nRankIndex = (int)nIndex;
		}
	}

	pActor->GetMiscSystem().UpdateRankPropData(m_nRankPropIndex, nRankIndex, pItem->nPoint);
	

}

void CRanking::AtachRankProp(LPCSTR sName)
{
	m_nRankPropIndex = -1;
	m_nRankPropMax = 0;

	if (!sName) return;
	DataList<RANKPROP>& rankProps = GetLogicServer()->GetDataProvider()->GetMiscConfig().GetRankProps();
	for (INT_PTR i = 0; i < rankProps.count; i++)
	{
		RANKPROP *pProps = rankProps.pData + i;
		if (pProps == NULL || pProps->rankName[0] == '\0') continue;

		if (strcmp(sName, pProps->rankName) == 0)
		{
			if(pProps->pRanking)
			{
				OutputMsg(rmError,"%s, sName=%s,pProps->pRanking !=NULL",__FUNCTION__,sName);
			}
			pProps->pRanking = this;
			m_nRankPropIndex = (int)i;
			m_nRankPropMax = (int)pProps->props.count;
			m_nRankPropValue = pProps->nValue;
			return;
		}
	}
}

void CRanking::DetachhRankProp(LPCSTR sName)
{
	if (!sName) return;
	RANKPROP *pProps = GetLogicServer()->GetDataProvider()->GetMiscConfig().GetRankPropByRankName(sName);
	if (pProps)
	{
		if(pProps->pRanking ==NULL)
		{
			OutputMsg(rmError,"%s, sName=%s,pProps->pRanking ==NULL",__FUNCTION__,sName);
		}
		pProps->pRanking = NULL;
	}
}


void CRanking::PushToPack(CActor* pActor, int nNum, CDataPacket* pDataPack)
{
	if (pDataPack==NULL || pActor == NULL)
	{
		return;
	}

	CDataPacket& pack = *pDataPack;
	
	int nItemCount = GetList().count();
	if (nNum < nItemCount)
	{
		nItemCount = nNum;
	}

	// 排行榜数据
	pack << BYTE(nItemCount);
	for (size_t i = 0; i < nItemCount; i++)
	{
		CRankingItem * pItem = GetItem(i);
		if (pItem)
		{
			int nActorid= (unsigned int)pItem->GetId();
			pack << (unsigned int)pItem->GetId();
			pack << (unsigned int)pItem->GetPoint();
			pack.writeString(pItem->GetSubData(0));//ACTORNAME
			pack << (unsigned int)pItem->GetParam();
			pack << (unsigned int)pItem->GetParam1();//yy超玩
			pack.writeString(pItem->GetSubData(1));//sexjob
			pack.writeString(pItem->GetSubData(2));//
			pack.writeString(pItem->GetSubData(3));//vip
		}
	}

	// 我的数据
	CRankingItem * pItem = GetPtrFromId(pActor->GetId());
	if (pItem)
	{
		pack << (unsigned int)pItem->GetPoint();
		int index = pItem->GetIndex()+1;
		pack << (short)index;
	}
	else
	{
		pack << (unsigned int)0;
		pack << (short)0;
	}
}


bool CRanking::CheckActorIdInRank(unsigned int nActorId)
{
	int nItemCount = GetList().count();
	for (size_t i = 0; i < nItemCount; i++)
	{
		CRankingItem * pItem = GetItem(i);
		if (pItem)
		{
			if ((unsigned int)pItem->GetId() == nActorId)
				return true;
		}
	}

	return false;
}

unsigned int CRanking::GetActorIdBySexJob(LPCSTR nCondition)
{
	int nItemCount = GetList().count();
	for (size_t i = 0; i < nItemCount; i++)
	{
		CRankingItem * pItem = GetItem(i);
		if (pItem)
		{
			LPCSTR sData = pItem->GetSubData(1);

			if(strcmp(sData, nCondition) == 0)
				return pItem->GetId();
	
		}
	}

	return 0;
}



void CRanking::PushRankToPack(int nNum, CDataPacket* pDataPack)
{
	if (pDataPack==NULL)
	{
		return;
	}

	CDataPacket& pack = *pDataPack;
	
	int nItemCount = GetList().count();
	if (nNum < nItemCount)
	{
		nItemCount = nNum;
	}

	// 排行榜数据
	pack << BYTE(nItemCount);
	for (size_t i = 0; i < nItemCount; i++)
	{
		CRankingItem * pItem = GetItem(i);
		if (pItem)
		{
			int nActorid= (unsigned int)pItem->GetId();
			pack << (unsigned int)pItem->GetId();
			pack << (unsigned int)pItem->GetPoint();
			pack.writeString(pItem->GetSubData(0));//ACTORNAME
			pack << (unsigned int)pItem->GetParam();
			pack << (unsigned int)pItem->GetParam1();//yy超玩
			pack.writeString(pItem->GetSubData(1));//sexjob
			pack.writeString(pItem->GetSubData(2));//
			// pack.writeString(pItem->GetSubData(3));//vip
			// pack.writeString("");//
			if (CActor* pUser = GetGlobalLogicEngine()->GetEntityMgr()->GetEntityPtrByActorID(nActorid))
			{
				pack << (BYTE)1;
				GetGlobalLogicEngine()->GetOfflineUserMgr().viewOnlineUserData(pUser, pack);
			}
			// 再从离线玩家找
			else if(COfflineUser *pUser = GetGlobalLogicEngine()->GetOfflineUserMgr().GetUserByActorId(nActorid))
			{
				pack << (BYTE)1;
				GetGlobalLogicEngine()->GetOfflineUserMgr().viewOfflineUserData(pUser, pack);
			}
			else
			{
				pack << (BYTE)0;
			}
			
		}
	}
}



void CRanking::PushEspecialRankToPack(int nNum, CDataPacket* pDataPack)
{
	if (pDataPack==NULL)
	{
		return;
	}

	CDataPacket& pack = *pDataPack;
	
	int nItemCount = GetList().count();
	if (nNum < nItemCount)
	{
		nItemCount = nNum;
	}

	// 排行榜数据
	pack << BYTE(nItemCount);
	for (size_t i = 0; i < nItemCount; i++)
	{
		CRankingItem * pItem = GetItem(i);
		if (pItem)
		{
			int nActorid= (unsigned int)pItem->GetId();
			pack << (unsigned int)pItem->GetId();
			pack << (unsigned int)pItem->GetPoint();
			pack.writeString(pItem->GetSubData(0));//ACTORNAME
		}
	}
}

void CRanking::SendCrossServerToPack(int nNum, CDataPacket& pack)
{
	int nItemCount = GetList().count();
	if (nNum < nItemCount)
	{
		nItemCount = nNum;
	}

	// 排行榜数据
	pack << BYTE(nItemCount);
	for (size_t i = 0; i < nItemCount; i++)
	{
		CRankingItem * pItem = GetItem(i);
		if (pItem)
		{
			int nActorid= (unsigned int)pItem->GetId();
			pack << (unsigned int)pItem->GetId();
			pack << (unsigned int)pItem->GetPoint();
			pack.writeString(pItem->GetSubData(0));//ACTORNAME
			pack << (unsigned int)pItem->GetParam();
			pack << (unsigned int)pItem->GetParam1();//yy超玩
			pack.writeString(pItem->GetSubData(1));//sexjob
			pack.writeString(pItem->GetSubData(2));//
			pack.writeString(pItem->GetSubData(3));// //vip
		}
	}
}
