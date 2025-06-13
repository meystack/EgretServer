#pragma once

class CCustomJXServerClientSocket :
	public CCustomServerClientSocket
{
public:
	typedef CCustomServerClientSocket Inherited;

public:
	//按通信协议申请一份发送数据包，nCmd是通信消息号，
	//数据包填充完毕后需要调用flushProtoPacket提交到发送队列
	CDataPacket& allocProtoPacket(const jxSrvDef::INTERSRVCMD nCmd);

	//提交发送数据包到发送缓冲，会自动计算数据长度并向数据包的通信协议头部分写入长度值
	VOID flushProtoPacket(CDataPacket& packet);

	//服务器是否注册
	inline bool registed(){ return m_boRegisted; }
	//获取连接的服务器ID
	inline unsigned int getClientServerIndex(){ return m_nClientSrvIdx; }
	//获取连接的服务器名称
	inline LPCSTR getClientName(){ return m_sClientName; }
	//获取连接的服务器类型
	inline jxSrvDef::SERVERTYPE getClientType(){ return m_eClientType; };

public:
	CCustomJXServerClientSocket();

protected:
	/*** 子类需覆盖的函数集 ***/
	/* 处理单个通信数据包 
	 * nCmd		通信消息命令
	 * inPacket	已经读取出通信命令的数据包，数据包的读取位置指向命令数据后的内容
	 */
	virtual VOID OnDispatchRecvPacket(const jxSrvDef::INTERSRVCMD nCmd, CDataPacketReader &inPacket) = 0;
	/* 验证客户端注册数据是否有效
	 *@return 返回true表示注册数据有效并，返回false则关闭连接
	 */
	virtual bool OnValidateRegData(const jxSrvDef::PSERVER_REGDATA pRegData) = 0;

	/*
	* Comments: 客户端验证通过
	* @Return void:
	* @Remark:
	*/
	virtual void OnRegDataValidated(){};
	
	VOID SendKeepAlive();
protected:
	/* 覆盖父类的网络数据包处理函数以便实现通信协议的数据包解析 */
	VOID ProcessRecvBuffers(PDATABUFFER pDataBuffer);
	/* 覆盖父类连接断开的函数以便情况注册数据，从而使得对象可以重用 */
	VOID Disconnected();
private:
	bool		m_boRegisted;			//客户端是否已经注册连接
	unsigned int			m_nClientSrvIdx;		//客户端的服务器ID
	CHAR		m_sClientName[40];		//客户端注册的服务器名称
	jxSrvDef::SERVERTYPE m_eClientType;	//客户端注册的服务器类型
};
