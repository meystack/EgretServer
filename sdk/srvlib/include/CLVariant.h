#pragma once

/************************************************************************

                 C/C++程序中为脚本提供的变量类

  变量支持double、char*、以及带有名称的成员值
************************************************************************/

#include "_osdef.h"
#include "ObjectCounter.h"
class CCLVariant : public Counter<CCLVariant>
{
public:
	/** 定义变量类型 **/
	typedef enum tagVarType
	{
		vNil = 0, //无值
		vNumber = 1,//数字值
		vStr = 2,//字符串值
		vStruct = 3,//结构值
	}VARTYPE;
public:
	
	inline  operator __int64 () const
	{
		if (m_btType == vNumber)
			return (__int64)m_Data.n;
		if (m_btType == vStr)
		{
			char *e;
			double d = strtod(m_Data.s.str, &e);
			if (!e || !*e)
				return (__int64)d;
		}
		return 0;
	}
	inline operator double () const
	{
		if (m_btType == vNumber)
			return m_Data.n;
		if (m_btType == vStr)
		{
			char *e;
			double d = strtod(m_Data.s.str, &e);
			if (!e || !*e)
				return d;
		}
		return 0;
	}
	inline operator const char* () const
	{
		if (m_btType == vStr)
		{
			return m_Data.s.str;
		}
		return NULL;
	}
	inline operator const CCLVariant** () const
	{
		if (m_btType == vStruct)
			return (const CCLVariant**)m_Data.a.list;
		return NULL;
	}
public:
	/** 比较判断运算符函数集 **/
	inline bool operator == (const CCLVariant &var)
	{
		char *e;
		double d;

		switch(m_btType)
		{
		case vNumber:
			if (var.m_btType == vNumber)
			{
				return m_Data.n == var.m_Data.n;
			}
			else if (var.m_btType == vStr)
			{
				d = strtod(var.m_Data.s.str, &e);
				if (!e || !*e)
					return m_Data.n == d;
			}
			break;
		case vStr:
			if (var.m_btType == vNumber)
			{
				 d = strtod(m_Data.s.str, &e);
				if (!e || !*e)
					return d == var.m_Data.n;
			}
			else if (var.m_btType == vStr)
				return (m_Data.s.len == var.m_Data.s.len) && !strcmp(m_Data.s.str, var.m_Data.s.str);
			break;
		}
		return false;
	}
	inline bool operator < (const CCLVariant &var)
	{
		char *e;
		double d;

		switch(m_btType)
		{
		case vNumber:
			if (var.m_btType == vNumber)
			{
				return m_Data.n < var.m_Data.n;
			}
			else if (var.m_btType == vStr)//此时不要进行将数字转换为字符而进行strcmp比较，因为进行>运算的时候，lua会将第二个参数传递为第一个参数
			{
				d = strtod(var.m_Data.s.str, &e);
				if (!e || !*e)
					return m_Data.n < d;
			}
			break;
		case vStr:
			if (var.m_btType == vNumber)
			{
				d = strtod(m_Data.s.str, &e);
				if (!e || !*e)
					return d < var.m_Data.n;
			}
			else if (var.m_btType == vStr)
				return (m_Data.s.len < var.m_Data.s.len) || strcmp(m_Data.s.str, var.m_Data.s.str) < 0;
			break;
		}
		return false;
	}
	inline bool operator > (const CCLVariant &var)
	{
		return !operator <= (var);
	}
	inline bool operator <= (const CCLVariant &var)
	{
		char *e;
		double d;

		switch(m_btType)
		{
		case vNumber:
			if (var.m_btType == vNumber)
			{
				return m_Data.n <= var.m_Data.n;
			}
			else if (var.m_btType == vStr)//此时不要进行将数字转换为字符而进行strcmp比较，因为进行>=运算的时候，lua会将第二个参数传递为第一个参数
			{
				d = strtod(var.m_Data.s.str, &e);
				if (!e || !*e)
					return m_Data.n <= d;
			}
			break;
		case vStr:
			if (var.m_btType == vNumber)
			{
				d = strtod(m_Data.s.str, &e);
				if (!e || !*e)
					return d <= var.m_Data.n;
			}
			else if (var.m_btType == vStr)
				return (m_Data.s.len </*这里不能用<=*/ var.m_Data.s.len) || (m_Data.s.len == var.m_Data.s.len && strcmp(m_Data.s.str, var.m_Data.s.str) <= 0);
			break;
		}
		return false;
	}
	inline bool operator >= (const CCLVariant &var)
	{
		return !operator < (var);
	}
public:
	/** 复制运算符函数集 **/
	inline CCLVariant& operator = (double val)
	{
		if (m_btType != vNumber && m_btType != vNil)
			clear();
		m_btType = vNumber;
		m_Data.n = val;
		return *this;
	}
	inline CCLVariant& operator = (const char* val)
	{
		if (m_btType == vStr && m_Data.s.str && !strcmp(m_Data.s.str, val))
			return *this;
		clear();
		m_btType = vStr;
		m_Data.s.len = (unsigned int)strlen(val);
		m_Data.s.str = (char*)malloc(m_Data.s.len + 1);
		memcpy(m_Data.s.str, val, m_Data.s.len + 1);
		return *this;
	}
	inline CCLVariant& operator = (const CCLVariant& val)
	{
		if (&val == this)
			return *this;

		if (val.m_btType == vNumber)
		{
			operator = (val.m_Data.n);
		}
		else if (val.m_btType == vStr)
		{
			if (m_btType == vStr && m_Data.s.len == val.m_Data.s.len && m_Data.s.str 
				&& val.m_Data.s.str && !strcmp(m_Data.s.str, val.m_Data.s.str))
			{
				return *this;
			}
			clear();
			m_btType = vStr;
			m_Data.s.str = (char*)malloc(val.m_Data.s.len + 1);
			memcpy(m_Data.s.str, val.m_Data.s.str, val.m_Data.s.len + 1);
		}
		else if (val.m_btType == vStruct)
		{
			clear();
			m_btType = vStruct;
			m_Data.a.list = (CCLVariant**)calloc(val.m_Data.a.len, sizeof(*m_Data.a.list));
			for (INT_PTR i=0; i<(INT_PTR)val.m_Data.a.len; ++i)
			{
				m_Data.a.list[i] = new CCLVariant(*val.m_Data.a.list[i]);
			}
		}
		return *this;
	}
	
public:
	inline CCLVariant* ipair(int idx)
	{
		if (m_btType != vStruct)
			return NULL;
		CCLVariant *pVar;
		if(idx <= m_Data.a.len && idx >= 0)
		{
			pVar = m_Data.a.list[idx];
			return pVar;
		}
		
		return NULL;
	}
	/** 取成员值以及设置成员值的函数集 **/
	//通过名称获取成员值
	inline CCLVariant* get(const char* sName)
	{
		if (m_btType != vStruct || !sName)
			return NULL;
		DWORD dwName = hashstr(sName);
		return get(dwName);
	}
	//通过数字名称或索引获取成员值
	inline CCLVariant* get(DWORD dwName)
	{
		if (m_btType != vStruct)
			return NULL;
		CCLVariant *pVar;
		if(m_Data.a.len )
		{
			for (INT_PTR i=m_Data.a.len-1; i>-1; --i)
			{
				pVar = m_Data.a.list[i];
				if (pVar->m_dwName == dwName)
				{
					return pVar;
				}
			}
		}
		
		return NULL;
	}
	//创建成员值，如果成员已经存在则什么也不做，否则创建按一个值为vNil的成员
	inline CCLVariant& set(const char* sName)
	{
		if (m_btType != vStruct)
			clear();
		m_btType = vStruct;
		DWORD dwName = hashstr(sName);
		CCLVariant *pVar = get(dwName);
		if (!pVar)
		{
			pVar = new CCLVariant();
			pVar->m_dwName = dwName;

			m_Data.a.list = (CCLVariant**)realloc(m_Data.a.list, sizeof(*m_Data.a.list) * (m_Data.a.len+1));
			m_Data.a.list[m_Data.a.len] = pVar;
			m_Data.a.len++;
		}
		return *pVar;
	}
	//设置一个成员的double值，如果成员值不存在则会自动创建，如果成员值存在则会改变成员值的类型
	inline const CCLVariant& set(const char* sName, double val)
	{
		CCLVariant &Var = set(sName);
		Var.operator = (val);
		return Var;
	}
	//设置一个成员的字符串值，如果成员值不存在则会自动创建，如果成员值存在则会改变成员值的类型
	inline const CCLVariant& set(const char* sName, const char* val)
	{
		CCLVariant &Var = set(sName);
		Var.operator = (val);
		return Var;
	}
	//设置为空数据结构体
	inline const void setToEmptyStruct()
	{
		clear();
		m_btType = vStruct;
	}
public:
	//字符串hash函数
	inline static unsigned int hashstr(const char* str)
	{
		return hashlstr(str, strlen(str));
	}
	//字符串hash函数，需要提供长度
	inline static unsigned int hashlstr(const char* str, size_t len)
	{
		unsigned int h = (unsigned int)len;
		size_t step = (len>>5)+1;  /* if string is too long, don't hash all its chars */
		size_t l1;
		for (l1=len; l1>=step; l1-=step)  /* compute hash */
			h = h ^ ((h<<5)+(h>>2)+(unsigned char)str[l1-1]);
		return h;
	}
public:
	//获取变量的类型
	inline int type()
	{
		return m_btType;
	}
	/* 获取变量的长度
	*  当变量的值类型为数字的时候，返回值为1；
	*  当变量的值类型为字符串的时候，返回值为字符串长度；
	*  当变量的值类型为结构化数据的时候，返回一级子成员的数量；
	*  返回0表示变量没有值
	*/
	inline size_t len()
	{
		if (m_btType == vStr)
			return m_Data.s.len;
		if (m_btType == vStruct)
			return m_Data.a.len;
		if (m_btType == vNumber)
			return 1;
		return 0;
	}
	//清空变量的值
	inline void clear()
	{
		if (m_btType == vStr)
		{
			free(m_Data.s.str);
			m_Data.s.str = NULL;
			m_Data.s.len = 0;
		}
		else if (m_btType == vStruct)
		{
			if(m_Data.a.len)
			{
				for (INT_PTR i=m_Data.a.len-1; i>-1; --i)
				{
					delete m_Data.a.list[i];
				}
			}
			free(m_Data.a.list);
			m_Data.a.list = NULL;
			m_Data.a.len = 0;
		}
		else if (m_btType == vNumber)
		{
			m_Data.n = 0;
		}
		m_btType = vNil;
	}
	//获取在vStr类型中排除vNil值的成员数量
	inline size_t packedlen()
	{
		if (vStruct != m_btType || m_Data.a.len ==0 )
		{
			return 0;
		}
		size_t result = 0;
		for (INT_PTR i=m_Data.a.len-1; i>-1; --i)
		{
			if (m_Data.a.list[i]->m_btType != vNil)
				result++;
		}
		return result;
	}
	/*
	* Comments: 从数据流中加载变量数据
	* Param const char * ptr: 数据流指针
	* Param size_t size: 数据流长度
	* Param size_t *pCount: 用于保存加载了多少个变量对象
	* @Return size_t: 函数返回加载了变量后的内存的新位置
	*/
	inline const char* loadFromMemory(const char* ptr, size_t size, size_t *pCount = NULL)
	{
		//内存长度小于5字节，不能读取
		if (size < 5)
		{
			if (pCount) *pCount = 0;
			return ptr;
		}
		clear();
		//读取类型
		m_btType = *(unsigned char*)ptr;
		ptr += sizeof(unsigned char);
		size -= sizeof(unsigned char);
		//读取名称
		m_dwName = *(unsigned int *)ptr;
		ptr += sizeof(unsigned int);
		size -= sizeof(unsigned int);
		//依据类型读取数据
		switch(m_btType)
		{
		case vNumber:
			if (size >= sizeof(m_Data.n))
			{
				m_Data.n = *(double*)ptr;
				ptr += sizeof(double);
				size -= sizeof(double);
			}
			else m_btType = vNil;
			break;
		case vStr:
			if (size >= sizeof(unsigned int))
			{
				unsigned int len = *(unsigned int*)ptr;
				ptr += sizeof(unsigned int);
				size -= sizeof(unsigned int);
				if (len > 0 && size >= len + 1)
				{
					operator = ((const char*)ptr);
					ptr += len + 1;
					size -= len + 1;
				}
			}
			else m_btType = vNil;
			break;
		case vStruct:
			if (size >= sizeof(unsigned int))
			{
				unsigned int len = *(unsigned int*)ptr;
				ptr += sizeof(unsigned int);
				size -= sizeof(unsigned int);
				if (len > 0 && size >= 5)
				{
					m_Data.a.list = (CCLVariant**)calloc(len, sizeof(*m_Data.a.list));
					for (INT_PTR i=0; i<(INT_PTR)len; ++i)
					{
						size_t n = 0;
						m_Data.a.list[i] = new CCLVariant();
						ptr = m_Data.a.list[i]->loadFromMemory(ptr, size, &n);
						if (n <= 0)
							break;
						m_Data.a.len++;
						if (pCount) *pCount += n;
					}
				}
			}
			else m_btType = vNil;
			break;
		}

		//增加读取的变量数据量
		if (pCount && m_btType != vNil)
			*pCount += 1;//不能用*pCount++，意义不一样的，*pCount++表示 *(pCount++)。

		return ptr;
	}

