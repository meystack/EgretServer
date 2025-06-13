#pragma once
class ENTITYAPPEARVALUE;

class CAlmirahSystem :
	public CEntitySubSystem<enAlmirahSystemID, CAlmirahSystem, CActor>
{
public:
	CAlmirahSystem(void);
	~CAlmirahSystem(void);

	/************************************************************************/
	/* 继承                                                                 */
	/************************************************************************/
	
	/*
	 *处理网络数据包
	 *nCmd ，分派到该系统里命令，从0开始
	 *packet，网络数据
	*/
	virtual  void  ProcessNetData(INT_PTR nCmd,CDataPacketReader &packet );

	/// 保存数据库
	virtual void Save(PACTORDBDATA  pData);

	virtual void OnDbRetData(INT_PTR nCmd,INT_PTR nErrorCode,CDataPacketReader &reader );

	/*
	* Comments: 计算衣橱的属性
	* Param CAttrCalc & cal:属性计算器
	* @Return CAttrCalc:属性计算器
	*/ 
	CAttrCalc& CalcAttr(CAttrCalc &calc);


	/// 发送翅膀信息给客户端
	void OnSendInitData();

public:
	//激活翅膀
	bool SetActivation(int nModleId);
	//翅膀是否激活
	bool GetActivation(int nModleId);
	//设置翅膀状态(穿上/脱下)
	bool SetTakeon(int nModleId,byte nState);
	//获得当前的翅膀Modeid
	int GetNowModeId();
	int GetFashionLv(int nModeId);
	//激活
	void ActivationAlmirah(int nModleId);
	//添加新的
	int AddNewFashion(int nModelId, int nWear = 0);

	void UpdateFashion(int nModelId, int nLv);

	void UpGradeFashion(int nModeId);

	FashionActive* GetFashionModeId(int nType );

	
	void returnInfo(CDataPacket& netPack);
	CVector<CAlmirahItem>& getAlmirahItemLists() {
		return m_cAlmirahLists;
	}
	void TakeOffAlmirah(std::set<int>& takeOffs);
private:
	typedef CVector<CAlmirahItem> AlmirahList;
	AlmirahList m_cAlmirahLists;	//时装
	CTimer<60000>	m_timer;	
public:
	//跨服数据
	void SendMsg2CrossServer(int nType);
	//跨服数据初始化
	void OnCrossInitData(std::vector<CAlmirahItem>& m_Almira);
};

