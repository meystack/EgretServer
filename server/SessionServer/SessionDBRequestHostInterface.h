#ifndef LOGIC_SESSION_REQUEST_HOST_INTERFACE_H
#define LOGIC_SESSION_REQUEST_HOST_INTERFACE_H

/*
	逻辑DB请求处理容器接口。派生类需要实现分配数据包对象和发送数据包接口，用于
	处理DB请求后给逻辑回的处理结果应答。
*/
class ISessionRequestHost
{
public:	
	/*
	* Comments: 分配数据包
	* Param const jxSrvDef::INTERSRVCMD nCmd:
	* @Return CDataPacket&:
	* @Remark:
	*/
	virtual CDataPacket& AllocDataPacket(const jxSrvDef::INTERSRVCMD nCmd) = 0;

	/*
	* Comments: 刷新数据包到发送队列
	* Param CDataPacket &packet
	* @Return void:
	* @Remark:
	*/
	virtual void FlushDataPacket(CDataPacket &packet) = 0;


};

#endif