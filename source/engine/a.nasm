; "Build Engine & Tools" Copyright (c) 1993-1997 Ken Silverman
; Ken Silverman's official web site: "http://www.advsys.net/ken"
; See the included license file "BUILDLIC.TXT" for license info.
;
; This file has been modified from Ken Silverman's original release
; Specifically, it has been reformatted for NASM and functions have
; been changed to take parameters off the stack, since VC++ doesn't
; handle assembly calls so nicely. Also did some general rearranging
; to the setup functions, since there parameters are no longer in
; registers. Changed all function names to have underscore prefixes
; instead of suffixes.

;Warning: IN THIS FILE, ALL SEGMENTS ARE REMOVED.  THIS MEANS THAT DS:[]
;MUST BE ADDED FOR ALL SELF-MODIFIES FOR MASM TO WORK.
;
;WASM PROBLEMS:
;   1. Requires all scaled registers (*1,*2,*4,*8) to be last thing on line
;   2. Using 'DATA' is nice for self-mod. code, but WDIS only works with 'CODE'
;
;MASM PROBLEMS:
;   1. Requires DS: to be written out for self-modifying code to work
;   2. Doesn't encode short jumps automatically like WASM
;   3. Stupidly adds wait prefix to ffree's

EXTERN _asm1		; dword
EXTERN _asm2		; dword
EXTERN _asm3		; dword
EXTERN _asm4		; dword
EXTERN _reciptable	; near
EXTERN _fpuasm		; dword
EXTERN _globalx3	; dword
EXTERN _globaly3	; dword
EXTERN _ylookup		; near

EXTERN _vplce		; near
EXTERN _vince		; near
EXTERN _palookupoffse	; near
EXTERN _bufplce		; near

EXTERN _espbak		; dword  <-- Safe, because esp never actually leaves the stack

EXTERN _pow2char	; near
EXTERN _pow2long	; near

BITS 32
SECTION .data

ALIGN 16
GLOBAL _sethlinesizes
; long, long, long
_sethlinesizes:
	mov eax,[esp+12]
	mov [hoffs1+2], eax
	mov [hoffs2+2], eax
	mov [hoffs3+2], eax
	mov [hoffs4+2], eax
	mov [hoffs5+2], eax
	mov [hoffs6+2], eax
	mov [hoffs7+2], eax
	mov [hoffs8+2], eax

	mov cl,[esp+4]
	mov [machxbits1+2], cl
	mov [machxbits2+2], cl
	mov [machxbits3+2], cl
	neg cl
	mov [hxsiz1+2], cl
	mov [hxsiz2+2], cl
	mov [hxsiz3+2], cl
	mov [hxsiz4+2], cl
	mov [machnegxbits1+2], cl

	mov al,[esp+8]
	mov [hysiz1+3], al
	mov [hysiz2+3], al
	mov [hysiz3+3], al
	mov [hysiz4+3], al
	mov [hmach3a+2], al
	mov [hmach3b+2], al
	mov [hmach3c+2], al
	mov [hmach3d+2], al

	sub cl, al
	mov eax, -1
	shr eax, cl
	mov [hmach2a+1], eax
	mov [hmach2b+1], eax
	mov [hmach2c+1], eax
	mov [hmach2d+1], eax

	ret

ALIGN 16
GLOBAL _prosethlinesizes
; long, long, long
_prosethlinesizes:
	mov eax,[esp+4]
	mov ecx,[esp+12]
	mov [prohbuf-4], ecx
	neg eax
	mov ecx, eax
	sub eax, [esp+8]
	mov [prohshru-1], al	;bl = 32-al-bl
	mov eax, -1
	shr eax, cl
	mov ecx, [esp+8]
	shl eax, cl
	mov [prohand-4], eax	;((-1>>(-oal))<<obl)
	neg ecx
	mov [prohshrv-1], cl	;cl = 32-bl
	ret

ALIGN 16
GLOBAL _setvlinebpl
; long
_setvlinebpl:
	mov eax,[esp+4]
	mov [fixchain1a+2], eax
	mov [fixchain1b+2], eax
	mov [fixchain1m+2], eax
	mov [fixchain1t+2], eax
	mov [fixchain1s+2], eax
	mov [mfixchain1s+2], eax
	mov [tfixchain1s+2], eax
	mov [fixchain2a+2], eax
	mov [profixchain2a+2], eax
	mov [fixchain2ma+2], eax
	mov [fixchain2mb+2], eax
	mov [fixchaint2a+1], eax
	mov [fixchaint2b+2], eax
	mov [fixchaint2c+2], eax
	mov [fixchaint2d+2], eax
	mov [fixchaint2e+2], eax
	mov [vltpitch+2], eax		; [RH] for vlinetallasm4
	ret

ALIGN 16
GLOBAL _setpalookupaddress
; long
_setpalookupaddress:
	mov eax, [esp+4]
	mov [pal1+2], eax
	mov [pal2+2], eax
	mov [pal3+2], eax
	mov [pal4+2], eax
	mov [pal5+2], eax
	mov [pal6+2], eax
	mov [pal7+2], eax
	mov [pal8+2], eax
	ret

ALIGN 16
GLOBAL _prosetpalookupaddress
; long
_prosetpalookupaddress:
	mov eax, [esp+4]
	mov [prohpala-4], eax
	ret

ALIGN 16
GLOBAL _setuphlineasm4
; long long
_setuphlineasm4:
machxbits3:
	mov eax,[esp+4]
	mov edx,[esp+8]
	rol eax, 6			;xbits
	mov [hmach4a+2], eax
	mov [hmach4b+2], eax
	mov dl, al
	mov [hmach4c+2], eax
	mov [hmach4d+2], eax
	mov [hmach1a+2], edx
	mov [hmach1b+2], edx
	mov [hmach1c+2], edx
	mov [hmach1d+2], edx
	ret

	;Non-256-stuffed ceiling&floor method with NO SHLD!:
	;yinc&0xffffff00   lea eax, [edx+88888800h]   1     1/2
	;ybits...xbits     and edx, 0x88000088        1     1/2
	;ybits             rol edx, 6                 2     1/2
	;xinc<<xbits       add esi, 0x88888888        1     1/2
	;xinc>>(32-xbits)  adc al, 88h                1     1/2
	;bufplc            mov cl, [edx+0x88888888]   1     1/2
	;paloffs&255       mov bl, [ecx+0x88888888]   1     1/2
ALIGN 16
GLOBAL _hlineasm4
; long long long long long long
_hlineasm4:
	push ebx
	push esi
	push edi
	push ebp

	mov eax, [esp+20]
	mov ebx, [esp+24]
	mov ecx, [esp+28]
	mov edx, [esp+32]
	mov esi, [esp+36]
	mov edi, [esp+40]

	lea ebp, [eax+1]

	cmp ebp, 8
	jle near shorthline

	test edi, 1
	jnz short skipthe1byte

	mov eax, esi
hxsiz1:	shr eax, 26
hysiz1:	shld eax, edx, 6
hoffs1:	mov cl, [eax+0x88888888]
pal1:	mov bl, [ecx+0x88888888]
	sub esi, [_asm1]
	sub edx, [_asm2]
	mov [edi], bl
	dec edi
	dec ebp

skipthe1byte:
	test edi, 2
	jnz short skipthe2byte

	mov eax, esi
hxsiz2:	shr eax, 26
hysiz2:	shld eax, edx, 6
hoffs2:	mov cl, [eax+0x88888888]
pal2:	mov bh, [ecx+0x88888888]
	sub esi, [_asm1]
	sub edx, [_asm2]

	mov eax, esi
hxsiz3:	shr eax, 26
hysiz3:	shld eax, edx, 6
hoffs3:	mov cl, [eax+0x88888888]
pal3:	mov bl, [ecx+0x88888888]
	sub esi, [_asm1]
	sub edx, [_asm2]
	mov [edi-1], bx
	sub edi, 2
	sub ebp, 2

skipthe2byte:

	mov eax, esi
machxbits1: shl esi, 6                     ;xbits
machnegxbits1: shr eax, 32-6               ;32-xbits
	mov dl, al

	inc edi

	add ebx, ebx
	mov eax, edx
	jc beginhline64

	mov eax, [_asm1]
machxbits2: rol eax, 6                     ;xbits
	mov [hmach4a+2], eax
	mov [hmach4b+2], eax
	mov [hmach4c+2], eax
	mov [hmach4d+2], eax
	mov ebx, eax
	mov eax, [_asm2]
	mov al, bl
	mov [hmach1a+2], eax
	mov [hmach1b+2], eax
	mov [hmach1c+2], eax
	mov [hmach1d+2], eax

	mov eax, edx
	jmp beginhline64
ALIGN 16
prebeginhline64:
	mov [edi], ebx
beginhline64:

hmach3a: rol eax, 6
hmach2a: and eax, 00008888h
hmach4a: sub esi, 0x88888888
hmach1a: sbb edx, 0x88888888
	sub edi, 4
hoffs4: mov cl, [eax+0x88888888]
	mov eax, edx

hmach3b: rol eax, 6
hmach2b: and eax, 00008888h
hmach4b: sub esi, 0x88888888
hmach1b: sbb edx, 0x88888888
pal4:	mov bh, [ecx+0x88888888]
hoffs5:	mov cl, [eax+0x88888888]
	mov eax, edx

hmach3c: rol eax, 6
pal5:	mov bl, [ecx+0x88888888]
hmach2c: and eax, 00008888h
	shl ebx, 16
