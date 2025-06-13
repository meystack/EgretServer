;ml64 /c /Cp /Zi ClassedWndProc-x64.asm

				OPTION PROLOGUE:NONE, EPILOGUE:NONE

				%public  ClassedWndProc
				
.code

;1、编译器不仅会将值赋给 RCX 和 RDX，还会从 RSP 堆栈指针寄存器中减去 32 个字节。在被调用函数中，可以在寄存器（RCX 和 RDX）中访问参数。如果被调用代码因其他目的而需要寄存器，可将寄存器复制到预留的 32 字节堆栈区域中。
;2、第四个参数后的参数按顺序反向压栈（8字节）
;3、为前四个参数预留的32字节占空间是固定的、预分配的，不用被调用函数的编写者负责，也就是说，不管你是否用到，这部分栈空间都占用了,rsp所指向的位置如上图。

ClassedWndProc   proc \
    hWnd:QWORD,\
    uMsg:QWORD,\
    wParam:QWORD,\
    lParam:QWORD

    mov              rax, qword ptr [rbx + 8h] ; WndObject instance
    push             rax                       ; arg5
    sub              rsp, 20h                  ; arg1 - arg4
    call             qword ptr [rbx]           ; CClassedWnd::StaticWndProc
    add              rsp, 28h
    ret
ClassedWndProc	endp

end

