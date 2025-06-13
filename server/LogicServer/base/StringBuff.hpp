#pragma once
//每次内存增长的长度，1个简单的类，用于字符串的存盘
#define INCREASE_SIZE 1048576

class CStringBuff
{
public:
	CStringBuff()
	{
		m_buff =NULL;
		m_totalSize =0;
		m_curretSize=0;
		m_bIsLock =false;
	}
	~CStringBuff()
	{
		if(m_buff)
		{
			free(m_buff);
			m_buff = nullptr;
		}
	}
	//是否锁上了 避免在使用的时候，2个地方都去写buff
	bool IsLock()
	{
		return m_bIsLock;
	}

	//设置 开或者关掉锁
	void SetLock(bool isLock)
	{
		m_bIsLock =isLock;
	}

	/*
	* Comments:往里边增加字符串
	* Param char * str:字符串
	* @Return int:往里边添加的字符串的长度
	*/
	CStringBuff& Append(char *str)
	{
		if(str !=NULL)
		{
			size_t newSize= strlen(str); //
			if(newSize ==0) return  *this;;
			if(m_totalSize <= m_curretSize +  newSize)
			{
				size_t allocSize= ( m_curretSize +  newSize - m_totalSize); //缺口字节
				allocSize = (allocSize/INCREASE_SIZE +1) * INCREASE_SIZE;   //缺少多少k，不够1k的算1k
				
				m_buff = (char*) realloc(m_buff,(allocSize +m_totalSize) );
				if(m_buff)
				{
					if(m_totalSize ==0)
					{
						memset(m_buff + m_totalSize, 0, allocSize);
					}
					m_totalSize += allocSize;
				}
				else
				{
					return *this;;
				}
			}
			//strcat(m_buff,str);
			//这里去掉strcat
			//strcpy(m_buff + m_curretSize,str);
			//strncpy()
			memcpy((void*)&m_buff[m_curretSize],str,newSize);
			
			m_curretSize += newSize; //当前的字符串的长度
			m_buff[m_curretSize]=0;
		}
		
		return *this;
	}
	
	inline CStringBuff& AppendChar(char str)
	{
		char buff[2];
		buff[1]=0;
		buff[0]= str;
		Append(buff);
		return *this;
	}

	inline CStringBuff& operator << (char* val)
	{
		return Append(val);
	}

	inline CStringBuff& operator << (char val)
	{
		return AppendChar(val);
	}


	//获取这个字符串
	char * Get()
	{
		return m_buff;
	}
	//重用一下
	inline void Reset()
	{
		m_curretSize =0;
		if(m_buff && m_totalSize >0 )
		{
			m_buff[0]=0;
		}
	}
	//释放内存
	inline void Empty()
	{
		if(m_buff)
		{
			free(m_buff);
			m_buff = nullptr;
		}
		m_totalSize =0;
		m_curretSize =0;
	}
private:
	char * m_buff;			//数据指针
	size_t    m_totalSize;  //总大小
	size_t    m_curretSize; //当前的大小
	bool      m_bIsLock;  //是否是锁住的	
};
