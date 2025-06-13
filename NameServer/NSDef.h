#pragma once

/***** 定义程序版本号 ********/
#define NSS_KN_VERION	 MAKEFOURCC(1, 10, 11, 23)

/***
/*  定义数据库查询语句
/**************************************/

//从数据库中加载角色名称过滤表
static const CHAR NSSQL_LoadFilterNames[]	= "CALL loadFilterNames()";
/* 向数据库添加新角色名称并增返回角色ID
  参数：角色名称
  @返回值：如果角色名称已被使用则返回null，否则返回角色ID
*/
static const CHAR NSSQL_AllocNewCharId[]	= "CALL v2_allocnewcharid(\"%s\",%u)";
/* 向数据库添加新帮会名称并增返回帮会ID
  参数：帮会名称
  @返回值：如果帮会名称已被使用则返回null，否则返回帮会ID
*/
static const CHAR NSSQL_AllocNewGuildId[]	= "CALL v2_allocnewguildid(\"%s\",%u)";
/* 向数据库中对角色进行改名操作
  参数：要改名的角色ID，新角色名称
  @返回值：改名成功则返回0，返回1表示角新色名称已被使用
*/
static const CHAR NSSQL_RenameChar[]	    = "CALL renameChar(%d,\"%s\")";
/* 向数据库中对帮会进行改名操作
  参数：要改名的帮会ID，新帮会名称
  @返回值：改名成功则返回0，返回1表示新帮会名称已被使用
*/
static const CHAR NSSQL_RenameGuild[]	    = "CALL renameGuild(%d,\"%s\")";
