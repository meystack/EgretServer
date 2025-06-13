#include "StdAfx.h"

#include "PetProvider.h"
#include "../base/Container.hpp"
using namespace wylib::stream;


CPetProvider::CPetProvider() : m_dataAllocator(_T("PetDataAlloc"))
{
	
}

CPetProvider::~CPetProvider()
{
	m_petData.empty();
}


void CPetProvider::showError(LPCTSTR sError)
{
	m_sLastErrDesc = sError;
	RefString s = _T("[Config Error]");
	s += sError;
	throw s;
}

//装载宠物的经验
bool CPetProvider::LoadPetExp(LPCTSTR sFilePath)
{
	//bool bResult = InheritedReader::LoadFile(this,sFilePath,&CPetProvider::ReadPetExp);
	bool bResult = false;
	bResult = Inherited::LoadFile(sFilePath);
	if (!bResult)
	{
		showError(_T("ReadPetExp config error!"));
		return false;
	}

	//bResult = InheritedReader::LoadFile(this,sFilePath,&CPetProvider::ReadPetMergePropRate);
	//if (!bResult)
	//{
	//	showError(_T("ReadPetMergeProp config error!"));
	//	return false;
	//}
	if(ReadPetExp() ==false)
	{
		return false;
	}
	setScript(NULL);

	bResult = Inherited::LoadFile(sFilePath);

	if( ReadPetMergePropRate() ==false)
	{
		return false;
	}
	setScript(NULL);

	return bResult;
}

bool CPetProvider::ReadPetExp()
{
	if ( !openGlobalTable("PetLevelUpExp") )
		return false;


	//获取配置中的物品数量
	const INT_PTR nCount = lua_objlen(m_pLua, -1);
	//如果物品数量为0则输出未配置任何物品的错误
	if ( nCount <= 0 )
		showError(_T("no exp data on pet exp config"));
	//申请出物品数据并将物品数据内存块清空
	m_petExp.count =nCount;

	if( nCount > 0 )
	{
		m_petExp.pData = ( DataList<unsigned int>*) m_dataAllocator.allocObjects( nCount * sizeof(DataList<unsigned int>)) ;
	}
	else
	{
		return false;
	}
	INT_PTR nIndex =0; //当前遍历的index
	if (enumTableFirst())
	{
		do 
		{
			DataList<unsigned int>* pData = m_petExp.pData + nIndex;

			const INT_PTR nItemCount = lua_objlen(m_pLua, -1); //有多少个物品
			pData->count = nItemCount; //拥有的物品的类型

			pData->pData =  ( unsigned int*) m_dataAllocator.allocObjects( nItemCount * sizeof(unsigned int)) ; 
			INT_PTR j=0;
			if (enumTableFirst())
			{
				do
				{
					pData->pData[j] = (unsigned int)getFieldInt(NULL);
					j++;
				} while (enumTableNext());
				nIndex ++;
			}

		}
		while (enumTableNext());
	}
	return true;
}

bool CPetProvider::ReadPetMergePropRate()
{
	if (!openGlobalTable("PetMergePropRate"))
		return false;

	//获取配置中的物品数量
	const INT_PTR nCount = lua_objlen(m_pLua, -1);
	//如果物品数量为0则输出未配置任何物品的错误
	if ( nCount <= 0 )
		showError(_T("no config data on PetMergePropRate config"));
	//申请出物品数据并将物品数据内存块清空
	m_petMergePropRate.count = nCount;

	if (nCount > 0)
	{
		m_petMergePropRate.pData = (DataList<PetMergePropRate>*)m_dataAllocator.allocObjects(nCount * sizeof(DataList<PetMergePropRate>));
	}
	else
	{
		return false;
	}

	INT_PTR nIndex =0; //当前遍历的index
	if (enumTableFirst())
	{
		do 
		{
			DataList<PetMergePropRate> *pData = m_petMergePropRate.pData + nIndex;

			const INT_PTR nItemCount = lua_objlen(m_pLua, -1); //有多少个物品
			pData->count = nItemCount; //拥有的物品的类型

			pData->pData =  (PetMergePropRate*)m_dataAllocator.allocObjects(nItemCount * sizeof(PetMergePropRate)) ; 
			INT_PTR j=0;
			if (enumTableFirst())
			{
				do
				{
					pData->pData[j].wType = (WORD)getFieldInt("type");
					pData->pData[j].fRate = (float)getFieldNumber("value");

					j++;
				} while (enumTableNext());
				nIndex ++;
			}

		}
		while (enumTableNext());
	}
	return true;
}


/*
* Comments:装载全部怪物的数据
* @Return bool: 失败返回false
*/
bool CPetProvider::LoadPets(LPCTSTR sFilePath)
{
	//return InheritedReader::LoadFile(this,sFilePath,&CPetProvider::ReadAllPet);
	if( Inherited::LoadFile(sFilePath) )
	{
		bool r= ReadAllPet();
		setScript(NULL);
		return r;

	}
	else
	{
		return false;
	}
}


bool CPetProvider::ReadAllPet()
{
	if ( !openGlobalTable("Pets") )
	{
		showError(_T("no global table Pets Found"));
		return false;
	}

	m_petData.clear();
	INT_PTR nIdx =0;
	if (enumTableFirst())
	{
		do 
		{
			PETCONFIG  onePet;
			ReadOnePet(m_dataAllocator,&onePet,nIdx);
			m_petData.add(onePet);
			nIdx++;
		}
		while (enumTableNext());
	}
	return true;
}

