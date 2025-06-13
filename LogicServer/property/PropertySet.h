/*
*属性集,玩家的属性的集合
*可以自己定义各种类型的数据类型,包括float,int,uint,long long等
*属性集的修改是4个字节为单位更新到客户端的
*属性可能是各种类型的数据
*
	class DATA:
		public CPropertySet
	{
	public:
		int a;
		int b;
		float c;
		unsigned d;
		long long e;
		int max_hp;
		struct VVV
		{
			float x;
			unsigned z;
		}sVVV;
	};
	DATA a;
	a.SetValue<int>(&a.a,int(23));
	a.SetValue<float>(&a.c,float(23.0));
	a.SetValue<long long >(&a.e,long long(2312121));
	a.SetValue<float>(&a.sVVV.x,float(0.3));
*/
#pragma once
#pragma pack(4)


class CUpdateMask
{
public:
	CUpdateMask()
	{
		ClearUpdateMask();		
	}
	
	/// 清空修改纪录,将不留纪录
	inline void ClearUpdateMask()
	{
		memset(m_updateMask,0,sizeof(m_updateMask) );
		m_nUsedCount =0;
	}

	inline CUpdateMask & operator &= (CUpdateMask &mask)
	{
		INT_PTR count = __max(m_nUsedCount,mask.GetUsedCount());
		if(count ==0) return  *this;
		
		//INT_PTR nIntCount = count >>3 ; //计算有多少个字节
		INT_PTR nMaxCount =0;
		for(INT_PTR i= 0; i< count; i++ )
		{
			m_updateMask[i] &= mask[i];
			if(m_updateMask[i] && i>= nMaxCount )
			{
				nMaxCount = i +1;
			}
		}
		
		m_nUsedCount = nMaxCount;
		return *this;
	}
	
	/*
	*设置第nPos个int是否修改过
	* nPos 第几个4字节
	* flag flag=true表示修改过,否则去掉这个修改标记
	*/
	inline void SetUpdateMaskFlag(INT_PTR nPos,bool flag=true)
	{
		if( IsBitForbidUpdate(nPos) ) return; //如果禁止更新的话
		UINT_PTR nBytePos= nPos % 8;
		UINT_PTR nByteId = nPos >> 3;

		if ((nByteId >=  MAX_MASK_BYTE_COUNT) || (nByteId <0)) return;
		unsigned char value= 1<< nBytePos;
		if(flag) //让其成立
		{
			m_updateMask[nByteId] |= value;
		}
		else
		{
			value = ~value; //取反
			m_updateMask[nByteId] &= value; //按位与
		}
		nByteId ++;
		if (m_nUsedCount<nByteId)
		{
			m_nUsedCount = nByteId;
		}
	}

	inline void SetUpdateMaskFlag(INT_PTR nPos,bool flag,char*pForbitMask)
	{
		if( IsBitForbidUpdate(pForbitMask,nPos) ) return; //如果禁止更新的话
		UINT_PTR nBytePos= nPos % 8;
		UINT_PTR nByteId = nPos >> 3;

		if ((nByteId >=  MAX_MASK_BYTE_COUNT) || (nByteId <0)) return;
		unsigned char value= 1<< nBytePos;
		if(flag) //让其成立
		{
			m_updateMask[nByteId] |= value;
		}
		else
		{
			value = ~value; //取反
			m_updateMask[nByteId] &= value; //按位与
		}
		nByteId ++;
		if (m_nUsedCount<nByteId)
		{
			m_nUsedCount = nByteId;
		}
	}
	
	//第nPos个int是否修改过
	inline bool GetUpdateMaskFlag(INT_PTR nPos)
	{
		INT_PTR nBytePos= nPos%8;
		INT_PTR nByteId = nPos >> 3;
		if ((nByteId >=  MAX_MASK_BYTE_COUNT) || (nByteId <0)) return false;
		unsigned char value= 1<< nBytePos;
		return m_updateMask[nByteId] & value ? true:false;
	}
	
	//修改过多少个int
	inline UINT_PTR GetUsedCount(){ return m_nUsedCount; }
	
	//获取修改buff的指针
	inline unsigned char * GetMaskPtr( ){ return m_updateMask;}
	
	//属性集是否修改过
	inline bool HasModified(  ){return m_nUsedCount>0;}
	
