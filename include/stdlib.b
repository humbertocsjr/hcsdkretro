/*
 * stdlib.b — HC B Standard Library declarations
 *
 * Include this file to use the B standard library:
 *   #include <stdlib.b>
 *
 * Supported targets: Z80 (CP/M), 8080 (CP/M), 8085 (CP/M), 8086 (MS-DOS)
 * Platform coverage varies — see notes on each function.
 */

/* ── Console I/O ─────────────────────────────────────────────────── */

extrn putchar;   /* putchar(c) — write char to console (all targets) */
extrn getchar;   /* getchar()  — read char from console, -1 on EOF (all targets) */
extrn puts;      /* puts(s)    — print '$'-terminated string + \r\n (all targets) */
extrn gets;      /* gets(s)    — read line into s, returns length (all targets) */
extrn printf;    /* printf(fmt, a, b, c, d, e, f, g, h) — formatted output (all targets)
                  *   %d signed, %u unsigned, %x hex, %s string, %c char, %% literal
                  *   supports up to 8 variadic args */

/* ── String Functions ────────────────────────────────────────────── */

extrn strlen;    /* strlen(s)        — string length (all targets) */
extrn strcpy;    /* strcpy(dst, src) — copy string, returns dst (all targets) */
extrn strcat;    /* strcat(dst, src) — concatenate, returns dst (all targets) */
extrn strcmp;    /* strcmp(a, b)     — compare: -1 a<b, 0 equal, 1 a>b (all targets) */

/* ── Memory Functions ────────────────────────────────────────────── */

extrn memcpy;    /* memcpy(dst, src, n) — copy n words, returns dst (all targets) */
extrn memset;    /* memset(ptr, val, n) — fill n words with val, returns ptr (all targets) */

/* ── Conversion ──────────────────────────────────────────────────── */

extrn atoi;      /* atoi(s)   — ASCII decimal to integer, handles +/- (all targets) */
extrn itoa;      /* itoa(n,s) — integer to ASCII decimal string, returns s (all targets) */
extrn abs;       /* abs(n)    — absolute value (all targets) */

/* ── System ──────────────────────────────────────────────────────── */

extrn exit;      /* exit(code) — exit to OS, code in L (all targets) */

/* ── File I/O (Z80 CP/M full; 8080/8085 not available; 8086 stubs) ─ */

extrn fopen;     /* fopen(name, mode)     — open file, 0=read 1=write 2=append */
extrn fclose;    /* fclose(fd)            — close file */
extrn fread;     /* fread(fd, buf, nwords) — read up to nwords, returns count */
extrn fwrite;    /* fwrite(fd, buf, nwords) — write nwords, returns count */
extrn fseek;     /* fseek(fd, offset, whence) — seek: 0=SET, 2=END */
extrn ftell;     /* ftell(fd)             — current file position in bytes */
extrn feof;      /* feof(fd)              — non-zero if at EOF */
extrn fdelete;   /* fdelete(name)         — delete file, 0 ok */
extrn frename;   /* frename(old, new)     — rename file, 0 ok */
extrn fgetc;     /* fgetc(fd)             — read one byte, -1 on EOF (8086 stub) */
extrn fputc;     /* fputc(c, fd)          — write one byte (8086 stub) */

/* ── Inline Functions (emitted by compiler, not real calls) ─────────
 *
 * These are NOT declared with extrn — the compiler recognizes them
 * by name and emits inline code directly.
 *
 *   peekb(addr)       — read 1 byte from address addr
 *   pokeb(addr, val)  — write 1 byte val to address addr
 *   peekw(addr)       — read 1 word (2 bytes) from address addr
 *   pokew(addr, val)  — write 1 word (2 bytes) val to address addr
 */
