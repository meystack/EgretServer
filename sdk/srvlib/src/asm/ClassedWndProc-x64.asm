;ml64 /c /Cp /Zi ClassedWndProc-x64.asm

				OPTION PROLOGUE:NONE, EPILOGUE:NONE

				%public  ClassedWndProc
				
.code

;1�������������Ὣֵ���� RCX �� RDX������� RSP ��ջָ��Ĵ����м�ȥ 32 ���ֽڡ��ڱ����ú����У������ڼĴ�����RCX �� RDX���з��ʲ�������������ô���������Ŀ�Ķ���Ҫ�Ĵ������ɽ��Ĵ������Ƶ�Ԥ���� 32 �ֽڶ�ջ�����С�
;2�����ĸ�������Ĳ�����˳����ѹջ��8�ֽڣ�
;3��Ϊǰ�ĸ�����Ԥ����32�ֽ�ռ�ռ��ǹ̶��ġ�Ԥ����ģ����ñ����ú����ı�д�߸���Ҳ����˵���������Ƿ��õ����ⲿ��ջ�ռ䶼ռ����,rsp��ָ���λ������ͼ��

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