	inline operator unsigned char *  (){return m_updateMask;}
	//获取数据的指针,this加上mask的长+使用长度
	
	//设置禁止更新的属性的位
	static void SetForbidUpdate(INT_PTR nPos,bool flag)
	{
		if( s_forbitUpdateMask ==NULL) return ;
		INT_PTR nBytePos= nPos%8;
		INT_PTR nByteId = nPos >> 3;
		if ((nByteId >= MAX_MASK_BYTE_COUNT) || (nByteId <0)) return ;
		unsigned char value= 1<< nBytePos;
		if(flag) //让其成立
		{
			s_forbitUpdateMask[nByteId] |= value;
		}
		else
		{
			value = ~value; //取反
			s_forbitUpdateMask[nByteId] &= value; //按位与
		}
	}
	//某个位置是否禁止更新
	inline  bool IsBitForbidUpdate(INT_PTR nPos)
	{
		if( s_forbitUpdateMask ==NULL) return false;
		INT_PTR nBytePos= nPos%8;
		INT_PTR nByteId = nPos >> 3;
		if ((nByteId >= MAX_MASK_BYTE_COUNT) || (nByteId <0)) return false;
		unsigned char value= 1<< nBytePos;
		return s_forbitUpdateMask[nByteId] & value ? true:false;
	}
	
	//设置禁止更新的属性的位
	static void SetForbidUpdate(char*pForbitMask, INT_PTR nPos,bool flag)
	{
		if( pForbitMask ==NULL) return ;
		INT_PTR nBytePos= nPos%8;
		INT_PTR nByteId = nPos >> 3;
		if ((nByteId >= MAX_MASK_BYTE_COUNT) || (nByteId <0)) return ;
		unsigned char value= 1<< nBytePos;
		if(flag) //让其成立
		{
			pForbitMask[nByteId] |= value;
		}
		else
		{
			value = ~value; //取反
			pForbitMask[nByteId] &= value; //按位与
		}
	}
	//某个位置是否禁止更新
	inline  bool IsBitForbidUpdate(char*pForbitMask, INT_PTR nPos)
	{
		if( pForbitMask ==NULL) return false;
		INT_PTR nBytePos= nPos%8;
		INT_PTR nByteId = nPos >> 3;
		if ((nByteId >= MAX_MASK_BYTE_COUNT) || (nByteId <0)) return false;
		unsigned char value= 1<< nBytePos;
		return pForbitMask[nByteId] & value ? true:false;
	}

public:
	const static int MAX_MASK_BYTE_COUNT = 32; //暂时支持32*8个属性，为了在各种机器对齐都有效，这里要使用8的倍数
	static char s_forbitUpdateMask[MAX_MASK_BYTE_COUNT]; //禁止更新的那些mask位
private:
	
	unsigned char m_updateMask[MAX_MASK_BYTE_COUNT];  //这个是mask的存储地方
	
	UINT_PTR m_nUsedCount ; //已经使用的字节数

	//DECLARE_OBJECT_COUNTER(CUpdateMask)
};



