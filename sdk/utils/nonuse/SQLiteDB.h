#pragma once

/******************************************************************


                     SQLite���ݿ�Ļ���������

				   ʵ���˻��������ݿ��ѯ�͸��²�����

	�����ݿ�Ĳ��������Ƕ��̰߳�ȫ�ģ������߱��븺��ά���ö��̵߳ĵ�����
  �⣬���߶���򿪵����ݿ�Ĳ���ֻ��һ���̶����̲߳�����

******************************************************************/
#ifdef WIN32
class CSQLiteDB
{
public:
	CSQLiteDB();
	~CSQLiteDB();

public:
	/*��һ�������ļ���
	* �����ǰ�Ѿ��д򿪵����ݿ⣬����ڳɹ����µ����ݿ��ر�ԭ�����ݿ��ļ���
	* ����޷����µ����ݿ⣬��ԭ�д򿪵����ݿⲻ�ᱻ�رա�
	* ��������SQLITE3_OK(0)��ʾ�򿪳ɹ���
	*/
	int copen(const char *sDBFile);
	int wopen(const wchar_t *wsDBFile);
	//�رյ�ǰ�򿪵����ݿ�
	void close();
	//�ж����ݿ��Ƿ��Ѿ���
	inline bool opened(){ return m_pDB != NULL; }

public:
	//��ȡsqlite3ָ��
	inline sqlite3* getSqlite3Ptr(){ return m_pDB; }
	//��ȡ���ݿ�ķ�ҳ��С
	unsigned short getPageSize();
	//��ȡ�ϴβ����Ĵ�����
	int getErrorCode();
	//��ȡ�ϴβ����Ĵ��������ִ�
	const wchar_t * getErrorMessage();

protected:
	//�������ݿ�Ľ���������к����Ĳ���
	int checkOpenResult(int nErr, sqlite3 *pdb);

protected:
	sqlite3		*m_pDB;		//���ݿ����Ӷ���
};


