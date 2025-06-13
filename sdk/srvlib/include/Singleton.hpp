#pragma once

/************************************************************************/
/*                       单例模式
/*
/*
/************************************************************************/


class CSingleton
{

private:
	static CSingleton*  m_pInstance;

public:
	static CSingleton * GetInstance ()
	{ 
		if (m_pInstance == 0)
		{
			m_pInstance = new CSingleton();
			
		}
		return m_pInstance;
	}

private:
	CSingleton(); 
};
 //CSingleton* CSingleton::m_pInstance= 0;