//实体属性集合,可以是各种数据,带有一个mask纪录第几个int的值修改过
class CPropertySet:
	public CUpdateMask
{
public:
	CPropertySet(){} //
	/*
		设置属性的值,同时会在mask做修改标记
		*pos,是这个属性的指针
		*value是值
		*修改完了会在mask里纪录第几个int的值修改过
	*/
	
	template<typename T>
	void SetValue( T* pos,CONST T &value)
	{
		if( *(T *)(pos) == value) return; //没改变
		INT_PTR nByteIntervel = (INT_PTR)( (char *)pos- (char*)this - sizeof(CUpdateMask)); //偏移了多少字节
		if (nByteIntervel < 0) return ; //计算错误了
		*(T *)(pos) = value; //设置值
		                  
		if( nByteIntervel >0)
		{
			nByteIntervel = nByteIntervel >>2; //多少个字节
		}
		SIZE_T size = sizeof(T);
		size = __max(size >>2,1); //多少个4字节
		for (UINT_PTR i=0; i< size ; i++)
		{
			SetUpdateMaskFlag(nByteIntervel +i); //
		}
	}

	template<typename T>
	inline void SetValue( int nPropertyID,CONST T &value)
	{
		T * pos = (T*)GetValuePtr(nPropertyID);
		SetValue(pos,value);
	}
	
	template<typename T>
	void SetValue( T* pos,CONST T &value, char*pForbitMask)
	{
		if( *(T *)(pos) == value) return; //没改变
		INT_PTR nByteIntervel = (INT_PTR)( (char *)pos- (char*)this - sizeof(CUpdateMask)); //偏移了多少字节
		if (nByteIntervel < 0) return ; //计算错误了
		*(T *)(pos) = value; //设置值
		                  
		if( nByteIntervel >0)
		{
			nByteIntervel = nByteIntervel >>2; //多少个字节
		}
		SIZE_T size = sizeof(T);
		size = __max(size >>2,1); //多少个4字节
		for (UINT_PTR i=0; i< size ; i++)
		{
			SetUpdateMaskFlag(nByteIntervel +i,true,pForbitMask); //
		}
	}
	template<typename T>
	inline void SetValue( int nPropertyID,CONST T &value, char*pForbitMask)
	{
		T * pos = (T*)GetValuePtr(nPropertyID);
		SetValue(pos,value,pForbitMask);
	}
	
	
	//获取属性集第nPos个int的指针
	inline const char *GetValuePtr(INT_PTR nPos) const
	{
		char *ptr= (char*)this + sizeof(CUpdateMask) + ( nPos << 2); 	
		return ptr;
	}
	
	inline INT_PTR GetDataIndex(char* pos)
	{
		INT_PTR nByteIntervel = (INT_PTR)( (char *)pos- (char*)this - sizeof(CUpdateMask)); //偏移了多少字节
		return nByteIntervel >>2 ; //在第几个int位置上
	}
};

//玩家的属性集,包括 玩家的db数据,1级属性数据
class CActorProperty:
	public CPropertySet,
	public ENTITYDATA,
	public CREATUREDATA,
	public ACTORHEADPARTDATA,
	public ACTORDBPARTDATA,
	public ACTORTAILDATA	// 角色尾部数据
{
public:
	CActorProperty()
	{
		memset(this, 0, sizeof(CActorProperty));
	}
};

class CActorOfflineProperty:
	public ENTITYDATA,
	public CREATUREDATA,
	public ACTORHEADPARTDATA,
	public ACTORDBPARTDATA,
	public ACTORTAILDATA
{
public:
	CActorOfflineProperty()
	{
		memset(this,0,sizeof(CActorOfflineProperty));
	}
};

static_assert( (sizeof(CActorProperty) - sizeof(CPropertySet))/sizeof(int) == PROP_MAX_ACTOR,
"Actor属性集定义属性长度 与 CActorProperty 枚举长度不一致！");

