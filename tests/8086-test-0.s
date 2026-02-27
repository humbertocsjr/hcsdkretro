je near dest
je far 0x1234:0x0005
aaa
add al, 0x12
add ax, 0x1234
add bl, 0x12
add bx, 0x1234
adc al, 0x12
adc ax, 0x1234
adc bl, 0x12
adc bx, 0x1234
add cl, bh
add dh, ch
adc cx, bx
adc bx, dx
adc byte al, [0x1234]
adc [0x1234], byte al
adc word ax, [0x1234]
adc word [0x1234], ax
cs adc [0x1234], ax
adc [bx], ax
adc [bp+si+1+5 *2], ax
adc [bx+si], al
adc ax, [bx]
adc ax, [bp+si+1+5]
adc al, [bx+si]

div ax
div al
div byte [0x1234]
div byte [bx+0x1234]
div byte [bx]
div byte [bx+si+0x1234]

in al, dx
in al, 70
in ax, dx
in ax, 70

out dx, al
out 70, al
out dx, ax
out 70, ax
int 3
int 4

lds ax, [0x1234]
lds ax, [bx+0x1234]
lds ax, [bx+si+0x1234]

mov ax, bx
mov bx, ax
mov cx, dx
mov ax, [0x1234]
mov [0x1234], ax
mov al, [0x1234]
mov [0x1234], al
mov ax, [bx+0x1234]
mov [bx+0x1234], ax
mov al, [bx+0x1234]
mov [bx+0x1234], al
mov cx, [bx+0x1234]
mov [bx+0x1234], cx
mov ax, ds
mov ds, ax
mov ds, [bx+0x1234]
mov [bx+0x1234], ds
mov ds, [0x1234]
mov [0x1234], ds
mov ax, 0x1234
mov bx, 0x1234
mov cx, 0x1234
mov al, 0x12
mov bl, 0x12
cmp ax, 0x1234

pop ax
pop bx
pop word [0x1234]
pop word [bx]
pop word [bx+0x1234]
pop ds
pop es
popf
popa
push ax
push bx
push word [0x1234]
push word [bx]
push word [bx+0x1234]
push ds
push es
pushf
pusha

rcl al, 1
rcl ax, 1
rcl al, cl
rcl ax, cl
rcl word [0x1234], cl
rcl word [bx], cl
rcl word [bx+0x1234], cl

ret
ret 1
retf 
retf 1
ret far 1
je dest
call far 0x1234:0x0005
call [dest]
call far [dest]
call near [bx+1]
call far [bx+1]
call [bx+1]
call dest

testloop:
jmp far 0x1234:0x0005
jmp [dest]
jmp far [dest]
jmp near [bx+1]
jmp far [bx+1]
jmp [bx+1]
jmp dest
loop testloop
loop near testloop
loop far 0x1234:0x0005
dest:


const_123: equ 123
const_456 equ 456
const_math equ 1+2*3
struct_test equ 2 ; size
    .field1 equ 0 ; offset
    .field2 equ 1 ; offset

section data
    obj_test: resb struct_test

section text

    mov ax, const_math ; simple example
    mov al, [obj_test + struct_test.field1]
    mov si, obj_test
    mov bl, [si+struct_test.field1]