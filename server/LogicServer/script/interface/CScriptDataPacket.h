#pragma once
//tolua_begin

namespace DataPack
{


	/*
		典型的使用方法为
		local pack = DataPack.allocPacket(pEntity,128,1) --分配一个数据包
		if(pack==nil) then return end
		DataPack.writeByte(pack,1); --压入一个Byte
		DataPack.writeByte(pack,2); --压入一个Byte
		DataPack.flush(pack); --向客户端发送
	*/
	/*
	* Comments: 申请一个数据包
	* Param void * pEntity:玩家的指针
	* Param int nSystemID: 子系统的ID
	* Param int nCmdID:  消息码
	* @Return void *: 返回数据包的指针，如果有没有发生的数据，返回空
	*/
	void * allocPacket(void* pEntity,int nSystemID,int nCmdID);

	/*
	* Comments: 向客户端发送刚刚打包进去的数据
	* Param void * pActorPacket:数据包的指针
	* @Return void :
	*/
	void flush(void* pActorPacket);

	/*
	* Comments: 向跨服发送刚刚打包进去的数据
	* Param void * pack:数据包的指针
	* @Return void :
	-- 例子
	local pack = allocPacketEx();
	-- 先写入消息号
	DataPack.writeWord(pack, nCmdId)
	-- 再写入具体的消息数据
	DataPack.writeByte(pack, handle)
	...
	-- 向跨服发送包
	DataPack.flushCs(pack)
	...
	-- 使用完之后使用， 否则会内存泄露！！！！！
	DataPack.freePacketEx(pack)

	*/
	void flushCs(void *packet);

	/*
	* Comments: 向一个网络包里写字符串
	* Param void * pPack: 数据包的指针
	* Param const char * str:字符串
	* @Return void:
	*/
	void writeString(void * pPack,const char *str);

	/*
	* Comments: 从网络包里读取字符串
	* Param void * pPack: 数据包的指针
	* @Return char*:字符串的指针
	*/
	char* readString(void * pPack);

	//下面是读取数据的接口

	unsigned char readByte(void * pPack);//读取无符号的1字节
	char readChar(void * pPack);//读取带符号的1字节
	unsigned short  readWord(void * pPack);//读取无符号的2字节
	short readShort(void * pPack); //读取带符号的2字节
	int readInt(void * pPack);//读取带符号的4字节
	unsigned int readUInt(void * pPack);	//读取无符号的4字节
	double readUint64(void * pPack);//读取无符号的8字节
	double readInt64(void * pPack); //读取有符号的8字节

	//读一系列的数据，避免多次调用c++接口造成性能问题
	int readData(lua_State *L);

	//下面是往数据包写数据的接口
	void writeByte(void * pPack,  unsigned char btValue); //写入无符号的1字节
	void writeChar(void * pPack,   char cValue);//写入带符号的1字节
	void writeWord(void * pPack, unsigned short wValue);//写入无符号的2字节
	void writeShort(void * pPack,  short wValue);//写入带符号的2字节
	void writeInt(void * pPack, int  nValue);//写入带符号的4字节
	void writeUInt(void * pPack, unsigned int  uValue);	//写入无符号的4字节
	void writeUint64(void * pPack, double value);//写入无符号的8字节
	void writeInt64(void * pPack, double value);//写入有符号的8字节
	//发送一系列的数据，避免多次调用c++接口造成性能问题
	int writeData(lua_State *L);

	/*
	* Comments: 申请一个数据包	
	* @Return void*: 返回数据包的指针。
	* @Remark:注意，这里申请的数据包仅仅是作为一个DataBuffer，序列化数据方便，不要调用flush方法来发送包！
	* 可以调用write之类的函数往包里头写数据。提供给脚本用于消息的广播。
	* 使用：
		local pack = allocPacketEx();
		-- 先写入子系统号和消息号
		DataPack.writeByte(pack, systemId)
		DataPack.writeByte(pack, msgId)
		-- 再写入具体的消息数据
		DataPack.writeByte(pack, handle)
		...
		-- 调用广播接口发送包
		Camp.broadCastMsg(pack) 
		-- 向跨服发送包
		DataPack.flushCs(pack)
		...
		
		-- 使用完之后使用， 否则会内存泄露！！！！！
		DataPack.freePacketEx(pack)
	*/
	void* allocPacketEx();

	/*
	* Comments: 释放创建的数据包
	* Param void* pack: 数据包指针
	* @Return void: 
	* @Remark: 和allocPacketEx()配对使用，不能对allocPack返回的包对象使用
	*/
	void freePacketEx(void* pack);

	// 广播场景内的所有玩家
	int broadcastScene(void* pActorPacket, unsigned int fbHandle, int nSceneId);

	// 广播世界所有玩家
	void broadcasetWorld(void* packet, int nLevel, int nCircle);
	//向一个行会广播
	void broadGuild(void* packet, int nGuildId);
};


//tolua_end
