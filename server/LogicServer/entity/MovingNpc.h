
#pragma once

/***************************************************************/
/*
/*                    移动的NPC，从NPC继承，具有移动功能
/*
/***************************************************************/

class CMovingNPC:
	public CNpc
{
public:
	typedef CNpc Inherited;

	//初始化
	bool Init(void * data, size_t size){if( Inherited::Init(data,size) ==false ) return false; return true;} 

	//删除的一些内存消耗操作
	inline void Destroy(){ Inherited::Destroy(); } //销毁一个实体

};
