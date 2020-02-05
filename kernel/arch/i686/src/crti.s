global _init

section .init
_init:
	push ebp
	mov ebp, esp
	; gcc will nicely put the contents of crtbegin.o's .init section here. 

global _fini

section .fini
_fini:
	push ebp
	mov ebp, esp
	; gcc will nicely put the contents of crtbegin.o's .fini section here.
