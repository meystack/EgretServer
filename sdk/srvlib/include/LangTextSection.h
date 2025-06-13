#pragma once

/************************************************************************

                          语言包配置节点类

           配置节点可以是一个字符串或者是文字包，这取决于getType的类型。

   此类的内存管理方式为使用CObjectAllocator<char>进行，因此意味着内存将不会被释放，
	除非构造时传递的内存管理器已经被释放。

************************************************************************/
class CLangTextSection;
class CLangTextPacket;

class CLangTextSection
{
public:
	enum ElementType
	{
		eInvalid,
		ePacket,	//类型是一个分类
		eText,		//类型是一个文字包
	};
	
public:
	CLangTextSection();
	CLangTextSection(CObjectAllocator<char> *);
	~CLangTextSection();

	//获取节点类型
	inline ElementType getType(){ return m_nType; }
	//获取字符串值
	inline LPCTSTR getText(){ return m_Data.m_pStr; }
	//获取名为sName的文字包节点
	CLangTextSection* getSection(LPCTSTR sName);
	//设置节点为字符串
	void setText(LPCTSTR str);
	//设置节点为文字包
	void setPacket(size_t len);
	//添加一个节点，此前必须调用setPacket设置当前节点类型为文字包
	CLangTextSection* addSection(LPCTSTR sName);

protected:
	//清空节点数据
	void clear();
	//内存申请函数，作用与c函数中的realloc实现相同，实现申请、扩展以及释放内存
	void* realloc(void *p, size_t s);

protected:
	ElementType m_nType;	//节点的类型
	union 
	{
		char*				m_pStr;
		CLangTextPacket*	m_pPack;
	}			m_Data;		//节点的数据
	CObjectAllocator<char>*	m_pAllocator;
};

class CLangTextPacket : 
	public CCustomHashTable<CLangTextSection>
{
public:
	typedef CCustomHashTable<CLangTextSection> Inherited;

public:
	CLangTextPacket(CObjectAllocator<char> *pAllocator, size_t len);
	~CLangTextPacket();

protected:
	//内存申请函数，作用与c函数中的realloc实现相同，实现申请、扩展以及释放内存
	void *realloc(void* p, size_t s);

private:
	CObjectAllocator<char>* m_pAllocator;
};