// 定义玩家自定义数据索引位置，方便离线查找修改
#define ACTORRULE_SPACE 4
#define CAL_ARS(num)  (num*ACTORRULE_SPACE)
enum
{
	ACTORRULE_PROP_ENTITY_ID 											= CAL_ARS(0),			//实体的id
	ACTORRULE_PROP_ENTITY_POSX 											= CAL_ARS(1),		//位置 posx
	ACTORRULE_PROP_ENTITY_POSY 											= CAL_ARS(2),		//位置pos y
	ACTORRULE_PROP_ENTITY_MODELID 										= CAL_ARS(3),		//实体的模型ID
	ACTORRULE_PROP_ENTITY_ICON 											= CAL_ARS(4), 		//头像ID
	ACTORRULE_PROP_ENTITY_DIR 											= CAL_ARS(5), 		//实体的朝向

	
	ACTORRULE_PROP_CREATURE_LEVEL 										= CAL_ARS(6), 				//等级
	ACTORRULE_PROP_CREATURE_HP 											= CAL_ARS(7), 					//
	ACTORRULE_PROP_CREATURE_MP 											= CAL_ARS(8), 					//
	ACTORRULE_PROP_CREATURE_STATE 										= CAL_ARS(9), 				//
	ACTORRULE_PROP_CREATURE_COLOR 										= CAL_ARS(10), 				//
	ACTORRULE_PROP_CREATURE_MAXHP 										= CAL_ARS(11), 				//
	ACTORRULE_PROP_CREATURE_MAXMP 										= CAL_ARS(12), 				//
	ACTORRULE_PROP_CREATURE_PHYSICAL_ATTACK_MIN 						= CAL_ARS(13), 	//
	ACTORRULE_PROP_CREATURE_PHYSICAL_ATTACK_MAX 						= CAL_ARS(14), 	//
	ACTORRULE_PROP_CREATURE_MAGIC_ATTACK_MIN 							= CAL_ARS(15), 	//最小魔法攻击
	ACTORRULE_PROP_CREATURE_MAGIC_ATTACK_MAX 							= CAL_ARS(16), 	//最大魔法攻击   
	ACTORRULE_PROP_CREATURE_WIZARD_ATTACK_MIN 							= CAL_ARS(17), 	//最小道术攻击  
	ACTORRULE_PROP_CREATURE_WIZARD_ATTACK_MAX 							= CAL_ARS(18), 	//最大道术攻击  
	ACTORRULE_PROP_CREATURE_PYSICAL_DEFENCE_MIN 						= CAL_ARS(19), 	//最小物理防御  
	ACTORRULE_PROP_CREATURE_PYSICAL_DEFENCE_MAX 						= CAL_ARS(20), 	//最大物理防御  
	ACTORRULE_PROP_CREATURE_MAGIC_DEFENCE_MIN 							= CAL_ARS(21), 	//最小魔法防御  
	ACTORRULE_PROP_CREATURE_MAGIC_DEFENCE_MAX 							= CAL_ARS(22), 	//最大魔法防御    
	ACTORRULE_PROP_CREATURE_HITVALUE 									= CAL_ARS(23), 	//物理命中    
	ACTORRULE_PROP_CREATURE_DODVALUE 									= CAL_ARS(24), 	//物理闪避    
	ACTORRULE_PROP_CREATURE_MAGIC_HITRATE 								= CAL_ARS(25), 	//魔法命中    
	ACTORRULE_PROP_CREATURE_MAGIC_DOGERATE 								= CAL_ARS(26), 	//魔法闪避  
	ACTORRULE_PROP_CREATURE_HP_RATE_RENEW 								= CAL_ARS(27), 	//HP万分比恢复  
	ACTORRULE_PROP_CREATURE_MP_RATE_RENEW 								= CAL_ARS(28), 	//MP万分比恢复   
	ACTORRULE_PROP_CREATURE_MOVEONESLOTTIME 							= CAL_ARS(29), 	//移动1格需要的时间，单位ms   
	ACTORRULE_PROP_CREATURE_ATTACK_SPEED 								= CAL_ARS(30), 	//攻击速度   
	ACTORRULE_PROP_CREATURE_LUCK 										= CAL_ARS(31), 			//幸运   
	ACTORRULE_PROP_CREATURE_HP_RENEW 									= CAL_ARS(32), 		//HP值恢复   
	ACTORRULE_PROP_CREATURE_MP_RENEW 									= CAL_ARS(33), 		//MP值恢复   
	ACTORRULE_PROP_CREATURE_DIZZY_STATUS 								= CAL_ARS(34),		//麻痹    
	//预留1000个给老的数据
	ACTORRULE_KICK_USER = 1000,  			//是否踢出玩家byte
	ACTORRULE_KICK_USER_FREETIME = 1001,	//踢出的玩家可以进入时间
	ACTORRULE_CUSTOMTITLE_ID = 1005,		// (int)后台 添加/删除 离线玩家自定义称号(添加自定义称号：(+)自定义称号Id；删除自定义称号：(-)自定义称号Id）	
};

#define MASK_BACKSTAGE_FORVID 1 			//查询标记后台封禁
#define MASK_BACKSTAGE_UNFORVID 2			//查询标记后台解开封禁

class CActorCustomProperty 
{
public: 
	//数据设置
	BYTE cbCustomProperty[2048];					//自定规则
	CActorCustomProperty()
	{
		memset(this, 0, sizeof(CActorCustomProperty));
	}
};

#define COPY_CUSTOM_PROPERTY(a, b)\
	{unsigned int * pDataUint = (unsigned int *)&m_ProCustom.cbCustomProperty[a]; \
	*pDataUint = GetProperty<unsigned int>(b); \
	}
#define COPY_CUSTOM_PROPERTY_INT(a, b)\
	{ int * pDataUint = ( int *)&m_ProCustom.cbCustomProperty[a]; \
	*pDataUint = GetProperty<int>(b); \
	}
	
