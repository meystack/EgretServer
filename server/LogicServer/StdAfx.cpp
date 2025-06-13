#include "StdAfx.h"

#include <sstream>

//字符串分割函数
std::vector<std::string> SplitStr(std::string str,const std::string pattern)
{
  	std::string::size_type pos;
  	std::vector<std::string> result;
  	if (str.length() != 0)
	{
		str+=pattern;//扩展字符串以方便操作
  		int size=str.size();
 
		for(int i=0; i<size; i++)
		{
			pos=str.find(pattern,i);
			if(pos<size)
			{
			std::string s=str.substr(i,pos-i);
			result.push_back(s);
			i=pos+pattern.size()-1;
			}
		}
	}
  return result;
}

unsigned int getNowZeroTime() 
{  
    time_t t = time(NULL); 
    struct tm tm;
    localtime_r(&t,&tm);  
    tm.tm_hour = 0;  
    tm.tm_min = 0;  
    tm.tm_sec = 0;  
    return mktime(&tm);  
}

std::string intToString(int nInt, std::string nLeftExtra, std::string nRightExtra)
{
	// int m = nInt;
    // char temp[10];
    // char temp_[10];
	// memset(&temp_, 0 ,sizeof(temp_));
	// memset(&temp, 0 ,sizeof(temp));
    // int i = 0, j = 0;

    // if(nLeftExtra != "" && nLeftExtra.length() ) {
    //     for(int k = 0; k < nLeftExtra.length(); k++) {

    //         if(j > sizeof(temp)- 2)
    //             break;

    //         temp[j++] = nLeftExtra[k];
    //     }
    // }
    // // 处理负数
    // if (nInt < 0) {

    //     m = 0 - m;
    //     temp[j++] = '-';
    // }

    // while (m>0) {

    //     temp_[i++] = m % 10 + '0';
    //     m /= 10;
    // }

    // temp_[i] = '\0';
    // i = i - 1;
    // while (i >= 0) {

    //     temp[j++] = temp_[i--];
    // } 

   

    // temp[j] = '\0';
    std::string nResult = nLeftExtra;
    std::stringstream ss;
    ss << nInt; 
    nResult += ss.str();
    nResult += nRightExtra;
    return nResult;
}
