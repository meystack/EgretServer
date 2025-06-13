#pragma  once 
/*
*基于多hash函判断bloomfilter 的屏蔽字管理器，经过性能测试和优化，性能还可以，内存稳定，执行时间稳定不随屏蔽字增长而增长
*能保证所有屏蔽字库里的字都能找出来
*但是有一定的错误率，错误率 取决hash函数的个数以及内存的大小
*hash函数个数越多，错误率越小，但是执行开销越大，内存越大，错误率会越小，所以需要在内存和执行效率之间权衡
* 测试库结构，单次的时间大约为0.04ms,在bloomfilter的基础上，做了局部的优化，比传统的bloomfilter测试效率提升15-20倍左右,使用了额外的2M内存，大概总占用内存4M
*/



#include "_osdef.h"
typedef struct 
{
	size_t asize;
	unsigned char *a;
	size_t nfuncs;
} BLOOM;


class CFilter 
{
public:
	typedef   unsigned int (CFilter::*hashfunc_t)( char *,unsigned int );

	
	CFilter()
	{
		m_pBloom =0;
	}

	~CFilter()
	{
		if(m_pBloom)
		{
			Destroy();
		}
	}

	

	
	/*
	* Comments:装载屏蔽字文件
	* Param char * sFileName:屏蔽字文件
	* Param int nSize:hash内存的长度，内存越大，碰撞的几率越小
	* Param int nHashFunCount:hash函数的个数，默认有13个hash函数，hash函数越多，效率越慢，错误率应该是越低,[2-13]有效
	* @Return int:加载的屏蔽字的个数,如果小于0表示出错,-1 nSize参数错误 -2内存不足 -3文件打开失败
	*/
	int Load(char *sFileName,int nSize = 2500000,int nHashFunCount=4);

	//初始化,使用Load装载可以不需要初始化
	bool Init(int nSize = 2500000,int nHashFunCount=3);

	
	/*
	* Comments:添加词汇到屏蔽字库
	* Param char * sStr:字
	* @Return bool:成功返回true，否则返回false
	*/
	bool AddFilterWord(char *sStr);

	/*
	* Comments:屏蔽字，最大只支持512个字节，超过则返回NULL
	* Param const char * pInput:输入字符串
	* Param int * pMatchLen:屏蔽字的长度
	* @Return char*:出现屏蔽字的首字母
	*/
	char* Filter( char *pInput,int *pMatchLen);

	// 补充：这个用以过滤间隔字词的屏蔽词处理
	char* Filter2( char *pInput,int *pMatchLen);
	
	//销毁内存
	void Destroy();

private:
	//创建
	BLOOM *bloom_create(size_t size, size_t nfuncs);

	//销毁 
	int bloom_destroy(BLOOM *bloom);

	//添加一个字符串
	int bloom_add(BLOOM *bloom,  char *s,unsigned int strLen);
	

	const static  int MAX_HASH_FUC_COUNT  =12; //最大的函数个数
	const static unsigned int MAX_BYTE_VALUE  =0X1FFFFF;
	const static unsigned int MAX_BIT_VALUE  =0XFFFFFF;
private:
	BLOOM *        m_pBloom;
	unsigned char  m_Flags[MAX_BYTE_VALUE];  //该字段用来表示单字是否出现过，如果出现过，这个bit设置为1，加快查找速度,这里占用 2M内存，将提升查找速度10倍

};