hmach4c: sub esi, 0x88888888
hmach1c: sbb edx, 0x88888888
hoffs6: mov cl, [eax+0x88888888]

	mov eax, edx
	;(

hmach3d: rol eax, 6
hmach2d: and eax, 00008888h
hmach4d: sub esi, 0x88888888
hmach1d: sbb edx, 0x88888888
pal6:	mov bh, [ecx+0x88888888]
hoffs7: mov cl, [eax+0x88888888]
	mov eax, edx
	sub ebp, 4
	nop
pal7:	mov bl, [ecx+0x88888888]
	jnc near prebeginhline64
skipthe4byte:

	test ebp, 2
	jz skipdrawthe2
	rol ebx, 16
	mov [edi+2], bx
	sub edi, 2
skipdrawthe2:
	test ebp, 1
	jz skipdrawthe1
	shr ebx, 24
	mov [edi+3], bl
skipdrawthe1:

	pop ebp
	ret

shorthline:
	test ebp, ebp
	jz endshorthline
begshorthline:
	mov eax, esi
hxsiz4: shr eax, 26
hysiz4: shld eax, edx, 6
hoffs8: mov cl, [eax+0x88888888]
pal8:	mov bl, [ecx+0x88888888]
	sub esi, [_asm1]
	sub edx, [_asm2]
	mov [edi], bl
	dec edi
	dec ebp
	jnz begshorthline
endshorthline:
	pop ebp
	pop edi
	pop esi
	pop ebx
	ret


	;eax: 00000000 00000000 00000000 temp----
	;ebx: 00000000 00000000 00000000 temp----
	;ecx: UUUUUUuu uuuuuuuu uuuuuuuu uuuuuuuu
	;edx: VVVVVVvv vvvvvvvv vvvvvvvv vvvvvvvv
	;esi: cnt----- -------- -------- --------
	;edi: vid----- -------- -------- --------
	;ebp: paloffs- -------- -------- --------
	;esp: ???????? ???????? ???????? ????????
ALIGN 16
GLOBAL _prohlineasm4
; long long long long long long
_prohlineasm4:
	push ebx
	push esi
	push edi
	push ebp

	mov eax, [esp+20]
	mov ebx, [esp+24]
	mov ecx, [esp+28]
	mov edx, [esp+32]
	mov esi, [esp+36]
	mov edi, [esp+40]

	lea ebp, [ecx+0x88888888]
prohpala:
	mov ecx, esi
	lea esi, [eax+1]
	sub edi, esi

prohbeg:
	mov eax, ecx
	shr eax, 20
prohshru:
	mov ebx, edx
	shr ebx, 26
prohshrv:
	and eax, 0x88888888
prohand:
	movzx eax, byte [eax+ebx+0x88888888]
prohbuf:
	mov al, [eax+ebp]
	sub ecx, [_asm1]
	sub edx, [_asm2]
	mov [edi+esi], al
	dec esi
	jnz prohbeg

	pop ebp
	pop edi
	pop esi
	pop ebx
	ret



ALIGN 16
GLOBAL _setupvlineasm
; long
_setupvlineasm:
	mov eax, [esp+4]
		;First 2 lines for VLINEASM1, rest for VLINEASM4
	mov [premach3a+2], al
	mov [mach3a+2], al

	push ecx
	mov [machvsh1+2], al      ;32-shy
	mov [machvsh3+2], al      ;32-shy
	mov [machvsh5+2], al      ;32-shy
	mov [machvsh6+2], al      ;32-shy
	mov ah, al
	sub ah, 16
	mov [machvsh8+2], ah      ;16-shy
	neg al
	mov [machvsh7+2], al      ;shy
	mov [machvsh9+2], al      ;shy
	mov [machvsh10+2], al     ;shy
	mov [machvsh11+2], al     ;shy
	mov [machvsh12+2], al     ;shy
	mov cl, al
	mov eax, 1
	shl eax, cl
	dec eax
	mov [machvsh2+2], eax    ;(1<<shy)-1
	mov [machvsh4+2], eax    ;(1<<shy)-1
	pop ecx
	ret

ALIGN 16
GLOBAL _prosetupvlineasm
; long
_prosetupvlineasm:
	mov eax, [esp+4]
		;First 2 lines for VLINEASM1, rest for VLINEASM4
	mov [premach3a+2], al
	mov [mach3a+2], al

	push ecx
	mov [promachvsh1+2], al      ;32-shy
	mov [promachvsh3+2], al      ;32-shy
	mov [promachvsh5+2], al      ;32-shy
	mov [promachvsh6+2], al      ;32-shy
	mov ah, al
	sub ah, 16
	mov [promachvsh8+2], ah      ;16-shy
	neg al
	mov [promachvsh7+2], al      ;shy
	mov [promachvsh9+2], al      ;shy
	mov [promachvsh10+2], al     ;shy
	mov [promachvsh11+2], al     ;shy
	mov [promachvsh12+2], al     ;shy
	mov cl, al
	mov eax, 1
	shl eax, cl
	dec eax
	mov [promachvsh2+2], eax    ;(1<<shy)-1
	mov [promachvsh4+2], eax    ;(1<<shy)-1
	pop ecx
	ret

ALIGN 16
GLOBAL _setupmvlineasm
; long
_setupmvlineasm:
	mov al, [esp+4]
	mov [maskmach3a+2], al
	mov [machmv13+2], al
	mov [machmv14+2], al
	mov [machmv15+2], al
	mov [machmv16+2], al
	ret

ALIGN 16
GLOBAL _setuptvlineasm
; long
_setuptvlineasm:
	mov al, [esp+4]
	mov [transmach3a+2], al
	ret

ALIGN 16
GLOBAL _prevlineasm1
; long long long long long long
_prevlineasm1:
	push ebx
	push esi
	push edi
	mov eax, [esp+16]
	mov ebx, [esp+20]
	mov ecx, [esp+24]
	mov edx, [esp+28]
	mov esi, [esp+32]
	mov edi, [esp+36]
	test ecx, ecx
	jnz fromprevlineasm1

	add eax, edx
premach3a: shr edx, 32
	mov dl, [esi+edx]
	mov cl, [ebx+edx]
	mov [edi], cl
	pop edi
	pop esi
	pop ebx
	ret

ALIGN 16
GLOBAL _vlineasm1
; long long long long long long
_vlineasm1:
	push ebx
	push esi
	push edi
	mov eax, [esp+16]
	mov ebx, [esp+20]
	mov ecx, [esp+24]
	mov edx, [esp+28]
	mov esi, [esp+32]
	mov edi, [esp+36]
fromprevlineasm1:
	push ebp
	mov ebp, ebx
	inc ecx
fixchain1a: sub edi, 320
beginvline:
	mov ebx, edx
	
mach3a: shr ebx, 32
fixchain1b: add edi, 320

	mov bl, [esi+ebx]
	add edx, eax
	
	dec ecx
	mov bl, [ebp+ebx]
	
	mov [edi], bl
	jnz short beginvline
	
	pop ebp
	pop edi
	pop esi
	pop ebx
	mov eax, edx
	ret

ALIGN 16
GLOBAL _mvlineasm1      ;Masked vline
; long long long long log long
_mvlineasm1:
	push ebx
	push esi
	push edi
	mov eax, [esp+16]
	mov ebx, [esp+20]
	mov ecx, [esp+24]
	mov edx, [esp+28]
	mov esi, [esp+32]
	mov edi, [esp+36]
	push ebp
	mov ebp, ebx
beginmvline:
	mov ebx, edx
maskmach3a: shr ebx, 32
	mov bl, [esi+ebx]
	cmp bl, 255
	je short skipmask1
maskmach3c: mov bl, [ebp+ebx]
	mov [edi], bl
skipmask1:
	add edx, eax
fixchain1m: add edi, 320
	sub ecx, 1
	jnc short beginmvline

	pop ebp
	pop edi
	pop esi
	pop ebx
	mov eax, edx
	ret

ALIGN 16
GLOBAL _fixtransluscence
; long
_fixtransluscence:
	mov eax, [esp+4]
	mov [transmach4+2], eax
	mov [tmach1+2], eax
	mov [tmach2+2], eax
	mov [tmach3+2], eax
	mov [tmach4+2], eax
	mov [tran2traa+2], eax
	mov [tran2trab+2], eax
	mov [tran2trac+2], eax
	mov [tran2trad+2], eax
	ret

ALIGN 16
GLOBAL _settransnormal
; (void)
_settransnormal:
	mov [transrev0+1], byte 0x83
	mov [transrev1+1], byte 0x27
	mov [transrev2+1], byte 0x3f
	mov [transrev3+1], byte 0x98
	mov [transrev4+1], byte 0x90
	mov [transrev5+1], byte 0x37
	mov [transrev6+1], byte 0x90
	mov [transrev7+0], word 0xf38a
	mov [transrev8+1], byte 0x90
	mov [transrev9+0], word 0xf78a
	mov [transrev10+1], byte 0xa7
	mov [transrev11+1], byte 0x81
	mov [transrev12+2], byte 0x9f
	mov [transrev13+0], word 0xdc88
	mov [transrev14+1], byte 0x81
	mov [transrev15+1], byte 0x9a
	mov [transrev16+1], byte 0xa7
	mov [transrev17+1], byte 0x82
	ret

ALIGN 16
GLOBAL _settransreverse
; (void)
_settransreverse:
	mov [transrev0+1], byte 0xa3
	mov [transrev1+1], byte 0x7
	mov [transrev2+1], byte 0x1f
	mov [transrev3+1], byte 0xb8
	mov [transrev4+1], byte 0xb0
	mov [transrev5+1], byte 0x17
	mov [transrev6+1], byte 0xb0
	mov [transrev7+0], word 0xd38a
	mov [transrev8+1], byte 0xb0
	mov [transrev9+0], word 0xd78a
	mov [transrev10+1], byte 0x87
	mov [transrev11+1], byte 0xa1
	mov [transrev12+2], byte 0x87
	mov [transrev13+0], word 0xe388
	mov [transrev14+1], byte 0xa1
	mov [transrev15+1], byte 0xba
	mov [transrev16+1], byte 0x87
	mov [transrev17+1], byte 0xa2
	ret

ALIGN 16
GLOBAL _tvlineasm1	;Masked & transluscent vline
; long long long long long long
_tvlineasm1:
	push ebx
	push esi
	push edi
	mov eax, [esp+16]
	mov ebx, [esp+20]
	mov ecx, [esp+24]
	mov edx, [esp+28]
	mov esi, [esp+32]
	mov edi, [esp+36]
	push ebp
	mov ebp, eax
	xor eax, eax
	inc ecx
	mov [transmach3c+2], ebx
	jmp short begintvline
ALIGN 16
begintvline:
	mov ebx, edx
transmach3a: shr ebx, 32
	mov bl, [esi+ebx]
	cmp bl, 255
	je short skiptrans1
transrev0:
transmach3c: mov al, [ebx+0x88888888]
transrev1:
	mov ah, [edi]
transmach4: mov al, [eax+0x88888888]   ;_transluc[eax]
	mov [edi], al
skiptrans1:
	add edx, ebp
fixchain1t: add edi, 320
	dec ecx
	jnz short begintvline

	pop ebp
	pop edi
	pop esi
	pop ebx
	mov eax, edx
	ret

	;eax: -------temp1-------
	;ebx: -------temp2-------
	;ecx:  dat  dat  dat  dat
	;edx: ylo2           ylo4
	;esi: yhi1           yhi2
	;edi: ---videoplc/cnt----
	;ebp: yhi3           yhi4
	;esp:
ALIGN 16
GLOBAL _vlineasm4
; long long
_vlineasm4:
	push ebx
	push esi
	push edi
	push ebp

	mov ecx, [esp+20]
	mov edi, [esp+24]

	mov eax, [_ylookup+ecx*4]
	add eax, edi
	mov [machvline4end+2], eax
	sub edi, eax

	mov eax, [_bufplce+0]
	mov ebx, [_bufplce+4]
	mov ecx, [_bufplce+8]
	mov edx, [_bufplce+12]
	mov [machvbuf1+2], ecx
	mov [machvbuf2+2], edx
	mov [machvbuf3+2], eax
	mov [machvbuf4+2], ebx

	mov eax, [_palookupoffse]
	mov ebx, [_palookupoffse+4]
	mov ecx, [_palookupoffse+8]
	mov edx, [_palookupoffse+12]
	mov [machvpal1+2], ecx
	mov [machvpal2+2], edx
	mov [machvpal3+2], eax
	mov [machvpal4+2], ebx

		;     +-------------------------------+
		;edx: |v3lo           |v1lo           |
		;     +-------------------------------+
		;esi: |v2hi  v2lo             |   v3hi|
		;     +-------------------------------+
		;ebp: |v0hi  v0lo             |   v1hi|
		;     +-------------------------------+

	mov ebp, [_vince]
	mov ebx, [_vince+4]
	mov esi, [_vince+8]
	mov eax, [_vince+12]
	and esi, 0xfffffe00
	and ebp, 0xfffffe00
machvsh9: rol eax, 0x88				;sh
machvsh10: rol ebx, 0x88			;sh
	mov edx, eax
	mov ecx, ebx
	shr ecx, 16
	and edx, 0xffff0000
	add edx, ecx
	and eax, 0x000001ff
	and ebx, 0x000001ff
	add esi, eax
	add ebp, ebx
	;
	mov eax, edx
	and eax, 0xffff0000
	mov [machvinc1+2], eax
	mov [machvinc2+2], esi
	mov [machvinc3+2], dl
	mov [machvinc4+2], dh
	mov [machvinc5+2], ebp

	mov ebp, [_vplce]
	mov ebx, [_vplce+4]
	mov esi, [_vplce+8]
	mov eax, [_vplce+12]
	and esi, 0xfffffe00
	and ebp, 0xfffffe00
machvsh11: rol eax, 0x88		;sh
machvsh12: rol ebx, 0x88		;sh
	mov edx, eax
	mov ecx, ebx
	shr ecx, 16
	and edx, 0xffff0000
	add edx, ecx
	and eax, 0x000001ff
	and ebx, 0x000001ff
	add esi, eax
	add ebp, ebx

	mov ecx, esi
	jmp short beginvlineasm4
ALIGN 16
	nop
	nop
	nop
beginvlineasm4:
machvsh1: shr ecx, 0x88			;32-sh
	mov ebx, esi
machvsh2: and ebx, 0x00000088		;(1<<sh)-1
machvinc1: add edx, 0x88880000
machvinc2: adc esi, 0x88888088
machvbuf1: mov cl, [ecx+0x88888888]
machvbuf2: mov bl, [ebx+0x88888888]
	mov eax, ebp
machvsh3: shr eax, 0x88          ;32-sh
machvpal1: mov cl, [ecx+0x88888888]
machvpal2: mov ch, [ebx+0x88888888]
	mov ebx, ebp
	shl ecx, 16
machvsh4: and ebx, 0x00000088    ;(1<<sh)-1
machvinc3: add dl, 0x88
machvbuf3: mov al, [eax+0x88888888]
machvinc4: adc dh, 0x88
machvbuf4: mov bl, [ebx+0x88888888]
machvinc5: adc ebp, 0x88888088
machvpal3: mov cl, [eax+0x88888888]
machvpal4: mov ch, [ebx+0x88888888]
machvline4end: mov [edi+0x88888888], ecx
fixchain2a: add edi, 0x88888888
	mov ecx, esi
	jnc short beginvlineasm4

		;     +-------------------------------+
		;edx: |v3lo           |v1lo           |
		;     +-------------------------------+
		;esi: |v2hi  v2lo             |   v3hi|
		;     +-------------------------------+
		;ebp: |v0hi  v0lo             |   v1hi|
		;     +-------------------------------+

	mov [_vplce+8], esi
	mov [_vplce], ebp
		;vplc2 = (esi<<(32-sh))+(edx>>sh)
		;vplc3 = (ebp<<(32-sh))+((edx&65535)<<(16-sh))
machvsh5: shl esi, 0x88		;32-sh
	mov eax, edx
machvsh6: shl ebp, 0x88		;32-sh
	and edx, 0x0000ffff
machvsh7: shr eax, 0x88		;sh
	add esi, eax
machvsh8: shl edx, 0x88		;16-sh
	add ebp, edx
	mov [_vplce+12], esi
	mov [_vplce+4], ebp

	pop ebp
	pop edi
	pop esi
	pop ebx
	ret

	;eax: -------temp1-------
	;ebx: -------temp2-------
	;ecx: ylo4      ---------
	;edx: ylo2      ---------
	;esi: yhi1           yhi2
	;edi: ---videoplc/cnt----
	;ebp: yhi3           yhi4
	;esp:
ALIGN 16
GLOBAL _provlineasm4
; long long
_provlineasm4:
	push ebx
	push esi
	push edi
	push ebp

	mov ecx,[esp+20]
	mov edi,[esp+24]

	mov eax, [_ylookup+ecx*4]
	add eax, edi
	mov [promachvline4end1+2], eax
	inc eax
	mov [promachvline4end2+2], eax
	inc eax
	mov [promachvline4end3+2], eax
	inc eax
	mov [promachvline4end4+2], eax
	sub eax, 3
	sub edi, eax

	mov eax, [_bufplce+0]
	mov ebx, [_bufplce+4]
	mov ecx, [_bufplce+8]
	mov edx, [_bufplce+12]
	mov [promachvbuf1+3], ecx
	mov [promachvbuf2+3], edx
	mov [promachvbuf3+3], eax
	mov [promachvbuf4+3], ebx

	mov eax, [_palookupoffse]
	mov ebx, [_palookupoffse+4]
	mov ecx, [_palookupoffse+8]
	mov edx, [_palookupoffse+12]
	mov [promachvpal1+2], ecx
	mov [promachvpal2+2], edx
	mov [promachvpal3+2], eax
	mov [promachvpal4+2], ebx

		;     +-------------------------------+
		;edx: |v3lo           |v1lo           |
		;     +-------------------------------+
		;esi: |v2hi  v2lo             |   v3hi|
		;     +-------------------------------+
		;ebp: |v0hi  v0lo             |   v1hi|
		;     +-------------------------------+

	mov ebp, [_vince]
	mov ebx, [_vince+4]
	mov esi, [_vince+8]
	mov eax, [_vince+12]
	and esi, 0xfffffe00
	and ebp, 0xfffffe00
promachvsh9: rol eax, 0x88		;sh
promachvsh10: rol ebx, 0x88		;sh
	mov edx, eax
	mov ecx, ebx
	shr ecx, 16
	and edx, 0xffff0000
	add edx, ecx
	and eax, 0x000001ff
	and ebx, 0x000001ff
	add esi, eax
	add ebp, ebx
	;
	mov eax, edx
	and eax, 0xffff0000
	mov [promachvinc1+2], eax
	mov [promachvinc2+2], esi
	shl edx, 16
	mov [promachvinc3+2], edx
	mov [promachvinc5+2], ebp

	mov ebp, [_vplce]
	mov ebx, [_vplce+4]
	mov esi, [_vplce+8]
	mov eax, [_vplce+12]
	and esi, 0xfffffe00
	and ebp, 0xfffffe00
promachvsh11: rol eax, 0x88		;sh
promachvsh12: rol ebx, 0x88		;sh
	mov edx, eax
	mov ecx, ebx
	shr ecx, 16
	and edx, 0xffff0000
	add edx, ecx
	and eax, 0x000001ff
	and ebx, 0x000001ff
	add esi, eax
	add ebp, ebx

	mov eax, esi
	mov ecx, edx
	shl ecx, 16
	jmp short probeginvlineasm4
ALIGN 16
	nop
	nop
	nop
probeginvlineasm4:
promachvsh1: shr eax, 0x88		;32-sh
	mov ebx, esi
promachvsh2: and ebx, 0x00000088	;(1<<sh)-1
promachvinc1: add edx, 0x88880000
promachvinc2: adc esi, 0x88888088
promachvbuf1: movzx eax, byte [eax+0x88888888]
promachvbuf2: movzx ebx, byte [ebx+0x88888888]
promachvpal1: mov al, [eax+0x88888888]
promachvline4end3: mov [edi+0x88888888], al
	mov eax, ebp
promachvsh3: shr eax, 0x88          ;32-sh
promachvpal2: mov bl, [ebx+0x88888888]
promachvline4end4: mov [edi+0x88888888], bl
	mov ebx, ebp
promachvsh4: and ebx, 0x00000088    ;(1<<sh)-1
promachvbuf3: movzx eax, byte [eax+0x88888888]
promachvinc3: add ecx, 0x88888888
promachvbuf4: movzx ebx, byte [ebx+0x88888888]
promachvinc5: adc ebp, 0x88888088
promachvpal3: mov al, [eax+0x88888888]
promachvline4end1: mov [edi+0x88888888], al
promachvpal4: mov bl, [ebx+0x88888888]
promachvline4end2: mov [edi+0x88888888], bl
profixchain2a: add edi, 0x88888888
	mov eax, esi
	jnc near probeginvlineasm4

		;     +-------------------------------+
		;edx: |v3lo           |v1lo           |
		;     +-------------------------------+
		;esi: |v2hi  v2lo             |   v3hi|
		;     +-------------------------------+
		;ebp: |v0hi  v0lo             |   v1hi|
		;     +-------------------------------+

	mov [_vplce+8], esi
	mov [_vplce], ebp
		;vplc2 = (esi<<(32-sh))+(edx>>sh)
		;vplc3 = (ebp<<(32-sh))+((edx&65535)<<(16-sh))
promachvsh5: shl esi, 0x88	;32-sh
	mov eax, edx
promachvsh6: shl ebp, 0x88	;32-sh
	and edx, 0x0000ffff
promachvsh7: shr eax, 0x88	;sh
	add esi, eax
promachvsh8: shl edx, 0x88	;16-sh
	add ebp, edx
	mov [_vplce+12], esi
	mov [_vplce+4], ebp

	pop ebp
	pop edi
	pop esi
	pop ebx
	ret


ALIGN 16
GLOBAL _mvlineasm4
; long long
_mvlineasm4:
	push ebx
	push esi
	push edi
	push ebp

	mov ecx,[esp+20]
	mov edi,[esp+24]

	mov eax, [_bufplce+0]
	mov ebx, [_bufplce+4]
	mov [machmv1+2], eax
	mov [machmv4+2], ebx
	mov eax, [_bufplce+8]
	mov ebx, [_bufplce+12]
	mov [machmv7+2], eax
	mov [machmv10+2], ebx

	mov eax, [_palookupoffse]
	mov ebx, [_palookupoffse+4]
	mov [machmv2+2], eax
	mov [machmv5+2], ebx
	mov eax, [_palookupoffse+8]
	mov ebx, [_palookupoffse+12]
	mov [machmv8+2], eax
	mov [machmv11+2], ebx

	mov eax, [_vince]        ;vince
	mov ebx, [_vince+4]
	xor al, al
	xor bl, bl
	mov [machmv3+2], eax
	mov [machmv6+2], ebx
	mov eax, [_vince+8]
	mov ebx, [_vince+12]
	mov [machmv9+2], eax
	mov [machmv12+2], ebx

	mov ebx, ecx
	mov ecx, [_vplce+0]
	mov edx, [_vplce+4]
	mov esi, [_vplce+8]
	mov ebp, [_vplce+12]
	mov cl, bl
	inc cl
	inc bh
	mov [_asm3], bh
fixchain2ma: sub edi, 320

	jmp short beginmvlineasm4
ALIGN 16
beginmvlineasm4:
	dec cl
	jz near endmvlineasm4
beginmvlineasm42:
	mov eax, ebp
	mov ebx, esi
machmv16: shr eax, 32
machmv15: shr ebx, 32
machmv12: add ebp, 0x88888888		;vince[3]
machmv9: add esi, 0x88888888		;vince[2]
machmv10: mov al, [eax+0x88888888]	;bufplce[3]
machmv7: mov bl, [ebx+0x88888888]	;bufplce[2]
	cmp al, 255
	adc dl, dl
	cmp bl, 255
	adc dl, dl
machmv8: mov bl, [ebx+0x88888888]	;palookupoffs[2]
machmv11: mov bh, [eax+0x88888888]	;palookupoffs[3]

	mov eax, edx
machmv14: shr eax, 32
	shl ebx, 16
machmv4: mov al, [eax+0x88888888]	;bufplce[1]
	cmp al, 255
	adc dl, dl
machmv6: add edx, 0x88888888		;vince[1]
machmv5: mov bh, [eax+0x88888888]	;palookupoffs[1]

	mov eax, ecx
machmv13: shr eax, 32
machmv3: add ecx, 0x88888888		;vince[0]
machmv1: mov al, [eax+0x88888888]	;bufplce[0]
	cmp al, 255
	adc dl, dl
machmv2: mov bl, [eax+0x88888888]	;palookupoffs[0]

	shl dl, 4
	xor eax, eax
fixchain2mb: add edi, 320
	mov al, dl
	add eax, mvcase0
	jmp eax		;16 byte cases

ALIGN 16
endmvlineasm4:
	dec byte [_asm3]
	jnz near beginmvlineasm42

	mov [_vplce], ecx
	mov [_vplce+4], edx
	mov [_vplce+8], esi
	mov [_vplce+12], ebp
	pop ebp
	pop edi
	pop esi
	pop ebx
	ret

	;5,7,8,8,11,13,12,14,11,13,14,14,12,14,15,7
ALIGN 16
mvcase0:
	jmp beginmvlineasm4
ALIGN 16
mvcase1:
	mov [edi], bl
	jmp beginmvlineasm4
ALIGN 16
mvcase2:
	mov [edi+1], bh
	jmp beginmvlineasm4
ALIGN 16
mvcase3:
	mov [edi], bx
	jmp beginmvlineasm4
ALIGN 16
mvcase4:
	shr ebx, 16
	mov [edi+2], bl
	jmp beginmvlineasm4
ALIGN 16
mvcase5:
	mov [edi], bl
	shr ebx, 16
	mov [edi+2], bl
	jmp beginmvlineasm4
ALIGN 16
	mvcase6:
	shr ebx, 8
	mov [edi+1], bx
	jmp beginmvlineasm4
ALIGN 16
mvcase7:
	mov [edi], bx
	shr ebx, 16
	mov [edi+2], bl
	jmp beginmvlineasm4
ALIGN 16
mvcase8:
	shr ebx, 16
	mov [edi+3], bh
	jmp beginmvlineasm4
ALIGN 16
mvcase9:
	mov [edi], bl
	shr ebx, 16
	mov [edi+3], bh
	jmp beginmvlineasm4
ALIGN 16
mvcase10:
	mov [edi+1], bh
	shr ebx, 16
	mov [edi+3], bh
	jmp beginmvlineasm4
ALIGN 16
mvcase11:
	mov [edi], bx
	shr ebx, 16
	mov [edi+3], bh
	jmp beginmvlineasm4
ALIGN 16
mvcase12:
	shr ebx, 16
	mov [edi+2], bx
	jmp beginmvlineasm4
ALIGN 16
mvcase13:
	mov [edi], bl
	shr ebx, 16
	mov [edi+2], bx
	jmp beginmvlineasm4
ALIGN 16
mvcase14:
	mov [edi+1], bh
	shr ebx, 16
	mov [edi+2], bx
	jmp beginmvlineasm4
ALIGN 16
mvcase15:
	mov [edi], ebx
	jmp beginmvlineasm4
	
ALIGN 16
GLOBAL _setupspritevline
; long long long long long long
_setupspritevline:
	mov eax, [esp+4]
	mov [spal+2], eax

	mov eax, [esp+20]		;xinc's
	shl eax, 16
	mov [smach1+2], eax
	mov [smach4+2], eax
	mov eax, [esp+20]
	sar eax, 16
	add eax, [esp+8]		;watch out with ebx - it's passed
	mov [smach2+2], eax
	add eax, [esp+16]
	mov [smach5+2], eax

	mov eax,[esp+12]		;yinc's
	mov [smach3+2], eax
	ret

ALIGN 16
GLOBAL _spritevline
; long long long long long
	;eax = 0, ebx = x, ecx = cnt, edx = y, esi = yplc, edi = p
_spritevline:
	push ebx
	push esi
	push edi

	xor eax,eax
	mov ebx,[esp+16]
	mov ecx,[esp+20]
	mov edx,[esp+24]
	mov esi,[esp+28]
	mov edi,[esp+32]
	jmp realsvline

prestartsvline:
smach1:	add ebx, 0x88888888		;xincshl16
	mov al, [esi]
smach2:	adc esi, 0x88888888		;xincshr16+yalwaysinc

startsvline:
spal:	mov al, [eax+0x88888888]	;palookup
	mov [edi], al
fixchain1s: add edi, 320

realsvline:
smach3: add edx, 0x88888888		;dayinc
	dec ecx
	ja short prestartsvline		;jump if (no carry (add)) and (not zero (dec))!
	jz short endsvline
smach4: add ebx, 0x88888888		;xincshl16
	mov al, [esi]
smach5: adc esi, 0x88888888		;xincshr16+yalwaysinc+daydime
	jmp short startsvline
endsvline:
	pop edi
	pop esi
	pop ebx
	ret

ALIGN 16
GLOBAL _msetupspritevline
; long long long long long
_msetupspritevline:
	mov eax, [esp+4]
	mov [mspal+2], eax

	mov eax, [esp+20]		;xinc's
	shl eax, 16
	mov [msmach1+2], eax
	mov [msmach4+2], eax
	mov eax, [esp+20]
	sar eax, 16
	add eax, [esp+8]		;watch out with ebx - it's passed
	mov [msmach2+2], eax
	add eax, [esp+16]
	mov [msmach5+2], eax

	mov eax, [esp+12]		;yinc's
	mov [msmach3+2], eax
	ret

ALIGN 16
GLOBAL _mspritevline
	;eax = 0, ebx = x, ecx = cnt, edx = y, esi = yplc, edi = p
_mspritevline:
	push ebx
	push esi
	push edi

	mov ebx,[esp+16]
	mov ecx,[esp+20]
	mov edx,[esp+24]
	mov esi,[esp+28]
	mov edi,[esp+32]
	jmp realmspritevline

mprestartsvline:
msmach1: add ebx, 0x88888888		;xincshl16
	mov al, [esi]
msmach2: adc esi, 0x88888888		;xincshr16+yalwaysinc

mstartsvline:
	cmp al, 255
	je short mskipsvline
mspal: mov al, [eax+0x88888888]		;palookup
	mov [edi], al
mskipsvline:
mfixchain1s: add edi, 320

realmspritevline:
msmach3: add edx, 0x88888888		;dayinc
	dec ecx
	ja short mprestartsvline	;jump if (no carry (add)) and (not zero (dec))!
	jz short mendsvline
msmach4: add ebx, 0x88888888		;xincshl16
	mov al, [esi]
msmach5: adc esi, 0x88888888		;xincshr16+yalwaysinc+daydime
	jmp short mstartsvline
mendsvline:
	pop edi
	pop esi
	pop ebx
	ret

ALIGN 16
GLOBAL _tsetupspritevline
; long long long long long
_tsetupspritevline:
	mov eax, [esp+4]
	mov [tspal+2], eax

	mov eax, [esp+20]					;xinc's
	shl eax, 16
	mov [tsmach1+2], eax
	mov [tsmach4+2], eax
	mov eax, [esp+20]
	sar eax, 16
	add eax, [esp+8]
	mov [tsmach2+2], eax
	add eax, [esp+16]
	mov [tsmach5+2], eax

	mov eax, [esp+12]					;yinc's
	mov [tsmach3+2], eax
	ret

ALIGN 16
GLOBAL _tspritevline
_tspritevline:
	;eax = 0, ebx = x, ecx = cnt, edx = y, esi = yplc, edi = p
	push ebx
	push esi
	push edi
	push ebp

	xor eax, eax
	xor ebx, ebx
	mov ebp, [esp+20]	; mov ebp, ebx
	mov ecx, [esp+24]
	mov edx, [esp+28]
	mov esi, [esp+32]
	mov edi, [esp+36]

	jmp tenterspritevline
ALIGN 16
tprestartsvline:
tsmach1: add ebp, 0x88888888		;xincshl16
	mov al, [esi]
tsmach2: adc esi, 0x88888888		;xincshr16+yalwaysinc

tstartsvline:
	cmp al, 255
	je short tskipsvline
transrev2:
	mov bh, [edi]
transrev3:
tspal:	mov bl, [eax+0x88888888]	;palookup
tmach4: mov al, [ebx+0x88888888]	;_transluc
	mov [edi], al
tskipsvline:
tfixchain1s: add edi, 320

tenterspritevline:
tsmach3: add edx, 0x88888888		;dayinc
	dec ecx
	ja short tprestartsvline	;jump if (no carry (add)) and (not zero (dec))!
	jz short tendsvline
tsmach4: add ebp, 0x88888888		;xincshl16
	mov al, [esi]
tsmach5: adc esi, 0x88888888		;xincshr16+yalwaysinc+daydime
	jmp short tstartsvline
tendsvline:
	pop ebp
	pop edi
	pop esi
	pop ebx
	ret

ALIGN 16
GLOBAL _msethlineshift
; long long
_msethlineshift:
	mov al,[esp+4]
	mov ah,[esp+8]
	neg al
	mov [msh1d+2], al
	mov [msh2d+3], ah
	mov [msh3d+2], al
	mov [msh4d+3], ah
	mov [msh5d+2], al
	mov [msh6d+3], ah
	ret

ALIGN 16
GLOBAL _mhline
_mhline:
	;_asm1 = bxinc
	;_asm2 = byinc
	;_asm3 = shadeoffs
	;eax = picoffs
	;ebx = bx
	;ecx = cnt
	;edx = ?
	;esi = by
	;edi = p

	push ebx
	push esi
	push edi
	mov eax, [esp+16]
	mov [mmach1d+2], eax
	mov [mmach5d+2], eax
	mov [mmach9d+2], eax
	mov eax, [_asm3]
	mov [mmach2d+2], eax
	mov [mmach2da+2], eax
	mov [mmach2db+2], eax
	mov [mmach6d+2], eax
	mov [mmach10d+2], eax
	mov eax, [_asm1]
	mov [mmach3d+2], eax
	mov [mmach7d+2], eax
	mov eax, [_asm2]
	mov edx, [esp+28]
	mov [mmach4d+2], eax
	mov [mmach8d+2], eax
	jmp short mhlineskipmodifygot
; 0 1 2 3 eax ebx ecx edx esi edi

ALIGN 16
GLOBAL _mhlineskipmodify
_mhlineskipmodify:

	push ebx
	push esi
	push edi
	mov edx, [esp+28]
mhlineskipmodifygot:
	mov ecx, [esp+24]
	mov ebx, [esp+20]
	mov esi, [esp+32]
	mov edi, [esp+36]
	push ebp

	xor eax, eax
	mov ebp, ebx

	test ecx, 00010000h
	jnz short mbeghline

msh1d:	shr ebx, 26
msh2d:	shld ebx, esi, 6
	add ebp, [_asm1]
mmach9d: mov al, [ebx+0x88888888]		;picoffs
	add esi, [_asm2]
	cmp al, 255
	je mskip5

	mmach10d: mov cl, [eax+0x88888888]	;shadeoffs
	mov [edi], cl
mskip5:
	inc edi
	sub ecx, 65536
	jc near mendhline
	jmp short mbeghline

ALIGN 16
mpreprebeghline:				;1st only
	mov al, cl
mmach2d: mov al, [eax+0x88888888]		;shadeoffs
	mov [edi], al

mprebeghline:
	add edi, 2
	sub ecx, 131072
	jc near mendhline
mbeghline:
mmach3d: lea ebx, [ebp+0x88888888]		;bxinc
msh3d:	shr ebp, 26
msh4d:	shld ebp, esi, 6
mmach4d: add esi, 0x88888888			;byinc
mmach1d: mov cl, [ebp+0x88888888]		;picoffs
mmach7d: lea ebp, [ebx+0x88888888]		;bxinc

msh5d:	shr ebx, 26
msh6d:	shld ebx, esi, 6
mmach8d: add esi, 0x88888888			;byinc
mmach5d: mov ch, [ebx+0x88888888]		;picoffs

	cmp cl, 255
	je short mskip1
	cmp ch, 255
	je short mpreprebeghline

	mov al, cl				;BOTH
mmach2da: mov bl, [eax+0x88888888]	;shadeoffs
	mov al, ch
mmach2db: mov bh, [eax+0x88888888]	;shadeoffs
	mov [edi], bx
	add edi, 2
	sub ecx, 131072
	jnc short mbeghline
	jmp mendhline
mskip1:						;2nd only
	cmp ch, 255
	je short mprebeghline

	mov al, ch
mmach6d: mov al, [eax+0x88888888]	;shadeoffs
	mov [edi+1], al
	add edi, 2
	sub ecx, 131072
	jnc short mbeghline
mendhline:

	pop ebp
	pop edi
	pop esi
	pop ebx
	ret

ALIGN 16
GLOBAL _tsethlineshift
; long long
_tsethlineshift:
	mov al, [esp+4]
	mov ah, [esp+8]
	neg al
	mov [tsh1d+2], al
	mov [tsh2d+3], ah
	mov [tsh3d+2], al
	mov [tsh4d+3], ah
	mov [tsh5d+2], al
	mov [tsh6d+3], ah
	ret

ALIGN 16
GLOBAL _thline
_thline:
	;_asm1 = bxinc
	;_asm2 = byinc
	;_asm3 = shadeoffs
	;eax = picoffs
	;ebx = bx
	;ecx = cnt
	;edx = ?
	;esi = by
	;edi = p

	mov eax, [esp+4]
	mov ecx, [esp+12]
	mov [tmach1d+2], eax
	mov [tmach5d+2], eax
	mov [tmach9d+2], eax
	mov eax, [_asm3]
	mov [tmach2d+2], eax
	mov [tmach6d+2], eax
	mov [tmach10d+2], eax
	mov eax, [_asm1]
	mov [tmach3d+2], eax
	mov [tmach7d+2], eax
	mov eax, [_asm2]
	mov edx, [esp+16]
	mov [tmach4d+2], eax
	mov [tmach8d+2], eax
	jmp thlineskipmodifygot

ALIGN 16
GLOBAL _thlineskipmodify
_thlineskipmodify:

	mov ecx, [esp+12]
	mov edx, [esp+16]
thlineskipmodifygot
	push ebx
	push esi
	push edi
	push ebp

	xor eax, eax
	xor edx, edx
	mov ebp, [esp+24]
	mov ebx, [esp+24]
	mov esi, [esp+36]
	mov edi, [esp+40]

	test ecx, 0x00010000
	jnz short tbeghline

tsh1d:	shr ebx, 26
tsh2d:	shld ebx, esi, 6
	add ebp, [_asm1]
tmach9d: mov al, [ebx+0x88888888]	;picoffs
	add esi, [_asm2]
	cmp al, 255
	je tskip5

transrev4:
tmach10d: mov dl, [eax+0x88888888]	;shadeoffs
transrev5:
	mov dh, [edi]
tmach1: mov al, [edx+0x88888888]	;_transluc
	mov [edi], al
tskip5:
	inc edi
	sub ecx, 65536
	jc near tendhline
	jmp short tbeghline

ALIGN 16
tprebeghline:
	add edi, 2
	sub ecx, 131072
	jc short tendhline
tbeghline:
tmach3d: lea ebx, [ebp+0x88888888]		;bxinc
tsh3d: shr ebp, 26
tsh4d: shld ebp, esi, 6
tmach4d: add esi, 0x88888888			;byinc
tmach1d: mov cl, [ebp+0x88888888]	;picoffs
tmach7d: lea ebp, [ebx+0x88888888]		;bxinc

tsh5d: shr ebx, 26
tsh6d: shld ebx, esi, 6
tmach8d: add esi, 0x88888888			;byinc
tmach5d: mov ch, [ebx+0x88888888]	;picoffs

	cmp cx, 0xffff
	je short tprebeghline

	mov bx, [edi]

	cmp cl, 255
	je short tskip1
	mov al, cl
transrev6:
tmach2d: mov dl, [eax+0x88888888]	;shadeoffs
transrev7:
	mov dh, bl
tmach2: mov al, [edx+0x88888888]	;_transluc
	mov [edi], al

	cmp ch, 255
	je short tskip2
tskip1:
	mov al, ch
transrev8:
tmach6d: mov dl, [eax+0x88888888]	;shadeoffs
transrev9:
	mov dh, bh
tmach3: mov al, [edx+0x88888888]	;_transluc
	mov [edi+1], al
tskip2:

	add edi, 2
	sub ecx, 131072
	jnc tbeghline
tendhline:

	pop ebp
	pop edi
	pop esi
	pop ebx
	ret


	;eax=shiftval, ebx=palookup1, ecx=palookup2
ALIGN 16
GLOBAL _setuptvlineasm2
_setuptvlineasm2:
	mov eax,[esp+4]
	mov edx,[esp+8]
	mov [tran2shra+2], al
	mov [tran2shrb+2], al
	mov [tran2pala+2], edx
	mov eax,[esp+12]
	mov [tran2palb+2], eax
	mov [tran2palc+2], edx
	mov [tran2pald+2], eax
	ret

	;Pass:   eax=vplc2, ebx=vinc1, ecx=bufplc1, edx=bufplc2, esi=vplc1, edi=p
	;        _asm1=vinc2, _asm2=pend
	;Return: _asm1=vplc1, _asm2=vplc2
ALIGN 16
GLOBAL _tvlineasm2
_tvlineasm2:
	push ebx
	push esi
	push edi
	push ebp

	mov ebp, [esp+20]	; was mov ebp, eax
	mov ebx, [esp+24]
	mov ecx, [esp+28]
	mov edx, [esp+32]
	mov esi, [esp+36]
	mov edi, [esp+40]

	mov [tran2inca+2], ebx
	mov eax, [_asm1]
	mov [tran2incb+2], eax

	mov [tran2bufa+2], ecx         ;bufplc1
	mov [tran2bufb+2], edx         ;bufplc2

	mov eax, [_asm2]
	sub edi, eax
	mov [tran2edia+3], eax
	mov [tran2edic+2], eax
	inc eax
	mov [tran2edie+2], eax
fixchaint2a: sub eax, 320
	mov [tran2edif+2], eax
	dec eax
	mov [tran2edib+3], eax
	mov [tran2edid+2], eax

	xor ecx, ecx
	xor edx, edx
	jmp short begintvline2

		;eax 0000000000  temp  temp
		;ebx 0000000000 odat2 odat1
		;ecx 0000000000000000 ndat1
		;edx 0000000000000000 ndat2
		;esi          vplc1
		;edi videoplc--------------
		;ebp          vplc2

ALIGN 16
		;LEFT ONLY
skipdraw2:
transrev10:
tran2edic: mov ah, [edi+0x88888888]		;getpixel
transrev11:
tran2palc: mov al, [ecx+0x88888888]		;palookup1
fixchaint2d: add edi, 320
tran2trac: mov bl, [eax+0x88888888]		;_transluc
tran2edid: mov [edi+0x88888888-320], bl		;drawpixel
	jnc short begintvline2
	jmp endtvline2

skipdraw1:
	cmp dl, 255
	jne short skipdraw3
fixchaint2b: add edi, 320
	jc short endtvline2

begintvline2:
	mov eax, esi
tran2shra: shr eax, 88h			;globalshift
	mov ebx, ebp
tran2shrb: shr ebx, 88h			;globalshift
tran2inca: add esi, 0x88888888		;vinc1
tran2incb: add ebp, 0x88888888		;vinc2
tran2bufa: mov cl, [eax+0x88888888]	;bufplc1
	cmp cl, 255
tran2bufb: mov dl, [ebx+0x88888888]	;bufplc2
	je short skipdraw1
	cmp dl, 255
	je short skipdraw2

	;mov ax		The transluscent reverse of both!
	;mov bl, ah
	;mov ah
	;mov bh

		;BOTH
transrev12:
tran2edia: mov bx, [edi+0x88888888]	;getpixels
transrev13:
	mov ah, bl
transrev14:
tran2pala: mov al, [ecx+0x88888888]	;palookup1
transrev15:
tran2palb: mov bl, [edx+0x88888888]	;palookup2
fixchaint2c: add edi, 320
tran2traa: mov al, [eax+0x88888888]	;_transluc
tran2trab: mov ah, [ebx+0x88888888]	;_transluc
tran2edib: mov [edi+0x88888888-320], ax	;drawpixels
	jnc short begintvline2
	jmp short endtvline2

	;RIGHT ONLY
skipdraw3:
transrev16:
tran2edie: mov ah, [edi+88888889h]	;getpixel
transrev17:
tran2pald: mov al, [edx+0x88888888]	;palookup2
fixchaint2e: add edi, 320
tran2trad: mov bl, [eax+0x88888888]	;_transluc
tran2edif: mov [edi+88888889h-320], bl	;drawpixel
	jnc short begintvline2

endtvline2:
	mov [_asm1], esi
	mov [_asm2], ebp

	pop ebp
	pop edi
	pop esi
	pop ebx
	ret


%define BITSOFPRECISION 3
%define BITSOFPRECISIONPOW 8

;Double-texture mapping with palette lookup
;eax:  ylo1------------|----dat|----dat
;ebx:  ylo2--------------------|----cnt
;ecx:  000000000000000000000000|---temp
;edx:  xhi1-xlo1---------------|---yhi1
;esi:  xhi2-xlo2---------------|---yhi2
;edi:  ------------------------videopos
;ebp:  ----------------------------temp

%if 0		; ---------unused------------
ALIGN 16
GLOBAL setupslopevlin2_
setupslopevlin2_:
	mov [slop3+2], edx	;ptr
	mov [slop7+2], edx	;ptr
	mov [slop4+2], esi	;tptr
	mov [slop8+2], esi	;tptr
	mov [slop2+2], ah	;ybits
	mov [slop6+2], ah	;ybits
	mov [slop9+2], edi	;pinc

	mov edx, 1
	mov cl, al
	add cl, ah
	shl edx, cl
	dec edx
	mov cl, ah
	ror edx, cl

	mov [slop1+2], edx   ;ybits...xbits
	mov [slop5+2], edx   ;ybits...xbits

	ret

ALIGN 16
GLOBAL slopevlin2_
slopevlin2_:
	push ebp
	xor ecx, ecx

slopevlin2begin:
	mov ebp, edx
slop1:	and ebp, 0x88000088		;ybits...xbits
slop2:	rol ebp, 6			;ybits
	add eax, [_asm1]		;xinc1<<xbits
	adc edx, [_asm2]		;(yinc1&0xffffff00)+(xinc1>>(32-xbits))
slop3:	mov cl, [ebp+0x88888888]	;bufplc

	mov ebp, esi
slop4:	mov al, [ecx+0x88888888]	;paloffs
slop5:	and ebp, 0x88000088		;ybits...xbits
slop6:	rol ebp, 6			;ybits
	add ebx, [_asm3]		;xinc2<<xbits
slop7:	mov cl, [ebp+0x88888888]	;bufplc
	adc esi, [_asm4]		;(yinc2&0xffffff00)+(xinc2>>(32-xbits))
slop8:	mov ah, [ecx+0x88888888]	;paloffs

	dec bl
	mov [edi], ax
slop9:	lea edi, [edi+0x88888888]	;pinc
	jnz short slopevlin2begin

	pop ebp
	mov eax, edi
	ret
%endif	; ---0---

ALIGN 16
GLOBAL _setupslopevlin
; long long long
_setupslopevlin:
	fild dword [_asm1]
	mov eax,[esp+8]
	mov ecx,[esp+12]
	mov [slopmach3+3], eax		;ptr
	mov [slopmach5+2], ecx		;pinc
	neg ecx
	mov [slopmach6+2], ecx		;-pinc

	mov eax,[esp+4]
	mov edx, 1
	mov cl, al
	shl edx, cl
	dec edx
	mov cl, ah
	shl edx, cl
	mov [slopmach7+2], edx

	neg ah
	mov [slopmach2+2], ah

	sub ah, al
	mov [slopmach1+2], ah

	fstp dword [_asm2]
	ret

ALIGN 16
GLOBAL _slopevlin
; long long long long long long
_slopevlin:
	push ebx
	push esi
	push edi
	push ebp
	mov [_espbak], esp

	mov eax, [esp+20]
	mov ebx, [esp+24]
	mov ecx, [esp+28]
	mov edx, [esp+32]
	mov esi, [esp+36]
	mov edi, [esp+40]

	sub ecx, esp
	mov [slopmach4+3], ecx

	fild dword [_asm3]
slopmach6: lea ebp, [eax+0x88888888]
	fadd dword [_asm2]

	mov [_asm1], ebx
	shl ebx, 3

	mov eax, [_globalx3]
	mov ecx, [_globaly3]
	imul eax, ebx
	imul ecx, ebx
	add esi, eax
	add edi, ecx

	mov ebx, edx
	jmp short bigslopeloop
ALIGN 16
bigslopeloop:
	fst dword [_fpuasm]

	mov eax, [_fpuasm]
	add eax, eax
	sbb edx, edx
	mov ecx, eax
	shr ecx, 24
	and eax, 0x00ffe000
	shr eax, 11
	sub cl, 2
	mov eax, [eax+_reciptable]
	shr eax, cl
	xor eax, edx
	mov edx, [_asm1]
	mov ecx, [_globalx3]
	mov [_asm1], eax
	sub eax, edx
	mov edx, [_globaly3]
	imul ecx, eax
	imul eax, edx

	fadd dword [_asm2]

	cmp ebx, BITSOFPRECISIONPOW
	mov [_asm4], ebx
	mov cl, bl
	jl short slopeskipmin
	mov cl, BITSOFPRECISIONPOW
slopeskipmin:

;eax: yinc.............
;ebx:   0   0   0   ?
;ecx: xinc......... cnt
;edx:         ?
;esi: xplc.............
;edi: yplc.............
;ebp:     videopos

	mov ebx, esi
	mov edx, edi

beginnerslopeloop:
slopmach1: shr ebx, 20
	add esi, ecx
slopmach2: shr edx, 26
slopmach7: and ebx, 0x88888888
	add edi, eax
slopmach5: add ebp, 0x88888888		;pinc
slopmach3: mov dl, [ebx+edx+0x88888888]	;ptr
slopmach4: mov ebx, [esp+0x88888888]
	sub esp, 4
	dec cl
	mov al, [ebx+edx]		;tptr
	mov ebx, esi
	mov [ebp], al
	mov edx, edi
	jnz short beginnerslopeloop

	mov ebx, [_asm4]
	sub ebx, BITSOFPRECISIONPOW
	jg near bigslopeloop

	ffree st0

	mov esp, [_espbak]
	pop ebp
	pop edi
	pop esi
	pop ebx
	ret


ALIGN 16
GLOBAL _setuprhlineasm4
; long long long long long (long)
_setuprhlineasm4:
	mov eax,[esp+4]
	mov [rmach1a+2], eax
	mov [rmach1b+2], eax
	mov [rmach1c+2], eax
	mov [rmach1d+2], eax
	mov [rmach1e+2], eax

	mov eax,[esp+8]
	mov [rmach2a+2], eax
	mov [rmach2b+2], eax
	mov [rmach2c+2], eax
	mov [rmach2d+2], eax
	mov [rmach2e+2], eax

	mov eax,[esp+12]
	mov [rmach3a+2], eax
	mov [rmach3b+2], eax
	mov [rmach3c+2], eax
	mov [rmach3d+2], eax
	mov [rmach3e+2], eax

	mov eax,[esp+16]
	mov [rmach4a+2], eax
	mov [rmach4b+2], eax
	mov [rmach4c+2], eax
	mov [rmach4d+2], eax
	mov [rmach4e+2], eax

	mov eax,[esp+20]
	mov [rmach5a+2], eax
	mov [rmach5b+2], eax
	mov [rmach5c+2], eax
	mov [rmach5d+2], eax
	mov [rmach5e+2], eax
	ret

	;Non power of 2, non masking, with palookup method #1 (6 clock cycles)
	;eax: dat dat dat dat
	;ebx:          bufplc
	;ecx:  0          dat
	;edx:  xlo
	;esi:  ylo
	;edi:  videopos/cnt
	;ebp:  tempvar
	;esp:
ALIGN 16
GLOBAL _rhlineasm4
_rhlineasm4:
	push ebx
	push esi
	push edi
	push ebp

	mov eax, [esp+20]
	mov ebx, [esp+24]
	mov ecx, [esp+28]
	mov edx, [esp+32]
	mov esi, [esp+36]
	mov edi, [esp+40]

	cmp eax, 0
	jle near endrhline

	lea ebp, [edi-4]
	sub ebp, eax
	mov [rmach6a+2], ebp
	add ebp, 3
	mov [rmach6b+2], ebp
	mov edi, eax
	test edi, 3
	jz short begrhline
	jmp short startrhline1

ALIGN 16
startrhline1:
	mov cl, [ebx]			;bufplc
rmach1e: sub edx, 0x88888888		;xlo
	sbb ebp, ebp
rmach2e: sub esi, 0x88888888		;ylo
rmach3e: sbb ebx, 0x88888888		;xhi*tilesizy + yhi+ycarry
rmach4e: mov al, [ecx+0x88888888]	;palookup
rmach5e: and ebp, 0x88888888		;tilesizy
rmach6b: mov [edi+0x88888888], al	;vidcntoffs
	sub ebx, ebp
	dec edi
	test edi, 3
	jnz short startrhline1
	test edi, edi
	jz near endrhline

begrhline:
	mov cl, [ebx]			;bufplc
rmach1a: sub edx, 0x88888888		;xlo
	sbb ebp, ebp
rmach2a: sub esi, 0x88888888		;ylo
rmach3a: sbb ebx, 0x88888888		;xhi*tilesizy + yhi+ycarry
rmach5a: and ebp, 0x88888888		;tilesizy
	sub ebx, ebp

rmach1b: sub edx, 0x88888888		;xlo
	sbb ebp, ebp
rmach4a: mov ah, [ecx+0x88888888]	;palookup
	mov cl, [ebx]			;bufplc
rmach2b: sub esi, 0x88888888		;ylo
rmach3b: sbb ebx, 0x88888888		;xhi*tilesizy + yhi+ycarry
rmach5b: and ebp, 0x88888888		;tilesizy
rmach4b: mov al, [ecx+0x88888888]	;palookup
	sub ebx, ebp

	shl eax, 16

	mov cl, [ebx]			;bufplc
rmach1c: sub edx, 0x88888888		;xlo
	sbb ebp, ebp
rmach2c: sub esi, 0x88888888		;ylo
rmach3c: sbb ebx, 0x88888888		;xhi*tilesizy + yhi+ycarry
rmach5c: and ebp, 0x88888888		;tilesizy
	sub ebx, ebp

rmach1d: sub edx, 0x88888888		;xlo
	sbb ebp, ebp
rmach4c: mov ah, [ecx+0x88888888]	;palookup
	mov cl, [ebx]			;bufplc
rmach2d: sub esi, 0x88888888		;ylo
rmach3d: sbb ebx, 0x88888888		;xhi*tilesizy + yhi+ycarry
rmach5d: and ebp, 0x88888888		;tilesizy
rmach4d: mov al, [ecx+0x88888888]	;palookup
	sub ebx, ebp

rmach6a: mov [edi+0x88888888], eax	;vidcntoffs
	sub edi, 4
	jnz near begrhline
endrhline:
	pop ebp
	pop edi
	pop esi
	pop ebx
	ret

ALIGN 16
GLOBAL _setuprmhlineasm4
; long long long long long (long)
_setuprmhlineasm4:
	mov eax, [esp+4]
	mov [rmmach1+2], eax
	mov eax, [esp+8]
	mov [rmmach2+2], eax
	mov eax, [esp+12]
	mov [rmmach3+2], eax
	mov eax, [esp+16]
	mov [rmmach4+2], eax
	mov eax, [esp+20]
	mov [rmmach5+2], eax
	ret

ALIGN 16
GLOBAL _rmhlineasm4
; long long long long long long
_rmhlineasm4:
	mov eax, [esp+4]
	mov ecx, [esp+12]
	cmp eax, 0
	jle short endrmhline
	push ebx
	push esi
	push edi
	push ebp
	mov ebx, [esp+24]
	mov edx, [esp+32]
	mov esi, [esp+36]
	mov edi, [esp+40]

	lea ebp, [edi-1]
	sub ebp, eax
	mov [rmmach6+2], ebp
	mov edi, eax
	jmp short begrmhline

ALIGN 16
begrmhline:
	mov cl, [ebx]			;bufplc
rmmach1: sub edx, 0x88888888		;xlo
	sbb ebp, ebp
rmmach2: sub esi, 0x88888888		;ylo
rmmach3: sbb ebx, 0x88888888		;xhi*tilesizy + yhi+ycarry
rmmach5: and ebp, 0x88888888		;tilesizy
	cmp cl, 255
	je short rmskip
rmmach4: mov al, [ecx+0x88888888]	;palookup
rmmach6: mov [edi+0x88888888], al	;vidcntoffs
rmskip:
	sub ebx, ebp
	dec edi
	jnz short begrmhline
	pop ebp
	pop edi
	pop esi
	pop ebx
endrmhline:
	ret

ALIGN 16
GLOBAL _setupqrhlineasm4
; (long) long long long (long) (long)
_setupqrhlineasm4:
	mov eax, [esp+8]
	mov ecx, [esp+12]
	mov [qrmach2e+2], eax
	mov [qrmach3e+2], ecx
	xor edi, edx
	sub edx, ecx
	mov [qrmach7a+2], edx
	mov [qrmach7b+2], edx

	add eax, eax
	adc ecx, ecx
	mov [qrmach2a+2], eax
	mov [qrmach2b+2], eax
	mov [qrmach3a+2], ecx
	mov [qrmach3b+2], ecx

	mov edx, [esp+16]
	mov [qrmach4a+2], edx
	mov [qrmach4b+2], edx
	mov [qrmach4c+2], edx
	mov [qrmach4d+2], edx
	mov [qrmach4e+2], edx
	ret

	;Non power of 2, non masking, with palookup method (FASTER BUT NO SBB'S)
	;eax: dat dat dat dat
	;ebx:          bufplc
	;ecx:  0          dat
	;edx:  0          dat
	;esi:  ylo
	;edi:  videopos/cnt
	;ebp:  ?
	;esp:
ALIGN 16
GLOBAL _qrhlineasm4	;4 pixels in 9 cycles!  2.25 cycles/pixel
; long long long long long long
_qrhlineasm4:
	push ebx
	push esi
	push edi
	push ebp

	mov eax, [esp+20]
	mov ebx, [esp+24]
	mov ecx, [esp+28]
	mov edx, [esp+32]
	mov esi, [esp+36]
	mov edi, [esp+40]

	cmp eax, 0
	jle near endqrhline

	mov ebp, eax
	test ebp, 3
	jz short skipqrhline1
	jmp short startqrhline1

ALIGN 16
startqrhline1:
	mov cl, [ebx]			;bufplc
	dec edi
qrmach2e: sub esi, 0x88888888		;ylo
	dec ebp
qrmach3e: sbb ebx, 0x88888888		;xhi*tilesizy + yhi+ycarry
qrmach4e: mov al, [ecx+0x88888888]	;palookup
	mov [edi], al			;vidcntoffs
	test ebp, 3
	jnz short startqrhline1
	test ebp, ebp
	jz short endqrhline

skipqrhline1:
	mov cl, [ebx]			;bufplc
	jmp short begqrhline
ALIGN 16
begqrhline:
qrmach7a: mov dl, [ebx+0x88888888]	;bufplc
qrmach2a: sub esi, 0x88888888		;ylo
qrmach3a: sbb ebx, 0x88888888		;xhi*tilesizy + yhi+ycarry
qrmach4a: mov ah, [ecx+0x88888888]	;palookup
qrmach4b: mov al, [edx+0x88888888]	;palookup
	sub edi, 4
	shl eax, 16
	mov cl, [ebx]			;bufplc
qrmach7b: mov dl, [ebx+0x88888888]	;bufplc
qrmach2b: sub esi, 0x88888888		;ylo
qrmach3b: sbb ebx, 0x88888888		;xhi*tilesizy + yhi+ycarry
qrmach4c: mov ah, [ecx+0x88888888]	;palookup
qrmach4d: mov al, [edx+0x88888888]	;palookup
	mov cl, [ebx]			;bufplc
	mov [edi], eax
	sub ebp, 4
	jnz short begqrhline

endqrhline:
	pop ebp
	pop edi
	pop esi
	pop ebx
	ret


GLOBAL _setupdrawslab
_setupdrawslab:
	mov eax, [esp+4]
	mov [voxbpl1+2], eax
	mov [voxbpl2+2], eax
	mov [voxbpl3+2], eax
	mov [voxbpl4+2], eax
	mov [voxbpl5+2], eax
	mov [voxbpl6+2], eax
	mov [voxbpl7+2], eax
	mov [voxbpl8+2], eax

	mov eax, [esp+8]
	mov [voxpal1+2], eax
	mov [voxpal2+2], eax
	mov [voxpal3+2], eax
	mov [voxpal4+2], eax
	mov [voxpal5+2], eax
	mov [voxpal6+2], eax
	mov [voxpal7+2], eax
	mov [voxpal8+2], eax
	ret

ALIGN 16
GLOBAL _drawslab
; long long long long long long
_drawslab:
	push ebx
	push esi
	push edi
	push ebp

	mov eax, [esp+20]
	mov ebx, [esp+24]
	mov ecx, [esp+28]
	mov edx, [esp+32]
	mov esi, [esp+36]
	mov edi, [esp+40]

	cmp eax, 2
	je voxbegdraw2
	ja voxskip2
	xor eax, eax
voxbegdraw1:
	mov ebp, ebx
	shr ebp, 16
	add ebx, edx
	dec ecx
	mov al, [esi+ebp]
voxpal1: mov al, [eax+0x88888888]
	mov [edi], al
voxbpl1: lea edi, [edi+0x88888888]
	jnz voxbegdraw1
	pop ebp
	pop edi
	pop esi
	pop ebx
	ret

voxbegdraw2:
	mov ebp, ebx
	shr ebp, 16
	add ebx, edx
	xor eax, eax
	dec ecx
	mov al, [esi+ebp]
voxpal2: mov al, [eax+0x88888888]
	mov ah, al
	mov [edi], ax
voxbpl2: lea edi, [edi+0x88888888]
	jnz voxbegdraw2
	pop ebp
	pop edi
	pop esi
	pop ebx
	ret

voxskip2:
	cmp eax, 4
	jne voxskip4
	xor eax, eax
voxbegdraw4:
	mov ebp, ebx
	add ebx, edx
	shr ebp, 16
	xor eax, eax
	mov al, [esi+ebp]
voxpal3: mov al, [eax+0x88888888]
	mov ah, al
	shl eax, 8
	mov al, ah
	shl eax, 8
	mov al, ah
	mov [edi], eax
voxbpl3: add edi, 0x88888888
	dec ecx
	jnz voxbegdraw4
	pop ebp
	pop edi
	pop esi
	pop ebx
	ret

voxskip4:
	add eax, edi

	test edi, 1
	jz voxskipslab1
	cmp edi, eax
	je voxskipslab1

	push eax
	push ebx
	push ecx
	push edi
voxbegslab1:
	mov ebp, ebx
	add ebx, edx
	shr ebp, 16
	xor eax, eax
	mov al, [esi+ebp]
voxpal4: mov al, [eax+0x88888888]
	mov [edi], al
voxbpl4: add edi, 0x88888888
	dec ecx
	jnz voxbegslab1
	pop edi
	pop ecx
	pop ebx
	pop eax
	inc edi

voxskipslab1:
	push eax
	test edi, 2
	jz voxskipslab2
	dec eax
	cmp edi, eax
	jge voxskipslab2

	push ebx
	push ecx
	push edi
voxbegslab2:
	mov ebp, ebx
	add ebx, edx
	shr ebp, 16
	xor eax, eax
	mov al, [esi+ebp]
voxpal5: mov al, [eax+0x88888888]
	mov ah, al
	mov [edi], ax
voxbpl5: add edi, 0x88888888
	dec ecx
	jnz voxbegslab2
	pop edi
	pop ecx
	pop ebx
	add edi, 2

voxskipslab2:
	mov eax, [esp]

	sub eax, 3
	cmp edi, eax
	jge voxskipslab3

voxprebegslab3:
	push ebx
	push ecx
	push edi
voxbegslab3:
	mov ebp, ebx
	add ebx, edx
	shr ebp, 16
	xor eax, eax
	mov al, [esi+ebp]
voxpal6: mov al, [eax+0x88888888]
	mov ah, al
	shl eax, 8
	mov al, ah
	shl eax, 8
	mov al, ah
	mov [edi], eax
voxbpl6: add edi, 0x88888888
	dec ecx
	jnz voxbegslab3
	pop edi
	pop ecx
	pop ebx
	add edi, 4

	mov eax, [esp]

	sub eax, 3
	cmp edi, eax
	jl voxprebegslab3

voxskipslab3:
	mov eax, [esp]

	dec eax
	cmp edi, eax
	jge voxskipslab4

	push ebx
	push ecx
	push edi
voxbegslab4:
	mov ebp, ebx
	add ebx, edx
	shr ebp, 16
	xor eax, eax
	mov al, [esi+ebp]
voxpal7: mov al, [eax+0x88888888]
	mov ah, al
	mov [edi], ax
voxbpl7: add edi, 0x88888888
	dec ecx
	jnz voxbegslab4
	pop edi
	pop ecx
	pop ebx
	add edi, 2

voxskipslab4:
	pop eax

	cmp edi, eax
	je voxskipslab5

voxbegslab5:
	mov ebp, ebx
	add ebx, edx
	shr ebp, 16
	xor eax, eax
	mov al, [esi+ebp]
voxpal8: mov al, [eax+0x88888888]
	mov [edi], al
voxbpl8: add edi, 0x88888888
	dec ecx
	jnz voxbegslab5

voxskipslab5:
	pop ebp
	pop edi
	pop esi
	pop ebx
	ret

;modify: loinc
;eax: |  dat   |  dat   |   dat  |   dat  |
;ebx: |      loplc1                       |
;ecx: |      loplc2     |  cnthi |  cntlo |
;edx: |--------|--------|--------| hiplc1 |
;esi: |--------|--------|--------| hiplc2 |
;edi: |--------|--------|--------| vidplc |
;ebp: |--------|--------|--------|  hiinc |

%if 0	; unused
GLOBAL _stretchhline
_stretchhline:
	push ebp

	mov eax, ebx
	shl ebx, 16
	sar eax, 16
	and ecx, 0x0000ffff
	or ecx, ebx

	add esi, eax
	mov eax, edx
	mov edx, esi

	mov ebp, eax
	shl eax, 16
	sar ebp, 16

	add ecx, eax
	adc esi, ebp

	add eax, eax
	adc ebp, ebp
	mov [loinc1+2], eax
	mov [loinc2+2], eax
	mov [loinc3+2], eax
	mov [loinc4+2], eax

	inc ch

	jmp begloop

begloop:
	mov al, [edx]
loinc1: sub ebx, 0x88888888
	sbb edx, ebp
	mov ah, [esi]
loinc2: sub ecx, 0x88888888
	sbb esi, ebp
	sub edi, 4
	shl eax, 16
loinc3: sub ebx, 0x88888888
	mov al, [edx]
	sbb edx, ebp
	mov ah, [esi]
loinc4: sub ecx, 0x88888888
	sbb esi, ebp
	mov [edi], eax
	dec cl
	jnz begloop
	dec ch
	jnz begloop

	pop ebp
	ret
%endif	; ---0---

; [RH] This is my PPro-optimized version of vlineasm4. In my
; tests with ZDoom, it outperforms provlineasm4 by ~7% on a
; Pentium 3.

ALIGN 16
GLOBAL _setuptallvlineasm
; long
_setuptallvlineasm:
	mov eax, [esp+4]
		;First 2 lines for VLINEASM1, rest for VLINEASM4
	mov [premach3a+2], al
	mov [mach3a+2], al

	mov		[shifter1+2], al
	mov		[shifter2+2], al
	mov		[shifter3+2], al
	mov		[shifter4+2], al
	ret

GLOBAL _vlinetallasm4
_vlinetallasm4:
	push	ebx
	mov		eax, [_bufplce+0]
	mov		ebx, [_bufplce+4]
	mov		ecx, [_bufplce+8]
	mov		edx, [_bufplce+12]
	mov		[source1+3], eax
	mov		[source2+3], ebx
	mov		[source3+3], ecx
	mov		[source4+3], edx
	mov		eax, [_palookupoffse+0]
	mov		ebx, [_palookupoffse+4]
	mov		ecx, [_palookupoffse+8]
	mov		edx, [_palookupoffse+12]
	mov		[lookup1+2], eax
	mov		[lookup2+2], ebx
	mov		[lookup3+2], ecx
	mov		[lookup4+2], edx
	mov		eax, [_vince+0]
	mov		ebx, [_vince+4]
	mov		ecx, [_vince+8]
	mov		edx, [_vince+12]
	mov		[step1+2], eax
	mov		[step2+2], ebx
	mov		[step3+2], ecx
	mov		[step4+1], edx
	push	ebp
	push	esi
	push	edi
	mov		ecx, [esp+20]
	mov		edi, [esp+24]
	mov		eax, dword [_ylookup+ecx*4-4]
	sub		eax, dword [_ylookup]
	add		eax, edi
	sub		edi, eax
	mov		[write1+2],eax
	inc		eax
	mov		[write2+2],eax
	inc		eax
	mov		[write3+2],eax
	inc		eax
	mov		[write4+2],eax
	mov		ebx, [_vplce]
	mov		ecx, [_vplce+4]
	mov		esi, [_vplce+8]
	mov		eax, [_vplce+12]
	jmp		loopit

ALIGN	16
loopit:
		mov	edx, ebx
shifter1:	shr	edx, 24
source1:	movzx	edx, BYTE [edx+0x88888888]
lookup1:	mov	dl, [edx+0x88888888]
write1:		mov	[edi+0x88888880], dl
step1:		add	ebx, 0x88888888
		mov	edx, ecx
shifter2:	shr	edx, 24
source2:	movzx	edx, BYTE [edx+0x88888888]
lookup2:	mov	dl, [edx+0x88888888]
write2:		mov	[edi+0x88888881], dl
step2:		add	ecx, 0x88888888
		mov	edx, esi
shifter3:	shr	edx, 24
source3:	movzx	edx, BYTE [edx+0x88888888]
lookup3:	mov	dl, BYTE [edx+0x88888888]
write3:		mov	[edi+0x88888882], dl
step3:		add	esi, 0x88888888
		mov	edx, eax
shifter4:	shr	edx, 24
source4:	movzx	edx, BYTE [edx+0x88888888]
lookup4:	mov	dl, [edx+0x88888888]
write4:		mov	[edi+0x88888883], dl
step4:		add	eax, 0x88888888
vltpitch:	add	edi, 320
		jle	near loopit

	mov		[_vplce], ebx
	mov		[_vplce+4], ecx
	mov		[_vplce+8], esi
	mov		[_vplce+12], eax

	pop		edi
	pop		esi
	pop		ebp
	pop		ebx

	ret

GLOBAL _mmxoverlay
_mmxoverlay:
	push ebx
	pushfd			;Check if CPUID is available
	pop eax
	mov ebx, eax
	xor eax, 0x00200000
	push eax
	popfd
	pushfd
	pop eax
	cmp eax, ebx
	je pentium
	xor eax, eax
	dw 0xa20f
	test eax, eax
	jz pentium
	mov eax, 1
	dw 0xa20f
	pop ebx
	and eax, 0x00000f00
	test edx, 0x00800000	;Check if MMX is available
	jz nommx
	cmp eax, 0x00000600	;Check if P6 Family or not
	jae pentiumii
	jmp pentiummmx
nommx:
	cmp eax, 0x00000600	;Check if P6 Family or not
	jae pentiumpro
pentium:
	ret

;旼컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴커
;�                    PENTIUM II Overlays                       �
;읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴켸
pentiumii:
		;Hline overlay (MMX doesn't help)
	mov [_sethlinesizes], byte 0xe9
	mov [_sethlinesizes+1], dword _prosethlinesizes-_sethlinesizes-5
	mov [_setpalookupaddress], byte 0xe9
	mov [_setpalookupaddress+1], dword _prosetpalookupaddress-_setpalookupaddress-5
	mov [_setuphlineasm4], byte 0xc3  ;ret (no code required)
	mov [_hlineasm4], byte 0xe9
	mov [_hlineasm4+1], dword _prohlineasm4-_hlineasm4-5

		;Vline overlay
	mov [_setupvlineasm], byte 0xe9
;	mov [_setupvlineasm+1], dword _prosetupvlineasm-_setupvlineasm-5
	mov [_setupvlineasm+1], dword _setuptallvlineasm-_setupvlineasm-5
	mov [_vlineasm4], byte 0xe9
;	mov [_vlineasm4+1], dword _provlineasm4-_vlineasm4-5
	mov [_vlineasm4+1], dword _vlinetallasm4-_vlineasm4-5

	ret

;旼컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴커
;�                    PENTIUM MMX Overlays                      �
;읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴켸
pentiummmx:
	ret

;旼컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴커
;�                    PENTIUM PRO Overlays                      �
;읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴켸
pentiumpro:
		;Hline overlay (MMX doens't help)
	mov [_sethlinesizes], byte 0xe9
	mov [_sethlinesizes+1], dword _prosethlinesizes-_sethlinesizes-5
	mov [_setpalookupaddress], byte 0xe9
	mov [_setpalookupaddress+1], dword _prosetpalookupaddress-_setpalookupaddress-5
	mov [_setuphlineasm4], byte 0xc3  ;ret (no code required)
	mov [_hlineasm4], byte 0xe9
	mov [_hlineasm4+1], dword _prohlineasm4-_hlineasm4-5

		;Vline overlay
	mov [_setupvlineasm], byte 0xe9
;	mov [_setupvlineasm+1], dword _prosetupvlineasm-_setupvlineasm-5
	mov [_setupvlineasm+1], dword _setuptallvlineasm-_setupvlineasm-5
	mov [_vlineasm4], byte 0xe9
;	mov [_vlineasm4+1], dword _provlineasm4-_vlineasm4-5
	mov [_vlineasm4+1], dword _vlinetallasm4-_vlineasm4-5

	ret
