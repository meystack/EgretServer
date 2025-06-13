#include "StdAfx.h"
#include "game_world_client.h"

GameWorldClient::GameWorldClient(GateServer* serv, GameClientMgr* gcmgr, const char* name) :
NetClient(name),
serv_(serv),
gcmgr_(gcmgr) {
	SetClientName(name);
	free_.setLock(&lock_);
}


GameWorldClient::~GameWorldClient() {
	free_.flush();

	for (int i = 0; i < free_.count(); i++)
	{
		DataPacket* dp = free_[i];
		dp->setPosition(0);
		Flush(*dp);
	}

	free_.clear();
}

void GameWorldClient::OnRecv(const uint16_t cmd, char * buf, int size)
{
	GATEMSGHDR* hdr = (GATEMSGHDR*)buf;
	buf = (char*)(hdr + 1);

	switch (cmd) {
	case GM_DATA: {
		//NetId net_id;
		//net_id.socket_ = hdr->nSocket;
		//net_id.index_ = hdr->wSessionIdx;
		//net_id.gate_id_ = hdr->wServerIdx;
		DataPacket *dp = AllocPostPacket();
		dp->writeBuf(buf , size);
		dp->setPosition(0);
		GateInterMsg msg;
		msg.msg_id_ = gcGWData;
		msg.data_.index_ = hdr->wSessionIdx;
		msg.data_.packet_ = dp;
		gcmgr_->PostMsg(msg);
		break;
	}
	case GM_CLOSE: {
		GateInterMsg msg;
		msg.msg_id_ = gcGWClose;
		msg.data_.index_ = hdr->wSessionIdx;
		gcmgr_->PostMsg(msg);
		break;
	}
	case GM_SERVERUSERINDEX: {
		GateInterMsg msg;
		msg.msg_id_ = gcServerIdx;
		msg.data_.idx_ = hdr->wSessionIdx;
		msg.data_.para_ = hdr->wServerIdx;
		gcmgr_->PostMsg(msg);
		break;
	}
	case GM_CHECKCLIENT: {
		break;
	}
	default:
		MSG_LOG("not handle gameword cmd:%d", cmd);
		break;
	}
}

void GameWorldClient::OnChannelMsg(char* buf, int size) {
	Channel info = *((Channel*)buf);
	switch (info.type) {
	case ccBroadCast: {
		uint8_t nsize = sizeof(Channel);
		DataPacket *dp = AllocPostPacket();
		dp->writeBuf(buf + nsize, size - nsize);
		dp->setPosition(0);
		GateInterMsg msg;
		msg.msg_id_ = gcChBro;
		msg.data_.b_channel_ = info.channelId;
		msg.data_.b_para_ = info.para;
		msg.data_.dp_ = dp;
		gcmgr_->PostMsg(msg);
		break;
	}
	case ccAddUser: {
		GateInterMsg msg;
		msg.msg_id_ = gcChAdd;
		msg.data_.channel_ = info.channelId;
		msg.data_.para_ = info.para;
		msg.data_.idx_ = info.index_;
		gcmgr_->PostMsg(msg);
		break;
	}
	case ccDelUser: {
		GateInterMsg msg;
		msg.msg_id_ = gcChDel;
		msg.data_.channel_ = info.channelId;
		msg.data_.para_ = info.para;
		msg.data_.idx_ = info.index_;
		gcmgr_->PostMsg(msg);
		break;
	}
	default:
		MSG_LOG("not handle channel type:%d", info.type);
		break;
	}
}

int GameWorldClient::GetLocalServerType() {
	return SrvDef::GateServer;
}

const char * GameWorldClient::GetLocalServerName() {
	return gcmgr_->GetServiceName();
}

void GameWorldClient::OnDisconnected() {
	GateInterMsg msg;
	msg.msg_id_ = gcGWDisconn;
	gcmgr_->PostMsg(msg);
}

void GameWorldClient::FreeBackUserDataPacket(DataPacket* pack) {
	free_.append(pack);
}

DataPacket* GameWorldClient::AllocPostPacket()
{
	if (free_.count() <= 0)
	{
		free_.flush();
	}

	if (free_.count() <= 0)
	{
		AllocSendPack(&free_, 512);
	}

	DataPacket* dp = free_.pop();//�õ�һ�����е�Datapacket
	dp->setLength(0);
	return dp;
}
