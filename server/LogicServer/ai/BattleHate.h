#pragma once
/*
* 怪物的战斗仇恨管理器
* 这里记录怪物的仇恨，从最大到最小排列
* 最下面的总是仇恨最大的,上面的是仇恨最小的
* 添加仇恨的原则是
*/

#define MAX_HATE_ENTITY_COUNT  5 //怪物的仇恨的列表
struct HateElement
{
	EntityHandle handle; //实体的类型
	unsigned int value;  //
};

class CBattleHate
{
public:
		
};