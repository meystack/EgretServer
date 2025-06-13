#pragma once

using namespace FileHeaders;

class CStdItemConfigCacher :
	public CCustomConfigCacher
{
public:
	typedef CCustomConfigCacher Inherited;

	/************ 标准物品配置缓存文件标志以及版本号 ****************/
	static const FILEIDENT CacheFileIdent;
	static const FILEVERSION CacheVersion;

protected:
	/**** ★★★★★★覆盖父类必须被覆盖的缓存数据处理函数集合★★★★★★ ****/
	//验证缓存文件头，判断缓存是否有效
	bool ValidateHeader(const CONF_DATA_CACHE_HDR& hdr);
	//调整单个一级数据对象的成员指针
	bool AdjustElementPointers(LPVOID lpElement);
	//填充缓存文件头数据，只需填充（文件头标志、版本号、以及dwSizeElement）其他的会自动填充和处理
	void FillHeaderData(CONF_DATA_CACHE_HDR& hdr);
};