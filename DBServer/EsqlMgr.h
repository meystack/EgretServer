//支持数据服执行esql并自动删除

#pragma once

typedef struct tagDbEsql
{
	int		nId;					//索引
	char	sFileName[256];			//文件名称

	tagDbEsql()
	{
		memset(this,0,sizeof(*this));
	}
}DBSQL;

class CEsqlManager
{
public:
	CEsqlManager();

	~CEsqlManager();

	//运行esql的主流程
	void EsqlMain(char* sToolPath,char* sDbName);

	bool FileExist(LPCTSTR lpszFilePath);

	//获取文件夹下所有文件名称
	void GetAllFileName();

	//获取esql的文件的id
	int GetEsqlId(char* strEsql);

	//添加到sql列表
	void AddEsqlToList(int nId,char *strEsql);

	//执行esql语句的工具
	void OnRunEsqlTool(char* sToolPath,char* sDbName);

	//删除文件
	void DeleteEsqlFile();

	//判断是否sql文件
	bool bSqlFile(char* fileName);
private:
	wylib::container::CBaseList<DBSQL>			m_DbSQl;
};
