#ifndef _UTF8_OUTPUT_H_
#define _UTF8_OUTPUT_H_

/************************************************************************/
/*
/*         支持UTF-8编码已经MBCS编码的_output_l的代码补丁库
/*
/*   库内部实现了一个支持utf-8以及mbcs编码的_output_l函数，如果需要针对utf8编码的
/* sprintf, _snsprintf或_vsnprintf的功能支持，则在main函数中使用代码补丁安装函数
/* (__install_utf8_output_patch__)安装代码补丁。
/*
/************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif


/*
* Comments: 安装支持utf-8以及mbcs编码的_output_l函数补丁
* @Return int: 返回0表示代码补丁安装成功，否则返回值为GetLastError值
*/
int __cdecl __install_utf8_output_patch__();

#ifdef __cplusplus
};
#endif

#endif
