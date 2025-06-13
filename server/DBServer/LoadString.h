
#pragma once

class CLoadString :
	public wylib::container::CBaseList<LPCTSTR>
{
protected:
	typedef wylib::container::CBaseList<LPCTSTR> Inherited;

public:
	CLoadString();
	~CLoadString();

	INT_PTR Count()
	{
		return count();
	}

	INT_PTR loadFromFile(LPCSTR sFileName);
	bool GetName(LPCSTR & sName, INT_PTR nIdx);

	const char * GetRandomName();
private:
	wylib::stream::CMemoryStream m_FileStream;
};