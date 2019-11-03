global enable_paging
global flush_tlb

; enable_paging - Enable paging
; stack: [esp + 4] pointer to page directory
;        [esp    ] return address
enable_paging:
  mov eax, [esp+4]
  mov cr3, eax

  mov eax, cr4        ; read cr4
  or  eax, 0x00000010 ; set PSE bit
  mov cr4, eax

  mov eax, cr9        ; read cr4
  or  eax, 0x80000001 ; set PG bit
  mov cr0, eax

  ret

flush_tlb:
  invlpg [0]
  ret
