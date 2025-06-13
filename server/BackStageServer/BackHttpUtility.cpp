#include "stdafx.h"

namespace BackHttpUtility
{

	typedef unsigned char BYTE;

	BYTE toHex(const BYTE &x)
	{
		return x > 9 ? x -10 + 'A': x + '0';
	}

	BYTE fromHex(const BYTE &x)
	{
		return isdigit(x) ? x-'0' : x-'A'+10;
	}

	string URLEncode(const string &sIn)
	{
		string sOut;
		for( size_t ix = 0; ix < sIn.size(); ix++ )
		{      
			BYTE buf[4];
			memset( buf, 0, 4 );
			if( isalnum( (BYTE)sIn[ix] ) )
			{      
				buf[0] = sIn[ix];
			}
			else
			{
				buf[0] = '%';
				buf[1] = toHex( (BYTE)sIn[ix] >> 4 );
				buf[2] = toHex( (BYTE)sIn[ix] % 16);
			}
			sOut += (char *)buf;
		}
		return sOut;
	};

	string URLDecode(const string &sIn)
	{
		string sOut;
		for( size_t ix = 0; ix < sIn.size(); ix++ )
		{
			BYTE ch = 0;
			if(sIn[ix]=='%')
			{
				ch = (fromHex(sIn[ix+1])<<4);
				ch |= fromHex(sIn[ix+2]);
				ix += 2;
			}
			else if(sIn[ix] == '+')
			{
				ch = ' ';
			}
			else
			{
				ch = sIn[ix];
			}
			sOut += (char)ch;
		}
		return sOut;
	}
}