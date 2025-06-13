#ifndef _WYL_ZLIB_STREAM_H_
#define _WYL_ZLIB_STREAM_H_


/******************************************************************
 *
 *							wyLib库	2008 - 2010
 *
 *	$ ZIP压缩、解压类 $
 *  
 *	- 主要功能 - 
 *
 *	使用流的方式封装ZIP压缩和解压缩代码。
 *
 *****************************************************************/


namespace wylib
{
	namespace zstream
	{

		/*	ZLib流的基类

		*/

		class CBaseZStream
			: public stream::CBaseStream
		{
		public:
			typedef CBaseStream Inherited;

		protected:
			CBaseStream		*m_pStrm;			//zlib操作的目标流
			LONGLONG		m_nStrmPos;			//目标流的指针位置
			z_stream		m_ZStrm;			//zlib操作的z_stream结构
			Bytef			m_sBuffer[260];	//zlib操作缓冲区
			int				m_nLastCode;		//上一次操作的返回码

		private:
			static void* zlibAllocMem(CBaseZStream& ZStream, uInt Items, uInt Size);
			static void zlibFreeMem(CBaseZStream& ZStream, void *lpBlock);

		protected:
			//zlib操作内存申请函数，如果关心内存的申请和释放，则可在继承类中覆盖此函数
			virtual void* Alloc(LONGLONG size);
			//zlib操作释放内存函数，如果关心内存的申请和释放，则可在继承类中覆盖此函数
			virtual void Free(void* ptr);
			//zlib操作进度通知函数，如果关心进度，则在子类中覆盖此函数即可
			virtual void DoProgress();
			//设置上当前操作的返回码
			inline void setLastCode(const int code){ m_nLastCode = code; }
		public:
			CBaseZStream(CBaseStream &stream);
			virtual ~CBaseZStream();

			//获取上一次操作的返回码
			inline int getLastCode(){ return m_nLastCode; }
			inline int succeeded(){ return m_nLastCode >= 0; }
		};


		/*	ZLib压缩流
			可对数据进行压缩并写入到流中
		*/
		class CZCompressionStream
			: public CBaseZStream
		{
		public:
			typedef CBaseZStream Inherited;

			enum CompressionLevel
			{
				clNone = 0,		//储存（未压缩）
				clFastest,		//最快压缩（速度快，节省时间但压缩比不高）
				clDefault,		//默认压缩等级（压缩速度和压缩比率均折中）
				clMax,			//最高压缩比（压缩速度慢，但压缩比较高）
			};

		public:
			CZCompressionStream(CBaseStream& destStream, CompressionLevel CmprsLvl);
			~CZCompressionStream();

			LONGLONG seek(const LONGLONG tOffset, const int Origin);
			LONGLONG read(LPVOID lpBuffer, const LONGLONG tSizeToRead){ return -1; };
			LONGLONG write(LPCVOID lpBuffer, const LONGLONG tSizeToWrite);
			//提交压缩缓冲中的数据到流中
			void finish();
			//获取压缩后的压缩比，返回的是一个浮点数
			inline float getCompressionRate()
			{
				if ( m_ZStrm.total_in == 0 )
					return 0;
				else return (1.0f - ((float)m_ZStrm.total_out / m_ZStrm.total_in)) * 100.0f;
			}
		};

		/*	ZLib解压流
			可对数据进行解压并写入到流中
		*/
		class CZDecompressionStream
			: public CBaseZStream
		{
		public:
			typedef CBaseZStream Inherited;
			
		public:
			CZDecompressionStream(CBaseStream &source);
			~CZDecompressionStream();

			LONGLONG seek(const LONGLONG tOffset, const int Origin);
			LONGLONG read(LPVOID lpBuffer, const LONGLONG tSizeToRead);
			LONGLONG write(LPCVOID lpBuffer, const LONGLONG tSizeToWrite){ return -1; };
		};
	};
}

#endif

