/*
	英雄相关的一些脚本的导出
*/

//tolua_begin
namespace Hero
{

	//添加英雄
	int addHero(void* pEntity, int nHeroId, int  nStage,  int nLevel);
	
	//判断一个玩家能否添加一个英雄
	bool  canAddHero(void *pEntity, bool bWithTipmsg =true);

	//英雄进阶
	bool  StageUp(void *pEntity, int  nHeroId, int  nStage,  int nBless);

	//获取英雄的属性
	unsigned int getHeroProperty(void *pEntity,int nHeroId, int nPropId);

	//设置英雄属性
	bool setHeroProperty(void *pEntity,int nHeroId, int nPropId,unsigned int nValue);

	//英雄学习技能
	bool learnSkill(void *pEntity,int nHeroID, int nSkillID);

	//英雄的技能升级
	bool skillLevelUp(void *pEntity,int nHeroID, int nSkillID );

	//获取英雄技能的等级
	int getSkillLevel(void* pEntity,int nHeroId, int nSkillId);

	//获取出战英雄ID
	int getBattleHeroId(void * pEntity);

	//英雄操作结果
	void sendHeroOpResult(void * pEntity,int nHeroId,int nOpId, bool result);

	//英雄出战
	void setHeroBattle(void* pEntity, int nHeroId);

	//获取英雄的名字
	char * getHeroName(void * pEntity ,int nHeroId);

	//获取英雄数量
	int getHeroCount(void * pEntity);

	//英雄添加经验
	void  addExp(void *pEntity, unsigned int nExp, int  nHeroId = 1);

	//指定英雄加经验
	void addExpByHeroId(void *pEntity, int  nHeroId, unsigned int nExp);

	//召回出战英雄
	void  CallBackHero(void *pEntity,bool boClient = true);
};

//tolua_end