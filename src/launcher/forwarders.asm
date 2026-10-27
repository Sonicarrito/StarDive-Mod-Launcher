option casemap:none
EXTERN Resolve:PROC
.code
Probe0 PROC FRAME
sub rsp, 0A8h
.allocstack 0A8h
.endprolog
mov [rsp+20h], rcx
mov [rsp+28h], rdx
mov [rsp+30h], r8
mov [rsp+38h], r9
movdqu [rsp+40h], xmm0
movdqu [rsp+50h], xmm1
movdqu [rsp+60h], xmm2
movdqu [rsp+70h], xmm3
mov ecx, 0
call Resolve
mov rcx, [rsp+20h]
mov rdx, [rsp+28h]
mov r8, [rsp+30h]
mov r9, [rsp+38h]
movdqu xmm0, [rsp+40h]
movdqu xmm1, [rsp+50h]
movdqu xmm2, [rsp+60h]
movdqu xmm3, [rsp+70h]
add rsp, 0A8h
jmp rax
Probe0 ENDP
Probe1 PROC FRAME
sub rsp, 0A8h
.allocstack 0A8h
.endprolog
mov [rsp+20h], rcx
mov [rsp+28h], rdx
mov [rsp+30h], r8
mov [rsp+38h], r9
movdqu [rsp+40h], xmm0
movdqu [rsp+50h], xmm1
movdqu [rsp+60h], xmm2
movdqu [rsp+70h], xmm3
mov ecx, 1
call Resolve
mov rcx, [rsp+20h]
mov rdx, [rsp+28h]
mov r8, [rsp+30h]
mov r9, [rsp+38h]
movdqu xmm0, [rsp+40h]
movdqu xmm1, [rsp+50h]
movdqu xmm2, [rsp+60h]
movdqu xmm3, [rsp+70h]
add rsp, 0A8h
jmp rax
Probe1 ENDP
Probe2 PROC FRAME
sub rsp, 0A8h
.allocstack 0A8h
.endprolog
mov [rsp+20h], rcx
mov [rsp+28h], rdx
mov [rsp+30h], r8
mov [rsp+38h], r9
movdqu [rsp+40h], xmm0
movdqu [rsp+50h], xmm1
movdqu [rsp+60h], xmm2
movdqu [rsp+70h], xmm3
mov ecx, 2
call Resolve
mov rcx, [rsp+20h]
mov rdx, [rsp+28h]
mov r8, [rsp+30h]
mov r9, [rsp+38h]
movdqu xmm0, [rsp+40h]
movdqu xmm1, [rsp+50h]
movdqu xmm2, [rsp+60h]
movdqu xmm3, [rsp+70h]
add rsp, 0A8h
jmp rax
Probe2 ENDP
Probe3 PROC FRAME
sub rsp, 0A8h
.allocstack 0A8h
.endprolog
mov [rsp+20h], rcx
mov [rsp+28h], rdx
mov [rsp+30h], r8
mov [rsp+38h], r9
movdqu [rsp+40h], xmm0
movdqu [rsp+50h], xmm1
movdqu [rsp+60h], xmm2
movdqu [rsp+70h], xmm3
mov ecx, 3
call Resolve
mov rcx, [rsp+20h]
mov rdx, [rsp+28h]
mov r8, [rsp+30h]
mov r9, [rsp+38h]
movdqu xmm0, [rsp+40h]
movdqu xmm1, [rsp+50h]
movdqu xmm2, [rsp+60h]
movdqu xmm3, [rsp+70h]
add rsp, 0A8h
jmp rax
Probe3 ENDP
Probe4 PROC FRAME
sub rsp, 0A8h
.allocstack 0A8h
.endprolog
mov [rsp+20h], rcx
mov [rsp+28h], rdx
mov [rsp+30h], r8
mov [rsp+38h], r9
movdqu [rsp+40h], xmm0
movdqu [rsp+50h], xmm1
movdqu [rsp+60h], xmm2
movdqu [rsp+70h], xmm3
mov ecx, 4
call Resolve
mov rcx, [rsp+20h]
mov rdx, [rsp+28h]
mov r8, [rsp+30h]
mov r9, [rsp+38h]
movdqu xmm0, [rsp+40h]
movdqu xmm1, [rsp+50h]
movdqu xmm2, [rsp+60h]
movdqu xmm3, [rsp+70h]
add rsp, 0A8h
jmp rax
Probe4 ENDP
Probe5 PROC FRAME
sub rsp, 0A8h
.allocstack 0A8h
.endprolog
mov [rsp+20h], rcx
mov [rsp+28h], rdx
mov [rsp+30h], r8
mov [rsp+38h], r9
movdqu [rsp+40h], xmm0
movdqu [rsp+50h], xmm1
movdqu [rsp+60h], xmm2
movdqu [rsp+70h], xmm3
mov ecx, 5
call Resolve
mov rcx, [rsp+20h]
mov rdx, [rsp+28h]
mov r8, [rsp+30h]
mov r9, [rsp+38h]
movdqu xmm0, [rsp+40h]
movdqu xmm1, [rsp+50h]
movdqu xmm2, [rsp+60h]
movdqu xmm3, [rsp+70h]
add rsp, 0A8h
jmp rax
Probe5 ENDP
Probe6 PROC FRAME
sub rsp, 0A8h
.allocstack 0A8h
.endprolog
mov [rsp+20h], rcx
mov [rsp+28h], rdx
mov [rsp+30h], r8
mov [rsp+38h], r9
movdqu [rsp+40h], xmm0
movdqu [rsp+50h], xmm1
movdqu [rsp+60h], xmm2
movdqu [rsp+70h], xmm3
mov ecx, 6
call Resolve
mov rcx, [rsp+20h]
mov rdx, [rsp+28h]
mov r8, [rsp+30h]
mov r9, [rsp+38h]
movdqu xmm0, [rsp+40h]
movdqu xmm1, [rsp+50h]
movdqu xmm2, [rsp+60h]
movdqu xmm3, [rsp+70h]
add rsp, 0A8h
jmp rax
Probe6 ENDP
Probe7 PROC FRAME
sub rsp, 0A8h
.allocstack 0A8h
.endprolog
mov [rsp+20h], rcx
mov [rsp+28h], rdx
mov [rsp+30h], r8
mov [rsp+38h], r9
movdqu [rsp+40h], xmm0
movdqu [rsp+50h], xmm1
movdqu [rsp+60h], xmm2
movdqu [rsp+70h], xmm3
mov ecx, 7
call Resolve
mov rcx, [rsp+20h]
mov rdx, [rsp+28h]
mov r8, [rsp+30h]
mov r9, [rsp+38h]
movdqu xmm0, [rsp+40h]
movdqu xmm1, [rsp+50h]
movdqu xmm2, [rsp+60h]
movdqu xmm3, [rsp+70h]
add rsp, 0A8h
jmp rax
Probe7 ENDP
Probe8 PROC FRAME
sub rsp, 0A8h
.allocstack 0A8h
.endprolog
mov [rsp+20h], rcx
mov [rsp+28h], rdx
mov [rsp+30h], r8
mov [rsp+38h], r9
movdqu [rsp+40h], xmm0
movdqu [rsp+50h], xmm1
movdqu [rsp+60h], xmm2
movdqu [rsp+70h], xmm3
mov ecx, 8
call Resolve
mov rcx, [rsp+20h]
mov rdx, [rsp+28h]
mov r8, [rsp+30h]
mov r9, [rsp+38h]
movdqu xmm0, [rsp+40h]
movdqu xmm1, [rsp+50h]
movdqu xmm2, [rsp+60h]
movdqu xmm3, [rsp+70h]
add rsp, 0A8h
jmp rax
Probe8 ENDP
Probe9 PROC FRAME
sub rsp, 0A8h
.allocstack 0A8h
.endprolog
mov [rsp+20h], rcx
mov [rsp+28h], rdx
mov [rsp+30h], r8
mov [rsp+38h], r9
movdqu [rsp+40h], xmm0
movdqu [rsp+50h], xmm1
movdqu [rsp+60h], xmm2
movdqu [rsp+70h], xmm3
mov ecx, 9
call Resolve
mov rcx, [rsp+20h]
mov rdx, [rsp+28h]
mov r8, [rsp+30h]
mov r9, [rsp+38h]
movdqu xmm0, [rsp+40h]
movdqu xmm1, [rsp+50h]
movdqu xmm2, [rsp+60h]
movdqu xmm3, [rsp+70h]
add rsp, 0A8h
jmp rax
Probe9 ENDP
Probe10 PROC FRAME
sub rsp, 0A8h
.allocstack 0A8h
.endprolog
mov [rsp+20h], rcx
mov [rsp+28h], rdx
mov [rsp+30h], r8
mov [rsp+38h], r9
movdqu [rsp+40h], xmm0
movdqu [rsp+50h], xmm1
movdqu [rsp+60h], xmm2
movdqu [rsp+70h], xmm3
mov ecx, 10
call Resolve
mov rcx, [rsp+20h]
mov rdx, [rsp+28h]
mov r8, [rsp+30h]
mov r9, [rsp+38h]
movdqu xmm0, [rsp+40h]
movdqu xmm1, [rsp+50h]
movdqu xmm2, [rsp+60h]
movdqu xmm3, [rsp+70h]
add rsp, 0A8h
jmp rax
Probe10 ENDP
Probe11 PROC FRAME
sub rsp, 0A8h
.allocstack 0A8h
.endprolog
mov [rsp+20h], rcx
mov [rsp+28h], rdx
mov [rsp+30h], r8
mov [rsp+38h], r9
movdqu [rsp+40h], xmm0
movdqu [rsp+50h], xmm1
movdqu [rsp+60h], xmm2
movdqu [rsp+70h], xmm3
mov ecx, 11
call Resolve
mov rcx, [rsp+20h]
mov rdx, [rsp+28h]
mov r8, [rsp+30h]
mov r9, [rsp+38h]
movdqu xmm0, [rsp+40h]
movdqu xmm1, [rsp+50h]
movdqu xmm2, [rsp+60h]
movdqu xmm3, [rsp+70h]
add rsp, 0A8h
jmp rax
Probe11 ENDP
Probe12 PROC FRAME
sub rsp, 0A8h
.allocstack 0A8h
.endprolog
mov [rsp+20h], rcx
mov [rsp+28h], rdx
mov [rsp+30h], r8
mov [rsp+38h], r9
movdqu [rsp+40h], xmm0
movdqu [rsp+50h], xmm1
movdqu [rsp+60h], xmm2
movdqu [rsp+70h], xmm3
mov ecx, 12
call Resolve
mov rcx, [rsp+20h]
mov rdx, [rsp+28h]
mov r8, [rsp+30h]
mov r9, [rsp+38h]
movdqu xmm0, [rsp+40h]
movdqu xmm1, [rsp+50h]
movdqu xmm2, [rsp+60h]
movdqu xmm3, [rsp+70h]
add rsp, 0A8h
jmp rax
Probe12 ENDP
Probe13 PROC FRAME
sub rsp, 0A8h
.allocstack 0A8h
.endprolog
mov [rsp+20h], rcx
mov [rsp+28h], rdx
mov [rsp+30h], r8
mov [rsp+38h], r9
movdqu [rsp+40h], xmm0
movdqu [rsp+50h], xmm1
movdqu [rsp+60h], xmm2
movdqu [rsp+70h], xmm3
mov ecx, 13
call Resolve
mov rcx, [rsp+20h]
mov rdx, [rsp+28h]
mov r8, [rsp+30h]
mov r9, [rsp+38h]
movdqu xmm0, [rsp+40h]
movdqu xmm1, [rsp+50h]
movdqu xmm2, [rsp+60h]
movdqu xmm3, [rsp+70h]
add rsp, 0A8h
jmp rax
Probe13 ENDP
Probe14 PROC FRAME
sub rsp, 0A8h
.allocstack 0A8h
.endprolog
mov [rsp+20h], rcx
mov [rsp+28h], rdx
mov [rsp+30h], r8
mov [rsp+38h], r9
movdqu [rsp+40h], xmm0
movdqu [rsp+50h], xmm1
movdqu [rsp+60h], xmm2
movdqu [rsp+70h], xmm3
mov ecx, 14
call Resolve
mov rcx, [rsp+20h]
mov rdx, [rsp+28h]
mov r8, [rsp+30h]
mov r9, [rsp+38h]
movdqu xmm0, [rsp+40h]
movdqu xmm1, [rsp+50h]
movdqu xmm2, [rsp+60h]
movdqu xmm3, [rsp+70h]
add rsp, 0A8h
jmp rax
Probe14 ENDP
Probe15 PROC FRAME
sub rsp, 0A8h
.allocstack 0A8h
.endprolog
mov [rsp+20h], rcx
mov [rsp+28h], rdx
mov [rsp+30h], r8
mov [rsp+38h], r9
movdqu [rsp+40h], xmm0
movdqu [rsp+50h], xmm1
movdqu [rsp+60h], xmm2
movdqu [rsp+70h], xmm3
mov ecx, 15
call Resolve
mov rcx, [rsp+20h]
mov rdx, [rsp+28h]
mov r8, [rsp+30h]
mov r9, [rsp+38h]
movdqu xmm0, [rsp+40h]
movdqu xmm1, [rsp+50h]
movdqu xmm2, [rsp+60h]
movdqu xmm3, [rsp+70h]
add rsp, 0A8h
jmp rax
Probe15 ENDP
Probe16 PROC FRAME
sub rsp, 0A8h
.allocstack 0A8h
.endprolog
mov [rsp+20h], rcx
mov [rsp+28h], rdx
mov [rsp+30h], r8
mov [rsp+38h], r9
movdqu [rsp+40h], xmm0
movdqu [rsp+50h], xmm1
movdqu [rsp+60h], xmm2
movdqu [rsp+70h], xmm3
mov ecx, 16
call Resolve
mov rcx, [rsp+20h]
mov rdx, [rsp+28h]
mov r8, [rsp+30h]
mov r9, [rsp+38h]
movdqu xmm0, [rsp+40h]
movdqu xmm1, [rsp+50h]
movdqu xmm2, [rsp+60h]
movdqu xmm3, [rsp+70h]
add rsp, 0A8h
jmp rax
Probe16 ENDP
END
