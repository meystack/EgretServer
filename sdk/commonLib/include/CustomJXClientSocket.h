#pragma once

class CCustomJXClientSocket :
	public CCustomClientSocket
{
public:
	typedef CCustomClientSocket Inherited;

public:
	//按通信协议申请一份发送数据包，nCmd是通信消息号，
	//数据包填充完毕后需要调用flushProtoPacket提交到发送队列
	CDataPacket& allocProtoPacket(const jxSrvDef::INTERSRVCMD nCmd);

	//提交发送数据包到发送缓冲，会自动计算数据长度并向数据包的通信协议头部分写入长度值
	VOID flushProtoPacket(CDataPacket& packet);

protected:
	/*** 子类需覆盖的函数集 ***/
	/* 处理单个通信数据包 
	 * nCmd		通信消息命令
	 * inPacket	已经读取出通信命令的数据包，数据包的读取位置指向命令数据后的内容
	 */
	virtual VOID OnDispatchRecvPacket(const jxSrvDef::INTERSRVCMD nCmd, CDataPacketReader &inPacket) = 0;

	/*** 下列函数为子类可能有必要覆盖的函数集 ***/
	/* ★查询本地服务器的类型，以便正确的发送注册数据 */
	virtual jxSrvDef::SERVERTYPE getLocalServerType() = 0;

	/* ★查询本地服务器的名称，以便正确的发送注册数据 */
	virtual LPCSTR getLocalServerName() = 0;
	/* ★查询本地服务器ID，以便正确的发送注册数据，默认的返回值是0 */
	virtual int getLocalServerIndex();

	 VOID SendKeepAlive();//heart tick
protected:
	/*** 覆盖父类的网络数据包处理函数以便实现通信协议的数据包解析 ***/
	VOID ProcessRecvBuffers(PDATABUFFER pDataBuffer);
	/* 发送注册服务器的消息 */
	VOID SendRegisteClient();
};
