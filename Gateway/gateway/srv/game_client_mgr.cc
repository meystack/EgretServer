#include "StdAfx.h"
#include <map>
#include "client/client_obj_mgr.h"
#include "event/select_event.h"
#include "event/epoll_event.h"

GameClientMgr::GameClientMgr(const char *name, GateServer *srv) {
	srv_ = srv;
	gw_cli_ = new GameWorldClient(srv, this, name);
	cli_obj_mgr_ = new ClientObjMgr();
	inner_msg_list_.setLock(&inner_msg_lock_);
#if USE_EPOLL
	event_ = new EpollEvent(this);
#else
	event_ = new SelectEvent(this);
#endif
}

GameClientMgr::~GameClientMgr() {
	cli_obj_mgr_->flush();
	SafeDelete(cli_obj_mgr_);
	inner_msg_list_.flush();
	inner_msg_list_.clear();
	SafeDelete(gw_cli_);
}

void GameClientMgr::SetMaxSession(int max_session) {
	cli_obj_mgr_->SetMaxSession(max_session);
}

void GameClientMgr::InitGameWorldClient(const char * ip, int port) {
	gw_cli_ = new GameWorldClient(srv_, this, GetServiceName());
	gw_cli_->UseBaseAlloc(true);
	gw_cli_->SetHost(ip);
	gw_cli_->SetPort(port);
}

bool GameClientMgr::onConnect(SOCKET nSocket, sockaddr_in * pAddrIn) {
	if (!gw_cli_->connected()) return false;

	Client *cli = new Client(this, nSocket, pAddrIn);
	cli_obj_mgr_->setNewClient(cli);
	cli_obj_mgr_->flush();
	cli->SetBlockMode(false);
	GateInterMsg msg;
	msg.msg_id_ = gcAddClient;
	msg.data_.fd_ = nSocket;
	msg.data_.sessidx_ = cli->GetSessionId();
	PostMsg(msg);
	OutputMsg(rmTip, _T("[Login] (1) 客户端连接上来： gatekey(%d)，socket(%lld)，GateIdx(%d)，LogicIdx(%d)，CurrentThreadId(%d)。"),
		cli->GetSKey(), nSocket, cli->GetNetId().index_, cli->GetNetId().gate_id_, GetCurrentThreadId());
	//MSG_TIP("new client connect fd:%d, sid:%d", cli->GetFd(), cli->GetSessionId());
	return true;
}

int GameClientMgr::Run() {
	inner_msg_list_.flush();
	int count = inner_msg_list_.count();
	for (int i = 0; i < count; ++i)
	{
		GateInterMsg& msg = inner_msg_list_[i];
		OnRecvSysMsg(msg);
	}
	inner_msg_list_.clear();
	event_->RunOne();
	return count;
}

void GameClientMgr::OnStart() {
	MSG_LOG("GameClientMgr::OnStart");
	event_->init();
}

void GameClientMgr::OnStop() {
	MSG_LOG("GameClientMgr::OnStop");
}

void GameClientMgr::Stop() {
	MSG_LOG("%s, start", GetServiceName());
	while (gw_cli_->GetPacketCount()) {//这里还有包没发完，导致一致没法停止
		gw_cli_->SingleRun();
		Sleep(1);
	}
	gw_cli_->Stop();
	Inherited::Stop();
	MSG_LOG("%s, ok", GetServiceName());
}

