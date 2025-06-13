//tolua_begin
namespace TeamFun
{
	/*
	* Comments:获取队伍的队长指针
	* Param unsigned int nTeamId:队伍id
	* @Return void*:
	*/
	void* getTeamCaptain(unsigned int nTeamId);

	/*
	* Comments:获取队伍的队长指针
	* Param void * pTeam:队伍指针
	* @Return void*:
	*/
	void* getTeamCaptain(void* pTeam);

	/*
	* Comments:获取本人所在的擂台的id,本擂台可能是单人也可能是队伍的
	* @Return unsigned int:没有则返回0
	*/
	unsigned int getChallenge(unsigned int nTeamId);

	/*
	* Comments:设置本人的擂台id
	* Param unsigned int nChalllengeId:擂台id
	* @Return void:
	*/
	void setChallenge(unsigned int nTeamId,unsigned int nChalllengeId);

	/*
	* Comments:设置队伍的副本id，如果队伍不在副本中，设置成0
	* Param unsigned int nTeamId:
	* Param int nFbId:
	* @Return void:
	*/
	void setTeamFubenId(unsigned int nTeamId,int nFbId);

	/*
	* Comments:返回队伍中的动态变量
	* Param lua_State * L:
	* @Return int:
	*/
	int getDyanmicVar( lua_State *L );

	/*
	* Comments:设置定时执行的时间
	* Param unsigned int nExpiredTime:过期时间
	* @Return bool:成功返回true
	*/
	bool setTeamTimer( unsigned int nTeamId, unsigned int nExpiredTime );
};

//tolua_end