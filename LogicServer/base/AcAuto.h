#pragma once


class AcAuto
{
	static const int kind = 256;
	struct node{
		node *fail;			///< 失败指针
		union{
			node *next[kind];	///< Tire每个节点的256个子节点（以char存储，最多256字符）,
			///< 构造fail指针的时候，作辅助作用时，next[0]保存树元素指针,next[1]保存下一个辅助节点
			struct{
				node* element;
				node* nextAuxNode;
			};
		};
		bool m_bUseAsAux;		///< 是否昨晚辅助节点
		//node* pre, *post;	///< 构造fail指针的时候，作辅助作用
		int count;			///< 是否为该单词的最后一个节点
		node(bool bUseAsAux = false);
		~node();
	};

	node* m_Root;
	bool m_bBuild;

public:
	AcAuto(void);
	~AcAuto(void);
	/// @param str 输入的字符串
	/// @param bOnlyCheckHasKeyWorld 只是简单检查是否有关键字，不需要统计共有多少个
	int Query(const char* str, bool bOnlyCheckHasKeyWorld = false);

	void InsertKeyWord(const char* keyWord);

	/// 设置 fail 指针
	void BuildAcAutomation();
};

