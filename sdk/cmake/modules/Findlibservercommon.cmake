if ( _DEBUG  )
    set( libservercommon "servercommon_d" )
else()
    set( libservercommon "servercommon_r" )
endif()

message( status " using bundled ${libservercommon}..." )

find_path(
	LIBSERVERCOMMON_INCLUDE_DIR
	NAMES AchieveData.h ActorCacheDef.h ActorDbData.h ActorOfflineMsg.h CommonDef.h Consignment.h DBProto.h Fuben.h GemItemProperty.h GuildDb.h HeroData.h InterAction.h InterServerComm.h Msg.h PetData.h ServerDef.h SkillDbData.h UserItem.h CustomGateManager.h CustomGlobalSession.h CustomJXClientSocket.h CustomJXServerClientSocket.h CustomServerClientSocketEx.h CustomServerGate.h CustomServerGateUser.h CustomSessionClient.h Encrypt.h FileLogger.h GateProto.h HandleMgr.h HandleStat.h JXAbsGameMap.h LogSender.h LogType.h MapSearchNode.h mjEdcode.h mjProto.h NetworkDataHandler.h SqlHelper.h stlastar.h 
	#NAMES *.h *.hpp
	PATHS ${CMAKE_CURRENT_LIST_DIR}/../../../sdk/commonLib/include ${CMAKE_CURRENT_LIST_DIR}/../../../sdk/commonLib/def 
)

find_library(
	LIBSERVERCOMMON_LIBRARIES
	NAMES ${libservercommon}
	PATHS ${CMAKE_CURRENT_LIST_DIR}/../../../libs
)
