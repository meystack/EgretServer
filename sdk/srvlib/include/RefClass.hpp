#pragma once

/*************************************************************************

					 自动管理生命期的引用计数对象实现类

	 以模板的函数提供对引用计数功能的实现，使用时直接将对象作为模板参数实
 例化CRefObject或CMTRefObject即可。实例在构造时引用计数为0。当引用计数变
 为0时会自动销毁实例。实现原理同于COM接口对象的引用计数；

   对计数对象的使用必须严格遵循获取指针后立刻addRef，不用时release。

   引用计数实现方式是多线程安全的。基于处理器原子保护锁的方式实现，增加以
 及解 除引用的时候会锁住总线以便在多个处理器之间保持缓存一致性。是依靠硬
 件实现的。

  通常情况下使用CRefObject类完成自动化管理的对象引用功能，但如果直接使用
 CRefObjectImpl模板类应当注意如下细节：
   ★在使用一个对象实例前必须调用addRef()函数以便增加引用计数；
  ★在解除对一个对象实例使用后必须调用release函数以便减少引用计数；
  ★不得使用delete销毁一个引用实例类，正确的做法是调用release解除引用；
  ★被实例化的类型只能使用new或malloc申请的内存块进行构造，不能对局部变
 量或非指针类成员的对象使用引用计数实现。

************************************************************************/

#include "_osdef.h"

namespace wylib
{
	namespace misc
	{
		template <typename T>
		class CRefObjectImpl : public T
		{
		public:
			CRefObjectImpl():T()
			{
				m_nRefer = 0;
			}

			//增加引用计数，函数返回增加引用后的引用计数值
			inline LONG addRef()
			{
				return InterlockedIncrement(&m_nRefer);
			}

			//减少引用计数，函数返回减少引用后的引用计数值
			//当减少引用后引用计数为0则自动销毁对象自身。
			inline LONG release()
			{
				LONG n = InterlockedDecrement(&m_nRefer);
				if ( n <= 0 )
					finallRelease();
				return n;
			}
		protected:
			virtual void finallRelease()
			{
				delete this;
			}
		private:
			LONG	m_nRefer;	//引用计数
		};

		template <typename T>
		class CRefObject
		{
		protected:
			CRefObjectImpl<T>*	m_ptr;//计数实现对象
		public:
			CRefObject()
			{
				m_ptr = NULL;
			}
			CRefObject(CRefObject<T>& obj)
			{
				m_ptr = obj.m_ptr;
				if ( m_ptr )
					m_ptr->addRef();
			}
			CRefObject(CRefObjectImpl<T>* pObj)
			{
				m_ptr = pObj;
				if ( m_ptr )
					m_ptr->addRef();
			}
			virtual ~CRefObject()
			{
				if (m_ptr) m_ptr->release();
			}
			
			//提供到模板实例化类型的转换函数
			inline operator T* ()
			{
				return m_ptr;
			}
			inline operator const T* () const
			{
				return m_ptr;
			}
			//获取私有实现对象指针
			inline CRefObjectImpl<T>* raw_ptr()
			{
				return m_ptr;
			}
			inline const CRefObjectImpl<T>* raw_ptr() const
			{
				return m_ptr;
			}

			//重载赋值运算函数，实现对象的引用
			inline void operator = (CRefObject<T>& obj)
			{
				if (&obj != NULL)
				{
					if ( obj.m_ptr == m_ptr )
						return;
					//必须先为另一个对象的实例增加引用再减少当前实例的引用。
					//藉此防止obj等于自己且引用计数为1在调用release后对象被删除而引发内存错误的问题。
					if (obj.m_ptr) obj.m_ptr->addRef();
					if (m_ptr) m_ptr->release();
					m_ptr = obj.m_ptr;
				}
				else if (m_ptr)
				{
					m_ptr->release();
					m_ptr = NULL;
				}
			}
			//重载赋值运算函数，实现对象的引用
			inline void operator = (CRefObjectImpl<T>* pObj)
			{
				if (pObj != NULL)
				{
					if ( pObj == m_ptr )
						return;
					if (pObj) pObj->addRef();
					if (m_ptr) m_ptr->release();
					m_ptr = pObj;
				}
				else if (m_ptr)
				{
					m_ptr->release();
					m_ptr = NULL;
				}
			}
			//重载相等性测试运算符
			inline bool operator == (const CRefObject<T>& obj) const
			{
				return obj.m_ptr == m_ptr;
			}
			inline void operator == (const CRefObjectImpl<T>* pObj) const
			{
				return pObj == m_ptr;
			}
			inline void operator == (const int n) const
			{
				if ( n == 0 )
					return m_ptr == NULL;
				else return m_ptr != NULL;
			}
			//重载不等性测试运算符
			inline bool operator != (const CRefObject<T>& obj) const
			{
				return obj.m_ptr != m_ptr;
			}
			inline bool operator != (const CRefObjectImpl<T>* pObj) const
			{
				return pObj != m_ptr;
			}
			inline void operator != (const int n) const
			{
				if ( n == 0 )
					return m_ptr != NULL;
				else return m_ptr == NULL;
			}
			inline bool operator ! () const
			{
				return m_ptr == NULL;
			}
		};
	};
};

