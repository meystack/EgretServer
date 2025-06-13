#pragma once
/*
*
*	Sql封装类，避免多线程环境下，执行SQL查询加锁后没有释放锁，导致其他线程阻塞得不到执行。
*	使用方式： 
		CSqlHelper sqlHelper(&m_SQLConn)
		sqlHelper.Exec(sQueryActorData, ...)
*		CSqlHelper内部有个变量，记录执行查询或者数据库操作命令是否成功，如果成功，析构函数会自动
*	调用SQL::ResetQuery()。注意，因为每次执行查询成功，都会修改CSqlHelper类的m_bNeedReset
*	所以，如果要连续执行多个db查询命令，每个命令执行完毕都需要手动调用Reset。
*	例如：
*	   // 执行SQL命令1
*	   CSqlHelper sqlHelper(&m_SQLConn)
*	   sqlHelper.Exec(sDBCmd1);
*	   // 这里需要手动重置
*	   sqlHelper.Reset();
*	   // 执行SQL命令2
*	   sqlHelper.Query(sDBQuery2, ...);
*	   // 如果这是最后一条命令，不需要手动重置，析构函数会自动重置
*/

#include "_osdef.h"
#include "SQL.h"

class CSQLConenction;
class CSqlHelper
{
public:
	CSqlHelper(CSQLConenction* conn);
	~CSqlHelper();
	/*
	* Comments: 重置本次查询结果
	* @Return void:
	* @Remark:
	*/
	void Reset();

	/*
	* Comments: 执行SQL查询，不定长参数
	* Param const char * sQueryFormat:
	* Param ...:
	* @Return int:
	* @Remark:
	*/
	int Query(const char* sQueryFormat, ...);

	/*
	* Comments: 执行SQL查询，定长参数
	* Param const char * sQueryText:
	* Param const size_t nTextLen:
	* @Return int:
	* @Remark:
	*/
	int RealQuery(const char* sQueryText, const size_t nTextLen);

	/*
	* Comments: 执行SQL命令，不定长参数
	* Param const char * sQueryFormat:
	* Param ...:
	* @Return int:
	* @Remark:
	*/
	int Exec(const char* sQueryFormat, ...);

	/*
	* Comments: 执行SQL命令，定长参数
	* Param const char * sExecText:
	* Param const size_t nTextLen:
	* @Return int:
	* @Remark:
	*/
	int RealExec(const char* sExecText, const size_t nTextLen);
private:
	CSQLConenction*					m_pSQLConnection;	
	bool						m_bNeedReset;
};
