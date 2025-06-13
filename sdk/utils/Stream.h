#ifndef _WYL_FILESTREAM_H_
#define _WYL_FILESTREAM_H_


/******************************************************************
 *
 *							wyLib库	2008 - 2010
 *
 *	$ 数据流库 $
 *  
 *	- 主要功能 - 
 *
 *	具有基本的数据流操作功能，写入、读取、调整流指针等基本操作和功能
 *	目前具有CFileStream、CMemoryStream等常用流类
 *
 *****************************************************************/

#include "_osdef.h"

namespace wylib
{
	namespace stream
	{
#ifndef WIN32
//#define INVALID_HANDLE_VALUE ((LONG_PTR)-1)
#endif

		/*	基础抽象流
			流的基础类，提供流最基本的虚函数接口声明	
		*/
		class CBaseStream
		{
		public:
			enum StreamSeekOrigin
			{
				soBeginning = 0,	//从流的起点开始
				soCurrent = 1,		//从流的当前位置开始
				soEnd = 2,			//从流的末尾开始
			};
		private:
		public:
			CBaseStream(){};
			virtual ~CBaseStream(){};

			//获取流大小
			virtual LONGLONG getSize();
			//设置流大小
			virtual bool setSize(LONGLONG tSize){ return false; };
			//获取流中当前指针位置
			inline LONGLONG getPosition(){ return seek( 0, soCurrent ); };
			//设置流当前指针位置
			LONGLONG setPosition(const LONGLONG tPosition);
			//调整流指针位置tOffset参数表示调整的长度，
			//Origin参数表示调整的启示位置，其值应当是StreamSeekOrigin枚举类型中的值
			virtual LONGLONG seek(const LONGLONG tOffset, const int Origin) = 0;
			//从流中读取数据lpBuffer为读取缓冲区，tSizeToRead为读取的字节数
			//如果读取失败，则函数返回-1，否则函数返回实际从流中读取的字节数
			virtual LONGLONG read(LPVOID lpBuffer, const LONGLONG tSizeToRead) = 0;
			//向流中写入数据lpBuffer为写入数据内容指针，tSizeToWrite为写入的字节数
			//如果写入失败，则函数返回-1，否则函数返回实际写入流中的字节数
			virtual LONGLONG write(LPCVOID lpBuffer, const LONGLONG tSizeToWrite) = 0;
			//从另一个流中拷贝数据lpStream为另一个流对象指针，tSizeToCopy表示拷贝的字节数
			//从lpStream中拷贝的数据的起始位置是lpStream的当前指针位置
			//若参数tSizeToCopy为0则表示拷贝lpStream整个流的数据
			virtual LONGLONG copyFrom(CBaseStream& stream, LONGLONG tSizeToCopy = 0);
		};



		/*	句柄流，
			流操作的数据是针对外部提供的句柄进行的	
		*/
		class CHandleStream
			: public CBaseStream
		{
		public:
			typedef CBaseStream Inherited;
		protected:
#ifdef WIN32
			HANDLE	m_hHandle;
			void setHandle(HANDLE Handle);
		public:
			CHandleStream(HANDLE StreamHandle);
			inline HANDLE getHandle(){ return m_hHandle; }
			inline bool handleValid(){ return m_hHandle != INVALID_HANDLE_VALUE; }
#else
			int	m_hHandle;
			void setHandle(int Handle);
		public:
			CHandleStream(int StreamHandle);
			inline int getHandle(){ return m_hHandle; }
			inline bool handleValid(){ return m_hHandle != INVALID_HANDLE_VALUE; }
#endif
			bool setSize(LONGLONG tSize);
			LONGLONG seek(const LONGLONG tOffset, const int Origin);
			LONGLONG read(LPVOID lpBuffer, const LONGLONG tSizeToRead);
			LONGLONG write(LPCVOID lpBuffer, const LONGLONG tSizeToWrite);
		};


		/*	文件流，
			对文件系统中的文件读写操作的类
		*/
		class CFileStream
			: public CHandleStream
		{
		public:
			typedef CHandleStream Inherited;
			//文件的打开方式
			enum FileAccessType
			{
				faRead		= 0x0001,		//读取
				faWrite		= 0x0002,		//写入
				faCreate	= 0x1000,		//创建

				faShareRead		= 0x0100,		//读共享
				faShareWrite	= 0x0200,		//写共享
				faShareDelete	= 0x0400,		//删除共享
			};
			//win32文件创建的模式
			enum OpenDisposition
			{
				CreateIfNotExists = 1,		//文件不存在则创建
				AlwaysCreate,				//总是创建文件，如果文件存在则将文件截断为0字节
				OpenExistsOnly,				//仅打开存在的文件
				AlwaysOpen,					//总是打开文件，若文件存在则直接打开，否则尝试创建文件并打开
				TruncExistsOnly,			//如果文件存在则打开文件并截断为0字节
			};
		private:
			LPTSTR	m_sFileName;
		protected:
			void setFileName(LPCTSTR lpFileName);
			void construct(LPCTSTR lpFileName, DWORD dwAccessType, DWORD dwWin32CreationDisposition);
		public:
			CFileStream(LPCTSTR lpFileName, DWORD dwAccessType);
			CFileStream(LPCTSTR lpFileName, DWORD dwAccessType, OpenDisposition eWin32CreateionDisposition);
			~CFileStream();

			inline LPCTSTR getFileName(){ return m_sFileName; }
		};


		/*	内存流
			使用内存作为数据存储区的流对象类
		*/
		class CMemoryStream
			: public CBaseStream
		{
		public:
			typedef CBaseStream Inherited;

		private:
			char	*m_pMemory;		//内存块指针
			char	*m_pPointer;	//流指针
			char	*m_pStreamEnd;	//文件尾指针
			char	*m_pMemoryEnd;	//内存块结束地址指针
		protected:
			//如果需要处理内存申请和释放相关事宜，则在继承类中覆盖此函数即可
			virtual char* Alloc(char* ptr, const LONGLONG size);

		public:
			CMemoryStream();
			~CMemoryStream();

			bool setSize(LONGLONG tSize);
			LONGLONG seek(const LONGLONG tOffset, const int Origin);
			LONGLONG read(LPVOID lpBuffer, const LONGLONG tSizeToRead);
			LONGLONG write(LPCVOID lpBuffer, const LONGLONG tSizeToWrite);

			//获取内存流的内存块指针
			inline LPVOID getMemory(){ return m_pMemory; }
			//从文件加流数据到内存
			LONGLONG loadFromFile(LPCTSTR lpFileName);
			//将流数据保存到文件
			LONGLONG saveToFile(LPCTSTR lpFileName);
		};
	};
};

#endif
