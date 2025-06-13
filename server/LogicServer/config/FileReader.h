#pragma once
/*
template<class SonClass>
class CFileReader
{
public:
	CFileReader(){};
	typedef bool (SonClass::*FileReaderFunc)(); //

	
    /*
	* Comments:读取文件，支持1个文件放多个table，最大支持4个处理函数
	* Param SonClass * pLuaConfig: 基类的指针
	* Param LPCTSTR sFilePath:文件的地址
	* Param FileReaderFunc reader:文件的读取函数
	* Param FileReaderFunc reader1:文件的读取函数（可选）
	* Param FileReaderFunc reader2:文件的读取函数（可选）
	* Param FileReaderFunc reader3:文件的读取函数（可选）
	* @Return bool:成功返回true，否则返回 false 
	
	inline bool LoadFile(SonClass *pLuaConfig,LPCTSTR sFilePath,FileReaderFunc reader, FileReaderFunc reader1=NULL,FileReaderFunc reader2=NULL,FileReaderFunc reader3=NULL)
	{
		
		CBaseList<FileReaderFunc> funcs;
		funcs.add(reader);
		if(reader1 !=NULL )
		{
			funcs.add(reader1);
		}
		if(reader2 !=NULL)
		{
			funcs.add(reader2);
		}
		if(reader3 !=NULL )
		{
			funcs.add(reader3);
		}
		return LoadFileFunc(pLuaConfig,sFilePath,funcs);
	}

	/*
	* Comments:读取一个lua文件里的系列的表
	* Param SonClass * pLuaConfig:子类的指针
	* Param LPCTSTR sFilePath:文件的地址
	* Param CVector<FileReaderFunc> & funcs:处理函数的列表
	* @Return bool:成功返回true，否则返回false
	
	bool LoadFileFunc(SonClass *pLuaConfig,LPCTSTR sFilePath,CBaseList<FileReaderFunc>& funcs)
	{
		using namespace wylib::stream;
		bool Result = false;		
		CMemoryStream ms;
		CCustomLuaPreProcessor pp;

		if(pLuaConfig ==NULL)
		{
			OutputMsg(rmError,_T("unable to load from %s,pLuaConfig ==NULL"), sFilePath);
			return false;
		}
		try
		{

			//从文件加载配置脚本
			if ( ms.loadFromFile(sFilePath) <= 0 )
				OutputMsg(rmError,_T("unable to load from %s"), sFilePath);
			//对配置脚本进行预处理
		
#ifdef __ROBOT_CLIENT__ 
			GetRobotMgr()->GetVSPDefine().RegisteToPreprocessor(pp);
#else
			GetLogicServer()->GetVSPDefine().RegisteToPreprocessor(pp);
#endif

			LPCTSTR sText = pp.parse((LPCTSTR)ms.getMemory(), sFilePath);
			//设置脚本内容
			if ( !pLuaConfig->setScript(sText) )
				OutputMsg(rmError,_T("syntax error on  %s"),sFilePath);
			//读取标准物品配置数据

			for(INT_PTR i=0; i< funcs.count(); i++)
			{
				if(funcs[i] ==NULL)
				{
					continue;
				}
				Result = (pLuaConfig->*funcs[i])();
				if(Result ==false)
				{
					break;
				}
			}
			

		}
		catch (RefString &s)
		{
			OutputMsg(rmError, _T("load %s config error: %s"),sFilePath, s.rawStr());
		}
		catch (...)
		{
			OutputMsg(rmError, _T("unexpected error on load %s"),sFilePath);
		}
		//销毁脚本虚拟机
		pLuaConfig->setScript(NULL);

		return Result;
	}

};
*/