bool CPetProvider::ReadOnePet(CDataAllocator &dataAllocator,PPETCONFIG pPet,INT_PTR nPetId)
{
	pPet->id = getFieldInt("id"); 
	
	

	if(nPetId != pPet->id)
	{
		OutputMsg(rmError,_T("宠物的ID错误，配置的id=%d，期望为%d"),pPet->id, (int)nPetId);
		return false;
	}
	getFieldStringBuffer("name",pPet->name,sizeof(pPet->name) );

	int nDef=0;
	//pPet->defaultSkill = 	getFieldInt("defaultSkillId",&nDef);
	
	pPet->nModel = getFieldInt("modelid",&nDef);
	pPet->icon =getFieldInt("icon",&nDef);
	pPet->aiId =getFieldInt("aiId",&nDef);

	nDef =36;
	
	//读取各级宠物的属性
	if ( feildTableExists("property") && openFieldTable("property") )
	{
		const INT_PTR nCount =(INT_PTR) lua_objlen(m_pLua, -1) ;
		
		if (enumTableFirst())
		{
			INT_PTR i=0;
			do
			{

				if(i >= MAX_PET_LEVEL_COUNT) break;

				CREATURBATTLEEDATA data;
				
				nDef =0;
				double fDef =0.0;
				data.nColor =0;

				data.nMaxHp = getFieldInt("nMaxHp",&nDef);
				data.nMaxMp = getFieldInt("nMaxMp",&nDef);

				data.nPhysicalAttackMin = getFieldInt("nPhysicalAttackMin",&nDef);

				data.nPhysicalAttackMax = getFieldInt("nPhysicalAttackMax",&nDef);
				data.nMagicAttackMin = getFieldInt("nMagicAttackMin",&nDef);
				data.nMagicAttackMax = getFieldInt("nMagicAttackMax",&nDef);
				data.nWizardAttackMin = getFieldInt("nWizardAttackMin",&nDef);
				data.nWizardAttackMax = getFieldInt("nWizardAttackMax",&nDef);
				data.nHysicalDefenceMin = getFieldInt("nHysicalDefenceMin",&nDef);
				data.nHysicalDefenceMax = getFieldInt("nHysicalDefenceMax",&nDef);
				data.nMagicDefenceMin = getFieldInt("nMagicDefenceMin",&nDef);
				data.nMagicDefenceMax = getFieldInt("nMagicDefenceMax",&nDef);

				data.nHitrate = getFieldInt("nHitrate",&nDef);
				data.nDogerate = getFieldInt("nDogerate",&nDef);

				data.nMagicHitRate = getFieldInt("nMagicHitRate",&nDef);
				data.nMagicDogerate = getFieldInt("nMagicDogerate",&nDef);

				data.nHpRateRenew =(float) getFieldNumber("fHpRateRenew",&fDef);
				data.nMpRateRenew = (float)getFieldNumber("fMpRateRenew",&fDef);

				data.nMoveOneSlotTime = getFieldInt("nMoveOneSlotTime",&nDef);
				data.nAttackSpeed = getFieldInt("nAttackSpeed",&nDef);

				data.nLuck = getFieldInt("nLuck",&nDef);

				CPropertySystem::InitMonsterCalculator(data,pPet->levels[i].cal);
				
				unsigned int clr = DEFAULT_NAME_COLOR;
				const char *pKey = "color";
				if (feildTableExists(pKey) && openFieldTable(pKey))
				{
					INT_PTR count = lua_objlen(m_pLua, -1);
					if (count > 0)
					{
						if (enumTableFirst())
						{
							int elem[4] = {0}, idx = 0;				
							do 
							{
								if (idx < 4)
									elem[idx] = (int)getFieldNumber(NULL);
								idx++;
							} while (enumTableNext());

							clr = (COLORREF) ( ((BYTE)(elem[3])) | (((BYTE)(elem[2]))<<8) | (((BYTE)(elem[1]))<<16) | ((BYTE)(elem[0])<<24) );
						}
					}		
					closeTable();
				}
				pPet->levels[i].color= clr;

				
				i++; 
			} while (enumTableNext());
		}
		closeTable();

	}



	return true;
}
bool CPetProvider::ReadTable(char *sName, int *pAddr,int nMaxCount)
{
	
	if ( feildTableExists(sName) && openFieldTable(sName) )
	{
		const INT_PTR nCount =(INT_PTR) lua_objlen(m_pLua, -1) ;
		if ( nCount <= 0 )
		{
			//pPet->.count =0;
		}
		else
		{

			if ( enumTableFirst() )
			{
				INT_PTR nIdx = 0;

				int * pBaseLevel =pAddr;
				int * pOneCondition  =NULL;
				do 
				{					
					pOneCondition= pBaseLevel + nIdx;
					if( nMaxCount > nIdx )
					{
						*pOneCondition = getFieldInt(NULL); 
					}
					nIdx++;
				}while (enumTableNext());
			}
		}
		closeTable();
		return true;
	}
	else
	{
		return false;
	}
	
}
