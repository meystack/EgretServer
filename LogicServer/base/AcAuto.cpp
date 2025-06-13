#include "StdAfx.h"

AcAuto::node::node(bool bUseAsAux)
	: fail(nullptr)
	, count(0)
	, m_bUseAsAux(bUseAsAux)
{ //构造函数初始化
	memset(next, 0, sizeof(next));
}

AcAuto::node::~node(){
	if (! m_bUseAsAux){
		for (int i = 0; i < kind; ++i){
			node*& p = next[i];
			if (p){
				delete p;
				p = nullptr;
			}
		}
	}else{
		element = nullptr;
		nextAuxNode = nullptr;
	}
}

AcAuto::AcAuto(void)
	: m_Root(new node)
	, m_bBuild(false)
{
}


AcAuto::~AcAuto(void)
{
	if (m_Root){
		delete m_Root;
		m_Root = nullptr;
	}
}

/// @param str 输入的字符串
/// @param bOnlyCheckHasKeyWorld 只是简单检查是否有关键字，不需要统计共有多少个
int AcAuto::Query(const char* str, bool bOnlyCheckHasKeyWorld ){
	if (! m_bBuild){
		return 0;
	}
	unsigned int i(0), cnt(0), len(strlen(str));
	unsigned char index ;
	node *p(m_Root);
	while(index = str[i++]){ // 取一个字节
		while(p->next[index] == nullptr && p != m_Root) p = p->fail;
		p = p->next[index];
		p= (p==nullptr) ? m_Root : p;
		node *temp(p);
		while(temp != m_Root && temp->count != -1){
			cnt += temp->count;
			temp->count = -1;
			temp = temp->fail;
			if(bOnlyCheckHasKeyWorld && cnt)return cnt;
		}
	}
	return cnt;
}

void AcAuto::InsertKeyWord(const char* keyWord){
	node *p(m_Root);
	unsigned int i(0);
	unsigned char index;
	while(index = keyWord[i]){ // 构造关键字树
		node*& next = p->next[index];
		if( ! next) next = new node();
		p = next;
		++i;
	}
	++p->count;
}

/// 设置 fail 指针
void AcAuto::BuildAcAutomation(){
	m_Root->fail = nullptr;
	node* auxList = new node(true);
	auxList->element = m_Root;// 插入首个
	node* temp;
	node* firstNode(auxList), *lastNode(auxList);
	while(firstNode && (temp = firstNode->element)){ // 取一个
		node* p(nullptr);
		for(int i = 0; i < kind; ++i){
			node* tnext(temp->next[i]);

			if(tnext){
				if(temp == m_Root) tnext->fail = m_Root;
				else{
					p = temp->fail;
					while(p){
						if(p->next[i]){
							tnext->fail = p->next[i];
							break;
						}
						p = p->fail;
					}
					if( ! p) tnext->fail = m_Root;
				}
				// 插入个新的
				node* auxNode(new node(true));					
				auxNode->element = tnext;
				lastNode->nextAuxNode = auxNode;
				lastNode = auxNode;
			}
		}
		node* cur(firstNode);
		firstNode = firstNode->nextAuxNode;
		delete cur;
	}
	m_bBuild = true;
}
