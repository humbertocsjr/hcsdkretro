; MSX-DOS 2 file I/O functions for B language runtime
; Uses BDOS functions 0x40-0x47 (ASCIIZ handle-based API) via CALL 5
;
; MSX-DOS 2 API reference (emulated by msxdosemu):
;   0x40 — OPEN:   C=0x40, DE=path, A=handle (0xFF=error)
;   0x41 — CLOSE:  B=handle, A=0 (ok) or 0xFF (error)
;   0x42 — READ:   B=handle, DE=buf, HL=count, HL=bytes_read
;   0x43 — WRITE:  B=handle, DE=buf, HL=count, HL=bytes_written
;   0x44 — CREATE: C=0x44, DE=path, A=handle (0xFF=error)
;   0x45 — DELETE: DE=path, A=0 (ok) or 0xFF (error)
;   0x46 — SEEK:   B=handle, DEHL=offset, A=method, DEHL=position
;   0x47 — RENAME: DE=old, HL=new, A=0 (ok) or 0xFF (error)
;
; File handle table: 6 slots × 4 bytes = 24 bytes (in data section)

section data
    global _ft
    _ft: ds 24          ; 6 slots: handle_lo, handle_hi, mode, pad

section text

; ==============================================================
; fopen — Open or create a file
; B signature: fopen(name, mode)
;   name: pointer to ASCIIZ string
;   mode: 0=read, 1=write/create, 2=read+write
; Returns: fd (0-5) or -1 on error
; ==============================================================
    global fopen
fopen:
    push ix
    ld ix, 0
    add ix, sp
    ; Find free slot in _ft
    ld hl, _ft
    ld de, _ft + 24
.slot_loop:
    ld a, [hl]
    inc hl
    or [hl]
    dec hl
    jr z, .slot_found
    inc hl
    inc hl
    inc hl
    inc hl
    ld a, l
    cp e
    jr nz, .slot_loop
    ; No free slot
    pop ix
    ld hl, -1
    ret
.slot_found:
    push hl             ; save slot pointer
    ld a, [ix+6]        ; A = mode
    or a
    jr nz, .write_mode
    ; Open existing: BC=0x40, DE=path
    ld bc, 0x40
    ld e, [ix+4]
    ld d, [ix+5]
    call 5
    cp 0xFF
    jr z, .err
    jr .store_handle
.write_mode:
    ; Create new: BC=0x44, DE=path
    ld bc, 0x44
    ld e, [ix+4]
    ld d, [ix+5]
    call 5
    cp 0xFF
    jr z, .err
.store_handle:
    ex [sp], hl
    ld [hl], a          ; store handle low
    inc hl
    ld [hl], a          ; store handle high (always 0)
    inc hl
    ld a, [ix+6]
    ld [hl], a          ; store mode
    ; fd = (slot - _ft - 2) >> 2
    ld de, _ft + 2
    ld a, l
    sub e
    ld l, a
    ld a, h
    sbc a, d
    ld h, a
    ld a, l
    and 0xFC
    rrca
    rrca
    ld l, a
    ld h, 0
    pop ix
    ret
.err:
    pop hl
    pop ix
    ld hl, -1
    ret

; ==============================================================
; fclose — Close a file
; B signature: fclose(fd)
; Returns: 0 on success, -1 on error
; ==============================================================
    global fclose
fclose:
    push ix
    ld ix, 0
    add ix, sp
    ld a, [ix+4]
    cp 6
    jr nc, .err
    ; slot = fd * 4
    add a, a
    add a, a
    ld e, a
    ld d, 0
    ld hl, _ft
    add hl, de
    ld b, [hl]          ; B = handle
    ld a, b
    or a
    jr z, .err
    ; Close: BDOS 0x41, B=handle
    ld c, 0x41
    call 5
    ; Mark slot free
    ld hl, _ft
    add hl, de
    xor a
    ld [hl], a
    inc hl
    ld [hl], a
    pop ix
    ld hl, 0
    ret
.err:
    pop ix
    ld hl, -1
    ret

; ==============================================================
; fread — Read words from file
; B signature: fread(fd, buf, nwords)
; Returns: words read, or -1 on error
; ==============================================================
    global fread
fread:
    push ix
    ld ix, 0
    add ix, sp
    ld a, [ix+4]        ; fd
    cp 6
    jr nc, .err
    add a, a
    add a, a
    ld e, a
    ld d, 0
    ld hl, _ft
    add hl, de
    ld b, [hl]          ; B = handle
    ld a, b
    or a
    jr z, .err
    ; Read: BDOS 0x42, B=handle, DE=buf, HL=byte_count
    ld a, [ix+9]        ; nwords high
    ld l, [ix+8]        ; nwords low
    ld h, a
    add hl, hl          ; HL = nwords * 2 (bytes)
    ld e, [ix+6]        ; buf low
    ld d, [ix+7]        ; buf high
    ld c, 0x42
    call 5
    ; Return HL = bytes read. Convert to words
    srl h
    rr l
    pop ix
    ret
