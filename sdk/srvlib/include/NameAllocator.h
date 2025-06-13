#pragma once

class CNameAllocator :
    protected wylib::container::CBaseList<LPCTSTR>
{
protected:
    typedef wylib::container::CBaseList<LPCTSTR> Inherited;

public:
    CNameAllocator();
    ~CNameAllocator();

    //************************************
    // Method:    getName
    // FullName:  CNameAllocator::getName
    // Access:    public 
    // Returns:   INT_PTR 返回名字在库中的索引,调用者必须保存此索引并在正式使用的时候调用allocName
    // Qualifier: 从名字库中提取一个名字
    // Parameter: LPCTSTR & lpNamePtr 输出名字保存对象
    //************************************
    INT_PTR getName(LPCTSTR &lpNamePtr);
    //************************************
    // Method:    allocName
    // FullName:  CNameAllocator::allocName
    // Access:    public 
    // Returns:   bool
    // Qualifier: 使用一个名字(将一个名字标记为被使用)
    // Parameter: INT_PTR index 
    //************************************
    bool allocName(INT_PTR index);
    //************************************
    // Method:    repack
    // FullName:  CNameAllocator::repack
    // Access:    public 
    // Returns:   INT_PTR
    // Qualifier: 重新整理名字库,删除所有已被使用的名字.
    // 请慎用,因为调用此函数后可能导致通过getName函数返回的索引不再有效!
    //************************************
    INT_PTR repack();

    //************************************
    // Method:    loadFromFile
    // FullName:  CNameAllocator::loadFromFile
    // Access:    public 
    // Returns:   INT_PTR 函数返回加载了多少个名字
    // Qualifier: 从文件加载名字库
    // Parameter: LPCTSTR sFileName
    //************************************
    INT_PTR loadFromFile(LPCTSTR sFileName);
    //************************************
    // Method:    saveToFile
    // FullName:  CNameAllocator::saveToFile
    // Access:    public 
    // Returns:   INT_PTR 函数返回保存了多少个未使用的名字
    // Qualifier: 向文件中保存名字库,已经被使用的名字将不会被保存
    // Parameter: LPCTSTR sFileName
    //************************************
    INT_PTR saveToFile(LPCTSTR sFileName);

protected:
    wylib::container::CBaseList<bool>              m_NameUseList;
    wylib::stream::CMemoryStream m_NameStream;
    INT_PTR                      m_nAllocNameIndex;
	INT_PTR						 m_nUsedCount;//已经被使用的名字数量统计
};
