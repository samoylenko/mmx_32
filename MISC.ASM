bits 32

;-----------------------------------------------------------------------------------------------

global _g_pScr
global _g_lPitch
global _g_pBlob

global _PutBlobMMX
global _PutBlob
global _TestMMX

;-----------------------------------------------------------------------------------------------

section .data

;-----------------------------------------------------------------------------------------------

_g_pScr     dd      0
_g_lPitch   dd      0
_g_pBlob	dd		0

;-----------------------------------------------------------------------------------------------

section .code

;-----------------------------------------------------------------------------------------------
; void PutBlob( int x, int y, DWORD *blob )

_PutBlob:
			push	ebp							; enter
			mov		ebp, esp					;	function
			
			push	edi							; save edi
			push	esi							; save esi

			mov		esi, [ebp + 16]				; esi = blob

			mov		eax, [ebp + 12]				; eax = y
			mul		dword [_g_lPitch]			; eax = size(x) * y
			add		eax, [_g_pScr]				; eax = addr ( x = 0 )
			mov		edi, eax					; edi = addr ( x = 0 )
			mov		eax, [ebp + 8]				; eax = x
			shl		eax, 2						; eax *= 4
			add     edi, eax					; edi = addr (x, y )

			mov		ecx, 300					; ecx = blob size(y) = 300
.box2:
			push	ecx							; save ecx
			push	edi							; save edi
			mov		ecx, 300 * 4				; ecx = 300 * 4 = blob size (x)
.box1:											;	in bytes
			lodsb								; take byte, next source
			add		al, [edi]					; add to destination
			jnc		.box3						; no overflow - jump to .box3
			mov		al, 0xff					; overflow - take maximum
.box3:
			stosb								; put byte, next destination
	
			loop	.box1						; repeat blob size (x)
			
			pop		edi							; restore edi
			pop		ecx							; restore esi
			add		edi, [_g_lPitch]			; next screen line
			
			loop	.box2						; repeat blob size (y)
			
			pop		esi							; restore esi
			pop		edi							; restore edi
			
			pop		ebp							; leave
            ret									;	function

;-----------------------------------------------------------------------------------------------
; void PutBlobMMX( int x, int y, DWORD *blob )

_PutBlobMMX:                  
			push	ebp
			mov		ebp, esp
			
			push	edi
			push	esi

			mov		esi, [ebp + 16]				; esi = blob

			mov		eax, [ebp + 12]				; eax = y
			mul		dword [_g_lPitch]			; eax = size(x) * y
			add		eax, [_g_pScr]				; eax = addr ( x = 0 )
			mov		edi, eax					; edi = addr ( x = 0 )
			mov		eax, [ebp + 8]				; eax = x
			shl		eax, 2						; eax *= 4
			add     edi, eax					; edi = addr (x, y )

			mov		ecx, 300					; ecx = blob size (y) = 300
.box2:
			push	ecx							; save ecx
			push	edi							; save edi
			mov		ecx, 150					; ecx = blob size (x) / 4
.box1:
			movq	mm0, [esi]					; take 16 bytes (4 pixels)
			paddusb	mm0, [edi]					; packed add with unsigned										
 			movq	[edi], mm0					; saturation to source
	
			add		esi, 8						; next 8 bytes - source
			add		edi, 8						; next 8 bytes - destination

			loop	.box1						; repeat blob size (x)
			
			pop		edi							; restore edi
			pop		ecx							; restore ecx
			add		edi, [_g_lPitch]			; next screen line
			
			loop	.box2						; repeat blob size (y)

			emms								; empty mmx state
			
			pop		esi							; restore esi
			pop		edi							; restore edi
			
			pop		ebp							; leave
            ret									;	function

;-----------------------------------------------------------------------------------------------
; bool TestMMX()

_TestMMX:
			push	edx
			mov		eax, 1		
			cpuid								; CPU ident
			xor		eax, eax					; eax = false
			bt		edx, 23						; 23 bit - MMX state
			jnc		.nommx
			mov		eax, 1						; eax = true, if mmx - on	
.nommx:		
			pop		edx
			ret									; return eax