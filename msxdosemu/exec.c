#include "emu.h"


void exec_step()
{
    memcpy(&_regs_prev, &_regs_curr, sizeof(z80_regs_t));
    _regs_prev.value = 0;
    _regs_curr.value = 0;
    bool reset_prefixes = true;
    uint16_t tmp;
    if(_regs_curr.ip == 5)
    {
        // MSX-DOS ABI
        _regs_curr.ip = mem_get_word(_regs_curr.sp);
        _regs_curr.sp += 2;
    }
    else if
    (
        !_regs_curr.prefix_cb &&
        !_regs_curr.prefix_dd &&
        !_regs_curr.prefix_ed &&
        !_regs_curr.prefix_fd
    ) switch(ip_get_byte())
    {
        case 0x00:
            break;
        case 0x01:
            _regs_prev.value = _regs_curr.bc.word;
            _regs_curr.bc.word = ip_get_word();
            _regs_curr.value = _regs_curr.bc.word;
            break;
        case 0x02:
            _regs_prev.value = _memory[_regs_curr.bc.word];
            _memory[_regs_curr.bc.word] = _regs_curr.af.a;
            _regs_curr.value = _memory[_regs_curr.bc.word];
            break;
        case 0x03:
            _regs_prev.value = _regs_curr.bc.word;
            _regs_curr.bc.word++;
            _regs_curr.value = _regs_curr.bc.word;
            break;
        case 0x04:
            _regs_prev.value = _regs_curr.bc.b;
            _regs_curr.bc.b = alu_inc_byte(_regs_curr.bc.b);
            _regs_curr.value = _regs_curr.bc.b;
            break;
        case 0x05:
            _regs_prev.value = _regs_curr.bc.b;
            _regs_curr.bc.b = alu_dec_byte(_regs_curr.bc.b);
            _regs_curr.value = _regs_curr.bc.b;
            break;
        case 0x06:
            _regs_prev.value = _regs_curr.bc.b;
            _regs_curr.bc.b = ip_get_byte();
            _regs_curr.value = _regs_curr.bc.b;
            break;
        case 0x07:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = alu_rcl(_regs_curr.af.a);
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0x08:
            tmp = _regs_curr.af.word;
            _regs_curr.af.word = _regs_curr.af_alt;
            _regs_curr.af_alt = tmp;
            break;
        case 0x09:
            _regs_prev.value = _regs_curr.hl.word;
            _regs_curr.hl.word = alu_add_word(_regs_curr.hl.word, _regs_curr.bc.word);
            _regs_curr.value = _regs_curr.hl.word;
            break;
        case 0x0a:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a =  _memory[_regs_curr.bc.word];
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0x0b:
            _regs_prev.value = _regs_curr.bc.word;
            _regs_curr.bc.word--;
            _regs_curr.value = _regs_curr.bc.word;
            break;
        case 0x0c:
            _regs_prev.value = _regs_curr.bc.c;
            _regs_curr.bc.c = alu_inc_byte(_regs_curr.bc.c);
            _regs_curr.value = _regs_curr.bc.c;
            break;
        case 0x0d:
            _regs_prev.value = _regs_curr.bc.c;
            _regs_curr.bc.c = alu_dec_byte(_regs_curr.bc.c);
            _regs_curr.value = _regs_curr.bc.c;
            break;
        case 0x0e:
            _regs_prev.value = _regs_curr.bc.c;
            _regs_curr.bc.c = ip_get_byte();
            _regs_curr.value = _regs_curr.bc.c;
            break;
        case 0x0f:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = alu_rrc(_regs_curr.af.a);
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0x10:
            _regs_prev.value = _regs_curr.bc.b;
            _regs_curr.bc.b--;
            if(_regs_curr.bc.b)
            {
                _regs_curr.ip += ip_get_byte_signed() - 1;
            }
            else ip_get_byte_signed();
            _regs_curr.value = _regs_curr.bc.b;
            break;
        case 0x11:
            _regs_prev.value = _regs_curr.de.word;
            _regs_curr.de.word = ip_get_word();
            _regs_curr.value = _regs_curr.de.word;
            break;
        case 0x12:
            _regs_prev.value = _memory[_regs_curr.de.word];
            _memory[_regs_curr.de.word] = _regs_curr.af.a;
            _regs_curr.value = _memory[_regs_curr.de.word];
            break;
        case 0x13:
            _regs_prev.value = _regs_curr.de.word;
            _regs_curr.de.word++;
            _regs_curr.value = _regs_curr.de.word;
            break;
        case 0x14:
            _regs_prev.value = _regs_curr.de.d;
            _regs_curr.de.d = alu_inc_byte(_regs_curr.de.d);
            _regs_curr.value = _regs_curr.de.d;
            break;
        case 0x15:
            _regs_prev.value = _regs_curr.de.d;
            _regs_curr.de.d = alu_dec_byte(_regs_curr.de.d);
            _regs_curr.value = _regs_curr.de.d;
            break;
        case 0x16:
            _regs_prev.value = _regs_curr.de.d;
            _regs_curr.de.d = ip_get_byte();
            _regs_curr.value = _regs_curr.de.d;
            break;
        case 0x17:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = alu_rl(_regs_curr.af.a);
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0x18:
            _regs_prev.value = _regs_curr.ip;
            _regs_curr.ip += ip_get_byte_signed() - 1;
            _regs_curr.value = _regs_curr.ip;
            break;
        case 0x19:
            _regs_prev.value = _regs_curr.hl.word;
            _regs_curr.hl.word = alu_add_word(_regs_curr.hl.word, _regs_curr.de.word);
            _regs_curr.value = _regs_curr.hl.word;
            break;
        case 0x1a:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = _memory[_regs_curr.de.word];
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0x1b:
            _regs_prev.value = _regs_curr.de.word;
            _regs_curr.de.word--;
            _regs_curr.value = _regs_curr.de.word;
            break;
        case 0x1c:
            _regs_prev.value = _regs_curr.de.e;
            _regs_curr.de.e = alu_inc_byte(_regs_curr.de.e);
            _regs_curr.value = _regs_curr.de.e;
            break;
        case 0x1d:
            _regs_prev.value = _regs_curr.de.e;
            _regs_curr.de.e = alu_dec_byte(_regs_curr.de.e);
            _regs_curr.value = _regs_curr.de.e;
            break;
        case 0x1e:
            _regs_prev.value = _regs_curr.de.e;
            _regs_curr.de.e = ip_get_byte();
            _regs_curr.value = _regs_curr.de.e;
            break;
        case 0x1f:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = alu_rr(_regs_curr.af.a);
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0x20:
            _regs_prev.value = _regs_curr.ip;
            if(!zf_get()) _regs_curr.ip += ip_get_byte_signed() - 1;
            else ip_get_byte_signed();
            _regs_curr.value = _regs_curr.ip;
            break;
        case 0x21:
            _regs_prev.value = _regs_curr.hl.word;
            _regs_curr.hl.word = ip_get_word();
            _regs_curr.value = _regs_curr.hl.word;
            break;
        case 0x22:
            tmp = ip_get_word();
            _regs_prev.value = mem_get_word(tmp);
            mem_set_word(tmp, _regs_curr.hl.word);
            _regs_curr.value = _regs_curr.hl.word;
            break;
        case 0x23:
            _regs_prev.value = _regs_curr.hl.word;
            _regs_curr.hl.word++;
            _regs_curr.value = _regs_curr.hl.word;
            break;
        case 0x24:
            _regs_prev.value = _regs_curr.hl.h;
            _regs_curr.hl.h = alu_inc_byte(_regs_curr.hl.h);
            _regs_curr.value = _regs_curr.hl.h;
            break;
        case 0x25:
            _regs_prev.value = _regs_curr.hl.h;
            _regs_curr.hl.h = alu_dec_byte(_regs_curr.hl.h);
            _regs_curr.value = _regs_curr.hl.h;
            break;
        case 0x26:
            _regs_prev.value = _regs_curr.hl.h;
            _regs_curr.hl.h = ip_get_byte();
            _regs_curr.value = _regs_curr.hl.h;
            break;
        case 0x27:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = alu_daa(_regs_curr.af.a);
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0x28:
            _regs_prev.value = _regs_curr.ip;
            if(zf_get()) _regs_curr.ip += ip_get_byte_signed() - 1;
            else ip_get_byte_signed();
            _regs_curr.value = _regs_curr.ip;
            break;
        case 0x29:
            _regs_prev.value = _regs_curr.hl.word;
            _regs_curr.hl.word = alu_add_word(_regs_curr.hl.word, _regs_curr.hl.word);
            _regs_curr.value = _regs_curr.hl.word;
            break;
        case 0x2a:
            _regs_prev.value = _regs_curr.hl.word;
            _regs_curr.hl.word = mem_get_word(ip_get_word());
            _regs_curr.value = _regs_curr.hl.word;
            break;
        case 0x2b:
            _regs_prev.value = _regs_curr.hl.word;
            _regs_curr.hl.word--;
            _regs_curr.value = _regs_curr.hl.word;
            break;
        case 0x2c:
            _regs_prev.value = _regs_curr.hl.l;
            _regs_curr.hl.l = alu_inc_byte(_regs_curr.hl.l);
            _regs_curr.value = _regs_curr.hl.l;
            break;
        case 0x2d:
            _regs_prev.value = _regs_curr.hl.l;
            _regs_curr.hl.l = alu_dec_byte(_regs_curr.hl.l);
            _regs_curr.value = _regs_curr.hl.l;
            break;
        case 0x2e:
            _regs_prev.value = _regs_curr.hl.l;
            _regs_curr.hl.l = ip_get_byte();
            _regs_curr.value = _regs_curr.hl.l;
            break;
        case 0x2f:
            _regs_prev.value = _regs_curr.af.a;
            nf_set(1);
            hf_set(1);
            _regs_curr.af.a = ~_regs_curr.af.a;
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0x30:
            _regs_prev.value = _regs_curr.ip;
            if(!cf_get()) _regs_curr.ip += ip_get_byte_signed() - 1;
            else ip_get_byte_signed();
            _regs_curr.value = _regs_curr.ip;
            break;
        case 0x31:
            _regs_prev.value = _regs_curr.sp;
            _regs_curr.sp = ip_get_word();
            _regs_curr.value = _regs_curr.sp;
            break;
        case 0x32:
            tmp = ip_get_word();
            _regs_prev.value = mem_get_byte(tmp);
            mem_set_byte(tmp, _regs_curr.af.a);
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0x33:
            _regs_prev.value = _regs_curr.sp;
            _regs_curr.sp++;
            _regs_curr.value = _regs_curr.sp;
            break;
        case 0x34:
            _regs_prev.value = mem_get_byte(_regs_curr.hl.word);
            mem_set_byte(_regs_curr.hl.word, mem_get_byte(_regs_curr.hl.word) + 1);
            _regs_curr.value = mem_get_byte(_regs_curr.hl.word);
            break;
        case 0x35:
            _regs_prev.value = mem_get_byte(_regs_curr.hl.word);
            mem_set_byte(_regs_curr.hl.word, mem_get_byte(_regs_curr.hl.word) - 1);
            _regs_curr.value = mem_get_byte(_regs_curr.hl.word);
            break;
        case 0x36:
            _regs_prev.value = mem_get_byte(_regs_curr.hl.word);
            mem_set_byte(_regs_curr.hl.word, ip_get_byte());
            _regs_curr.value = mem_get_byte(_regs_curr.hl.word);
            break;
        case 0x37:
            cf_set(1);
            nf_set(0);
            hf_set(0);
            break;
        case 0x38:
            _regs_prev.value = _regs_curr.ip;
            if(cf_get()) _regs_curr.ip += ip_get_byte_signed() - 1;
            else ip_get_byte_signed();
            _regs_curr.value = _regs_curr.ip;
            break;
        case 0x39:
            _regs_prev.value = _regs_curr.hl.word;
            _regs_curr.hl.word = alu_add_word(_regs_curr.hl.word, _regs_curr.sp);
            _regs_curr.value = _regs_curr.hl.word;
            break;
        case 0x3a:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = mem_get_byte(ip_get_word());
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0x3b:
            _regs_prev.value = _regs_curr.sp;
            _regs_curr.sp--;
            _regs_curr.value = _regs_curr.sp;
            break;
        case 0x3c:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = alu_inc_byte(_regs_curr.af.a);
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0x3d:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = alu_dec_byte(_regs_curr.af.a);
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0x3e:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = ip_get_byte();
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0x3f:
            cf_set(!cf_get());
            nf_set(1);
            hf_set(!hf_get());
            break;
        case 0x40:
            _regs_prev.value = _regs_curr.bc.b;
            _regs_curr.bc.b = _regs_curr.bc.b;
            _regs_curr.value = _regs_curr.bc.b;
            break;
        case 0x41:
            _regs_prev.value = _regs_curr.bc.b;
            _regs_curr.bc.b = _regs_curr.bc.c;
            _regs_curr.value = _regs_curr.bc.b;
            break;
        case 0x42:
            _regs_prev.value = _regs_curr.bc.b;
            _regs_curr.bc.b = _regs_curr.de.d;
            _regs_curr.value = _regs_curr.bc.b;
            break;
        case 0x43:
            _regs_prev.value = _regs_curr.bc.b;
            _regs_curr.bc.b = _regs_curr.de.e;
            _regs_curr.value = _regs_curr.bc.b;
            break;
        case 0x44:
            _regs_prev.value = _regs_curr.bc.b;
            _regs_curr.bc.b = _regs_curr.hl.h;
            _regs_curr.value = _regs_curr.bc.b;
            break;
        case 0x45:
            _regs_prev.value = _regs_curr.bc.b;
            _regs_curr.bc.b = _regs_curr.hl.l;
            _regs_curr.value = _regs_curr.bc.b;
            break;
        case 0x46:
            _regs_prev.value = _regs_curr.bc.b;
            _regs_curr.bc.b = mem_get_byte(_regs_curr.hl.word);
            _regs_curr.value = _regs_curr.bc.b;
            break;
        case 0x47:
            _regs_prev.value = _regs_curr.bc.b;
            _regs_curr.bc.b = _regs_curr.af.a;
            _regs_curr.value = _regs_curr.bc.b;
            break;
        case 0x48:
            _regs_prev.value = _regs_curr.bc.c;
            _regs_curr.bc.c = _regs_curr.bc.b;
            _regs_curr.value = _regs_curr.bc.c;
            break;
        case 0x49:
            _regs_prev.value = _regs_curr.bc.c;
            _regs_curr.bc.c = _regs_curr.bc.c;
            _regs_curr.value = _regs_curr.bc.c;
            break;
        case 0x4a:
            _regs_prev.value = _regs_curr.bc.c;
            _regs_curr.bc.c = _regs_curr.de.d;
            _regs_curr.value = _regs_curr.bc.c;
            break;
        case 0x4b:
            _regs_prev.value = _regs_curr.bc.c;
            _regs_curr.bc.c = _regs_curr.de.e;
            _regs_curr.value = _regs_curr.bc.c;
            break;
        case 0x4c:
            _regs_prev.value = _regs_curr.bc.c;
            _regs_curr.bc.c = _regs_curr.hl.h;
            _regs_curr.value = _regs_curr.bc.c;
            break;
        case 0x4d:
            _regs_prev.value = _regs_curr.bc.c;
            _regs_curr.bc.c = _regs_curr.hl.l;
            _regs_curr.value = _regs_curr.bc.c;
            break;
        case 0x4e:
            _regs_prev.value = _regs_curr.bc.c;
            _regs_curr.bc.c = mem_get_byte(_regs_curr.hl.word);
            _regs_curr.value = _regs_curr.bc.c;
            break;
        case 0x4f:
            _regs_prev.value = _regs_curr.bc.c;
            _regs_curr.bc.c = _regs_curr.af.a;
            _regs_curr.value = _regs_curr.bc.c;
            break;
        case 0x50:
            _regs_prev.value = _regs_curr.de.d;
            _regs_curr.de.d = _regs_curr.bc.b;
            _regs_curr.value = _regs_curr.de.d;
            break;
        case 0x51:
            _regs_prev.value = _regs_curr.de.d;
            _regs_curr.de.d = _regs_curr.bc.c;
            _regs_curr.value = _regs_curr.de.d;
            break;
        case 0x52:
            _regs_prev.value = _regs_curr.de.d;
            _regs_curr.de.d = _regs_curr.de.d;
            _regs_curr.value = _regs_curr.de.d;
            break;
        case 0x53:
            _regs_prev.value = _regs_curr.de.d;
            _regs_curr.de.d = _regs_curr.de.e;
            _regs_curr.value = _regs_curr.de.d;
            break;
        case 0x54:
            _regs_prev.value = _regs_curr.de.d;
            _regs_curr.de.d = _regs_curr.hl.h;
            _regs_curr.value = _regs_curr.de.d;
            break;
        case 0x55:
            _regs_prev.value = _regs_curr.de.d;
            _regs_curr.de.d = _regs_curr.hl.l;
            _regs_curr.value = _regs_curr.de.d;
            break;
        case 0x56:
            _regs_prev.value = _regs_curr.de.d;
            _regs_curr.de.d = mem_get_byte(_regs_curr.hl.word);
            _regs_curr.value = _regs_curr.de.d;
            break;
        case 0x57:
            _regs_prev.value = _regs_curr.de.d;
            _regs_curr.de.d = _regs_curr.af.a;
            _regs_curr.value = _regs_curr.de.d;
            break;
        case 0x58:
            _regs_prev.value = _regs_curr.de.e;
            _regs_curr.de.e = _regs_curr.bc.b;
            _regs_curr.value = _regs_curr.de.e;
            break;
        case 0x59:
            _regs_prev.value = _regs_curr.de.e;
            _regs_curr.de.e = _regs_curr.bc.c;
            _regs_curr.value = _regs_curr.de.e;
            break;
        case 0x5a:
            _regs_prev.value = _regs_curr.de.e;
            _regs_curr.de.e = _regs_curr.de.d;
            _regs_curr.value = _regs_curr.de.e;
            break;
        case 0x5b:
            _regs_prev.value = _regs_curr.de.e;
            _regs_curr.de.e = _regs_curr.de.e;
            _regs_curr.value = _regs_curr.de.e;
            break;
        case 0x5c:
            _regs_prev.value = _regs_curr.de.e;
            _regs_curr.de.e = _regs_curr.hl.h;
            _regs_curr.value = _regs_curr.de.e;
            break;
        case 0x5d:
            _regs_prev.value = _regs_curr.de.e;
            _regs_curr.de.e = _regs_curr.hl.l;
            _regs_curr.value = _regs_curr.de.e;
            break;
        case 0x5e:
            _regs_prev.value = _regs_curr.de.e;
            _regs_curr.de.e = mem_get_byte(_regs_curr.hl.word);
            _regs_curr.value = _regs_curr.de.e;
            break;
        case 0x5f:
            _regs_prev.value = _regs_curr.de.e;
            _regs_curr.de.e = _regs_curr.af.a;
            _regs_curr.value = _regs_curr.de.e;
            break;
        case 0x60:
            _regs_prev.value = _regs_curr.hl.h;
            _regs_curr.hl.h = _regs_curr.bc.b;
            _regs_curr.value = _regs_curr.hl.h;
            break;
        case 0x61:
            _regs_prev.value = _regs_curr.hl.h;
            _regs_curr.hl.h = _regs_curr.bc.c;
            _regs_curr.value = _regs_curr.hl.h;
            break;
        case 0x62:
            _regs_prev.value = _regs_curr.hl.h;
            _regs_curr.hl.h = _regs_curr.de.d;
            _regs_curr.value = _regs_curr.hl.h;
            break;
        case 0x63:
            _regs_prev.value = _regs_curr.hl.h;
            _regs_curr.hl.h = _regs_curr.de.e;
            _regs_curr.value = _regs_curr.hl.h;
            break;
        case 0x64:
            _regs_prev.value = _regs_curr.hl.h;
            _regs_curr.hl.h = _regs_curr.hl.h;
            _regs_curr.value = _regs_curr.hl.h;
            break;
        case 0x65:
            _regs_prev.value = _regs_curr.hl.h;
            _regs_curr.hl.h = _regs_curr.hl.l;
            _regs_curr.value = _regs_curr.hl.h;
            break;
        case 0x66:
            _regs_prev.value = _regs_curr.hl.h;
            _regs_curr.hl.h = mem_get_byte(_regs_curr.hl.word);
            _regs_curr.value = _regs_curr.hl.h;
            break;
        case 0x67:
            _regs_prev.value = _regs_curr.hl.h;
            _regs_curr.hl.h = _regs_curr.af.a;
            _regs_curr.value = _regs_curr.hl.h;
            break;
        case 0x68:
            _regs_prev.value = _regs_curr.hl.l;
            _regs_curr.hl.l = _regs_curr.bc.b;
            _regs_curr.value = _regs_curr.hl.l;
            break;
        case 0x69:
            _regs_prev.value = _regs_curr.hl.l;
            _regs_curr.hl.l = _regs_curr.bc.c;
            _regs_curr.value = _regs_curr.hl.l;
            break;
        case 0x6a:
            _regs_prev.value = _regs_curr.hl.l;
            _regs_curr.hl.l = _regs_curr.de.d;
            _regs_curr.value = _regs_curr.hl.l;
            break;
        case 0x6b:
            _regs_prev.value = _regs_curr.hl.l;
            _regs_curr.hl.l = _regs_curr.de.e;
            _regs_curr.value = _regs_curr.hl.l;
            break;
        case 0x6c:
            _regs_prev.value = _regs_curr.hl.l;
            _regs_curr.hl.l = _regs_curr.hl.h;
            _regs_curr.value = _regs_curr.hl.l;
            break;
        case 0x6d:
            _regs_prev.value = _regs_curr.hl.l;
            _regs_curr.hl.l = _regs_curr.hl.l;
            _regs_curr.value = _regs_curr.hl.l;
            break;
        case 0x6e:
            _regs_prev.value = _regs_curr.hl.l;
            _regs_curr.hl.l = mem_get_byte(_regs_curr.hl.word);
            _regs_curr.value = _regs_curr.hl.l;
            break;
        case 0x6f:
            _regs_prev.value = _regs_curr.hl.l;
            _regs_curr.hl.l = _regs_curr.af.a;
            _regs_curr.value = _regs_curr.hl.l;
            break;
        case 0x70:
            _regs_prev.value = mem_get_byte(_regs_curr.hl.word);
            mem_set_byte(_regs_curr.hl.word, _regs_curr.bc.b);
            _regs_curr.value = mem_get_byte(_regs_curr.hl.word);
            break;
        case 0x71:
            _regs_prev.value = mem_get_byte(_regs_curr.hl.word);
            mem_set_byte(_regs_curr.hl.word, _regs_curr.bc.c);
            _regs_curr.value = mem_get_byte(_regs_curr.hl.word);
            break;
        case 0x72:
            _regs_prev.value = mem_get_byte(_regs_curr.hl.word);
            mem_set_byte(_regs_curr.hl.word, _regs_curr.de.d);
            _regs_curr.value = mem_get_byte(_regs_curr.hl.word);
            break;
        case 0x73:
            _regs_prev.value = mem_get_byte(_regs_curr.hl.word);
            mem_set_byte(_regs_curr.hl.word, _regs_curr.de.e);
            _regs_curr.value = mem_get_byte(_regs_curr.hl.word);
            break;
        case 0x74:
            _regs_prev.value = mem_get_byte(_regs_curr.hl.word);
            mem_set_byte(_regs_curr.hl.word, _regs_curr.hl.h);
            _regs_curr.value = mem_get_byte(_regs_curr.hl.word);
            break;
        case 0x75:
            _regs_prev.value = mem_get_byte(_regs_curr.hl.word);
            mem_set_byte(_regs_curr.hl.word, _regs_curr.hl.l);
            _regs_curr.value = mem_get_byte(_regs_curr.hl.word);
            break;
        case 0x76:
            _regs_curr.ip --; // Halt infinity loop
            break;
        case 0x77:
            _regs_prev.value = mem_get_byte(_regs_curr.hl.word);
            mem_set_byte(_regs_curr.hl.word, _regs_curr.af.a);
            _regs_curr.value = mem_get_byte(_regs_curr.hl.word);
            break;
        case 0x78:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = _regs_curr.bc.b;
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0x79:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = _regs_curr.bc.c;
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0x7a:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = _regs_curr.de.d;
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0x7b:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = _regs_curr.de.e;
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0x7c:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = _regs_curr.hl.h;
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0x7d:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = _regs_curr.hl.l;
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0x7e:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = mem_get_byte(_regs_curr.hl.word);
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0x7f:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = _regs_curr.af.a;
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0x80:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = alu_add_byte(_regs_curr.af.a, _regs_curr.bc.b);
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0x81:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = alu_add_byte(_regs_curr.af.a, _regs_curr.bc.c);
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0x82:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = alu_add_byte(_regs_curr.af.a, _regs_curr.de.d);
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0x83:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = alu_add_byte(_regs_curr.af.a, _regs_curr.de.e);
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0x84:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = alu_add_byte(_regs_curr.af.a, _regs_curr.hl.h);
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0x85:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = alu_add_byte(_regs_curr.af.a, _regs_curr.hl.l);
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0x86:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = alu_add_byte(_regs_curr.af.a, mem_get_byte(_regs_curr.hl.word));
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0x87:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = alu_add_byte(_regs_curr.af.a, _regs_curr.af.a);
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0x88:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = alu_adc_byte(_regs_curr.af.a, _regs_curr.bc.b);
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0x89:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = alu_adc_byte(_regs_curr.af.a, _regs_curr.bc.c);
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0x8a:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = alu_adc_byte(_regs_curr.af.a, _regs_curr.de.d);
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0x8b:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = alu_adc_byte(_regs_curr.af.a, _regs_curr.de.e);
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0x8c:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = alu_adc_byte(_regs_curr.af.a, _regs_curr.hl.h);
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0x8d:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = alu_adc_byte(_regs_curr.af.a, _regs_curr.hl.l);
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0x8e:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = alu_adc_byte(_regs_curr.af.a, mem_get_byte(_regs_curr.hl.word));
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0x8f:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = alu_adc_byte(_regs_curr.af.a, _regs_curr.af.a);
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0x90:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = alu_sub_byte(_regs_curr.af.a, _regs_curr.bc.b);
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0x91:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = alu_sub_byte(_regs_curr.af.a, _regs_curr.bc.c);
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0x92:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = alu_sub_byte(_regs_curr.af.a, _regs_curr.de.d);
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0x93:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = alu_sub_byte(_regs_curr.af.a, _regs_curr.de.e);
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0x94:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = alu_sub_byte(_regs_curr.af.a, _regs_curr.hl.h);
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0x95:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = alu_sub_byte(_regs_curr.af.a, _regs_curr.hl.l);
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0x96:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = alu_sub_byte(_regs_curr.af.a, mem_get_byte(_regs_curr.hl.word));
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0x97:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = alu_sub_byte(_regs_curr.af.a, _regs_curr.af.a);
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0x98:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = alu_sbc_byte(_regs_curr.af.a, _regs_curr.bc.b);
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0x99:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = alu_sbc_byte(_regs_curr.af.a, _regs_curr.bc.c);
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0x9a:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = alu_sbc_byte(_regs_curr.af.a, _regs_curr.de.d);
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0x9b:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = alu_sbc_byte(_regs_curr.af.a, _regs_curr.de.e);
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0x9c:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = alu_sbc_byte(_regs_curr.af.a, _regs_curr.hl.h);
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0x9d:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = alu_sbc_byte(_regs_curr.af.a, _regs_curr.hl.l);
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0x9e:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = alu_sbc_byte(_regs_curr.af.a, mem_get_byte(_regs_curr.hl.word));
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0x9f:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = alu_sbc_byte(_regs_curr.af.a, _regs_curr.af.a);
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0xa0:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = alu_and_byte(_regs_curr.af.a, _regs_curr.bc.b);
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0xa1:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = alu_and_byte(_regs_curr.af.a, _regs_curr.bc.c);
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0xa2:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = alu_and_byte(_regs_curr.af.a, _regs_curr.de.d);
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0xa3:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = alu_and_byte(_regs_curr.af.a, _regs_curr.de.e);
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0xa4:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = alu_and_byte(_regs_curr.af.a, _regs_curr.hl.h);
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0xa5:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = alu_and_byte(_regs_curr.af.a, _regs_curr.hl.l);
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0xa6:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = alu_and_byte(_regs_curr.af.a, mem_get_byte(_regs_curr.hl.word));
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0xa7:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = alu_and_byte(_regs_curr.af.a, _regs_curr.af.a);
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0xa8:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = alu_xor_byte(_regs_curr.af.a, _regs_curr.bc.b);
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0xa9:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = alu_xor_byte(_regs_curr.af.a, _regs_curr.bc.c);
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0xaa:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = alu_xor_byte(_regs_curr.af.a, _regs_curr.de.d);
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0xab:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = alu_xor_byte(_regs_curr.af.a, _regs_curr.de.e);
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0xac:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = alu_xor_byte(_regs_curr.af.a, _regs_curr.hl.h);
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0xad:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = alu_xor_byte(_regs_curr.af.a, _regs_curr.hl.l);
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0xae:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = alu_xor_byte(_regs_curr.af.a, mem_get_byte(_regs_curr.hl.word));
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0xaf:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = alu_xor_byte(_regs_curr.af.a, _regs_curr.af.a);
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0xb0:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = alu_or_byte(_regs_curr.af.a, _regs_curr.bc.b);
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0xb1:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = alu_or_byte(_regs_curr.af.a, _regs_curr.bc.c);
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0xb2:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = alu_or_byte(_regs_curr.af.a, _regs_curr.de.d);
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0xb3:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = alu_or_byte(_regs_curr.af.a, _regs_curr.de.e);
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0xb4:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = alu_or_byte(_regs_curr.af.a, _regs_curr.hl.h);
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0xb5:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = alu_or_byte(_regs_curr.af.a, _regs_curr.hl.l);
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0xb6:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = alu_or_byte(_regs_curr.af.a, mem_get_byte(_regs_curr.hl.word));
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0xb7:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = alu_or_byte(_regs_curr.af.a, _regs_curr.af.a);
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0xb8:
            _regs_prev.value = _regs_curr.af.a;
            alu_sub_byte(_regs_curr.af.a, _regs_curr.bc.b);
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0xb9:
            _regs_prev.value = _regs_curr.af.a;
            alu_sub_byte(_regs_curr.af.a, _regs_curr.bc.c);
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0xba:
            _regs_prev.value = _regs_curr.af.a;
            alu_sub_byte(_regs_curr.af.a, _regs_curr.de.d);
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0xbb:
            _regs_prev.value = _regs_curr.af.a;
            alu_sub_byte(_regs_curr.af.a, _regs_curr.de.e);
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0xbc:
            _regs_prev.value = _regs_curr.af.a;
            alu_sub_byte(_regs_curr.af.a, _regs_curr.hl.h);
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0xbd:
            _regs_prev.value = _regs_curr.af.a;
            alu_sub_byte(_regs_curr.af.a, _regs_curr.hl.l);
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0xbe:
            _regs_prev.value = _regs_curr.af.a;
            alu_sub_byte(_regs_curr.af.a, mem_get_byte(_regs_curr.hl.word));
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0xbf:
            _regs_prev.value = _regs_curr.af.a;
            alu_sub_byte(_regs_curr.af.a, _regs_curr.af.a);
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0xc0:
            if(!zf_get())
            {
                _regs_prev.value = _regs_curr.ip;
                _regs_curr.ip = mem_get_word(_regs_curr.sp);
                _regs_curr.sp+=2;
                _regs_curr.value = _regs_curr.ip;
            }
            break;
        case 0xc1:
            _regs_prev.value = _regs_curr.bc.word;
            _regs_curr.bc.word = mem_get_word(_regs_curr.sp);
            _regs_curr.sp+=2;
            _regs_curr.value = _regs_curr.bc.word;
            break;
        case 0xc2:
            tmp = ip_get_word();
            if(!zf_get())
            {
                _regs_prev.value = _regs_curr.ip;
                _regs_curr.ip = tmp;
                _regs_curr.value = _regs_curr.ip;
            }
            break;
        case 0xc3:
            _regs_prev.value = _regs_curr.ip;
            _regs_curr.ip = ip_get_word();
            _regs_curr.value = _regs_curr.ip;
            break;
        case 0xc4:
            tmp = ip_get_word();
            if(!zf_get())
            {
                _regs_curr.sp-=2;
                mem_set_word(_regs_curr.sp, _regs_curr.ip);
                _regs_prev.value = _regs_curr.ip;
                _regs_curr.ip = tmp;
                _regs_curr.value = _regs_curr.ip;
            }
            break;
        case 0xc5:
            _regs_prev.value = _regs_curr.sp;
            _regs_curr.sp-=2;
            mem_set_word(_regs_curr.sp, _regs_curr.bc.word);
            _regs_curr.value = _regs_curr.sp;
            break;
        case 0xc6:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = alu_add_byte(_regs_curr.af.a, ip_get_byte());
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0xc7:
            _regs_curr.sp-=2;
            mem_set_word(_regs_curr.sp, _regs_curr.ip);
            _regs_prev.value = _regs_curr.ip;
            _regs_curr.ip = 0;
            _regs_curr.value = _regs_curr.ip;
            break;
        case 0xc8:
            if(zf_get())
            {
                _regs_prev.value = _regs_curr.ip;
                _regs_curr.ip = mem_get_word(_regs_curr.sp);
                _regs_curr.sp+=2;
                _regs_curr.value = _regs_curr.ip;
            }
            break;
        case 0xc9:
            _regs_prev.value = _regs_curr.ip;
            _regs_curr.ip = mem_get_word(_regs_curr.sp);
            _regs_curr.sp+=2;
            _regs_curr.value = _regs_curr.ip;
            break;
        case 0xca:
            tmp = ip_get_word();
            if(zf_get())
            {
                _regs_prev.value = _regs_curr.ip;
                _regs_curr.ip = tmp;
                _regs_curr.value = _regs_curr.ip;
            }
            break;
        case 0xcb:
            _regs_curr.prefix_cb = true;
            break;
        case 0xcc:
            tmp = ip_get_word();
            if(zf_get())
            {
                _regs_curr.sp-=2;
                mem_set_word(_regs_curr.sp, _regs_curr.ip);
                _regs_prev.value = _regs_curr.ip;
                _regs_curr.ip = tmp;
                _regs_curr.value = _regs_curr.ip;
            }
            break;
        case 0xcd:
            tmp = ip_get_word();
            _regs_curr.sp-=2;
            mem_set_word(_regs_curr.sp, _regs_curr.ip);
            _regs_prev.value = _regs_curr.ip;
            _regs_curr.ip = tmp;
            _regs_curr.value = _regs_curr.ip;
            break;
        case 0xce:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = alu_adc_byte(_regs_curr.af.a, ip_get_byte());
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0xcf:
            _regs_curr.sp-=2;
            mem_set_word(_regs_curr.sp, _regs_curr.ip);
            _regs_prev.value = _regs_curr.ip;
            _regs_curr.ip = 8;
            _regs_curr.value = _regs_curr.ip;
            break;
        case 0xd0:
            if(!cf_get())
            {
                _regs_prev.value = _regs_curr.ip;
                _regs_curr.ip = mem_get_word(_regs_curr.sp);
                _regs_curr.sp+=2;
                _regs_curr.value = _regs_curr.ip;
            }
            break;
        case 0xd1:
            _regs_prev.value = _regs_curr.de.word;
            _regs_curr.de.word = mem_get_word(_regs_curr.sp);
            _regs_curr.sp+=2;
            _regs_curr.value = _regs_curr.de.word;
            break;
        case 0xd2:
            tmp = ip_get_word();
            if(!cf_get())
            {
                _regs_prev.value = _regs_curr.ip;
                _regs_curr.ip = tmp;
                _regs_curr.value = _regs_curr.ip;
            }
            break;
        case 0xd3:
            _regs_prev.value = _regs_curr.af.a;
            hardware_out(ip_get_byte(), _regs_curr.af.a);
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0xd4:
            tmp = ip_get_word();
            if(!cf_get())
            {
                _regs_curr.sp-=2;
                mem_set_word(_regs_curr.sp, _regs_curr.ip);
                _regs_prev.value = _regs_curr.ip;
                _regs_curr.ip = tmp;
                _regs_curr.value = _regs_curr.ip;
            }
            break;
        case 0xd5:
            _regs_prev.value = _regs_curr.sp;
            _regs_curr.sp-=2;
            mem_set_word(_regs_curr.sp, _regs_curr.de.word);
            _regs_curr.value = _regs_curr.sp;
            break;
        case 0xd6:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = alu_sub_byte(_regs_curr.af.a, ip_get_byte());
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0xd7:
            _regs_curr.sp-=2;
            mem_set_word(_regs_curr.sp, _regs_curr.ip);
            _regs_prev.value = _regs_curr.ip;
            _regs_curr.ip = 0x10;
            _regs_curr.value = _regs_curr.ip;
            break;
        case 0xd8:
            if(cf_get())
            {
                _regs_prev.value = _regs_curr.ip;
                _regs_curr.ip = mem_get_word(_regs_curr.sp);
                _regs_curr.sp+=2;
                _regs_curr.value = _regs_curr.ip;
            }
            break;
        case 0xd9:
            tmp = _regs_curr.hl.word;
            _regs_curr.hl.word = _regs_curr.hl_alt;
            _regs_curr.hl_alt = tmp;
            tmp = _regs_curr.bc.word;
            _regs_curr.bc.word = _regs_curr.bc_alt;
            _regs_curr.bc_alt = tmp;
            tmp = _regs_curr.de.word;
            _regs_curr.de.word = _regs_curr.de_alt;
            _regs_curr.de_alt = tmp;
            break;
        case 0xda:
            tmp = ip_get_word();
            if(cf_get())
            {
                _regs_prev.value = _regs_curr.ip;
                _regs_curr.ip = tmp;
                _regs_curr.value = _regs_curr.ip;
            }
            break;
        case 0xdb:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = hardware_in(ip_get_byte());
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0xdc:
            tmp = ip_get_word();
            if(cf_get())
            {
                _regs_curr.sp-=2;
                mem_set_word(_regs_curr.sp, _regs_curr.ip);
                _regs_prev.value = _regs_curr.ip;
                _regs_curr.ip = tmp;
                _regs_curr.value = _regs_curr.ip;
            }
            break;
        case 0xdd:
            _regs_curr.prefix_dd = true;
            break;
        case 0xde:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = alu_sbc_byte(_regs_curr.af.a, ip_get_byte());
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0xdf:
            _regs_curr.sp-=2;
            mem_set_word(_regs_curr.sp, _regs_curr.ip);
            _regs_prev.value = _regs_curr.ip;
            _regs_curr.ip = 0x18;
            _regs_curr.value = _regs_curr.ip;
            break;
        case 0xe0:
            if(!pvf_get())
            {
                _regs_prev.value = _regs_curr.ip;
                _regs_curr.ip = mem_get_word(_regs_curr.sp);
                _regs_curr.sp+=2;
                _regs_curr.value = _regs_curr.ip;
            }
            break;
        case 0xe1:
            _regs_prev.value = _regs_curr.hl.word;
            _regs_curr.hl.word = mem_get_word(_regs_curr.sp);
            _regs_curr.sp+=2;
            _regs_curr.value = _regs_curr.hl.word;
            break;
        case 0xe2:
            tmp = ip_get_word();
            if(!pvf_get())
            {
                _regs_prev.value = _regs_curr.ip;
                _regs_curr.ip = tmp;
                _regs_curr.value = _regs_curr.ip;
            }
            break;
        case 0xe3:
            _regs_prev.value = _regs_curr.hl.word;
            tmp = mem_get_word(_regs_curr.sp);
            mem_set_word(_regs_curr.sp, _regs_curr.hl.word);
            _regs_curr.hl.word = tmp;
            _regs_curr.value = _regs_curr.hl.word;
            break;
        case 0xe4:
            tmp = ip_get_word();
            if(!pvf_get())
            {
                _regs_curr.sp-=2;
                mem_set_word(_regs_curr.sp, _regs_curr.ip);
                _regs_prev.value = _regs_curr.ip;
                _regs_curr.ip = tmp;
                _regs_curr.value = _regs_curr.ip;
            }
            break;
        case 0xe5:
            _regs_prev.value = _regs_curr.sp;
            _regs_curr.sp-=2;
            mem_set_word(_regs_curr.sp, _regs_curr.hl.word);
            _regs_curr.value = _regs_curr.sp;
            break;
        case 0xe6:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = alu_and_byte(_regs_curr.af.a, ip_get_byte());
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0xe7:
            _regs_curr.sp-=2;
            mem_set_word(_regs_curr.sp, _regs_curr.ip);
            _regs_prev.value = _regs_curr.ip;
            _regs_curr.ip = 0x20;
            _regs_curr.value = _regs_curr.ip;
            break;
        case 0xe8:
            if(pvf_get())
            {
                _regs_prev.value = _regs_curr.ip;
                _regs_curr.ip = mem_get_word(_regs_curr.sp);
                _regs_curr.sp+=2;
                _regs_curr.value = _regs_curr.ip;
            }
            break;
        case 0xe9:
            _regs_prev.value = _regs_curr.ip;
            _regs_curr.ip = mem_get_word(_regs_curr.hl.word);
            _regs_curr.value = _regs_curr.ip;
            break;
        case 0xea:
            tmp = ip_get_word();
            if(pvf_get())
            {
                _regs_prev.value = _regs_curr.ip;
                _regs_curr.ip = tmp;
                _regs_curr.value = _regs_curr.ip;
            }
            break;
        case 0xeb:
            _regs_prev.value = _regs_curr.hl.word;
            tmp = _regs_curr.hl.word;
            _regs_curr.hl.word = _regs_curr.de.word;
            _regs_curr.de.word = tmp;
            _regs_curr.value = _regs_curr.hl.word;
            break;
        case 0xec:
            tmp = ip_get_word();
            if(pvf_get())
            {
                _regs_curr.sp-=2;
                mem_set_word(_regs_curr.sp, _regs_curr.ip);
                _regs_prev.value = _regs_curr.ip;
                _regs_curr.ip = tmp;
                _regs_curr.value = _regs_curr.ip;
            }
            break;
        case 0xed:
            _regs_curr.prefix_ed = true;
            break;
        case 0xee:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = alu_xor_byte(_regs_curr.af.a, ip_get_byte());
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0xef:
            _regs_curr.sp-=2;
            mem_set_word(_regs_curr.sp, _regs_curr.ip);
            _regs_prev.value = _regs_curr.ip;
            _regs_curr.ip = 0x28;
            _regs_curr.value = _regs_curr.ip;
            break;
        case 0xf0:
            if(!sf_get())
            {
                _regs_prev.value = _regs_curr.ip;
                _regs_curr.ip = mem_get_word(_regs_curr.sp);
                _regs_curr.sp+=2;
                _regs_curr.value = _regs_curr.ip;
            }
            break;
        case 0xf1:
            _regs_prev.value = _regs_curr.af.word;
            _regs_curr.af.word = mem_get_word(_regs_curr.sp);
            _regs_curr.sp+=2;
            _regs_curr.value = _regs_curr.af.word;
            break;
        case 0xf2:
            tmp = ip_get_word();
            if(!sf_get())
            {
                _regs_prev.value = _regs_curr.ip;
                _regs_curr.ip = tmp;
                _regs_curr.value = _regs_curr.ip;
            }
            break;
        case 0xf3:
            _regs_curr.interrupts = false;
            break;
        case 0xf4:
            tmp = ip_get_word();
            if(!sf_get())
            {
                _regs_curr.sp-=2;
                mem_set_word(_regs_curr.sp, _regs_curr.ip);
                _regs_prev.value = _regs_curr.ip;
                _regs_curr.ip = tmp;
                _regs_curr.value = _regs_curr.ip;
            }
            break;
        case 0xf5:
            _regs_prev.value = _regs_curr.sp;
            _regs_curr.sp-=2;
            mem_set_word(_regs_curr.sp, _regs_curr.af.word);
            _regs_curr.value = _regs_curr.sp;
            break;
        case 0xf6:
            _regs_prev.value = _regs_curr.af.a;
            _regs_curr.af.a = alu_or_byte(_regs_curr.af.a, ip_get_byte());
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0xf7:
            _regs_curr.sp-=2;
            mem_set_word(_regs_curr.sp, _regs_curr.ip);
            _regs_prev.value = _regs_curr.ip;
            _regs_curr.ip = 0x30;
            _regs_curr.value = _regs_curr.ip;
            break;
        case 0xf8:
            if(sf_get())
            {
                _regs_prev.value = _regs_curr.ip;
                _regs_curr.ip = mem_get_word(_regs_curr.sp);
                _regs_curr.sp+=2;
                _regs_curr.value = _regs_curr.ip;
            }
            break;
        case 0xf9:
            _regs_prev.value = _regs_curr.sp;
            _regs_curr.sp = _regs_curr.hl.word;
            _regs_curr.value = _regs_curr.sp;
            break;
        case 0xfa:
            tmp = ip_get_word();
            if(sf_get())
            {
                _regs_prev.value = _regs_curr.ip;
                _regs_curr.ip = tmp;
                _regs_curr.value = _regs_curr.ip;
            }
            break;
        case 0xfb:
            _regs_curr.interrupts = true;
            break;
        case 0xfc:
            tmp = ip_get_word();
            if(sf_get())
            {
                _regs_curr.sp-=2;
                mem_set_word(_regs_curr.sp, _regs_curr.ip);
                _regs_prev.value = _regs_curr.ip;
                _regs_curr.ip = tmp;
                _regs_curr.value = _regs_curr.ip;
            }
            break;
        case 0xfd:
            _regs_curr.prefix_fd = true;
            break;
        case 0xfe:
            _regs_prev.value = _regs_curr.af.a;
            alu_sub_byte(_regs_curr.af.a, ip_get_byte());
            _regs_curr.value = _regs_curr.af.a;
            break;
        case 0xff:
            _regs_curr.sp-=2;
            mem_set_word(_regs_curr.sp, _regs_curr.ip);
            _regs_prev.value = _regs_curr.ip;
            _regs_curr.ip = 0x38;
            _regs_curr.value = _regs_curr.ip;
            break;
    }
    else if(_regs_curr.prefix_ed)
    {
        switch(ip_get_byte())
        {
            case 0x00:
                _regs_prev.value = _regs_curr.bc.b;
                _regs_curr.bc.b = hardware_in(ip_get_byte());
                _regs_curr.value = _regs_curr.bc.b;
                nf_set(0);
                hf_set(0);
                alu_parity(_regs_curr.value);
                zf_set(_regs_curr.value == 0);
                sf_set(_regs_curr.value & 128);
                break;
            case 0x01:
                _regs_prev.value = _regs_curr.bc.b;
                hardware_out(ip_get_byte(), _regs_curr.bc.b);
                _regs_curr.value = _regs_curr.bc.b;
                break;
            case 0x04:
                _regs_prev.value = _regs_curr.bc.b;
                alu_and_byte(_regs_curr.af.a, _regs_curr.bc.b);
                _regs_curr.value = _regs_curr.bc.b;
                break;
            case 0x08:
                _regs_prev.value = _regs_curr.bc.c;
                _regs_curr.bc.c = hardware_in(ip_get_byte());
                _regs_curr.value = _regs_curr.bc.c;
                nf_set(0);
                hf_set(0);
                alu_parity(_regs_curr.value);
                zf_set(_regs_curr.value == 0);
                sf_set(_regs_curr.value & 128);
                break;
            case 0x09:
                _regs_prev.value = _regs_curr.bc.c;
                hardware_out(ip_get_byte(), _regs_curr.bc.c);
                _regs_curr.value = _regs_curr.bc.c;
                break;
            case 0x0c:
                _regs_prev.value = _regs_curr.bc.c;
                alu_and_byte(_regs_curr.af.a, _regs_curr.bc.c);
                _regs_curr.value = _regs_curr.bc.c;
                break;
            case 0x10:
                _regs_prev.value = _regs_curr.de.d;
                _regs_curr.de.d = hardware_in(ip_get_byte());
                _regs_curr.value = _regs_curr.de.d;
                nf_set(0);
                hf_set(0);
                alu_parity(_regs_curr.value);
                zf_set(_regs_curr.value == 0);
                sf_set(_regs_curr.value & 128);
                break;
            case 0x11:
                _regs_prev.value = _regs_curr.de.d;
                hardware_out(ip_get_byte(), _regs_curr.de.d);
                _regs_curr.value = _regs_curr.de.d;
                break;
            case 0x14:
                _regs_prev.value = _regs_curr.de.d;
                alu_and_byte(_regs_curr.af.a, _regs_curr.de.d);
                _regs_curr.value = _regs_curr.de.d;
                break;
            case 0x18:
                _regs_prev.value = _regs_curr.de.e;
                _regs_curr.de.e = hardware_in(ip_get_byte());
                _regs_curr.value = _regs_curr.de.e;
                nf_set(0);
                hf_set(0);
                alu_parity(_regs_curr.value);
                zf_set(_regs_curr.value == 0);
                sf_set(_regs_curr.value & 128);
                break;
            case 0x19:
                _regs_prev.value = _regs_curr.de.e;
                hardware_out(ip_get_byte(), _regs_curr.de.e);
                _regs_curr.value = _regs_curr.de.e;
                break;
            case 0x1c:
                _regs_prev.value = _regs_curr.de.e;
                alu_and_byte(_regs_curr.af.a, _regs_curr.de.e);
                _regs_curr.value = _regs_curr.de.e;
                break;
            case 0x20:
                _regs_prev.value = _regs_curr.hl.h;
                _regs_curr.hl.h = hardware_in(ip_get_byte());
                _regs_curr.value = _regs_curr.hl.h;
                nf_set(0);
                hf_set(0);
                alu_parity(_regs_curr.value);
                zf_set(_regs_curr.value == 0);
                sf_set(_regs_curr.value & 128);
                break;
            case 0x21:
                _regs_prev.value = _regs_curr.hl.h;
                hardware_out(ip_get_byte(), _regs_curr.hl.h);
                _regs_curr.value = _regs_curr.hl.h;
                break;
            case 0x24:
                _regs_prev.value = _regs_curr.hl.h;
                alu_and_byte(_regs_curr.af.a, _regs_curr.hl.h);
                _regs_curr.value = _regs_curr.hl.h;
                break;
            case 0x28:
                _regs_prev.value = _regs_curr.hl.l;
                _regs_curr.hl.l = hardware_in(ip_get_byte());
                _regs_curr.value = _regs_curr.hl.l;
                nf_set(0);
                hf_set(0);
                alu_parity(_regs_curr.value);
                zf_set(_regs_curr.value == 0);
                sf_set(_regs_curr.value & 128);
                break;
            case 0x29:
                _regs_prev.value = _regs_curr.hl.l;
                hardware_out(ip_get_byte(), _regs_curr.hl.l);
                _regs_curr.value = _regs_curr.hl.l;
                break;
            case 0x2c:
                _regs_prev.value = _regs_curr.hl.l;
                alu_and_byte(_regs_curr.af.a, _regs_curr.hl.l);
                _regs_curr.value = _regs_curr.hl.l;
                break;
            case 0x34:
                _regs_prev.value = mem_get_byte(_regs_curr.hl.word);
                alu_and_byte(_regs_curr.af.a, mem_get_byte(_regs_curr.hl.word));
                _regs_curr.value = mem_get_byte(_regs_curr.hl.word);
                break;
            case 0x38:
                _regs_prev.value = _regs_curr.af.a;
                _regs_curr.af.a = hardware_in(ip_get_byte());
                _regs_curr.value = _regs_curr.af.a;
                nf_set(0);
                hf_set(0);
                alu_parity(_regs_curr.value);
                zf_set(_regs_curr.value == 0);
                sf_set(_regs_curr.value & 128);
                break;
            case 0x39:
                _regs_prev.value = _regs_curr.af.a;
                hardware_out(ip_get_byte(), _regs_curr.af.a);
                _regs_curr.value = _regs_curr.af.a;
                break;
            case 0x3c:
                _regs_prev.value = _regs_curr.af.a;
                alu_and_byte(_regs_curr.af.a, _regs_curr.af.a);
                _regs_curr.value = _regs_curr.af.a;
                break;
            default: break;
        }
    }
    if(reset_prefixes)
    {
        _regs_curr.prefix_cb = false;
        _regs_curr.prefix_dd = false;
        _regs_curr.prefix_ed = false;
        _regs_curr.prefix_fd = false;
    }
}


void exec()
{
    memset(&_regs_curr, 0, sizeof(z80_regs_t));
    _regs_curr.ip = 0x100;
    _regs_curr.sp = 0xf000;
    while(_executing)
    {
        exec_step();
        if(_debug) screen_draw();
        else screen_draw_if_changed();
    }
}