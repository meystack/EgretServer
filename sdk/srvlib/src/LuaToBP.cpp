#include <stdio.h>

extern "C"
{
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

#ifdef WIN32
	#include <Windows.h>
	#include <tchar.h>
#endif

#include <zlib.h>
#include <_ast.h>
#include <_memchk.h>
#include <List.h>
#include <Tick.h>
#include <Stream.h>
#include <ZStream.h>
#include <RefString.hpp>
#include "ShareUtil.h"
#include "../include/CustomLuaScript.h"
#include "../include/CustomLuaConfig.h"
#include "../include/CBPTypes.h"
#include "../include/LuaToBP.h"

using namespace wylib::string;
using namespace wylib::stream;
using namespace wylib::zstream;

class CCustomPropWriter
{
public:
	CCustomPropWriter(CBaseStream &stream, LPCSTR name, CBPDATATYPE type)
	{
		ZeroMemory(&m_Prop, sizeof(m_Prop));
		m_Prop.data.type = type;
		m_pStream = &stream;
		//记录属性头偏移量
		m_dwPropPos = (size_t)stream.getPosition();
		//调整流偏移量，保留属性头空间
		stream.setPosition(m_dwPropPos + sizeof(m_Prop));
		//写入属性名称
		m_Prop.namelen = (BYTE)(name ? strlen(name) : 0);
		if ( m_Prop.namelen > 0 )
			stream.write(name, m_Prop.namelen);
	}
	~CCustomPropWriter()
	{
		//在构造函数中保留的属性头空间中写回属性头
		LONGLONG dwPos = m_pStream->getPosition();
		m_pStream->setPosition(m_dwPropPos);
		m_pStream->write(&m_Prop, sizeof(m_Prop));
		//恢复流最终偏移量
		m_pStream->setPosition(dwPos);
	}
	size_t getSizeWriten()
	{
		return (size_t)m_pStream->getPosition() - m_dwPropPos;
	}
public:
	COMBINPROPERTY	 m_Prop;
	CBaseStream		*m_pStream;
	size_t			m_dwPropPos;
};

//通配符对比函数
bool WildcardCompare(LPCTSTR s, LPCTSTR sWildCards);

/****
 *
 *  CLuaToBP
 *
 ***************************/

CLuaToBP::CLuaToBP()
	:Inherited()
{

}

CLuaToBP::LPCREATELUAFN  CLuaToBP::getCreateLuaFn()
{
	return m_pCreateLuaFn;
}

void CLuaToBP::setCreateLuaFn(LPCREATELUAFN lpFn)
{
	m_pCreateLuaFn = lpFn;
}

lua_State *CLuaToBP::createLuaVM()
{
	if (m_pCreateLuaFn)
		return m_pCreateLuaFn();
	else return luaL_newstate();
}

void CLuaToBP::showError(LPCTSTR sError)
{
	m_sLastErrDesc = sError;
	RefString sErr;
	sErr = _T("[L2B]");
	sErr += sError;
	throw sErr;
}

bool CLuaToBP::isFiltePath(LPCSTR sPath)
{
	INT_PTR i;
	//优先从匹配列表匹配
	if (m_Matchers.count())
	{
		for (i=m_Matchers.count()-1; i>-1; --i)
		{
			if ( WildcardCompare(sPath, m_Matchers[i]) )
				return false;
		}
	}
	//不存在任何匹配器则进行过滤
	for (i=m_Filters.count()-1; i>-1; --i)
	{
		if ( WildcardCompare(sPath, m_Filters[i]) )
			return true;
	}
	return false;
}

size_t CLuaToBP::writeBoolean(wylib::stream::CBaseStream& stream, LPCSTR sKeyName)
{
	CCustomPropWriter writer(stream, sKeyName, v_boolean);
	writer.m_Prop.data.desc.b = getFieldBoolean(NULL);
	return writer.getSizeWriten();
}

size_t CLuaToBP::writeNumber(wylib::stream::CBaseStream& stream, LPCSTR sKeyName)
{
	BOOL boIsValid;
	CCustomPropWriter writer(stream, sKeyName, v_number);
	writer.m_Prop.data.desc.n = getFieldNumber(NULL, NULL, &boIsValid);
	return writer.getSizeWriten();
}


size_t CLuaToBP::writeString(wylib::stream::CBaseStream& stream, LPCSTR sKeyName)
{
	BOOL boIsValid;
	CCustomPropWriter writer(stream, sKeyName, v_string);

	LPCSTR s = getFieldString(NULL, NULL, &boIsValid);

	writer.m_Prop.data.desc.s.len = strlen(s);
	stream.write(s, writer.m_Prop.data.desc.s.len);

	return writer.getSizeWriten();
}

size_t CLuaToBP::writeTable(wylib::stream::CBaseStream& stream, LPCSTR sKeyName)
{
	static LPCTSTR luaT_typenames[] = {
	  _T("nil"), _T("boolean"), _T("userdata"), _T("number"),
	  _T("string"), _T("table"), _T("function"), _T("userdata"),
	  _T("thread"), _T("proto"), _T("upval")
	};
	static bool luaT_isWriteable[] = {
		false, true, false, true,
		true, true, false, false,
		false, false, false
	};

	CCustomPropWriter writer(stream, sKeyName, v_object);
	
	if ( enumTableFirst() )
	{
		int nType;
		CAnsiString sFeildName;
		LPSTR sNamePtr = m_sNamePtr;
		//添加路径分隔符
		if (sNamePtr > m_sNamePath)
		{
			sNamePtr[0] = '.';
			sNamePtr[1] = 0;
			sNamePtr++;
		}

		do
		{
			//获取并拷贝数据项名称
			m_sNamePtr = sNamePtr;
			getKeyName(sFeildName);
			if ( !sFeildName.isEmpty() && sFeildName[0] )
			{
				strcpy(m_sNamePtr, sFeildName);
				m_sNamePtr += strlen(m_sNamePtr);
			}

			//如果是可以写入的数据且不是过滤路径则写入数据
			nType = getValueType();
			if ( luaT_isWriteable[nType] && !isFiltePath(m_sNamePath) )
			{	
				//依据不同的数据类型按格式写入数据
				switch ( nType )
				{
				case LUA_TBOOLEAN:
					writeBoolean(stream, sFeildName);
					break;
				case LUA_TNUMBER:
					writeNumber(stream, sFeildName);
					break;
				case LUA_TSTRING:
					writeString(stream, sFeildName);
					break;
				case LUA_TTABLE:
					writeTable(stream, sFeildName);
					break;
				default:
					//showErrorFormat(_T("can not publish this type of data [%s]"), luaT_typenames[nType]);
					continue;
					break;
				}
				writer.m_Prop.data.desc.t.len++;
			}
		}
		while ( enumTableNext() );
	}

	return writer.getSizeWriten();
}

size_t CLuaToBP::saveToStream(LPCSTR sTableName, wylib::stream::CBaseStream& stream)
{
	size_t result = 0;

	//初始化表路径
	m_sNamePath[0] = 0;
	m_sNamePtr = m_sNamePath;

	if ( openGlobalTable(sTableName) )
	{
		//在内存流中写入属性集数据，根表的名称必须是""
		CMemoryStream ms;
		writeTable(ms, "");

		//保留文件头空间
		CBPFILE_HEADER hdr;
		ZeroMemory(&hdr, sizeof(hdr));
		hdr.dwIdent = 0x00504243;
		hdr.dwVersion = 0x010A0C0A;
		LONGLONG hdrPos = stream.getPosition();
		stream.setPosition(hdrPos + sizeof(hdr));

		//压缩属性集数据
		CZCompressionStream zStm(stream, CZCompressionStream::clMax);
		ms.setPosition(0);
		zStm.copyFrom(ms);
		zStm.finish();
		result = (size_t)(stream.getPosition() - hdrPos);

		//写入CBP文件头
		hdr.dwDataSourceSize = (DWORD)ms.getSize();
		hdr.dwDataCompressed = (DWORD)result - sizeof(hdr);
		stream.setPosition(hdrPos);
		stream.write(&hdr, sizeof(hdr));
		stream.setPosition(hdrPos + result);

		closeTable();
	}
	return result;
}

size_t CLuaToBP::saveToFile(LPCSTR sTableName, LPCTSTR sFileName)
{
	CFileStream fs(sFileName, CFileStream::faCreate, CFileStream::AlwaysCreate);
	return saveToStream(sTableName, fs);
}

void CLuaToBP::addFiltePath(LPCSTR sPath)
{
	m_Filters.add(sPath);
}

void CLuaToBP::addFiltePathList(const CBaseList<LPCSTR> &filterList)
{
	m_Filters.addList(filterList);
}

void CLuaToBP::clearFiltePath()
{
	m_Filters.empty();
}

void CLuaToBP::addMatcher(LPCSTR sMatching)
{
	m_Matchers.add(sMatching);
}

void CLuaToBP::addMatcherList(const CBaseList<LPCSTR> &matcherList)
{
	m_Matchers.addList(matcherList);
}

void CLuaToBP::clearMatchers()
{
	m_Matchers.empty();
}

LPCTSTR _tcsnstr(LPCTSTR s, LPCTSTR p, size_t nlen)
{
	size_t i;
	while (*s)
	{
		if (*s == *p)
		{
			i = 1;
			while (i<nlen)
			{
				if (!s[i])
					break;
				if (s[i] != p[i])
					break;
				i++;
			}
			if (i >= nlen)
				return s;
		}
		s++;
	}
	return NULL;
}

bool WildcardCompare(LPCTSTR s, LPCTSTR sWildCards)
{
	LPCTSTR sptr, swild, stag;
	SIZE_T nlen, nlen2;
	bool Result = false;

	sptr = s;
	swild = sWildCards;
	while (true)
	{
		if (swild[0] == '*')
		{
			//'*'  -->  仅仅星号，匹配任何字符
			if (swild[1] == 0)
			{
				Result = true;
				break;
			}
			//查找下一个星号
			swild++;
			stag = _tcschr(swild, '*');
			if ( stag == swild ) stag++;//连续两个'*'用于匹配一个*
			if ( stag == NULL )
			{
				//'*abc' ---> '*'号后面不再有'*'号则对比abc
				nlen = _tcslen(sptr);
				nlen2 = _tcslen(swild);
				if ( nlen >= nlen2 )
				{
					sptr += nlen - nlen2;
					if (_tcscmp(sptr, swild) == 0 )
						Result = true;
				}
				break;
			}
			
			//'*abc*' ---> '*'号后面还有'*'号则对比下一个星号之前的abc
			sptr = _tcsnstr( sptr, swild, stag - swild );
			if ( sptr == NULL )
				break;
			sptr += stag - swild;

			swild = stag;
		}
		else
		{
			//查找下一个星号
			stag = _tcschr( swild, '*' );
			if ( !stag )
			{
				//没有'*'号则对比字符串
				if ( _tcscmp(sptr, swild) == 0 )
					Result = true;
				break;
			}
			
			//'abc*' --->  有'*'号则对比字符串开头
			if ( _tcsncmp(sptr, swild, stag - swild) != 0 )
				break;
			sptr += stag - swild;

			swild = stag;
		}
	}
	return Result;
}