.err:
    pop ix
    ld hl, -1
    ret

; ==============================================================
; fwrite — Write words to file
; B signature: fwrite(fd, buf, nwords)
; Returns: words written, or -1 on error
; ==============================================================
    global fwrite
fwrite:
    push ix
    ld ix, 0
    add ix, sp
    ld a, [ix+4]        ; fd
    cp 6
    jr nc, .err
    add a, a
    add a, a
    ld e, a
    ld d, 0
    ld hl, _ft
    add hl, de
    ld b, [hl]          ; B = handle
    ld a, b
    or a
    jr z, .err
    ; Write: BDOS 0x43, B=handle, DE=buf, HL=byte_count
    ld a, [ix+9]        ; nwords high
    ld l, [ix+8]        ; nwords low
    ld h, a
    add hl, hl          ; HL = nwords * 2 (bytes)
    ld e, [ix+6]        ; buf low
    ld d, [ix+7]        ; buf high
    ld c, 0x43
    call 5
    ; Return HL = bytes written. Convert to words
    srl h
    rr l
    pop ix
    ret
.err:
    pop ix
    ld hl, -1
    ret

; ==============================================================
; fdelete — Delete a file
; B signature: fdelete(name)
; Returns: 0 on success, -1 on error
; ==============================================================
    global fdelete
fdelete:
    push ix
    ld ix, 0
    add ix, sp
    ld c, 0x45
    ld e, [ix+4]
    ld d, [ix+5]
    call 5
    or a
    jr nz, .err
    pop ix
    ld hl, 0
    ret
.err:
    pop ix
    ld hl, -1
    ret

; ==============================================================
; frename — Rename a file
; B signature: frename(oldname, newname)
; Returns: 0 on success, -1 on error
; ==============================================================
    global frename
frename:
    push ix
    ld ix, 0
    add ix, sp
    ld c, 0x47
    ld e, [ix+4]
    ld d, [ix+5]        ; DE = old name
    ld l, [ix+6]
    ld h, [ix+7]        ; HL = new name
    call 5
    or a
    jr nz, .err
    pop ix
    ld hl, 0
    ret
.err:
    pop ix
    ld hl, -1
    ret

; ==============================================================
; fseek — Seek within file
; B signature: fseek(fd, offset, whence)
;   fd: file descriptor
;   offset: 16-bit offset (unsigned)
;   whence: 0=SET, 1=CUR, 2=END
; Returns: 0 on success, -1 on error
; ==============================================================
    global fseek
fseek:
    push ix
    ld ix, 0
    add ix, sp
    ld a, [ix+4]        ; fd
    cp 6
    jr nc, .err
    add a, a
    add a, a
    ld e, a
    ld d, 0
    ld hl, _ft
    add hl, de
    ld b, [hl]          ; B = handle
    ld a, b
    or a
    jr z, .err
    ; Seek: BDOS 0x46, B=handle, A=method, DE=offset_lo, HL=offset_hi
    ; Since offset is 16-bit, offset_hi = 0
    ld c, 0x46
    ld a, [ix+8]        ; A = whence (method)
    ld e, [ix+6]        ; offset low
    ld d, [ix+7]        ; offset high
    ld hl, 0            ; offset high = 0
    call 5
    pop ix
    ld hl, 0
    ret
.err:
    pop ix
    ld hl, -1
    ret

; ==============================================================
; ftell — Get current file position
; B signature: ftell(fd)
; Returns: position (16-bit), or -1 on error
; ==============================================================
    global ftell
ftell:
    push ix
    ld ix, 0
    add ix, sp
    ld a, [ix+4]        ; fd
    cp 6
    jr nc, .err
    add a, a
    add a, a
    ld e, a
    ld d, 0
    ld hl, _ft
    add hl, de
    ld b, [hl]          ; B = handle
    ld a, b
    or a
    jr z, .err
    ; Seek 0 bytes from current: BDOS 0x46, B=handle, A=1, DEHL=0
    ld c, 0x46
    ld a, 1             ; SEEK_CUR
    ld de, 0
    ld hl, 0
    call 5
    ; Returns DEHL = position. Return HL (low 16 bits)
    ; Actually the emulator returns DE=low, HL=high, so we need to swap
    ; But for 16-bit values, DE=low, HL=0, so return DE
    push de
    pop hl
    pop ix
    ret
.err:
    pop ix
    ld hl, -1
    ret

; ==============================================================
; feof — Test end of file
; B signature: feof(fd)
; Returns: 1 if at EOF, 0 otherwise, -1 on error
;
; Note: MSX-DOS 2 does not provide direct EOF query.
; fread returns fewer words than requested on EOF.
; This function always returns 0; check fread return value.
; ==============================================================
    global feof
feof:
    push ix
    ld ix, 0
    add ix, sp
    ld a, [ix+4]
    cp 6
    jr nc, .err
    pop ix
    ld hl, 0
    ret
.err:
    pop ix
    ld hl, -1
    ret
