#ifdef WIN32

#include <Windows.h>

/**
 * 跳转指令结构
 *************************/
#pragma pack(push, 1)
struct jmpInstruct
{
	unsigned char code;
	unsigned int offset;
};
#pragma pack(pop)

//计算跳转偏移地址
#define CalcJmpOffset(s, d)	((SIZE_T)(d) - ((SIZE_T)(s) + 5))

int patchFunction(void *dest, void *src)
{
	struct jmpInstruct* fn = (struct jmpInstruct*)src;
	DWORD dwOldProtect;
	MEMORY_BASIC_INFORMATION mbi;

	if ( VirtualQuery(fn, &mbi, sizeof(mbi)) != sizeof(mbi) )
		return GetLastError();

	//修改_output_l函数的内存保护模式，增加可读写保护
	if ( !VirtualProtect(fn, sizeof(*fn), PAGE_EXECUTE_READWRITE, &dwOldProtect) )
		return GetLastError();

	fn->code = 0xE9;
	fn->offset = (UINT)CalcJmpOffset(fn, dest);

	//还原_output_l函数的内存保护模式
	if ( !VirtualProtect(fn, sizeof(*fn), dwOldProtect, &dwOldProtect) )
		return GetLastError();

	return 0;
}

#endif