class CSQLiteStatement
{
public:
	CSQLiteStatement(){ zd(); }
	CSQLiteStatement(CSQLiteDB *lpDB, const wchar_t *wsSQL){ zd(); m_pDB = lpDB; if (wsSQL) wprepare(wsSQL); }
	CSQLiteStatement(CSQLiteDB *lpDB, const char *sSQL){ zd(); m_pDB = lpDB; if (sSQL) cprepare(sSQL); }
	~CSQLiteStatement(){ if (m_pStmt) finalize(); }
	/***************** ���������� *****************/
	inline sqlite3_stmt* stmt(){ return m_pStmt; }
	/*����Ŀ�����ݿ����
	*�����ǰ��Ϊ���ٵ�stmt����������ʧ�ܲ�����false��
	*����������true����lpDB����Ϊ�µ����ݿ��������
	*/
	inline bool setDB(CSQLiteDB *lpDB){ if (m_pStmt) return false; m_pDB = lpDB; return true; }
	/*��ansi�ַ������ʼ��stmt
	*��������ɹ����������֮ǰ��ʼ����stmt���󣬷�����ǰ�Ķ��󲻻ᱻ�ı䡣
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
	/*��utf16�ַ������ʼ��stmt
	*��������ɹ����������֮ǰ��ʼ����stmt���󣬷�����ǰ�Ķ��󲻻ᱻ�ı䡣
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
	//���stmt���Ѿ��󶨵�����
	inline int reset(){ return sqlite3_reset(m_pStmt); }
	//����ִ�гɹ���select��䣬���α��������һ��
	inline int step(){ return sqlite3_step(m_pStmt); }
	//����stmt
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
	/***************** �����󶨺�����������漰����������������������1��ʼ�� *****************/
	//��ȡstmt�еĲ�������
	inline int param_count(){ return sqlite3_bind_parameter_count(m_pStmt); }
	//��ȡ������ʱ��������������
	inline sqlite3_destructor_type param_destruct_type(){ return m_pDstrFunc; }
	/*���ò�����ʱ��������������
	*��text��text16��blob����ʱ��Ҫ�ṩһ�����ݵ�����������
	*���һ��ڶ����ݰ���ɺ���ô����������Ӷ��ʵ��Ĵ����ṩ�����ݣ�
	*�����л������free�ͷ����ݡ�
	*����ʹ��Ԥ�������������SQLITE_STATIC��SQLITE_TRANSIENT��ǰ�߲������ݽ����ͷţ�
	*�����߻����滻����ǰʹ��SQLite���ڲ�ʹ��һ�����е��ڴ����������ݣ������Զ��������ڴ棬�Ҳ�������ݱ�������ͷ�
	*/
	inline void set_param_destruct_type(sqlite3_destructor_type lpFn){ m_pDstrFunc = lpFn; }
	inline int bind_int(int nIdx, int nVal){ return sqlite3_bind_int(m_pStmt, nIdx, nVal); }
	inline int bind_int64(int nIdx, sqlite3_int64 nVal){ return sqlite3_bind_int64(m_pStmt, nIdx, nVal); }
	inline int bind_double(int nIdx, double dVal){ return sqlite3_bind_double(m_pStmt, nIdx, dVal); }
	inline int bind_ctext(int nIdx, const char* sVal){ return sqlite3_bind_text(m_pStmt, nIdx, sVal, -1, m_pDstrFunc); }
	inline int bind_wtext(int nIdx, const wchar_t* wsVal){ return sqlite3_bind_text16(m_pStmt, nIdx, wsVal, -1, m_pDstrFunc); }
	inline int bind_blob(int nIdx, const void* pVal, const int bytes){ return sqlite3_bind_blob(m_pStmt, nIdx, pVal, bytes, m_pDstrFunc); }
	/***************** �����״̬�������� *****************/
	inline int col_count(){ return sqlite3_column_count(m_pStmt); }
	//��ȡselect���ص����ݣ��У��������������ǳ��ź�sqlite3û��ֱ��֧�֣�
	//inline int row_count(){ return sqlite3_data_count(m_pStmt); }
	//��ȡ���һ��������е�����IDֵ
	inline sqlite3_int64 last_row_id(){ return sqlite3_last_insert_rowid(m_pDB->getSqlite3Ptr()); }
	/***************** ȡ��������� *****************/
	inline int val_int(int nCol){ return sqlite3_column_int(m_pStmt, nCol); }
	inline sqlite3_int64 val_int64(int nCol){ return sqlite3_column_int64(m_pStmt, nCol); }
	inline double val_double(int nCol){ return sqlite3_column_double(m_pStmt, nCol); }
	inline const char* val_ctext(int nCol){ return (const char*)sqlite3_column_text(m_pStmt, nCol); }
	inline const wchar_t* val_wtext(int nCol){ return (const wchar_t*)sqlite3_column_text16(m_pStmt, nCol); }
	inline const unsigned char* val_binary(int nCol){ return (const unsigned char*)sqlite3_column_blob(m_pStmt, nCol); }
	/*��ȡĳ�����ݵ��ֽڴ�С
	*�����ַ��������ݣ��䷵��ֵ��ʾ�ַ������ڴ��ֽڳ��ȣ�������ֹ�ַ��������ڡ�
	*/
	inline int val_size(int nCol){ return sqlite3_column_bytes(m_pStmt, nCol); }
	/*��ȡĳ�����ݵ�����
	*����SQLite���������׺��ԣ����Դ�ɲ�����ȷ���ݵ����͡�
	*/
	inline int val_type(int nCol){ return sqlite3_column_type(m_pStmt, nCol); }
private:
	//�����ڹ��캯���г�ʼ����Ա�ĺ���
	inline void zd()
	{
		m_pDB = NULL;
		m_pDstrFunc = SQLITE_STATIC;
		m_pStmt = NULL;
	}
private:
	sqlite3_stmt *m_pStmt;	//stmtָ��
	CSQLiteDB	 *m_pDB;	//stmt���õ�Ŀ�����ݿ����
	sqlite3_destructor_type	m_pDstrFunc;//text��text16��blob������������������Ĭ��ΪSQLITE_STATIC����ʾ������������κ�����
};

#endif
