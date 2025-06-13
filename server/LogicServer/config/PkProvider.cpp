#include "StdAfx.h"
#include "PkProvider.h"

using namespace wylib::stream;

CPkProvider::CPkProvider()
	: Inherited(), m_DataAllocator(_T("PKDataAlloc"))
{

}

CPkProvider::~CPkProvider()
{
}

bool CPkProvider::LoadPkConfig(LPCTSTR sFilePath)
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
			showError(_T("syntax error on PK config"));
		//读取标准物品配置数据
		Result = readConfigs();
	}
	catch (RefString &s)
	{
		OutputMsg(rmError, _T("load pk config error: %s"), s.rawStr());
	}
	catch (...)
	{
		OutputMsg(rmError, _T("unexpected error on load PK config"));
	}
	//销毁脚本虚拟机
	setScript(NULL);
	return Result;
}

bool CPkProvider::readConfigs()
{
	CDataAllocator dataAllocator;
	if ( !openGlobalTable("PkConfig") ) return false;

	int nDef_1 = 1;
	int nDef_0 = 0;
	pkSubTime = getFieldInt("subPk",&nDef_1);//单位是秒
	redName = getFieldInt("redName",&nDef_1);//红名
	m_pkValue = getFieldInt("pkValue",&nDef_1);

	if(!openFieldTable("nEnterRange")) return false;
	{
		m_nPrisonSceneId = getIndexInt(1,&nDef_0);
		m_nPrisonPosX = getIndexInt(2,&nDef_0);
		m_nPrisonPosY = getIndexInt(3,&nDef_0);
		m_nPrisonWeight = getIndexInt(4,&nDef_0);
		m_nPrisonHeight = getIndexInt(5,&nDef_0);
	}
	closeTable();
	m_ClearTime = getFieldInt("nameColorClearTime",&nDef_1);
	m_nSubPkValue = getFieldInt("nSubPkValue",&nDef_1);
	m_nCountPkValue = getFieldInt("nCountPkValue",&nDef_1);

	//关闭全局表
	closeTable();

	//原来的内存块释放到
	m_DataAllocator.~CObjectAllocator();
	//将临时的内存块复制到正式的
	m_DataAllocator = dataAllocator;
	//清空临时的
	ZeroMemory(&dataAllocator, sizeof(dataAllocator));

	return true;
}


void CPkProvider::showError(LPCTSTR sError)
{
	m_sLastErrDesc = sError;
	RefString s = _T("[PkProvider]");
	s += sError;
	throw s;
}
