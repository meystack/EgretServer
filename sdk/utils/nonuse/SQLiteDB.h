#pragma once

/******************************************************************


                     SQLite数据库的基本操作类

				   实现了基本的数据库查询和更新操作。

	对数据库的操作并不是多线程安全的，调用者必须负责维护好多线程的调用问
  题，或者对类打开的数据库的操作只由一个固定的线程操作。

******************************************************************/
#ifdef WIN32
class CSQLiteDB
{
public:
	CSQLiteDB();
	~CSQLiteDB();

public:
	/*打开一个数据文件。
	* 如果当前已经有打开的数据库，则会在成功打开新的数据库后关闭原有数据库文件，
	* 如果无法打开新的数据库，则原有打开的数据库不会被关闭。
	* 函数返回SQLITE3_OK(0)表示打开成功。
	*/
	int copen(const char *sDBFile);
	int wopen(const wchar_t *wsDBFile);
	//关闭当前打开的数据库
	void close();
	//判断数据库是否已经打开
	inline bool opened(){ return m_pDB != NULL; }

public:
	//获取sqlite3指针
	inline sqlite3* getSqlite3Ptr(){ return m_pDB; }
	//获取数据库的分页大小
	unsigned short getPageSize();
	//获取上次操作的错误码
	int getErrorCode();
	//获取上次操作的错误描述字串
	const wchar_t * getErrorMessage();

protected:
	//检查打开数据库的结果，并进行后续的操作
	int checkOpenResult(int nErr, sqlite3 *pdb);

protected:
	sqlite3		*m_pDB;		//数据库连接对象
};


