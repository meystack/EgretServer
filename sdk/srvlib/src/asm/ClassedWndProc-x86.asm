;ml /c /Cp /Zi ClassedWndProc-x86.asm

.model flat, stdcall
.code
				
				OPTION PROLOGUE:NONE, EPILOGUE:NONE

				%public  ClassedWndProc


ClassedWndProc   proc \
    hWnd:DWORD,\
    uMsg:DWORD,\
    wParam:DWORD,\
    lParam:DWORD

    mov              eax, dword ptr [ebx+4]    ; WndObject instance
    push             eax                       ; arg5
    mov              eax, dword ptr [esp+14h]
    push             eax                       ; arg4 lParam
    mov              eax, dword ptr [esp+14h]
    push             eax                       ; arg3 wParam
    mov              eax, dword ptr [esp+14h]
    push             eax                       ; arg2 uMsg
    mov              eax, dword ptr [esp+14h]
    push             eax                       ; arg1 hWnd
    call dword ptr   [ebx]                     ; CClassedWnd::StaticWndProc
    ret              10h
ClassedWndProc	endp


end

