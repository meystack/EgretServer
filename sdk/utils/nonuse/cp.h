#ifndef _CODE_PATCH_SETUP_H_
#define _CODE_PATCH_SETUP_H_

/************************************************************************/
/*
/*                      运行时函数代码补丁安装库
/*
/*
/*   函数代码补丁的实现原理为在原始代码位置写入跳转指令跳转到新的函数位置。期间用到内存
/* 页属性更改函数VirtualProtect。
/*
/************************************************************************/
#ifdef __cplusplus
extern "C"{
#endif

/*
* Comments: 应用代码补丁
* Param void * dest: 补丁后代码的执行位置
* Param void * src: 要打补丁的代码位置
* @Return int: 返回0表示应用代码补丁成功，否则返回值表示系统错误码
*/
int patchFunction(void *dest, void *src);

#ifdef __cplusplus
};
#endif

#endif _CODE_PATCH_SETUP_H_
