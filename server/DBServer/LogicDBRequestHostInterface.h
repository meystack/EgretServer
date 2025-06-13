#ifndef LOGIC_DB_REQUEST_HOST_INTERFACE_H
#define LOGIC_DB_REQUEST_HOST_INTERFACE_H

/*
	逻辑DB请求处理容器接口。派生类需要实现分配数据包对象和发送数据包接口，用于
	处理DB请求后给逻辑回的处理结果应答。
*/
class ILogicDBRequestHost
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

	// 此函数用到DataClient里头的一些内容。暂时没有将内部消息导入到DBRequestHandler里头处理，所以
	// 暂时将次函数作为虚函数来实现。以后可以考虑在DBDataClient里头Post的内部消息也导入到Requesthandler
	// 里头处理，然后将所有相关的数据结构和实现都转移到RequestHandler里头
	virtual int CatchCreateGuild(int nActor,
								 LPCSTR sGuildName,	
								 LPCSTR sFoundName,
								 int nIndex,
								 Uint64	hActor,
								 BYTE nZY,
								 INT_PTR nIcon) = 0;

								 
	virtual int CatchRemoveGuild(int) = 0;
};

#endif