#define ADD_CUSTOM_PROPERTY_BYTE(index, a)\
	{ BYTE * pDataUint = ( BYTE *)&m_ProCustom.cbCustomProperty[index]; \
	*pDataUint = a; \
	}

#define ADD_CUSTOM_PROPERTY_INT(index, a)\
	{ int * pDataUint = ( int *)&m_ProCustom.cbCustomProperty[index]; \
	*pDataUint = a; \
	}
#define ADD_CUSTOM_PROPERTY(index, a)\
	{ unsigned int * pDataUint = ( unsigned int *)&m_ProCustom.cbCustomProperty[index]; \
	*pDataUint = a; \
	}
 

#define GET_CUSTOM_PROPERTY_INT(a, b)\
	{ int * pDataUint = ( int *)&m_ProCustom.cbCustomProperty[a]; \
	*pDataUint = GetProperty<int>(b); \
	} 

	
//NPC的属性集合
class CNPCProperty:
	public CPropertySet,
	public ENTITYDATA
{
public:
	CNPCProperty()
	{
		memset(this,0,sizeof(CNPCProperty));
	}
};

static_assert( (sizeof(CNPCProperty) - sizeof(CPropertySet))/sizeof(int) == PROP_MAX_ENTITY,
"NPC属性集定义属性长度 与 CNPCProperty 枚举长度不一致！");

//怪物的属性集合
class CMonsterProperty:
	public CPropertySet,
	public ENTITYDATA,
	public CREATUREDATA,
	public MONSTERDATA
{
public:
	CMonsterProperty()
	{
		memset(this,0,sizeof(CMonsterProperty));
	}
};

static_assert( (sizeof(CMonsterProperty) - sizeof(CPropertySet))/sizeof(int) == PROP_MAX_MONSTER,
"Monster属性集定义属性长度 与 CMonsterProperty 枚举长度不一致！");

//掉落物品的属性集合
class CDropItemProperty:
	public CPropertySet,
	public ENTITYDATA
{
public:
	CDropItemProperty()
	{
		memset(this,0,sizeof(CDropItemProperty));
	}
};

static_assert( (sizeof(CDropItemProperty) - sizeof(CPropertySet))/sizeof(int) == PROP_MAX_ENTITY,
"DropItem属性集定义属性长度 与 CDropItemProperty 枚举长度不一致！");

//传送门的属性集合
class CTransferProperty:
	public CPropertySet,
	public ENTITYDATA
{
public:
	CTransferProperty()
	{
		memset(this,0,sizeof(CTransferProperty));
	}
};

static_assert( (sizeof(CTransferProperty) - sizeof(CPropertySet))/sizeof(int) == PROP_MAX_ENTITY,
"Transfer属性集定义属性长度 与 CTransferProperty 枚举长度不一致！");

//宠物的属性计算器
class CPetProperty:
	public CPropertySet,
	public ENTITYDATA,
	public CREATUREDATA
{
public:
	CPetProperty()
	{
		memset(this,0,sizeof(CPetProperty));
	}
};

static_assert( (sizeof(CPetProperty) - sizeof(CPropertySet))/sizeof(int) == PROP_MAX_CREATURE,
"Pet属性集定义属性长度 与 CPetProperty 枚举长度不一致！");

//火的属性集合
class CFireProperty:
	public CPropertySet,
	public ENTITYDATA
{
public:
	CFireProperty()
	{
		memset(this,0,sizeof(CFireProperty));
	}
};

static_assert( (sizeof(CFireProperty) - sizeof(CPropertySet))/sizeof(int) == PROP_MAX_ENTITY,
"Fire属性集定义属性长度 与 CFireProperty 枚举长度不一致！");

// 这个有问题，注意（待添加英雄需求时再处理）
class CHeroProperty:
	public CPropertySet,
	public ENTITYDATA,
	public CREATUREDATA,
	public ACTORHEADPARTDATA
{
public:
	CHeroProperty()
	{
		memset(this,0,sizeof(CHeroProperty));
	}
};

class CWalkRideProperty:
	public CPropertySet,
	public ENTITYDATA,
	public CREATUREDATA
{
public:
	CWalkRideProperty()
	{
		memset(this, 0, sizeof(CWalkRideProperty));
	}
};

#pragma  pack()