	/*
	* Comments:获取这个对象存储的时候需要多少内存
	* @Return size_t:返回需要的内存的数目
	*/
	inline size_t getStoreMemory()
	{
		size_t reqSize = 0;
		//计算存储所需的空间
		switch(m_btType)
		{
		case vNil:
			reqSize  =0; //nil值没有值，但是数据的长度里还是包含了类型以及名字
			break;
		case vNumber: 
			reqSize = 1 + sizeof(m_dwName) + sizeof(m_Data.n);
			break;
		case vStr:
			reqSize = 1 + sizeof(m_dwName) + sizeof(m_Data.s.len) + ((m_Data.s.len > 0) ? m_Data.s.len + 1  : 0);
			break;
		case vStruct:
			reqSize = 1 + sizeof(m_dwName) + sizeof(m_Data.a.len);
			if (m_Data.a.len >0)//仅当参数size为0时用于计算保存变量所需的内存字节长度
			{
				for (INT_PTR i=m_Data.a.len-1;i>-1; --i)
				{
					reqSize += m_Data.a.list[i]->getStoreMemory();
				}
			}
			break;
		}
		return reqSize;
	}

	/*
	* Comments: 将变量保存到内存中
	* Param char * ptr: 内存流指针
	* Param size_t size: 内存缓存长度，如果值为0，则函数不会写入变量数据，且返回写入变量所需的内存字节长度
	@ Param CCLVariant **pNextSaveVar: 由于内存长度不够而终止写入的变量位置，如果此值输出非空，
                                       则调用者必须增长内存并再次调用输出值的saveToStream函数来保存数据，
									   否则之间保存的数据将导致读取时出现丢失数据
	* @Return size_t: 函数返回向内存中写入了多少字节的数据
	*
	* & 示例代码 &
	*
	* char *buff = NULL;
	* size_t bufsize = 0;
	* size_t totalsize = 0;
	* CCLVariant *pContinueVar = &var;
	* do
	* {
	*   bufsize += 1024;
	*   buff = realloc(buff, buffsize);
	*   totalsize += pContinueVar->saveToMemory(&buff[totalsize], bufsize - totalsize, &pContinueVar);
	* }
	* while (pContinueVar);
	*
	*/
	inline size_t saveToMemory(char *ptr, size_t size, CCLVariant **pNextSaveVar)
	{
		size_t reqSize = getStoreMemory(); //获取存储需要的内存的量

		size_t saveSize =0; //这个变量写进去多少的内存

		//计算存储所需的空间
		//没有任何数据可以写入
		if (reqSize <= 0)
		{
			*pNextSaveVar = NULL;
			return 0;
		}
		else if (size > 0)
		{
			//仅在剩余内存空间满足所需空间长度的时候才书写内容
			if (size >= reqSize)
			{
				if (pNextSaveVar) *pNextSaveVar = NULL;

				if(m_btType == vNil) //如果是空的话
				{
					return saveSize;
				}
				//写入类型
				*(unsigned char*)ptr = (unsigned char)m_btType;
				ptr += sizeof(unsigned char);
				size -= sizeof(unsigned char);
				saveSize += sizeof(unsigned char);

				//写入名称
				*(unsigned int*)ptr = m_dwName;
				ptr += sizeof(unsigned int);
				size -= sizeof(unsigned int);
				saveSize += sizeof(unsigned int);
				
				//写入数据
				switch(m_btType)
				{
				case vNumber: 
					//写入值
					*(double*)ptr = m_Data.n;
					ptr += sizeof(double);
					size -= sizeof(double);
					saveSize += sizeof(double);
					break;
				case vStr:
					//写入字符串长度
					*(unsigned int*)ptr = m_Data.s.len;
					ptr += sizeof(unsigned int);
					size -= sizeof(unsigned int);
					saveSize += sizeof(unsigned int);

					//写入字符串内容
					if (m_Data.s.len > 0)
					{
						memcpy(ptr, m_Data.s.str, m_Data.s.len + 1);
						ptr += m_Data.s.len + 1;
						size -= m_Data.s.len + 1;
						saveSize += m_Data.s.len + 1;

					}
					
					break;
				case vStruct:
					//写入成员数量
					{
						
						*(unsigned int*)ptr =(unsigned int)packedlen() ;
						ptr += sizeof(unsigned int);
						size -= sizeof(unsigned int);
						saveSize += sizeof(unsigned int);
						//循环写入成员内容
						if(m_Data.a.len >0)
						{
							for (INT_PTR i=m_Data.a.len-1;i>-1; --i)
							{
								size_t nb = m_Data.a.list[i]->saveToMemory(ptr, size, pNextSaveVar);
								//没有写入数据则终止，不能判断写入长度是否大于0，因为nil值不会写入
								if(nb >0)
								{
									saveSize += nb;
									ptr += nb;
									size -= nb;
								}
								if (pNextSaveVar && *pNextSaveVar)
									break;
								
							}
						}
					
					}
					break;
				}
			}
			else
			{
				if (pNextSaveVar) *pNextSaveVar = this;
				saveSize = 0;//没有写入任何数据
			}
		}
		return saveSize;
	}
private:
	inline void zd()
	{
		m_btType = vNil;
		m_dwName = 0;
		m_Data.s.len = 0;
		m_Data.n = 0;
	}

protected:
	unsigned int m_btType; //数据类型
	unsigned int m_dwName; //哈希名称
	union
	{
		//整数值
		double n;
		//字符串值
		struct
		{
			char *str;
			unsigned int len;
		}s;
		//数组值
		struct  
		{
			CCLVariant** list;
			unsigned int len;
		}a;
	} m_Data;
		
public:
	CCLVariant()
	{
		zd();
	}
	CCLVariant(double val)
	{
		zd();
		m_btType = vNumber;
		m_Data.n = val;
	}
	CCLVariant(const char* val)
	{
		zd();
		this->operator = (val);
	}
	CCLVariant(const CCLVariant& val)
	{
		zd();
		this->operator = (val);
	}
	~CCLVariant()
	{
		clear();
	}
};

