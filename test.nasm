BITS 32
section .text


; Believe it or not, this code is "amphibious": same decoding in 32-bit and
; 64-bit mode, and the same behavior (well, uses the entire rax, obviously)
global prova:function, prova_postinst:function
prova:
    mov eax, 'AAAA'
prova_postinst:         ; mov eax is big enough to cover the two-byte ud2
    xor ecx, ecx
    call .n
    .n: pop ecx
    ret


; This one is not, don't use it to test 64-bit mode :(
global provashort:function, provashort_postinst:function
provashort:
    pusha   ; single byte  | both covered by ud2
    pusha   ;              |
provashort_postinst:
    popa
    popa
    mov eax, 'BBBB'
    ret