class CSQLiteStatement
{
public:
	CSQLiteStatement(){ zd(); }
	CSQLiteStatement(CSQLiteDB *lpDB, const wchar_t *wsSQL){ zd(); m_pDB = lpDB; if (wsSQL) wprepare(wsSQL); }
	CSQLiteStatement(CSQLiteDB *lpDB, const char *sSQL){ zd(); m_pDB = lpDB; if (sSQL) cprepare(sSQL); }
	~CSQLiteStatement(){ if (m_pStmt) finalize(); }
	/***************** 操作函数集 *****************/
	inline sqlite3_stmt* stmt(){ return m_pStmt; }
	/*设置目标数据库对象
	*如果当前有为销毁的stmt，则函数调用失败并返回false，
	*否则函数返回true并将lpDB设置为新的数据库操作对象
	*/
	inline bool setDB(CSQLiteDB *lpDB){ if (m_pStmt) return false; m_pDB = lpDB; return true; }
	/*从ansi字符编码初始化stmt
	*如果操作成功，则会销毁之前初始化的stmt对象，否则先前的对象不会被改变。
	*/
	inline int cprepare(const char* sSQL)
	{
		sqlite3_stmt *pStmt;
		int result = sqlite3_prepare(m_pDB->getSqlite3Ptr(), sSQL, -1, &pStmt, NULL);
		if (result == SQLITE_OK)
		{
			finalize();
			m_pStmt = pStmt;
		}
		return result;
	}
	/*从utf16字符编码初始化stmt
	*如果操作成功，则会销毁之前初始化的stmt对象，否则先前的对象不会被改变。
	*/
	inline int wprepare(const wchar_t* wsSQL)
	{
		sqlite3_stmt *pStmt;
		int result = sqlite3_prepare16(m_pDB->getSqlite3Ptr(), wsSQL, -1, &pStmt, NULL);
		if (result == SQLITE_OK)
		{
			finalize();
			m_pStmt = pStmt;
		}
		return result;
	}
	//清空stmt中已经绑定的数据
	inline int reset(){ return sqlite3_reset(m_pStmt); }
	//对于执行成功的select语句，将游标调整至下一行
	inline int step(){ return sqlite3_step(m_pStmt); }
	//销毁stmt
	inline int finalize()
	{
		int result = 0;
		if (m_pStmt)
		{
			result = sqlite3_finalize(m_pStmt); 
			m_pStmt = NULL;
		}
		return result;
	}
	/***************** 参数绑定函数集，如果涉及到参数的索引，则索引从1开始！ *****************/
	//获取stmt中的参数数量
	inline int param_count(){ return sqlite3_bind_parameter_count(m_pStmt); }
	//获取参数绑定时的数据析构函数
	inline sqlite3_destructor_type param_destruct_type(){ return m_pDstrFunc; }
	/*设置参数绑定时的数据析构函数
	*绑定text、text16、blob参数时需要提供一个数据的析构函数，
	*并且会在对数据绑定完成后调用此析构函数从而适当的处理提供的数据，
	*例如有机会调用free释放数据。
	*可以使用预定义的析构函数SQLITE_STATIC和SQLITE_TRANSIENT，前者不对数据进行释放，
	*而后者会在替换参数前使得SQLite在内部使用一份似有的内存来拷贝数据，并且自动管理新内存，且不会对数据本身进行释放
	*/
	inline void set_param_destruct_type(sqlite3_destructor_type lpFn){ m_pDstrFunc = lpFn; }
	inline int bind_int(int nIdx, int nVal){ return sqlite3_bind_int(m_pStmt, nIdx, nVal); }
	inline int bind_int64(int nIdx, sqlite3_int64 nVal){ return sqlite3_bind_int64(m_pStmt, nIdx, nVal); }
	inline int bind_double(int nIdx, double dVal){ return sqlite3_bind_double(m_pStmt, nIdx, dVal); }
	inline int bind_ctext(int nIdx, const char* sVal){ return sqlite3_bind_text(m_pStmt, nIdx, sVal, -1, m_pDstrFunc); }
	inline int bind_wtext(int nIdx, const wchar_t* wsVal){ return sqlite3_bind_text16(m_pStmt, nIdx, wsVal, -1, m_pDstrFunc); }
	inline int bind_blob(int nIdx, const void* pVal, const int bytes){ return sqlite3_bind_blob(m_pStmt, nIdx, pVal, bytes, m_pDstrFunc); }
	/***************** 结果集状态函数集合 *****************/
	inline int col_count(){ return sqlite3_column_count(m_pStmt); }
	//获取select返回的数据（行）数量！！！！非常遗憾sqlite3没有直接支持！
	//inline int row_count(){ return sqlite3_data_count(m_pStmt); }
	//获取最后一个插入的行的自增ID值
	inline sqlite3_int64 last_row_id(){ return sqlite3_last_insert_rowid(m_pDB->getSqlite3Ptr()); }
	/***************** 取结果函数集 *****************/
	inline int val_int(int nCol){ return sqlite3_column_int(m_pStmt, nCol); }
	inline sqlite3_int64 val_int64(int nCol){ return sqlite3_column_int64(m_pStmt, nCol); }
	inline double val_double(int nCol){ return sqlite3_column_double(m_pStmt, nCol); }
	inline const char* val_ctext(int nCol){ return (const char*)sqlite3_column_text(m_pStmt, nCol); }
	inline const wchar_t* val_wtext(int nCol){ return (const wchar_t*)sqlite3_column_text16(m_pStmt, nCol); }
	inline const unsigned char* val_binary(int nCol){ return (const unsigned char*)sqlite3_column_blob(m_pStmt, nCol); }
	/*获取某列数据的字节大小
	*对于字符串型数据，其返回值表示字符串的内存字节长度，并且终止字符不算在内。
	*/
	inline int val_size(int nCol){ return sqlite3_column_bytes(m_pStmt, nCol); }
	/*获取某列数据的类型
	*由于SQLite具有类型亲和性，所以大可不必明确数据的类型。
	*/
	inline int val_type(int nCol){ return sqlite3_column_type(m_pStmt, nCol); }
private:
	//用于在构造函数中初始化成员的函数
	inline void zd()
	{
		m_pDB = NULL;
		m_pDstrFunc = SQLITE_STATIC;
		m_pStmt = NULL;
	}
private:
	sqlite3_stmt *m_pStmt;	//stmt指针
	CSQLiteDB	 *m_pDB;	//stmt作用的目标数据库对象
	sqlite3_destructor_type	m_pDstrFunc;//text、text16、blob参数数据析构函数，默认为SQLITE_STATIC，表示不会对内容做任何析构
};

#endif