void GameClientMgr::OnRecvSysMsg(GateInterMsg & msg)
{
	switch (msg.msg_id_)
	{
		case gcAddClient:
		{
			event_->AddReadFd(msg.data_.fd_, cli_obj_mgr_->get(msg.data_.sessidx_));
			break;
		}
		case gcServerIdx:
		{
			Client* cli = cli_obj_mgr_->get(msg.data_.idx_);
			if (cli) {
				cli->GetNetId().gate_id_ = msg.data_.para_;
				OutputMsg(rmTip, _T("[Login] (5) LogicServer已成功添加玩家Gate： gatekey(%d)，socket(%lld)，GateIdx(%d)，LogicIdx(%d)，CurrentThreadId(%d)。"),
					cli->GetSKey(), cli->GetNetId().socket_, cli->GetNetId().index_, cli->GetNetId().gate_id_, GetCurrentThreadId());
			}
			break;
		}
		case gcGWData:
		{
			Client* cli = cli_obj_mgr_->get(msg.data_.index_);
			if (cli) {
				auto dp = msg.data_.packet_;
				cli->OnGameWorldRecv(dp->getOffsetPtr(), dp->getAvaliableLength());
			}
			gw_cli_->FreeBackUserDataPacket(msg.data_.packet_);
			break;
		}
		case gcGWClose:
		{
			Client* cli = cli_obj_mgr_->get(msg.data_.index_);
			if (cli) {
				cli->Close();
			}
			break;
		}
		case gcChAdd:
		{
			uint64_t key = MAKEINT64(msg.data_.channel_, msg.data_.para_);
			channel_indexs_map_[key].insert(msg.data_.idx_);
			break;
		}
		case gcChDel:
		{
			uint64_t key = MAKEINT64(msg.data_.channel_, msg.data_.para_);
			channel_indexs_map_[key].erase(msg.data_.idx_);
			break;
		}
		case gcChBro:
		{
			uint64_t key = MAKEINT64(msg.data_.b_channel_, msg.data_.b_para_);
			auto &list = channel_indexs_map_[key];
			for (auto idx:list) {
				Client* cli = cli_obj_mgr_->get(idx);
				if (cli) {
					auto dp = msg.data_.packet_;
					cli->OnGameWorldRecv(dp->getOffsetPtr(), dp->getAvaliableLength());
				}
			}
			gw_cli_->FreeBackUserDataPacket(msg.data_.dp_);
			break;
		}
		case gcGWDisconn:
		{
			CloseAllClient();
			break;
		}
	}
}

void GameClientMgr::ProssClient(void) {
	//cli_obj_mgr_->flush();
	auto &list = cli_obj_mgr_->getClientMap();
	std::vector<uint16_t> remove_list;
	for (auto it:list)	{
		Client *cli = it.second;
		if(cli)
		{
			if (cli->IsClose()) {
				remove_list.push_back(cli->GetSessionId());
				continue;
			}
			cli->RecvData();
			cli->PacketHandle();
			cli->WriteEvent();
			cli->SendData();
		}
	}
	for (auto idx: remove_list) {
		Client *cli = cli_obj_mgr_->remove(idx);
		SafeDelete(cli);
	}
	remove_list.clear();
}

void GameClientMgr::HandleReadEvent(SOCKET fd, Client * cli) {
	if (cli) {
		cli->RecvData();
		cli->PacketHandle();
		event_->AddWriteFd(fd, cli);
	}
}

void GameClientMgr::HandleWriteEvent(SOCKET fd, Client * cli) {
	if (cli) {
		cli->WriteEvent();
		if (!cli->SendData()) {
			event_->AddWriteFd(fd, cli);
		}
	}
}

void GameClientMgr::CloseAllClient() {
	cli_obj_mgr_->flush();
	auto &list = cli_obj_mgr_->getClientMap();
	std::vector<uint16_t> remove_list;
	for (auto it : list) {
		Client *cli = it.second;
		cli->Close();
	}
}

void GameClientMgr::HaveData(Client * cli) {
	if(cli) {
		event_->AddWriteFd(cli->GetFd(), cli);
	}
}

void GameClientMgr::CloseClient(Client * cli) {
	event_->DelFd(cli->GetFd());
	closesocket(cli->GetFd());
	if (cli_obj_mgr_->get(cli->GetSessionId()))
	{
		cli_obj_mgr_->remove(cli->GetSessionId());
	}
	
}

bool GameClientMgr::Startup() {
	if (!Inherited::Startup()) {
		return false;
	}
	// 创建发送线程
	
	return gw_cli_->Startup();
}
