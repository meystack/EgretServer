#ifndef			_FUNCTIONNAME_SENDER_H_
#define			_FUNCTIONNAME_SENDER_H_

typedef struct tagFunctionName
{
	ACTORNAME	sName;	

}FUNCTIONNAMES;


//加密脚本函数名称
class CEncryptFunctionName
{
public:
	CEncryptFunctionName();
	~CEncryptFunctionName();

	/* 从脚本字符串中提取函数名加密 
	* strVal	脚本字符串
	* nKey		秘钥
	*/
	char* EncryptExtractString(LPCSTR strVal,int nKey);

	/* 解密函数名 
	* StrSrc	需要解密的字符串
	* nKey		秘钥
	*/
	void DecryptFunctionName(char* StrSrc,char* StrDest,int nKey);

	/* 加密函数名 
	* StrSrc	需要加密的字符串
	* nKey		秘钥
	*/
	void EncryptFunctionName(char* StrSrc,char* StrDest,int nKey);

	/*判断是否是需要加密的字符串
	* StrSrc	是否需要加密的字符串
	*/
	bool IsNeedEncrypt(const char* strSrc);

	/*脚本里message脚本中的加密函数名
	* StrSrc	是否需要加密的字符串
	*/
	char* ExtractMessageBox(char* strVal,int nKey);

	//查找是否存在目标字符串
	bool StringMatching(const char* srcStr, const char *matchStr);

	void AddFunctionName(char* sName)
	{
		if(sName) 
		{
			FUNCTIONNAMES tmpStr;
			strcpy(tmpStr.sName,sName);
			m_FunctionName.add(tmpStr);
		}
	}
private:
	static char*		m_ValStr;		//记录脚本长度
	static int			m_ValLen;		//长度

	CVector<FUNCTIONNAMES> m_FunctionName;	//函数名
};

#endif