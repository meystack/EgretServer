;ml /c /Cp /Zi AtomInc-x86.asm

;-----------------------------------------------------------------
;
;                           加减法运算器
;
;               运算函数能够捕获运算结果的数据类型溢出的错误
;   当加法或减法运算结果产生溢出时，运算函数能够给予返回值指定数据类型的最低值或
; 最高值，以保证运算的结果在数据类型的设置范围内。
;
;
;                      additional operator
;
;           operator proc will catch opeation overflow.
; wen overflow occured, result value will set to lowest or highest
; value of special data type.
;
;-----------------------------------------------------------------

.code
				
				OPTION PROLOGUE:NONE, EPILOGUE:NONE
				
				%public  UInt64Add
				%public  Int64Add
				%public  UIntAdd
				%public  IntAdd
				%public  UShortAdd
				%public  ShortAdd
				%public  USmallAdd
				%public  SmallAdd


UInt64Add   proc \
    a:QWORD,\
    b:QWORD

	mov   rax, rcx
    add   rax, rdx
    jc    @@overflow
    ret
@@overflow:
    mov   rax, 0FFFFFFFFFFFFFFFFh
    ret
UInt64Add	endp


Int64Add   proc \
    a:QWORD,\
    b:QWORD
	
	mov   rax, rcx
    add   rax, rdx
    jo    @@overflow
    ret
@@overflow:
    jns   @@negative
    mov   rax, 7FFFFFFFFFFFFFFFh
    ret
@@negative:
    mov   rax, 8000000000000000h
    ret
Int64Add	endp


UIntAdd   proc \
    a:QWORD,\
    b:QWORD
	
	mov   eax, ecx
    add   eax, edx
    jc    @@overflow
    ret
@@overflow:
    mov   eax, 0FFFFFFFFh
    ret
UIntAdd	endp


IntAdd   proc \
    a:QWORD,\
    b:QWORD
	
	mov   eax, ecx
    add   eax, edx
    jo    @@overflow
    ret
@@overflow:
    jns   @@negative
    mov   eax, 7FFFFFFFh
    ret
@@negative:
    mov   eax, 80000000h
    ret
IntAdd	endp

UShortAdd   proc \
    a:QWORD,\
    b:QWORD
	
	mov   ax, cx
    add   ax, dx
    jc    @@overflow
    ret
@@overflow:
    mov   ax, 0FFFFh
    ret
UShortAdd	endp

ShortAdd   proc \
    a:QWORD,\
    b:QWORD
	
	mov   ax, cx
	add   ax, dx
    jo    @@overflow
    ret
@@overflow:
    jns   @@negative
    mov   ax, 7FFFh
    ret
@@negative:
    mov   ax, 8000h
    ret
ShortAdd endp

USmallAdd   proc \
    a:QWORD,\
    b:QWORD
	
	mov   al, cl
    add   al, dl
    jc    @@overflow
    ret
@@overflow:
    or    al, 0FFh
    ret
USmallAdd endp

SmallAdd   proc \
    a:QWORD,\
    b:QWORD
	
	mov   al, cl
    add   al, dl
    jo    @@overflow
    ret
@@overflow:
    jns   @@negative
    mov   al, 7Fh
    ret
@@negative:
    mov   al, 80h
    ret
SmallAdd endp

end
