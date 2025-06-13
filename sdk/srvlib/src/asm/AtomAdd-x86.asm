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

.model flat, stdcall
.code
				
				OPTION PROLOGUE:NONE, EPILOGUE:NONE

				%public  UIntAdd
				%public  IntAdd
				%public  UShortAdd
				%public  ShortAdd
				%public  USmallAdd
				%public  SmallAdd

UIntAdd   proc \
    a:DWORD,\
    b:DWORD

	push  edx
	mov   eax, dword ptr [a]
	mov   edx, dword ptr [b]

    add   eax, edx
    jc    @@overflow
	pop   edx
    ret   08h
@@overflow:
    mov   eax, 0FFFFFFFFh
	pop   edx
    ret   08h
UIntAdd	endp


IntAdd   proc \
    a:DWORD,\
    b:DWORD
	
	push  edx
	mov   eax, dword ptr [a]
	mov   edx, dword ptr [b]

    add   eax, edx
    jo    @@overflow
	pop   edx
    ret   08h
@@overflow:
    jns   @@negative
    mov   eax, 07FFFFFFFh
	pop   edx
    ret   08h
@@negative:
    mov   eax, 080000000h
	pop   edx
    ret   08h
IntAdd	endp


UShortAdd   proc \
    a:DWORD,\
    b:DWORD
	
	push  edx
	mov   ax, word ptr [a]
	mov   dx, word ptr [b]
	
    add   ax, dx
    jc    @@overflow
	pop   edx
    ret   08h
@@overflow:
    mov   ax, 0FFFFh
	pop   edx
    ret   08h
UShortAdd	endp

ShortAdd   proc \
    a:DWORD,\
    b:DWORD
	
	push  edx
	mov   ax, word ptr [a]
	mov   dx, word ptr [b]
	
	add   ax, dx
    jo    @@overflow
	pop   edx
    ret   08h
@@overflow:
    jns   @@negative
    mov   ax, 07FFFh
	pop   edx
    ret   08h
@@negative:
    mov   ax, 08000h
	pop   edx
    ret   08h
ShortAdd endp

USmallAdd   proc \
    a:DWORD,\
    b:DWORD
	
	push  edx
	mov   al, byte ptr [a]
	mov   dl, byte ptr [b]

    add   al, dl
    jc    @@overflow
	pop   edx
    ret   08h
@@overflow:
    or    al, 0FFh
	pop   edx
    ret	  08h
USmallAdd endp

SmallAdd   proc \
    a:DWORD,\
    b:DWORD
	
	push  edx
	mov   al, byte ptr [a]
	mov   dl, byte ptr [b]

    add   al, dl
    jo    @@overflow
	pop   edx
    ret	  08h
@@overflow:
    jns   @@negative
    mov   al, 07Fh
	pop   edx
    ret	  08h
@@negative:
    mov   al, 080h
	pop   edx
    ret	  08h
SmallAdd endp

end
