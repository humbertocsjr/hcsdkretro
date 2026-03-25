#include "emu.h"


void exec_step()
{
    memcpy(&_regs_prev, &_regs_curr, sizeof(z80_regs_t));
    _regs_prev.value = 0;
    _regs_curr.value = 0;
    bool reset_prefixes = true;
    uint16_t tmp;
    int8_t offset_value;
    if(_regs_curr.ip == 5)
    {
        // MSX-DOS ABI
        abi_dos_call_5();
    }
    else if(_regs_curr.ip == 0)
    {
        _executing = false;
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
            _regs_curr.af.a = alu_rlc(_regs_curr.af.a);
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
                if(_skip_call_step && _skip_call_step_address < 0)
                {
                    _skip_call_step_address = mem_get_word(_regs_curr.sp);
                }
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
            if(_skip_call_step && _skip_call_step_address < 0)
            {
                _skip_call_step_address = mem_get_word(_regs_curr.sp);
            }
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
            reset_prefixes = false;
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
                if(_skip_call_step && _skip_call_step_address < 0)
                {
                    _skip_call_step_address = mem_get_word(_regs_curr.sp);
                }
            }
            break;
        case 0xcd:
            tmp = ip_get_word();
            _regs_curr.sp-=2;
            mem_set_word(_regs_curr.sp, _regs_curr.ip);
            _regs_prev.value = _regs_curr.ip;
            _regs_curr.ip = tmp;
            _regs_curr.value = _regs_curr.ip;
            if(_skip_call_step && _skip_call_step_address < 0)
            {
                _skip_call_step_address = mem_get_word(_regs_curr.sp);
            }
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
            if(_skip_call_step && _skip_call_step_address < 0)
            {
                _skip_call_step_address = mem_get_word(_regs_curr.sp);
            }
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
                if(_skip_call_step && _skip_call_step_address < 0)
                {
                    _skip_call_step_address = mem_get_word(_regs_curr.sp);
                }
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
            if(_skip_call_step && _skip_call_step_address < 0)
            {
                _skip_call_step_address = mem_get_word(_regs_curr.sp);
            }
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
                if(_skip_call_step && _skip_call_step_address < 0)
                {
                    _skip_call_step_address = mem_get_word(_regs_curr.sp);
                }
            }
            break;
        case 0xdd:
            _regs_curr.prefix_dd = true;
            reset_prefixes = false;
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
            if(_skip_call_step && _skip_call_step_address < 0)
            {
                _skip_call_step_address = mem_get_word(_regs_curr.sp);
            }
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
                if(_skip_call_step && _skip_call_step_address < 0)
                {
                    _skip_call_step_address = mem_get_word(_regs_curr.sp);
                }
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
            if(_skip_call_step && _skip_call_step_address < 0)
            {
                _skip_call_step_address = mem_get_word(_regs_curr.sp);
            }
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
                if(_skip_call_step && _skip_call_step_address < 0)
                {
                    _skip_call_step_address = mem_get_word(_regs_curr.sp);
                }
            }
            break;
        case 0xed:
            _regs_curr.prefix_ed = true;
            reset_prefixes = false;
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
            if(_skip_call_step && _skip_call_step_address < 0)
            {
                _skip_call_step_address = mem_get_word(_regs_curr.sp);
            }
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
                if(_skip_call_step && _skip_call_step_address < 0)
                {
                    _skip_call_step_address = mem_get_word(_regs_curr.sp);
                }
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
            if(_skip_call_step && _skip_call_step_address < 0)
            {
                _skip_call_step_address = mem_get_word(_regs_curr.sp);
            }
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
                if(_skip_call_step && _skip_call_step_address < 0)
                {
                    _skip_call_step_address = mem_get_word(_regs_curr.sp);
                }
            }
            break;
        case 0xfd:
            _regs_curr.prefix_fd = true;
            reset_prefixes = false;
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
            if(_skip_call_step && _skip_call_step_address < 0)
            {
                _skip_call_step_address = mem_get_word(_regs_curr.sp);
            }
            break;
    }
    else if(_regs_curr.prefix_ed)
    {
        switch(ip_get_byte())
        {
            case 0x40:
                _regs_prev.value = _regs_curr.bc.b;
                _regs_curr.bc.b = hardware_in(_regs_curr.bc.c);
                _regs_curr.value = _regs_curr.bc.b;
                break;
            case 0x41:
                _regs_prev.value = _regs_curr.bc.b;
                hardware_out(_regs_curr.bc.c, _regs_curr.bc.b);
                _regs_curr.value = _regs_curr.bc.b;
                break;
            case 0x42:
                _regs_prev.value = _regs_curr.hl.word;
                _regs_curr.hl.word = alu_sbc_word(_regs_curr.hl.word, _regs_curr.bc.word);
                _regs_curr.value = _regs_curr.hl.word;
                break;
            case 0x43:
                tmp = ip_get_word();
                _regs_prev.value = mem_get_word(tmp);
                mem_set_word(tmp, _regs_curr.bc.word);
                _regs_curr.value = mem_get_word(tmp);
                break;
            case 0x44:
                _regs_prev.value = _regs_curr.af.a;
                _regs_curr.af.a = alu_sbc_byte(0, _regs_curr.af.a);
                _regs_curr.value = _regs_curr.af.a;
                break;
            case 0x45:
                _regs_prev.value = _regs_curr.ip;
                _regs_curr.ip = mem_get_word(_regs_curr.sp);
                _regs_curr.sp+=2;
                _regs_curr.value = _regs_curr.ip;
                break;
            case 0x46:
                _regs_prev.value = _regs_curr.interrupt_mode;
                _regs_curr.interrupt_mode = 0;
                _regs_curr.value = _regs_curr.interrupt_mode;
                break;
            case 0x47:
                _regs_prev.value = _regs_curr.i;
                _regs_curr.i = _regs_curr.af.a;
                _regs_curr.value = _regs_curr.i;
                break;
            case 0x48:
                _regs_prev.value = _regs_curr.bc.c;
                _regs_curr.bc.c = hardware_in(_regs_curr.bc.c);
                _regs_curr.value = _regs_curr.bc.c;
                break;
            case 0x49:
                _regs_prev.value = _regs_curr.bc.c;
                hardware_out(_regs_curr.bc.c, _regs_curr.bc.c);
                _regs_curr.value = _regs_curr.bc.c;
                break;
            case 0x4a:
                _regs_prev.value = _regs_curr.hl.word;
                _regs_curr.hl.word = alu_adc_word(_regs_curr.hl.word, _regs_curr.bc.word);
                _regs_curr.value = _regs_curr.hl.word;
                break;
            case 0x4b:
                tmp = ip_get_word();
                _regs_prev.value = _regs_curr.bc.word;
                _regs_curr.bc.word = mem_get_word(tmp);
                _regs_curr.value = _regs_curr.bc.word;
                break;
            case 0x4d:
                _regs_prev.value = _regs_curr.ip;
                _regs_curr.ip = mem_get_word(_regs_curr.sp);
                _regs_curr.sp+=2;
                _regs_curr.value = _regs_curr.ip;
                break;
            case 0x4f:
                _regs_prev.value = _regs_curr.r;
                _regs_curr.r = _regs_curr.af.a;
                _regs_curr.value = _regs_curr.r;
                break;
            case 0x50:
                _regs_prev.value = _regs_curr.de.d;
                _regs_curr.de.d = hardware_in(_regs_curr.bc.c);
                _regs_curr.value = _regs_curr.de.d;
                break;
            case 0x51:
                _regs_prev.value = _regs_curr.de.d;
                hardware_out(_regs_curr.bc.c, _regs_curr.de.d);
                _regs_curr.value = _regs_curr.de.d;
                break;
            case 0x52:
                _regs_prev.value = _regs_curr.hl.word;
                _regs_curr.hl.word = alu_sbc_word(_regs_curr.hl.word, _regs_curr.de.word);
                _regs_curr.value = _regs_curr.hl.word;
                break;
            case 0x53:
                tmp = ip_get_word();
                _regs_prev.value = mem_get_word(tmp);
                mem_set_word(tmp, _regs_curr.de.word);
                _regs_curr.value = mem_get_word(tmp);
                break;
            case 0x56:
                _regs_prev.value = _regs_curr.interrupt_mode;
                _regs_curr.interrupt_mode = 1;
                _regs_curr.value = _regs_curr.interrupt_mode;
                break;
            case 0x57:
                _regs_prev.value = _regs_curr.af.a;
                _regs_curr.af.a = _regs_curr.i;
                _regs_curr.value = _regs_curr.af.a;
                break;
            case 0x58:
                _regs_prev.value = _regs_curr.de.e;
                _regs_curr.de.e = hardware_in(_regs_curr.bc.c);
                _regs_curr.value = _regs_curr.de.e;
                break;
            case 0x59:
                _regs_prev.value = _regs_curr.de.e;
                hardware_out(_regs_curr.bc.c, _regs_curr.de.e);
                _regs_curr.value = _regs_curr.de.e;
                break;
            case 0x5a:
                _regs_prev.value = _regs_curr.hl.word;
                _regs_curr.hl.word = alu_adc_word(_regs_curr.hl.word, _regs_curr.de.word);
                _regs_curr.value = _regs_curr.hl.word;
                break;
            case 0x5b:
                tmp = ip_get_word();
                _regs_prev.value = _regs_curr.de.word;
                _regs_curr.de.word = mem_get_word(tmp);
                _regs_curr.value = _regs_curr.de.word;
                break;
            case 0x5e:
                _regs_prev.value = _regs_curr.interrupt_mode;
                _regs_curr.interrupt_mode = 2;
                _regs_curr.value = _regs_curr.interrupt_mode;
                break;
            case 0x5f:
                _regs_prev.value = _regs_curr.af.a;
                _regs_curr.af.a = _regs_curr.r;
                _regs_curr.value = _regs_curr.af.a;
                break;
            case 0x60:
                _regs_prev.value = _regs_curr.hl.h;
                _regs_curr.hl.h = hardware_in(_regs_curr.bc.c);
                _regs_curr.value = _regs_curr.hl.h;
                break;
            case 0x61:
                _regs_prev.value = _regs_curr.hl.h;
                hardware_out(_regs_curr.bc.c, _regs_curr.hl.h);
                _regs_curr.value = _regs_curr.hl.h;
                break;
            case 0x62:
                _regs_prev.value = _regs_curr.hl.word;
                _regs_curr.hl.word = alu_sbc_word(_regs_curr.hl.word, _regs_curr.hl.word);
                _regs_curr.value = _regs_curr.hl.word;
                break;
            case 0x63:
                tmp = ip_get_word();
                _regs_prev.value = mem_get_word(tmp);
                mem_set_word(tmp, _regs_curr.hl.word);
                _regs_curr.value = mem_get_word(tmp);
                break;
            case 0x67:
                _regs_prev.value = mem_get_byte(_regs_curr.hl.word) | (_regs_curr.af.a << 8);
                tmp = _regs_curr.af.a & 0xf;
                _regs_curr.af.a = (mem_get_byte(_regs_curr.hl.word) & 0xf) | (_regs_curr.af.a & 0xf0);
                mem_set_byte(_regs_curr.hl.word, (mem_get_byte(_regs_curr.hl.word) >> 4) | (tmp << 4));
                _regs_curr.value = mem_get_byte(_regs_curr.hl.word) | (_regs_curr.af.a << 8);
                alu_parity(_regs_curr.af.a);
                nf_set(0);
                hf_set(0);
                zf_set(_regs_curr.af.a == 0);
                sf_set(_regs_curr.af.a & 128);
                break;
            case 0x68:
                _regs_prev.value = _regs_curr.hl.l;
                _regs_curr.hl.l = hardware_in(_regs_curr.bc.c);
                _regs_curr.value = _regs_curr.hl.l;
                break;
            case 0x69:
                _regs_prev.value = _regs_curr.hl.l;
                hardware_out(_regs_curr.bc.c, _regs_curr.hl.l);
                _regs_curr.value = _regs_curr.hl.l;
                break;
            case 0x6a:
                _regs_prev.value = _regs_curr.hl.word;
                _regs_curr.hl.word = alu_adc_word(_regs_curr.hl.word, _regs_curr.hl.word);
                _regs_curr.value = _regs_curr.hl.word;
                break;
            case 0x6b:
                tmp = ip_get_word();
                _regs_prev.value = _regs_curr.hl.word;
                _regs_curr.hl.word = mem_get_word(tmp);
                _regs_curr.value = _regs_curr.hl.word;
                break;
            case 0x6f:
                _regs_prev.value = mem_get_byte(_regs_curr.hl.word) | (_regs_curr.af.a << 8);
                tmp = _regs_curr.af.a & 0xf;
                _regs_curr.af.a = ((mem_get_byte(_regs_curr.hl.word) >> 4) & 0xf) | (_regs_curr.af.a & 0xf0);
                mem_set_byte(_regs_curr.hl.word, (mem_get_byte(_regs_curr.hl.word) << 4) | (tmp >> 4));
                _regs_curr.value = mem_get_byte(_regs_curr.hl.word) | (_regs_curr.af.a << 8);
                alu_parity(_regs_curr.af.a);
                nf_set(0);
                hf_set(0);
                zf_set(_regs_curr.af.a == 0);
                sf_set(_regs_curr.af.a & 128);
                break;
            case 0x70:
                _regs_curr.value = hardware_in(_regs_curr.bc.c);
                break;
            case 0x71:
                _regs_prev.value = 0;
                hardware_out(_regs_curr.bc.c, 0);
                _regs_curr.value = 0;
                break;
            case 0x72:
                _regs_prev.value = _regs_curr.hl.word;
                _regs_curr.hl.word = alu_sbc_word(_regs_curr.hl.word, _regs_curr.sp);
                _regs_curr.value = _regs_curr.hl.word;
                break;
            case 0x73:
                tmp = ip_get_word();
                _regs_prev.value = mem_get_word(tmp);
                mem_set_word(tmp, _regs_curr.sp);
                _regs_curr.value = mem_get_word(tmp);
                break;
            case 0x78:
                _regs_prev.value = _regs_curr.af.a;
                _regs_curr.af.a = hardware_in(_regs_curr.bc.c);
                _regs_curr.value = _regs_curr.af.a;
                break;
            case 0x79:
                _regs_prev.value = _regs_curr.af.a;
                hardware_out(_regs_curr.bc.c, _regs_curr.af.a);
                _regs_curr.value = _regs_curr.af.a;
                break;
            case 0x7a:
                _regs_prev.value = _regs_curr.hl.word;
                _regs_curr.hl.word = alu_adc_word(_regs_curr.hl.word, _regs_curr.sp);
                _regs_curr.value = _regs_curr.hl.word;
                break;
            case 0x7b:
                tmp = ip_get_word();
                _regs_prev.value = _regs_curr.sp;
                _regs_curr.sp = mem_get_word(tmp);
                _regs_curr.value = _regs_curr.sp;
                break;
            case 0x80:
                _regs_prev.value = mem_get_byte(_regs_curr.de.word);
                mem_set_byte(_regs_curr.de.word, mem_get_byte(_regs_curr.hl.word));
                _regs_curr.hl.word++;
                _regs_curr.de.word++;
                _regs_curr.bc.word--;
                nf_set(0);
                hf_set(0);
                pvf_set(_regs_curr.bc.word != 0);
                _regs_curr.value = mem_get_byte(_regs_curr.hl.word);
                break;
            case 0x81:
                _regs_prev.value = mem_get_byte(_regs_curr.de.word);
                _regs_curr.value = mem_get_byte(_regs_curr.de.word);
                tmp = cf_get();
                alu_sub_byte(mem_get_byte(_regs_curr.hl.word), _regs_curr.af.a);
                cf_set(tmp);
                _regs_curr.hl.word++;
                _regs_curr.bc.word--;
                pvf_set(_regs_curr.bc.word != 0);
                break;
            case 0x82:
                _regs_prev.value = mem_get_byte(_regs_curr.hl.word);
                mem_set_byte(_regs_curr.hl.word, hardware_in(_regs_curr.bc.c));
                _regs_curr.bc.b--;
                nf_set(1);
                zf_set(_regs_curr.bc.b == 0);
                _regs_curr.value = mem_get_byte(_regs_curr.hl.word);
                _regs_curr.hl.word ++;
                break;
            case 0x83:
                _regs_prev.value = mem_get_byte(_regs_curr.hl.word);
                hardware_out(_regs_curr.bc.c, mem_get_byte(_regs_curr.hl.word));
                _regs_curr.bc.b--;
                nf_set(1);
                zf_set(_regs_curr.bc.b == 0);
                _regs_curr.value = mem_get_byte(_regs_curr.hl.word);
                _regs_curr.hl.word ++;
                break;
            case 0x88:
                _regs_prev.value = mem_get_byte(_regs_curr.de.word);
                mem_set_byte(_regs_curr.de.word, mem_get_byte(_regs_curr.hl.word));
                _regs_curr.hl.word--;
                _regs_curr.de.word--;
                _regs_curr.bc.word--;
                nf_set(0);
                hf_set(0);
                pvf_set(_regs_curr.bc.word != 0);
                _regs_curr.value = mem_get_byte(_regs_curr.hl.word);
                break;
            case 0x89:
                _regs_prev.value = mem_get_byte(_regs_curr.de.word);
                _regs_curr.value = mem_get_byte(_regs_curr.de.word);
                tmp = cf_get();
                alu_sub_byte(mem_get_byte(_regs_curr.hl.word), _regs_curr.af.a);
                cf_set(tmp);
                _regs_curr.hl.word--;
                _regs_curr.bc.word--;
                pvf_set(_regs_curr.bc.word != 0);
                break;
            case 0x8a:
                _regs_prev.value = mem_get_byte(_regs_curr.hl.word);
                mem_set_byte(_regs_curr.hl.word, hardware_in(_regs_curr.bc.c));
                _regs_curr.bc.b--;
                nf_set(1);
                zf_set(_regs_curr.bc.b == 0);
                _regs_curr.value = mem_get_byte(_regs_curr.hl.word);
                _regs_curr.hl.word --;
                break;
            case 0x8b:
                _regs_prev.value = mem_get_byte(_regs_curr.hl.word);
                hardware_out(_regs_curr.bc.c, mem_get_byte(_regs_curr.hl.word));
                _regs_curr.bc.b--;
                nf_set(1);
                zf_set(_regs_curr.bc.b == 0);
                _regs_curr.value = mem_get_byte(_regs_curr.hl.word);
                _regs_curr.hl.word --;
                break;
            case 0x90:
                do
                {
                    _regs_prev.value = mem_get_byte(_regs_curr.de.word);
                    mem_set_byte(_regs_curr.de.word, mem_get_byte(_regs_curr.hl.word));
                    _regs_curr.hl.word++;
                    _regs_curr.de.word++;
                    _regs_curr.bc.word--;
                    nf_set(0);
                    hf_set(0);
                    pvf_set(_regs_curr.bc.word != 0);
                    _regs_curr.value = mem_get_byte(_regs_curr.hl.word);
                }while(_regs_curr.bc.word != 0);
                break;
            case 0x91:
                do
                {
                    _regs_prev.value = mem_get_byte(_regs_curr.de.word);
                    _regs_curr.value = mem_get_byte(_regs_curr.de.word);
                    tmp = cf_get();
                    alu_sub_byte(mem_get_byte(_regs_curr.hl.word), _regs_curr.af.a);
                    cf_set(tmp);
                    _regs_curr.hl.word++;
                    _regs_curr.bc.word--;
                    pvf_set(_regs_curr.bc.word != 0);
                }while(_regs_curr.bc.word != 0);
                break;
            case 0x92:
                do
                {
                    _regs_prev.value = mem_get_byte(_regs_curr.hl.word);
                    mem_set_byte(_regs_curr.hl.word, hardware_in(_regs_curr.bc.c));
                    _regs_curr.bc.b--;
                    nf_set(1);
                    zf_set(_regs_curr.bc.b == 0);
                    _regs_curr.value = mem_get_byte(_regs_curr.hl.word);
                    _regs_curr.hl.word ++;
                }while(_regs_curr.bc.b != 0);
                break;
            case 0x93:
                do
                {
                    _regs_prev.value = mem_get_byte(_regs_curr.hl.word);
                    hardware_out(_regs_curr.bc.c, mem_get_byte(_regs_curr.hl.word));
                    _regs_curr.bc.b--;
                    nf_set(1);
                    zf_set(_regs_curr.bc.b == 0);
                    _regs_curr.value = mem_get_byte(_regs_curr.hl.word);
                    _regs_curr.hl.word ++;
                }while(_regs_curr.bc.b != 0);
                break;
            case 0x98:
                do
                {
                    _regs_prev.value = mem_get_byte(_regs_curr.de.word);
                    mem_set_byte(_regs_curr.de.word, mem_get_byte(_regs_curr.hl.word));
                    _regs_curr.hl.word--;
                    _regs_curr.de.word--;
                    _regs_curr.bc.word--;
                    nf_set(0);
                    hf_set(0);
                    pvf_set(_regs_curr.bc.word != 0);
                    _regs_curr.value = mem_get_byte(_regs_curr.hl.word);
                }while(_regs_curr.bc.word != 0);
                break;
            case 0x99:
                do
                {
                    _regs_prev.value = mem_get_byte(_regs_curr.de.word);
                    _regs_curr.value = mem_get_byte(_regs_curr.de.word);
                    tmp = cf_get();
                    alu_sub_byte(mem_get_byte(_regs_curr.hl.word), _regs_curr.af.a);
                    cf_set(tmp);
                    _regs_curr.hl.word--;
                    _regs_curr.bc.word--;
                    pvf_set(_regs_curr.bc.word != 0);
                }while(_regs_curr.bc.word != 0);
                break;
            case 0x9a:
                do
                {
                    _regs_prev.value = mem_get_byte(_regs_curr.hl.word);
                    mem_set_byte(_regs_curr.hl.word, hardware_in(_regs_curr.bc.c));
                    _regs_curr.bc.b--;
                    nf_set(1);
                    zf_set(_regs_curr.bc.b == 0);
                    _regs_curr.value = mem_get_byte(_regs_curr.hl.word);
                    _regs_curr.hl.word --;
                } while(_regs_curr.bc.b != 0);
                break;
            case 0x9b:
                do
                {
                    _regs_prev.value = mem_get_byte(_regs_curr.hl.word);
                    hardware_out(_regs_curr.bc.c, mem_get_byte(_regs_curr.hl.word));
                    _regs_curr.bc.b--;
                    nf_set(1);
                    zf_set(_regs_curr.bc.b == 0);
                    _regs_curr.value = mem_get_byte(_regs_curr.hl.word);
                    _regs_curr.hl.word --;
                }while(_regs_curr.bc.b != 0);
                break;
            default: break;
        }
    }
    else if(_regs_curr.prefix_cb)
    {
        //TODO: if have IX/IY prefix change the register
        if(_regs_curr.prefix_dd || _regs_curr.prefix_fd)
        {
            offset_value = ip_get_byte_signed();
        }
        switch(ip_get_byte())
        {
            case 0x00:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.bc.b;
                _regs_prev.value = tmp;
                tmp = alu_rlc(tmp);
                _regs_curr.value = tmp;
                _regs_curr.bc.b = tmp;
                break;
            case 0x01:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.bc.c;
                _regs_prev.value = tmp;
                tmp = alu_rlc(tmp);
                _regs_curr.value = tmp;
                _regs_curr.bc.c = tmp;
                break;
            case 0x02:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.de.d;
                _regs_prev.value = tmp;
                tmp = alu_rlc(tmp);
                _regs_curr.value = tmp;
                _regs_curr.de.d = tmp;
                break;
            case 0x03:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.de.e;
                _regs_prev.value = tmp;
                tmp = alu_rlc(tmp);
                _regs_curr.value = tmp;
                _regs_curr.de.e = tmp;
                break;
            case 0x04:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.hl.h;
                _regs_prev.value = tmp;
                tmp = alu_rlc(tmp);
                _regs_curr.value = tmp;
                _regs_curr.hl.h = tmp;
                break;
            case 0x05:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.hl.l;
                _regs_prev.value = tmp;
                tmp = alu_rlc(tmp);
                _regs_curr.value = tmp;
                _regs_curr.hl.l = tmp;
                break;
            case 0x06:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = mem_get_byte(_regs_curr.hl.word);
                _regs_prev.value = tmp;
                tmp = alu_rlc(tmp);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else mem_set_byte(_regs_curr.hl.word, tmp);
                break;
            case 0x07:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.af.a;
                _regs_prev.value = tmp;
                tmp = alu_rlc(tmp);
                _regs_curr.value = tmp;
                _regs_curr.af.a = tmp;
                break;
            case 0x08:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.bc.b;
                _regs_prev.value = tmp;
                tmp = alu_rrc(tmp);
                _regs_curr.value = tmp;
                _regs_curr.bc.b = tmp;
                break;
            case 0x09:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.bc.c;
                _regs_prev.value = tmp;
                tmp = alu_rrc(tmp);
                _regs_curr.value = tmp;
                _regs_curr.bc.c = tmp;
                break;
            case 0x0a:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.de.d;
                _regs_prev.value = tmp;
                tmp = alu_rrc(tmp);
                _regs_curr.value = tmp;
                _regs_curr.de.d = tmp;
                break;
            case 0x0b:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.de.e;
                _regs_prev.value = tmp;
                tmp = alu_rrc(tmp);
                _regs_curr.value = tmp;
                _regs_curr.de.e = tmp;
                break;
            case 0x0c:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.hl.h;
                _regs_prev.value = tmp;
                tmp = alu_rrc(tmp);
                _regs_curr.value = tmp;
                _regs_curr.hl.h = tmp;
                break;
            case 0x0d:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.hl.l;
                _regs_prev.value = tmp;
                tmp = alu_rrc(tmp);
                _regs_curr.value = tmp;
                _regs_curr.hl.l = tmp;
                break;
            case 0x0e:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = mem_get_byte(_regs_curr.hl.word);
                _regs_prev.value = tmp;
                tmp = alu_rrc(tmp);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else mem_set_byte(_regs_curr.hl.word, tmp);
                break;
            case 0x0f:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.af.a;
                _regs_prev.value = tmp;
                tmp = alu_rrc(tmp);
                _regs_curr.value = tmp;
                _regs_curr.af.a = tmp;
                break;
            case 0x10:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.bc.b;
                _regs_prev.value = tmp;
                tmp = alu_rl(tmp);
                _regs_curr.value = tmp;
                _regs_curr.bc.b = tmp;
                break;
            case 0x11:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.bc.c;
                _regs_prev.value = tmp;
                tmp = alu_rl(tmp);
                _regs_curr.value = tmp;
                _regs_curr.bc.c = tmp;
                break;
            case 0x12:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.de.d;
                _regs_prev.value = tmp;
                tmp = alu_rl(tmp);
                _regs_curr.value = tmp;
                _regs_curr.de.d = tmp;
                break;
            case 0x13:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.de.e;
                _regs_prev.value = tmp;
                tmp = alu_rl(tmp);
                _regs_curr.value = tmp;
                _regs_curr.de.e = tmp;
                break;
            case 0x14:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.hl.h;
                _regs_prev.value = tmp;
                tmp = alu_rl(tmp);
                _regs_curr.value = tmp;
                _regs_curr.hl.h = tmp;
                break;
            case 0x15:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.hl.l;
                _regs_prev.value = tmp;
                tmp = alu_rl(tmp);
                _regs_curr.value = tmp;
                _regs_curr.hl.l = tmp;
                break;
            case 0x16:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = mem_get_byte(_regs_curr.hl.word);
                _regs_prev.value = tmp;
                tmp = alu_rl(tmp);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else mem_set_byte(_regs_curr.hl.word, tmp);
                break;
            case 0x17:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.af.a;
                _regs_prev.value = tmp;
                tmp = alu_rl(tmp);
                _regs_curr.value = tmp;
                _regs_curr.af.a = tmp;
                break;
            case 0x18:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.bc.b;
                _regs_prev.value = tmp;
                tmp = alu_rr(tmp);
                _regs_curr.value = tmp;
                _regs_curr.bc.b = tmp;
                break;
            case 0x19:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.bc.c;
                _regs_prev.value = tmp;
                tmp = alu_rr(tmp);
                _regs_curr.value = tmp;
                _regs_curr.bc.c = tmp;
                break;
            case 0x1a:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.de.d;
                _regs_prev.value = tmp;
                tmp = alu_rr(tmp);
                _regs_curr.value = tmp;
                _regs_curr.de.d = tmp;
                break;
            case 0x1b:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.de.e;
                _regs_prev.value = tmp;
                tmp = alu_rr(tmp);
                _regs_curr.value = tmp;
                _regs_curr.de.e = tmp;
                break;
            case 0x1c:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.hl.h;
                _regs_prev.value = tmp;
                tmp = alu_rr(tmp);
                _regs_curr.value = tmp;
                _regs_curr.hl.h = tmp;
                break;
            case 0x1d:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.hl.l;
                _regs_prev.value = tmp;
                tmp = alu_rr(tmp);
                _regs_curr.value = tmp;
                _regs_curr.hl.l = tmp;
                break;
            case 0x1e:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = mem_get_byte(_regs_curr.hl.word);
                _regs_prev.value = tmp;
                tmp = alu_rr(tmp);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else mem_set_byte(_regs_curr.hl.word, tmp);
                break;
            case 0x1f:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.af.a;
                _regs_prev.value = tmp;
                tmp = alu_rr(tmp);
                _regs_curr.value = tmp;
                _regs_curr.af.a = tmp;
                break;
            case 0x20:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.bc.b;
                _regs_prev.value = tmp;
                tmp = alu_sla(tmp);
                _regs_curr.value = tmp;
                _regs_curr.bc.b = tmp;
                break;
            case 0x21:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.bc.c;
                _regs_prev.value = tmp;
                tmp = alu_sla(tmp);
                _regs_curr.value = tmp;
                _regs_curr.bc.c = tmp;
                break;
            case 0x22:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.de.d;
                _regs_prev.value = tmp;
                tmp = alu_sla(tmp);
                _regs_curr.value = tmp;
                _regs_curr.de.d = tmp;
                break;
            case 0x23:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.de.e;
                _regs_prev.value = tmp;
                tmp = alu_sla(tmp);
                _regs_curr.value = tmp;
                _regs_curr.de.e = tmp;
                break;
            case 0x24:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.hl.h;
                _regs_prev.value = tmp;
                tmp = alu_sla(tmp);
                _regs_curr.value = tmp;
                _regs_curr.hl.h = tmp;
                break;
            case 0x25:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.hl.l;
                _regs_prev.value = tmp;
                tmp = alu_sla(tmp);
                _regs_curr.value = tmp;
                _regs_curr.hl.l = tmp;
                break;
            case 0x26:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = mem_get_byte(_regs_curr.hl.word);
                _regs_prev.value = tmp;
                tmp = alu_sla(tmp);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else mem_set_byte(_regs_curr.hl.word, tmp);
                break;
            case 0x27:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.af.a;
                _regs_prev.value = tmp;
                tmp = alu_sla(tmp);
                _regs_curr.value = tmp;
                _regs_curr.af.a = tmp;
                break;
            case 0x28:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.bc.b;
                _regs_prev.value = tmp;
                tmp = alu_sra(tmp);
                _regs_curr.value = tmp;
                _regs_curr.bc.b = tmp;
                break;
            case 0x29:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.bc.c;
                _regs_prev.value = tmp;
                tmp = alu_sra(tmp);
                _regs_curr.value = tmp;
                _regs_curr.bc.c = tmp;
                break;
            case 0x2a:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.de.d;
                _regs_prev.value = tmp;
                tmp = alu_sra(tmp);
                _regs_curr.value = tmp;
                _regs_curr.de.d = tmp;
                break;
            case 0x2b:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.de.e;
                _regs_prev.value = tmp;
                tmp = alu_sra(tmp);
                _regs_curr.value = tmp;
                _regs_curr.de.e = tmp;
                break;
            case 0x2c:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.hl.h;
                _regs_prev.value = tmp;
                tmp = alu_sra(tmp);
                _regs_curr.value = tmp;
                _regs_curr.hl.h = tmp;
                break;
            case 0x2d:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.hl.l;
                _regs_prev.value = tmp;
                tmp = alu_sra(tmp);
                _regs_curr.value = tmp;
                _regs_curr.hl.l = tmp;
                break;
            case 0x2e:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = mem_get_byte(_regs_curr.hl.word);
                _regs_prev.value = tmp;
                tmp = alu_sra(tmp);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else mem_set_byte(_regs_curr.hl.word, tmp);
                break;
            case 0x2f:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.af.a;
                _regs_prev.value = tmp;
                tmp = alu_sra(tmp);
                _regs_curr.value = tmp;
                _regs_curr.af.a = tmp;
                break;
            case 0x30:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.bc.b;
                _regs_prev.value = tmp;
                tmp = alu_sll(tmp);
                _regs_curr.value = tmp;
                _regs_curr.bc.b = tmp;
                break;
            case 0x31:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.bc.c;
                _regs_prev.value = tmp;
                tmp = alu_sll(tmp);
                _regs_curr.value = tmp;
                _regs_curr.bc.c = tmp;
                break;
            case 0x32:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.de.d;
                _regs_prev.value = tmp;
                tmp = alu_sll(tmp);
                _regs_curr.value = tmp;
                _regs_curr.de.d = tmp;
                break;
            case 0x33:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.de.e;
                _regs_prev.value = tmp;
                tmp = alu_sll(tmp);
                _regs_curr.value = tmp;
                _regs_curr.de.e = tmp;
                break;
            case 0x34:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.hl.h;
                _regs_prev.value = tmp;
                tmp = alu_sll(tmp);
                _regs_curr.value = tmp;
                _regs_curr.hl.h = tmp;
                break;
            case 0x35:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.hl.l;
                _regs_prev.value = tmp;
                tmp = alu_sll(tmp);
                _regs_curr.value = tmp;
                _regs_curr.hl.l = tmp;
                break;
            case 0x36:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = mem_get_byte(_regs_curr.hl.word);
                _regs_prev.value = tmp;
                tmp = alu_sll(tmp);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else mem_set_byte(_regs_curr.hl.word, tmp);
                break;
            case 0x37:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.af.a;
                _regs_prev.value = tmp;
                tmp = alu_sll(tmp);
                _regs_curr.value = tmp;
                _regs_curr.af.a = tmp;
                break;
            case 0x38:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.bc.b;
                _regs_prev.value = tmp;
                tmp = alu_srl(tmp);
                _regs_curr.value = tmp;
                _regs_curr.bc.b = tmp;
                break;
            case 0x39:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.bc.c;
                _regs_prev.value = tmp;
                tmp = alu_srl(tmp);
                _regs_curr.value = tmp;
                _regs_curr.bc.c = tmp;
                break;
            case 0x3a:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.de.d;
                _regs_prev.value = tmp;
                tmp = alu_srl(tmp);
                _regs_curr.value = tmp;
                _regs_curr.de.d = tmp;
                break;
            case 0x3b:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.de.e;
                _regs_prev.value = tmp;
                tmp = alu_srl(tmp);
                _regs_curr.value = tmp;
                _regs_curr.de.e = tmp;
                break;
            case 0x3c:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.hl.h;
                _regs_prev.value = tmp;
                tmp = alu_srl(tmp);
                _regs_curr.value = tmp;
                _regs_curr.hl.h = tmp;
                break;
            case 0x3d:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.hl.l;
                _regs_prev.value = tmp;
                tmp = alu_srl(tmp);
                _regs_curr.value = tmp;
                _regs_curr.hl.l = tmp;
                break;
            case 0x3e:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = mem_get_byte(_regs_curr.hl.word);
                _regs_prev.value = tmp;
                tmp = alu_srl(tmp);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else mem_set_byte(_regs_curr.hl.word, tmp);
                break;
            case 0x3f:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.af.a;
                _regs_prev.value = tmp;
                tmp = alu_srl(tmp);
                _regs_curr.value = tmp;
                _regs_curr.af.a = tmp;
                break;
            case 0x40:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.bc.b;
                _regs_prev.value = tmp;
                _regs_curr.value = tmp;
                nf_set(0);
                hf_set(0);
                zf_set(tmp & (1 << 0));
                break;
            case 0x41:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.bc.c;
                _regs_prev.value = tmp;
                _regs_curr.value = tmp;
                nf_set(0);
                hf_set(0);
                zf_set(tmp & (1 << 0));
                break;
            case 0x42:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.de.d;
                _regs_prev.value = tmp;
                _regs_curr.value = tmp;
                nf_set(0);
                hf_set(0);
                zf_set(tmp & (1 << 0));
                break;
            case 0x43:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.de.e;
                _regs_prev.value = tmp;
                _regs_curr.value = tmp;
                nf_set(0);
                hf_set(0);
                zf_set(tmp & (1 << 0));
                break;
            case 0x44:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.hl.h;
                _regs_prev.value = tmp;
                _regs_curr.value = tmp;
                nf_set(0);
                hf_set(0);
                zf_set(tmp & (1 << 0));
                break;
            case 0x45:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.hl.l;
                _regs_prev.value = tmp;
                _regs_curr.value = tmp;
                nf_set(0);
                hf_set(0);
                zf_set(tmp & (1 << 0));
                break;
            case 0x46:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = mem_get_byte(_regs_curr.hl.word);
                _regs_prev.value = tmp;
                _regs_curr.value = tmp;
                nf_set(0);
                hf_set(0);
                zf_set(tmp & (1 << 0));
                break;
            case 0x47:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.af.a;
                _regs_prev.value = tmp;
                _regs_curr.value = tmp;
                nf_set(0);
                hf_set(0);
                zf_set(tmp & (1 << 0));
                break;
            case 0x48:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.bc.b;
                _regs_prev.value = tmp;
                _regs_curr.value = tmp;
                nf_set(0);
                hf_set(0);
                zf_set(tmp & (1 << 1));
                break;
            case 0x49:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.bc.c;
                _regs_prev.value = tmp;
                _regs_curr.value = tmp;
                nf_set(0);
                hf_set(0);
                zf_set(tmp & (1 << 1));
                break;
            case 0x4a:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.de.d;
                _regs_prev.value = tmp;
                _regs_curr.value = tmp;
                nf_set(0);
                hf_set(0);
                zf_set(tmp & (1 << 1));
                break;
            case 0x4b:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.de.e;
                _regs_prev.value = tmp;
                _regs_curr.value = tmp;
                nf_set(0);
                hf_set(0);
                zf_set(tmp & (1 << 1));
                break;
            case 0x4c:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.hl.h;
                _regs_prev.value = tmp;
                _regs_curr.value = tmp;
                nf_set(0);
                hf_set(0);
                zf_set(tmp & (1 << 1));
                break;
            case 0x4d:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.hl.l;
                _regs_prev.value = tmp;
                _regs_curr.value = tmp;
                nf_set(0);
                hf_set(0);
                zf_set(tmp & (1 << 1));
                break;
            case 0x4e:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = mem_get_byte(_regs_curr.hl.word);
                _regs_prev.value = tmp;
                _regs_curr.value = tmp;
                nf_set(0);
                hf_set(0);
                zf_set(tmp & (1 << 1));
                break;
            case 0x4f:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.af.a;
                _regs_prev.value = tmp;
                _regs_curr.value = tmp;
                nf_set(0);
                hf_set(0);
                zf_set(tmp & (1 << 1));
                break;
            case 0x50:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.bc.b;
                _regs_prev.value = tmp;
                _regs_curr.value = tmp;
                nf_set(0);
                hf_set(0);
                zf_set(tmp & (1 << 2));
                break;
            case 0x51:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.bc.c;
                _regs_prev.value = tmp;
                _regs_curr.value = tmp;
                nf_set(0);
                hf_set(0);
                zf_set(tmp & (1 << 2));
                break;
            case 0x52:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.de.d;
                _regs_prev.value = tmp;
                _regs_curr.value = tmp;
                nf_set(0);
                hf_set(0);
                zf_set(tmp & (1 << 2));
                break;
            case 0x53:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.de.e;
                _regs_prev.value = tmp;
                _regs_curr.value = tmp;
                nf_set(0);
                hf_set(0);
                zf_set(tmp & (1 << 2));
                break;
            case 0x54:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.hl.h;
                _regs_prev.value = tmp;
                _regs_curr.value = tmp;
                nf_set(0);
                hf_set(0);
                zf_set(tmp & (1 << 2));
                break;
            case 0x55:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.hl.l;
                _regs_prev.value = tmp;
                _regs_curr.value = tmp;
                nf_set(0);
                hf_set(0);
                zf_set(tmp & (1 << 2));
                break;
            case 0x56:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = mem_get_byte(_regs_curr.hl.word);
                _regs_prev.value = tmp;
                _regs_curr.value = tmp;
                nf_set(0);
                hf_set(0);
                zf_set(tmp & (1 << 2));
                break;
            case 0x57:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.af.a;
                _regs_prev.value = tmp;
                _regs_curr.value = tmp;
                nf_set(0);
                hf_set(0);
                zf_set(tmp & (1 << 3));
                break;
            case 0x58:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.bc.b;
                _regs_prev.value = tmp;
                _regs_curr.value = tmp;
                nf_set(0);
                hf_set(0);
                zf_set(tmp & (1 << 3));
                break;
            case 0x59:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.bc.c;
                _regs_prev.value = tmp;
                _regs_curr.value = tmp;
                nf_set(0);
                hf_set(0);
                zf_set(tmp & (1 << 3));
                break;
            case 0x5a:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.de.d;
                _regs_prev.value = tmp;
                _regs_curr.value = tmp;
                nf_set(0);
                hf_set(0);
                zf_set(tmp & (1 << 3));
                break;
            case 0x5b:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.de.e;
                _regs_prev.value = tmp;
                _regs_curr.value = tmp;
                nf_set(0);
                hf_set(0);
                zf_set(tmp & (1 << 3));
                break;
            case 0x5c:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.hl.h;
                _regs_prev.value = tmp;
                _regs_curr.value = tmp;
                nf_set(0);
                hf_set(0);
                zf_set(tmp & (1 << 3));
                break;
            case 0x5d:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.hl.l;
                _regs_prev.value = tmp;
                _regs_curr.value = tmp;
                nf_set(0);
                hf_set(0);
                zf_set(tmp & (1 << 3));
                break;
            case 0x5e:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = mem_get_byte(_regs_curr.hl.word);
                _regs_prev.value = tmp;
                _regs_curr.value = tmp;
                nf_set(0);
                hf_set(0);
                zf_set(tmp & (1 << 3));
                break;
            case 0x5f:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.af.a;
                _regs_prev.value = tmp;
                _regs_curr.value = tmp;
                nf_set(0);
                hf_set(0);
                zf_set(tmp & (1 << 3));
                break;
            case 0x60:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.bc.b;
                _regs_prev.value = tmp;
                _regs_curr.value = tmp;
                nf_set(0);
                hf_set(0);
                zf_set(tmp & (1 << 4));
                break;
            case 0x61:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.bc.c;
                _regs_prev.value = tmp;
                _regs_curr.value = tmp;
                nf_set(0);
                hf_set(0);
                zf_set(tmp & (1 << 4));
                break;
            case 0x62:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.de.d;
                _regs_prev.value = tmp;
                _regs_curr.value = tmp;
                nf_set(0);
                hf_set(0);
                zf_set(tmp & (1 << 4));
                break;
            case 0x63:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.de.e;
                _regs_prev.value = tmp;
                _regs_curr.value = tmp;
                nf_set(0);
                hf_set(0);
                zf_set(tmp & (1 << 4));
                break;
            case 0x64:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.hl.h;
                _regs_prev.value = tmp;
                _regs_curr.value = tmp;
                nf_set(0);
                hf_set(0);
                zf_set(tmp & (1 << 4));
                break;
            case 0x65:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.hl.l;
                _regs_prev.value = tmp;
                _regs_curr.value = tmp;
                nf_set(0);
                hf_set(0);
                zf_set(tmp & (1 << 4));
                break;
            case 0x66:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = mem_get_byte(_regs_curr.hl.word);
                _regs_prev.value = tmp;
                _regs_curr.value = tmp;
                nf_set(0);
                hf_set(0);
                zf_set(tmp & (1 << 4));
                break;
            case 0x67:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.af.a;
                _regs_prev.value = tmp;
                _regs_curr.value = tmp;
                nf_set(0);
                hf_set(0);
                zf_set(tmp & (1 << 4));
                break;
            case 0x68:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.bc.b;
                _regs_prev.value = tmp;
                _regs_curr.value = tmp;
                nf_set(0);
                hf_set(0);
                zf_set(tmp & (1 << 5));
                break;
            case 0x69:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.bc.c;
                _regs_prev.value = tmp;
                _regs_curr.value = tmp;
                nf_set(0);
                hf_set(0);
                zf_set(tmp & (1 << 5));
                break;
            case 0x6a:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.de.d;
                _regs_prev.value = tmp;
                _regs_curr.value = tmp;
                nf_set(0);
                hf_set(0);
                zf_set(tmp & (1 << 5));
                break;
            case 0x6b:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.de.e;
                _regs_prev.value = tmp;
                _regs_curr.value = tmp;
                nf_set(0);
                hf_set(0);
                zf_set(tmp & (1 << 5));
                break;
            case 0x6c:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.hl.h;
                _regs_prev.value = tmp;
                _regs_curr.value = tmp;
                nf_set(0);
                hf_set(0);
                zf_set(tmp & (1 << 5));
                break;
            case 0x6d:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.hl.l;
                _regs_prev.value = tmp;
                _regs_curr.value = tmp;
                nf_set(0);
                hf_set(0);
                zf_set(tmp & (1 << 5));
                break;
            case 0x6e:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = mem_get_byte(_regs_curr.hl.word);
                _regs_prev.value = tmp;
                _regs_curr.value = tmp;
                nf_set(0);
                hf_set(0);
                zf_set(tmp & (1 << 5));
                break;
            case 0x6f:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.af.a;
                _regs_prev.value = tmp;
                _regs_curr.value = tmp;
                nf_set(0);
                hf_set(0);
                zf_set(tmp & (1 << 5));
                break;
            case 0x70:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.bc.b;
                _regs_prev.value = tmp;
                _regs_curr.value = tmp;
                nf_set(0);
                hf_set(0);
                zf_set(tmp & (1 << 7));
                break;
            case 0x71:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.bc.c;
                _regs_prev.value = tmp;
                _regs_curr.value = tmp;
                nf_set(0);
                hf_set(0);
                zf_set(tmp & (1 << 7));
                break;
            case 0x72:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.de.d;
                _regs_prev.value = tmp;
                _regs_curr.value = tmp;
                nf_set(0);
                hf_set(0);
                zf_set(tmp & (1 << 7));
                break;
            case 0x73:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.de.e;
                _regs_prev.value = tmp;
                _regs_curr.value = tmp;
                nf_set(0);
                hf_set(0);
                zf_set(tmp & (1 << 7));
                break;
            case 0x74:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.hl.h;
                _regs_prev.value = tmp;
                _regs_curr.value = tmp;
                nf_set(0);
                hf_set(0);
                zf_set(tmp & (1 << 7));
                break;
            case 0x75:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.hl.l;
                _regs_prev.value = tmp;
                _regs_curr.value = tmp;
                nf_set(0);
                hf_set(0);
                zf_set(tmp & (1 << 7));
                break;
            case 0x76:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = mem_get_byte(_regs_curr.hl.word);
                _regs_prev.value = tmp;
                _regs_curr.value = tmp;
                nf_set(0);
                hf_set(0);
                zf_set(tmp & (1 << 7));
                break;
            case 0x77:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.af.a;
                _regs_prev.value = tmp;
                _regs_curr.value = tmp;
                nf_set(0);
                hf_set(0);
                zf_set(tmp & (1 << 7));
                break;
            case 0x78:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.bc.b;
                _regs_prev.value = tmp;
                _regs_curr.value = tmp;
                nf_set(0);
                hf_set(0);
                zf_set(tmp & (1 << 8));
                break;
            case 0x79:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.bc.c;
                _regs_prev.value = tmp;
                _regs_curr.value = tmp;
                nf_set(0);
                hf_set(0);
                zf_set(tmp & (1 << 8));
                break;
            case 0x7a:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.de.d;
                _regs_prev.value = tmp;
                _regs_curr.value = tmp;
                nf_set(0);
                hf_set(0);
                zf_set(tmp & (1 << 8));
                break;
            case 0x7b:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.de.e;
                _regs_prev.value = tmp;
                _regs_curr.value = tmp;
                nf_set(0);
                hf_set(0);
                zf_set(tmp & (1 << 8));
                break;
            case 0x7c:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.hl.h;
                _regs_prev.value = tmp;
                _regs_curr.value = tmp;
                nf_set(0);
                hf_set(0);
                zf_set(tmp & (1 << 8));
                break;
            case 0x7d:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.hl.l;
                _regs_prev.value = tmp;
                _regs_curr.value = tmp;
                nf_set(0);
                hf_set(0);
                zf_set(tmp & (1 << 8));
                break;
            case 0x7e:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = mem_get_byte(_regs_curr.hl.word);
                _regs_prev.value = tmp;
                _regs_curr.value = tmp;
                nf_set(0);
                hf_set(0);
                zf_set(tmp & (1 << 8));
                break;
            case 0x7f:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.af.a;
                _regs_prev.value = tmp;
                _regs_curr.value = tmp;
                nf_set(0);
                hf_set(0);
                zf_set(tmp & (1 << 8));
                break;
            case 0x80:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.bc.b;
                _regs_prev.value = tmp;
                tmp &= ~(1 << 0);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.bc.b = tmp;
                break;
            case 0x81:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.bc.c;
                _regs_prev.value = tmp;
                tmp &= ~(1 << 0);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.bc.c = tmp;
                break;
            case 0x82:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.de.d;
                _regs_prev.value = tmp;
                tmp &= ~(1 << 0);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.de.d = tmp;
                break;
            case 0x83:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.de.e;
                _regs_prev.value = tmp;
                tmp &= ~(1 << 0);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.de.e = tmp;
                break;
            case 0x84:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.hl.h;
                _regs_prev.value = tmp;
                tmp &= ~(1 << 0);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.hl.h = tmp;
                break;
            case 0x85:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.hl.l;
                _regs_prev.value = tmp;
                tmp &= ~(1 << 0);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.hl.l = tmp;
                break;
            case 0x86:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = mem_get_byte(_regs_curr.hl.word);
                _regs_prev.value = tmp;
                tmp &= ~(1 << 0);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else mem_set_byte(_regs_curr.hl.word, tmp);
                break;
            case 0x87:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.af.a;
                _regs_prev.value = tmp;
                tmp &= ~(1 << 0);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.af.a = tmp;
                break;
            case 0x88:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.bc.b;
                _regs_prev.value = tmp;
                tmp &= ~(1 << 1);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.bc.b = tmp;
                break;
            case 0x89:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.bc.c;
                _regs_prev.value = tmp;
                tmp &= ~(1 << 1);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.bc.c = tmp;
                break;
            case 0x8a:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.de.d;
                _regs_prev.value = tmp;
                tmp &= ~(1 << 1);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.de.d = tmp;
                break;
            case 0x8b:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.de.e;
                _regs_prev.value = tmp;
                tmp &= ~(1 << 1);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.de.e = tmp;
                break;
            case 0x8c:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.hl.h;
                _regs_prev.value = tmp;
                tmp &= ~(1 << 1);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.hl.h = tmp;
                break;
            case 0x8d:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.hl.l;
                _regs_prev.value = tmp;
                tmp &= ~(1 << 1);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.hl.l = tmp;
                break;
            case 0x8e:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = mem_get_byte(_regs_curr.hl.word);
                _regs_prev.value = tmp;
                tmp &= ~(1 << 1);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else mem_set_byte(_regs_curr.hl.word, tmp);
                break;
            case 0x8f:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.af.a;
                _regs_prev.value = tmp;
                tmp &= ~(1 << 1);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.af.a = tmp;
                break;
            case 0x90:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.bc.b;
                _regs_prev.value = tmp;
                tmp &= ~(1 << 2);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.bc.b = tmp;
                break;
            case 0x91:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.bc.c;
                _regs_prev.value = tmp;
                tmp &= ~(1 << 2);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.bc.c = tmp;
                break;
            case 0x92:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.de.d;
                _regs_prev.value = tmp;
                tmp &= ~(1 << 2);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.de.d = tmp;
                break;
            case 0x93:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.de.e;
                _regs_prev.value = tmp;
                tmp &= ~(1 << 2);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.de.e = tmp;
                break;
            case 0x94:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.hl.h;
                _regs_prev.value = tmp;
                tmp &= ~(1 << 2);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.hl.h = tmp;
                break;
            case 0x95:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.hl.l;
                _regs_prev.value = tmp;
                tmp &= ~(1 << 2);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.hl.l = tmp;
                break;
            case 0x96:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = mem_get_byte(_regs_curr.hl.word);
                _regs_prev.value = tmp;
                tmp &= ~(1 << 2);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else mem_set_byte(_regs_curr.hl.word, tmp);
                break;
            case 0x97:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.af.a;
                _regs_prev.value = tmp;
                tmp &= ~(1 << 2);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.af.a = tmp;
                break;
            case 0x98:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.bc.b;
                _regs_prev.value = tmp;
                tmp &= ~(1 << 3);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.bc.b = tmp;
                break;
            case 0x99:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.bc.c;
                _regs_prev.value = tmp;
                tmp &= ~(1 << 3);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.bc.c = tmp;
                break;
            case 0x9a:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.de.d;
                _regs_prev.value = tmp;
                tmp &= ~(1 << 3);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.de.d = tmp;
                break;
            case 0x9b:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.de.e;
                _regs_prev.value = tmp;
                tmp &= ~(1 << 3);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.de.e = tmp;
                break;
            case 0x9c:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.hl.h;
                _regs_prev.value = tmp;
                tmp &= ~(1 << 3);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.hl.h = tmp;
                break;
            case 0x9d:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.hl.l;
                _regs_prev.value = tmp;
                tmp &= ~(1 << 3);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.hl.l = tmp;
                break;
            case 0x9e:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = mem_get_byte(_regs_curr.hl.word);
                _regs_prev.value = tmp;
                tmp &= ~(1 << 3);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else mem_set_byte(_regs_curr.hl.word, tmp);
                break;
            case 0x9f:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.af.a;
                _regs_prev.value = tmp;
                tmp &= ~(1 << 3);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.af.a = tmp;
                break;
            case 0xa0:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.bc.b;
                _regs_prev.value = tmp;
                tmp &= ~(1 << 4);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.bc.b = tmp;
                break;
            case 0xa1:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.bc.c;
                _regs_prev.value = tmp;
                tmp &= ~(1 << 4);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.bc.c = tmp;
                break;
            case 0xa2:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.de.d;
                _regs_prev.value = tmp;
                tmp &= ~(1 << 4);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.de.d = tmp;
                break;
            case 0xa3:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.de.e;
                _regs_prev.value = tmp;
                tmp &= ~(1 << 4);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.de.e = tmp;
                break;
            case 0xa4:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.hl.h;
                _regs_prev.value = tmp;
                tmp &= ~(1 << 4);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.hl.h = tmp;
                break;
            case 0xa5:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.hl.l;
                _regs_prev.value = tmp;
                tmp &= ~(1 << 4);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.hl.l = tmp;
                break;
            case 0xa6:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = mem_get_byte(_regs_curr.hl.word);
                _regs_prev.value = tmp;
                tmp &= ~(1 << 4);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else mem_set_byte(_regs_curr.hl.word, tmp);
                break;
            case 0xa7:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.af.a;
                _regs_prev.value = tmp;
                tmp &= ~(1 << 4);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.af.a = tmp;
                break;
            case 0xa8:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.bc.b;
                _regs_prev.value = tmp;
                tmp &= ~(1 << 5);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.bc.b = tmp;
                break;
            case 0xa9:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.bc.c;
                _regs_prev.value = tmp;
                tmp &= ~(1 << 5);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.bc.c = tmp;
                break;
            case 0xaa:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.de.d;
                _regs_prev.value = tmp;
                tmp &= ~(1 << 5);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.de.d = tmp;
                break;
            case 0xab:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.de.e;
                _regs_prev.value = tmp;
                tmp &= ~(1 << 5);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.de.e = tmp;
                break;
            case 0xac:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.hl.h;
                _regs_prev.value = tmp;
                tmp &= ~(1 << 5);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.hl.h = tmp;
                break;
            case 0xad:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.hl.l;
                _regs_prev.value = tmp;
                tmp &= ~(1 << 5);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.hl.l = tmp;
                break;
            case 0xae:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = mem_get_byte(_regs_curr.hl.word);
                _regs_prev.value = tmp;
                tmp &= ~(1 << 5);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else mem_set_byte(_regs_curr.hl.word, tmp);
                break;
            case 0xaf:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.af.a;
                _regs_prev.value = tmp;
                tmp &= ~(1 << 5);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.af.a = tmp;
                break;
            case 0xb0:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.bc.b;
                _regs_prev.value = tmp;
                tmp &= ~(1 << 6);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.bc.b = tmp;
                break;
            case 0xb1:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.bc.c;
                _regs_prev.value = tmp;
                tmp &= ~(1 << 6);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.bc.c = tmp;
                break;
            case 0xb2:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.de.d;
                _regs_prev.value = tmp;
                tmp &= ~(1 << 6);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.de.d = tmp;
                break;
            case 0xb3:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.de.e;
                _regs_prev.value = tmp;
                tmp &= ~(1 << 6);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.de.e = tmp;
                break;
            case 0xb4:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.hl.h;
                _regs_prev.value = tmp;
                tmp &= ~(1 << 6);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.hl.h = tmp;
                break;
            case 0xb5:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.hl.l;
                _regs_prev.value = tmp;
                tmp &= ~(1 << 6);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.hl.l = tmp;
                break;
            case 0xb6:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = mem_get_byte(_regs_curr.hl.word);
                _regs_prev.value = tmp;
                tmp &= ~(1 << 6);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else mem_set_byte(_regs_curr.hl.word, tmp);
                break;
            case 0xb7:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.af.a;
                _regs_prev.value = tmp;
                tmp &= ~(1 << 6);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.af.a = tmp;
                break;
            case 0xb8:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.bc.b;
                _regs_prev.value = tmp;
                tmp &= ~(1 << 7);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.bc.b = tmp;
                break;
            case 0xb9:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.bc.c;
                _regs_prev.value = tmp;
                tmp &= ~(1 << 7);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.bc.c = tmp;
                break;
            case 0xba:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.de.d;
                _regs_prev.value = tmp;
                tmp &= ~(1 << 7);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.de.d = tmp;
                break;
            case 0xbb:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.de.e;
                _regs_prev.value = tmp;
                tmp &= ~(1 << 7);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.de.e = tmp;
                break;
            case 0xbc:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.hl.h;
                _regs_prev.value = tmp;
                tmp &= ~(1 << 7);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.hl.h = tmp;
                break;
            case 0xbd:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.hl.l;
                _regs_prev.value = tmp;
                tmp &= ~(1 << 7);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.hl.l = tmp;
                break;
            case 0xbe:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = mem_get_byte(_regs_curr.hl.word);
                _regs_prev.value = tmp;
                tmp &= ~(1 << 7);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else mem_set_byte(_regs_curr.hl.word, tmp);
                break;
            case 0xbf:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.af.a;
                _regs_prev.value = tmp;
                tmp &= ~(1 << 7);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.af.a = tmp;
                break;
            case 0xc0:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.bc.b;
                _regs_prev.value = tmp;
                tmp |= (1 << 0);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.bc.b = tmp;
                break;
            case 0xc1:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.bc.c;
                _regs_prev.value = tmp;
                tmp |= (1 << 0);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.bc.c = tmp;
                break;
            case 0xc2:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.de.d;
                _regs_prev.value = tmp;
                tmp |= (1 << 0);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.de.d = tmp;
                break;
            case 0xc3:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.de.e;
                _regs_prev.value = tmp;
                tmp |= (1 << 0);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.de.e = tmp;
                break;
            case 0xc4:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.hl.h;
                _regs_prev.value = tmp;
                tmp |= (1 << 0);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.hl.h = tmp;
                break;
            case 0xc5:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.hl.l;
                _regs_prev.value = tmp;
                tmp |= (1 << 0);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.hl.l = tmp;
                break;
            case 0xc6:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = mem_get_byte(_regs_curr.hl.word);
                _regs_prev.value = tmp;
                tmp |= (1 << 0);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else mem_set_byte(_regs_curr.hl.word, tmp);
                break;
            case 0xc7:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.af.a;
                _regs_prev.value = tmp;
                tmp |= (1 << 0);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.af.a = tmp;
                break;
            case 0xc8:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.bc.b;
                _regs_prev.value = tmp;
                tmp |= (1 << 1);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.bc.b = tmp;
                break;
            case 0xc9:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.bc.c;
                _regs_prev.value = tmp;
                tmp |= (1 << 1);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.bc.c = tmp;
                break;
            case 0xca:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.de.d;
                _regs_prev.value = tmp;
                tmp |= (1 << 1);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.de.d = tmp;
                break;
            case 0xcb:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.de.e;
                _regs_prev.value = tmp;
                tmp |= (1 << 1);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.de.e = tmp;
                break;
            case 0xcc:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.hl.h;
                _regs_prev.value = tmp;
                tmp |= (1 << 1);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.hl.h = tmp;
                break;
            case 0xcd:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.hl.l;
                _regs_prev.value = tmp;
                tmp |= (1 << 1);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.hl.l = tmp;
                break;
            case 0xce:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = mem_get_byte(_regs_curr.hl.word);
                _regs_prev.value = tmp;
                tmp |= (1 << 1);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else mem_set_byte(_regs_curr.hl.word, tmp);
                break;
            case 0xcf:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.af.a;
                _regs_prev.value = tmp;
                tmp |= (1 << 1);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.af.a = tmp;
                break;
            case 0xd0:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.bc.b;
                _regs_prev.value = tmp;
                tmp |= (1 << 2);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.bc.b = tmp;
                break;
            case 0xd1:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.bc.c;
                _regs_prev.value = tmp;
                tmp |= (1 << 2);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.bc.c = tmp;
                break;
            case 0xd2:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.de.d;
                _regs_prev.value = tmp;
                tmp |= (1 << 2);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.de.d = tmp;
                break;
            case 0xd3:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.de.e;
                _regs_prev.value = tmp;
                tmp |= (1 << 2);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.de.e = tmp;
                break;
            case 0xd4:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.hl.h;
                _regs_prev.value = tmp;
                tmp |= (1 << 2);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.hl.h = tmp;
                break;
            case 0xd5:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.hl.l;
                _regs_prev.value = tmp;
                tmp |= (1 << 2);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.hl.l = tmp;
                break;
            case 0xd6:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = mem_get_byte(_regs_curr.hl.word);
                _regs_prev.value = tmp;
                tmp |= (1 << 2);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else mem_set_byte(_regs_curr.hl.word, tmp);
                break;
            case 0xd7:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.af.a;
                _regs_prev.value = tmp;
                tmp |= (1 << 2);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.af.a = tmp;
                break;
            case 0xd8:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.bc.b;
                _regs_prev.value = tmp;
                tmp |= (1 << 3);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.bc.b = tmp;
                break;
            case 0xd9:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.bc.c;
                _regs_prev.value = tmp;
                tmp |= (1 << 3);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.bc.c = tmp;
                break;
            case 0xda:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.de.d;
                _regs_prev.value = tmp;
                tmp |= (1 << 3);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.de.d = tmp;
                break;
            case 0xdb:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.de.e;
                _regs_prev.value = tmp;
                tmp |= (1 << 3);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.de.e = tmp;
                break;
            case 0xdc:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.hl.h;
                _regs_prev.value = tmp;
                tmp |= (1 << 3);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.hl.h = tmp;
                break;
            case 0xdd:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.hl.l;
                _regs_prev.value = tmp;
                tmp |= (1 << 3);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.hl.l = tmp;
                break;
            case 0xde:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = mem_get_byte(_regs_curr.hl.word);
                _regs_prev.value = tmp;
                tmp |= (1 << 3);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else mem_set_byte(_regs_curr.hl.word, tmp);
                break;
            case 0xdf:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.af.a;
                _regs_prev.value = tmp;
                tmp |= (1 << 3);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.af.a = tmp;
                break;
            case 0xe0:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.bc.b;
                _regs_prev.value = tmp;
                tmp |= (1 << 4);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.bc.b = tmp;
                break;
            case 0xe1:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.bc.c;
                _regs_prev.value = tmp;
                tmp |= (1 << 4);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.bc.c = tmp;
                break;
            case 0xe2:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.de.d;
                _regs_prev.value = tmp;
                tmp |= (1 << 4);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.de.d = tmp;
                break;
            case 0xe3:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.de.e;
                _regs_prev.value = tmp;
                tmp |= (1 << 4);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.de.e = tmp;
                break;
            case 0xe4:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.hl.h;
                _regs_prev.value = tmp;
                tmp |= (1 << 4);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.hl.h = tmp;
                break;
            case 0xe5:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.hl.l;
                _regs_prev.value = tmp;
                tmp |= (1 << 4);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.hl.l = tmp;
                break;
            case 0xe6:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = mem_get_byte(_regs_curr.hl.word);
                _regs_prev.value = tmp;
                tmp |= (1 << 4);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else mem_set_byte(_regs_curr.hl.word, tmp);
                break;
            case 0xe7:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.af.a;
                _regs_prev.value = tmp;
                tmp |= (1 << 4);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.af.a = tmp;
                break;
            case 0xe8:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.bc.b;
                _regs_prev.value = tmp;
                tmp |= (1 << 5);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.bc.b = tmp;
                break;
            case 0xe9:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.bc.c;
                _regs_prev.value = tmp;
                tmp |= (1 << 5);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.bc.c = tmp;
                break;
            case 0xea:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.de.d;
                _regs_prev.value = tmp;
                tmp |= (1 << 5);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.de.d = tmp;
                break;
            case 0xeb:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.de.e;
                _regs_prev.value = tmp;
                tmp |= (1 << 5);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.de.e = tmp;
                break;
            case 0xec:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.hl.h;
                _regs_prev.value = tmp;
                tmp |= (1 << 5);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.hl.h = tmp;
                break;
            case 0xed:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.hl.l;
                _regs_prev.value = tmp;
                tmp |= (1 << 5);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.hl.l = tmp;
                break;
            case 0xee:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = mem_get_byte(_regs_curr.hl.word);
                _regs_prev.value = tmp;
                tmp |= (1 << 5);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else mem_set_byte(_regs_curr.hl.word, tmp);
                break;
            case 0xef:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.af.a;
                _regs_prev.value = tmp;
                tmp |= (1 << 5);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.af.a = tmp;
                break;
            case 0xf0:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.bc.b;
                _regs_prev.value = tmp;
                tmp |= (1 << 6);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.bc.b = tmp;
                break;
            case 0xf1:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.bc.c;
                _regs_prev.value = tmp;
                tmp |= (1 << 6);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.bc.c = tmp;
                break;
            case 0xf2:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.de.d;
                _regs_prev.value = tmp;
                tmp |= (1 << 6);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.de.d = tmp;
                break;
            case 0xf3:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.de.e;
                _regs_prev.value = tmp;
                tmp |= (1 << 6);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.de.e = tmp;
                break;
            case 0xf4:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.hl.h;
                _regs_prev.value = tmp;
                tmp |= (1 << 6);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.hl.h = tmp;
                break;
            case 0xf5:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.hl.l;
                _regs_prev.value = tmp;
                tmp |= (1 << 6);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.hl.l = tmp;
                break;
            case 0xf6:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = mem_get_byte(_regs_curr.hl.word);
                _regs_prev.value = tmp;
                tmp |= (1 << 6);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else mem_set_byte(_regs_curr.hl.word, tmp);
                break;
            case 0xf7:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.af.a;
                _regs_prev.value = tmp;
                tmp |= (1 << 6);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.af.a = tmp;
                break;
            case 0xf8:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.bc.b;
                _regs_prev.value = tmp;
                tmp |= (1 << 7);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.bc.b = tmp;
                break;
            case 0xf9:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.bc.c;
                _regs_prev.value = tmp;
                tmp |= (1 << 7);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.bc.c = tmp;
                break;
            case 0xfa:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.de.d;
                _regs_prev.value = tmp;
                tmp |= (1 << 7);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.de.d = tmp;
                break;
            case 0xfb:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.de.e;
                _regs_prev.value = tmp;
                tmp |= (1 << 7);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.de.e = tmp;
                break;
            case 0xfc:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.hl.h;
                _regs_prev.value = tmp;
                tmp |= (1 << 7);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.hl.h = tmp;
                break;
            case 0xfd:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.hl.l;
                _regs_prev.value = tmp;
                tmp |= (1 << 7);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.hl.l = tmp;
                break;
            case 0xfe:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = mem_get_byte(_regs_curr.hl.word);
                _regs_prev.value = tmp;
                tmp |= (1 << 7);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else mem_set_byte(_regs_curr.hl.word, tmp);
                break;
            case 0xff:
                if(_regs_curr.prefix_dd) tmp = mem_get_byte(_regs_curr.ix.word + offset_value);
                else if(_regs_curr.prefix_fd) tmp = mem_get_byte(_regs_curr.iy.word + offset_value);
                else tmp = _regs_curr.af.a;
                _regs_prev.value = tmp;
                tmp |= (1 << 7);
                _regs_curr.value = tmp;
                if(_regs_curr.prefix_dd) mem_set_byte(_regs_curr.ix.word + offset_value, tmp);
                else if(_regs_curr.prefix_fd) mem_set_byte(_regs_curr.iy.word + offset_value, tmp);
                else _regs_curr.af.a = tmp;
                break;

        }
    }
    else if(_regs_curr.prefix_dd || _regs_curr.prefix_fd)
    {
        switch(ip_get_byte())
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
                _regs_curr.af.a = alu_rlc(_regs_curr.af.a);
                _regs_curr.value = _regs_curr.af.a;
                break;
            case 0x08:
                tmp = _regs_curr.af.word;
                _regs_curr.af.word = _regs_curr.af_alt;
                _regs_curr.af_alt = tmp;
                break;
            case 0x09:
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = _regs_curr.ix.word;
                    _regs_curr.ix.word = alu_add_word(_regs_curr.ix.word, _regs_curr.bc.word);
                    _regs_curr.value = _regs_curr.ix.word;
                }
                else
                {
                    _regs_prev.value = _regs_curr.iy.word;
                    _regs_curr.iy.word = alu_add_word(_regs_curr.iy.word, _regs_curr.bc.word);
                    _regs_curr.value = _regs_curr.iy.word;
                }
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
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = _regs_curr.ix.word;
                    _regs_curr.ix.word = alu_add_word(_regs_curr.ix.word, _regs_curr.de.word);
                    _regs_curr.value = _regs_curr.ix.word;
                }
                else
                {
                    _regs_prev.value = _regs_curr.iy.word;
                    _regs_curr.iy.word = alu_add_word(_regs_curr.iy.word, _regs_curr.de.word);
                    _regs_curr.value = _regs_curr.iy.word;
                }
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
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = _regs_curr.ix.word;
                    _regs_curr.ix.word = ip_get_word();
                    _regs_curr.value = _regs_curr.ix.word;
                }
                else
                {
                    _regs_prev.value = _regs_curr.iy.word;
                    _regs_curr.iy.word = ip_get_word();
                    _regs_curr.value = _regs_curr.iy.word;
                }
                break;
            case 0x22:
                tmp = ip_get_word();
                _regs_prev.value = mem_get_word(tmp);
                if(_regs_curr.prefix_dd)
                {
                    mem_set_word(tmp, _regs_curr.ix.word);
                    _regs_curr.value = _regs_curr.ix.word;
                }
                else
                {
                    mem_set_word(tmp, _regs_curr.iy.word);
                    _regs_curr.value = _regs_curr.iy.word;
                }
                break;
            case 0x23:
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = _regs_curr.ix.word;
                    _regs_curr.ix.word++;
                    _regs_curr.value = _regs_curr.ix.word;
                }
                else
                {
                    _regs_prev.value = _regs_curr.iy.word;
                    _regs_curr.iy.word++;
                    _regs_curr.value = _regs_curr.iy.word;
                }
                break;
            case 0x24:
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = _regs_curr.ix.h;
                    _regs_curr.ix.h = alu_inc_byte(_regs_curr.ix.h);
                    _regs_curr.value = _regs_curr.ix.h;
                }
                else
                {
                    _regs_prev.value = _regs_curr.iy.h;
                    _regs_curr.iy.h = alu_inc_byte(_regs_curr.iy.h);
                    _regs_curr.value = _regs_curr.iy.h;
                }
                break;
            case 0x25:
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = _regs_curr.ix.h;
                    _regs_curr.ix.h = alu_dec_byte(_regs_curr.ix.h);
                    _regs_curr.value = _regs_curr.ix.h;
                }
                else
                {
                    _regs_prev.value = _regs_curr.iy.h;
                    _regs_curr.iy.h = alu_dec_byte(_regs_curr.iy.h);
                    _regs_curr.value = _regs_curr.iy.h;
                }
                break;
            case 0x26:
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = _regs_curr.ix.h;
                    _regs_curr.ix.h = ip_get_byte();
                    _regs_curr.value = _regs_curr.ix.h;
                }
                else
                {
                    _regs_prev.value = _regs_curr.iy.h;
                    _regs_curr.iy.h = ip_get_byte();
                    _regs_curr.value = _regs_curr.iy.h;
                }
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
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = _regs_curr.ix.word;
                    _regs_curr.ix.word = alu_add_word(_regs_curr.ix.word, _regs_curr.ix.word);
                    _regs_curr.value = _regs_curr.ix.word;
                }
                else
                {
                    _regs_prev.value = _regs_curr.iy.word;
                    _regs_curr.iy.word = alu_add_word(_regs_curr.iy.word, _regs_curr.iy.word);
                    _regs_curr.value = _regs_curr.iy.word;
                }
                break;
            case 0x2a:
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = _regs_curr.ix.word;
                    _regs_curr.ix.word = mem_get_word(ip_get_word());
                    _regs_curr.value = _regs_curr.ix.word;
                }
                else
                {
                    _regs_prev.value = _regs_curr.iy.word;
                    _regs_curr.iy.word = mem_get_word(ip_get_word());
                    _regs_curr.value = _regs_curr.iy.word;
                }
                break;
            case 0x2b:
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = _regs_curr.ix.word;
                    _regs_curr.ix.word--;
                    _regs_curr.value = _regs_curr.ix.word;
                }
                else
                {
                    _regs_prev.value = _regs_curr.iy.word;
                    _regs_curr.iy.word--;
                    _regs_curr.value = _regs_curr.iy.word;
                }
                break;
            case 0x2c:
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = _regs_curr.ix.l;
                    _regs_curr.ix.l = alu_inc_byte(_regs_curr.ix.l);
                    _regs_curr.value = _regs_curr.ix.l;
                }
                else
                {
                    _regs_prev.value = _regs_curr.iy.l;
                    _regs_curr.iy.l = alu_inc_byte(_regs_curr.iy.l);
                    _regs_curr.value = _regs_curr.iy.l;
                }
                break;
            case 0x2d:
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = _regs_curr.ix.l;
                    _regs_curr.ix.l = alu_dec_byte(_regs_curr.ix.l);
                    _regs_curr.value = _regs_curr.ix.l;
                }
                else
                {
                    _regs_prev.value = _regs_curr.iy.l;
                    _regs_curr.iy.l = alu_dec_byte(_regs_curr.iy.l);
                    _regs_curr.value = _regs_curr.iy.l;
                }
                break;
            case 0x2e:
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = _regs_curr.ix.l;
                    _regs_curr.ix.l = ip_get_byte();
                    _regs_curr.value = _regs_curr.ix.l;
                }
                else
                {
                    _regs_prev.value = _regs_curr.iy.l;
                    _regs_curr.iy.l = ip_get_byte();
                    _regs_curr.value = _regs_curr.iy.l;
                }
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
                offset_value = ip_get_byte_signed();
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = mem_get_byte(_regs_curr.ix.word + offset_value);
                    mem_set_byte(_regs_curr.ix.word + offset_value, mem_get_byte(_regs_curr.ix.word + offset_value) + 1);
                    _regs_curr.value = mem_get_byte(_regs_curr.ix.word + offset_value);
                }
                else
                {
                    _regs_prev.value = mem_get_byte(_regs_curr.iy.word + offset_value);
                    mem_set_byte(_regs_curr.iy.word + offset_value, mem_get_byte(_regs_curr.iy.word + offset_value) + 1);
                    _regs_curr.value = mem_get_byte(_regs_curr.iy.word + offset_value);
                }
                break;
            case 0x35:
                offset_value = ip_get_byte_signed();
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = mem_get_byte(_regs_curr.ix.word + offset_value);
                    mem_set_byte(_regs_curr.ix.word + offset_value, mem_get_byte(_regs_curr.ix.word + offset_value) - 1);
                    _regs_curr.value = mem_get_byte(_regs_curr.ix.word + offset_value);
                }
                else
                {
                    _regs_prev.value = mem_get_byte(_regs_curr.iy.word + offset_value);
                    mem_set_byte(_regs_curr.iy.word + offset_value, mem_get_byte(_regs_curr.iy.word + offset_value) - 1);
                    _regs_curr.value = mem_get_byte(_regs_curr.iy.word + offset_value);
                }
                break;
            case 0x36:
                offset_value = ip_get_byte_signed();
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = mem_get_byte(_regs_curr.ix.word + offset_value);
                    mem_set_byte(_regs_curr.ix.word + offset_value, ip_get_byte());
                    _regs_curr.value = mem_get_byte(_regs_curr.ix.word + offset_value);
                }
                else
                {
                    _regs_prev.value = mem_get_byte(_regs_curr.iy.word + offset_value);
                    mem_set_byte(_regs_curr.iy.word + offset_value, ip_get_byte());
                    _regs_curr.value = mem_get_byte(_regs_curr.iy.word + offset_value);
                }
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
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = _regs_curr.ix.word;
                    _regs_curr.ix.word = alu_add_word(_regs_curr.ix.word, _regs_curr.sp);
                    _regs_curr.value = _regs_curr.ix.word;
                }
                else
                {
                    _regs_prev.value = _regs_curr.iy.word;
                    _regs_curr.iy.word = alu_add_word(_regs_curr.iy.word, _regs_curr.sp);
                    _regs_curr.value = _regs_curr.iy.word;
                }
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
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = _regs_curr.bc.b;
                    _regs_curr.bc.b = _regs_curr.ix.h;
                    _regs_curr.value = _regs_curr.bc.b;
                }
                else
                {
                    _regs_prev.value = _regs_curr.bc.b;
                    _regs_curr.bc.b = _regs_curr.iy.h;
                    _regs_curr.value = _regs_curr.bc.b;
                }
                break;
            case 0x45:
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = _regs_curr.bc.b;
                    _regs_curr.bc.b = _regs_curr.ix.l;
                    _regs_curr.value = _regs_curr.bc.b;
                }
                else
                {
                    _regs_prev.value = _regs_curr.bc.b;
                    _regs_curr.bc.b = _regs_curr.iy.l;
                    _regs_curr.value = _regs_curr.bc.b;
                }
                break;
            case 0x46:
                offset_value = ip_get_byte_signed();
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = _regs_curr.bc.b;
                    _regs_curr.bc.b = mem_get_byte(_regs_curr.ix.word + offset_value);
                    _regs_curr.value = _regs_curr.bc.b;
                }
                else
                {
                    _regs_prev.value = _regs_curr.bc.b;
                    _regs_curr.bc.b = mem_get_byte(_regs_curr.iy.word + offset_value);
                    _regs_curr.value = _regs_curr.bc.b;
                }
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
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = _regs_curr.bc.c;
                    _regs_curr.bc.c = _regs_curr.ix.h;
                    _regs_curr.value = _regs_curr.bc.c;
                }
                else
                {
                    _regs_prev.value = _regs_curr.bc.c;
                    _regs_curr.bc.c = _regs_curr.iy.h;
                    _regs_curr.value = _regs_curr.bc.c;
                }
                break;
            case 0x4d:
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = _regs_curr.bc.c;
                    _regs_curr.bc.c = _regs_curr.ix.l;
                    _regs_curr.value = _regs_curr.bc.c;
                }
                else
                {
                    _regs_prev.value = _regs_curr.bc.c;
                    _regs_curr.bc.c = _regs_curr.iy.l;
                    _regs_curr.value = _regs_curr.bc.c;
                }
                break;
            case 0x4e:
                offset_value = ip_get_byte_signed();
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = _regs_curr.bc.c;
                    _regs_curr.bc.c = mem_get_byte(_regs_curr.ix.word + offset_value);
                    _regs_curr.value = _regs_curr.bc.c;
                }
                else
                {
                    _regs_prev.value = _regs_curr.bc.c;
                    _regs_curr.bc.c = mem_get_byte(_regs_curr.iy.word + offset_value);
                    _regs_curr.value = _regs_curr.bc.c;
                }
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
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = _regs_curr.de.d;
                    _regs_curr.de.d = _regs_curr.ix.h;
                    _regs_curr.value = _regs_curr.de.d;
                }
                else
                {
                    _regs_prev.value = _regs_curr.de.d;
                    _regs_curr.de.d = _regs_curr.iy.h;
                    _regs_curr.value = _regs_curr.de.d;
                }
                break;
            case 0x55:
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = _regs_curr.de.d;
                    _regs_curr.de.d = _regs_curr.ix.l;
                    _regs_curr.value = _regs_curr.de.d;
                }
                else
                {
                    _regs_prev.value = _regs_curr.de.d;
                    _regs_curr.de.d = _regs_curr.iy.l;
                    _regs_curr.value = _regs_curr.de.d;
                }
                break;
            case 0x56:
                offset_value = ip_get_byte_signed();
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = _regs_curr.de.d;
                    _regs_curr.de.d = mem_get_byte(_regs_curr.ix.word + offset_value);
                    _regs_curr.value = _regs_curr.de.d;
                }
                else
                {
                    _regs_prev.value = _regs_curr.de.d;
                    _regs_curr.de.d = mem_get_byte(_regs_curr.iy.word + offset_value);
                    _regs_curr.value = _regs_curr.de.d;
                }
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
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = _regs_curr.de.e;
                    _regs_curr.de.e = _regs_curr.ix.h;
                    _regs_curr.value = _regs_curr.de.e;
                }
                else
                {
                    _regs_prev.value = _regs_curr.de.e;
                    _regs_curr.de.e = _regs_curr.iy.h;
                    _regs_curr.value = _regs_curr.de.e;
                }
                break;
            case 0x5d:
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = _regs_curr.de.e;
                    _regs_curr.de.e = _regs_curr.ix.l;
                    _regs_curr.value = _regs_curr.de.e;
                }
                else
                {
                    _regs_prev.value = _regs_curr.de.e;
                    _regs_curr.de.e = _regs_curr.iy.l;
                    _regs_curr.value = _regs_curr.de.e;
                }
                break;
            case 0x5e:
                offset_value = ip_get_byte_signed();
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = _regs_curr.de.e;
                    _regs_curr.de.e = mem_get_byte(_regs_curr.ix.word + offset_value);
                    _regs_curr.value = _regs_curr.de.e;
                }
                else
                {
                    _regs_prev.value = _regs_curr.de.e;
                    _regs_curr.de.e = mem_get_byte(_regs_curr.iy.word + offset_value);
                    _regs_curr.value = _regs_curr.de.e;
                }
                break;
            case 0x5f:
                _regs_prev.value = _regs_curr.de.e;
                _regs_curr.de.e = _regs_curr.af.a;
                _regs_curr.value = _regs_curr.de.e;
                break;
            case 0x60:
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = _regs_curr.ix.h;
                    _regs_curr.ix.h = _regs_curr.bc.b;
                    _regs_curr.value = _regs_curr.ix.h;
                }
                else
                {
                    _regs_prev.value = _regs_curr.iy.h;
                    _regs_curr.iy.h = _regs_curr.bc.b;
                    _regs_curr.value = _regs_curr.iy.h;
                }
                break;
            case 0x61:
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = _regs_curr.ix.h;
                    _regs_curr.ix.h = _regs_curr.bc.c;
                    _regs_curr.value = _regs_curr.ix.h;
                }
                else
                {
                    _regs_prev.value = _regs_curr.iy.h;
                    _regs_curr.iy.h = _regs_curr.bc.c;
                    _regs_curr.value = _regs_curr.iy.h;
                }
                break;
            case 0x62:
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = _regs_curr.ix.h;
                    _regs_curr.ix.h = _regs_curr.de.d;
                    _regs_curr.value = _regs_curr.ix.h;
                }
                else
                {
                    _regs_prev.value = _regs_curr.iy.h;
                    _regs_curr.iy.h = _regs_curr.de.d;
                    _regs_curr.value = _regs_curr.iy.h;
                }
                break;
            case 0x63:
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = _regs_curr.ix.h;
                    _regs_curr.ix.h = _regs_curr.de.e;
                    _regs_curr.value = _regs_curr.ix.h;
                }
                {
                    _regs_prev.value = _regs_curr.iy.h;
                    _regs_curr.iy.h = _regs_curr.de.e;
                    _regs_curr.value = _regs_curr.iy.h;
                }
                break;
            case 0x64:
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = _regs_curr.ix.h;
                    _regs_curr.ix.h = _regs_curr.ix.h;
                    _regs_curr.value = _regs_curr.ix.h;
                }
                else
                {
                    _regs_prev.value = _regs_curr.iy.h;
                    _regs_curr.iy.h = _regs_curr.iy.h;
                    _regs_curr.value = _regs_curr.iy.h;
                }
                break;
            case 0x65:
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = _regs_curr.ix.h;
                    _regs_curr.ix.h = _regs_curr.ix.l;
                    _regs_curr.value = _regs_curr.ix.h;
                }
                else
                {
                    _regs_prev.value = _regs_curr.iy.h;
                    _regs_curr.iy.h = _regs_curr.iy.l;
                    _regs_curr.value = _regs_curr.iy.h;
                }
                break;
            case 0x66:
                offset_value = ip_get_byte_signed();
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = _regs_curr.ix.h;
                    _regs_curr.ix.h = mem_get_byte(_regs_curr.ix.word + offset_value);
                    _regs_curr.value = _regs_curr.ix.h;
                }
                else
                {
                    _regs_prev.value = _regs_curr.iy.h;
                    _regs_curr.iy.h = mem_get_byte(_regs_curr.iy.word + offset_value);
                    _regs_curr.value = _regs_curr.iy.h;
                }
                break;
            case 0x67:
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = _regs_curr.ix.h;
                    _regs_curr.ix.h = _regs_curr.af.a;
                    _regs_curr.value = _regs_curr.ix.h;
                }
                else
                {
                    _regs_prev.value = _regs_curr.iy.h;
                    _regs_curr.iy.h = _regs_curr.af.a;
                    _regs_curr.value = _regs_curr.iy.h;
                }
                break;
            case 0x68:
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = _regs_curr.ix.l;
                    _regs_curr.ix.l = _regs_curr.bc.b;
                    _regs_curr.value = _regs_curr.ix.l;
                }
                else
                {
                    _regs_prev.value = _regs_curr.iy.l;
                    _regs_curr.iy.l = _regs_curr.bc.b;
                    _regs_curr.value = _regs_curr.iy.l;
                }
                break;
            case 0x69:
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = _regs_curr.ix.l;
                    _regs_curr.ix.l = _regs_curr.bc.c;
                    _regs_curr.value = _regs_curr.ix.l;
                }
                else
                {
                    _regs_prev.value = _regs_curr.iy.l;
                    _regs_curr.iy.l = _regs_curr.bc.c;
                    _regs_curr.value = _regs_curr.iy.l;
                }
                break;
            case 0x6a:
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = _regs_curr.ix.l;
                    _regs_curr.ix.l = _regs_curr.de.d;
                    _regs_curr.value = _regs_curr.ix.l;
                }
                else
                {
                    _regs_prev.value = _regs_curr.iy.l;
                    _regs_curr.iy.l = _regs_curr.de.d;
                    _regs_curr.value = _regs_curr.iy.l;
                }
                break;
            case 0x6b:
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = _regs_curr.ix.l;
                    _regs_curr.ix.l = _regs_curr.de.e;
                    _regs_curr.value = _regs_curr.ix.l;
                }
                else
                {
                    _regs_prev.value = _regs_curr.iy.l;
                    _regs_curr.iy.l = _regs_curr.de.e;
                    _regs_curr.value = _regs_curr.iy.l;
                }
                break;
            case 0x6c:
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = _regs_curr.ix.l;
                    _regs_curr.ix.l = _regs_curr.ix.h;
                    _regs_curr.value = _regs_curr.ix.l;
                }
                else
                {
                    _regs_prev.value = _regs_curr.iy.l;
                    _regs_curr.iy.l = _regs_curr.iy.h;
                    _regs_curr.value = _regs_curr.iy.l;
                }
                break;
            case 0x6d:
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = _regs_curr.ix.l;
                    _regs_curr.ix.l = _regs_curr.ix.l;
                    _regs_curr.value = _regs_curr.ix.l;
                }
                else
                {
                    _regs_prev.value = _regs_curr.iy.l;
                    _regs_curr.iy.l = _regs_curr.iy.l;
                    _regs_curr.value = _regs_curr.iy.l;
                }
                break;
            case 0x6e:
                offset_value = ip_get_byte_signed();
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = _regs_curr.ix.l;
                    _regs_curr.ix.l = mem_get_byte(_regs_curr.ix.word + offset_value);
                    _regs_curr.value = _regs_curr.ix.l;
                }
                else
                {
                    _regs_prev.value = _regs_curr.iy.l;
                    _regs_curr.iy.l = mem_get_byte(_regs_curr.iy.word + offset_value);
                    _regs_curr.value = _regs_curr.iy.l;
                }
                break;
            case 0x6f:
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = _regs_curr.ix.l;
                    _regs_curr.ix.l = _regs_curr.af.a;
                    _regs_curr.value = _regs_curr.ix.l;
                }
                else
                {
                    _regs_prev.value = _regs_curr.iy.l;
                    _regs_curr.iy.l = _regs_curr.af.a;
                    _regs_curr.value = _regs_curr.iy.l;
                }
                break;
            case 0x70:
                offset_value = ip_get_byte_signed();
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = mem_get_byte(_regs_curr.ix.word + offset_value);
                    mem_set_byte(_regs_curr.ix.word + offset_value, _regs_curr.bc.b);
                    _regs_curr.value = mem_get_byte(_regs_curr.ix.word + offset_value);
                }
                else
                {
                    _regs_prev.value = mem_get_byte(_regs_curr.iy.word + offset_value);
                    mem_set_byte(_regs_curr.iy.word + offset_value, _regs_curr.bc.b);
                    _regs_curr.value = mem_get_byte(_regs_curr.iy.word + offset_value);
                }
                break;
            case 0x71:
                offset_value = ip_get_byte_signed();
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = mem_get_byte(_regs_curr.ix.word + offset_value);
                    mem_set_byte(_regs_curr.ix.word + offset_value, _regs_curr.bc.c);
                    _regs_curr.value = mem_get_byte(_regs_curr.ix.word + offset_value);
                }
                else
                {
                    _regs_prev.value = mem_get_byte(_regs_curr.iy.word + offset_value);
                    mem_set_byte(_regs_curr.iy.word + offset_value, _regs_curr.bc.c);
                    _regs_curr.value = mem_get_byte(_regs_curr.iy.word + offset_value);
                }
                break;
            case 0x72:
                offset_value = ip_get_byte_signed();
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = mem_get_byte(_regs_curr.ix.word + offset_value);
                    mem_set_byte(_regs_curr.ix.word + offset_value, _regs_curr.de.d);
                    _regs_curr.value = mem_get_byte(_regs_curr.ix.word + offset_value);
                }
                else
                {
                    _regs_prev.value = mem_get_byte(_regs_curr.iy.word + offset_value);
                    mem_set_byte(_regs_curr.iy.word + offset_value, _regs_curr.de.d);
                    _regs_curr.value = mem_get_byte(_regs_curr.iy.word + offset_value);
                }
                break;
            case 0x73:
                offset_value = ip_get_byte_signed();
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = mem_get_byte(_regs_curr.ix.word + offset_value);
                    mem_set_byte(_regs_curr.ix.word + offset_value, _regs_curr.de.e);
                    _regs_curr.value = mem_get_byte(_regs_curr.ix.word + offset_value);
                }
                else
                {
                    _regs_prev.value = mem_get_byte(_regs_curr.iy.word + offset_value);
                    mem_set_byte(_regs_curr.iy.word + offset_value, _regs_curr.de.e);
                    _regs_curr.value = mem_get_byte(_regs_curr.iy.word + offset_value);
                }
                break;
            case 0x74:
                offset_value = ip_get_byte_signed();
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = mem_get_byte(_regs_curr.ix.word + offset_value);
                    mem_set_byte(_regs_curr.ix.word + offset_value, _regs_curr.iy.h);
                    _regs_curr.value = mem_get_byte(_regs_curr.ix.word + offset_value);
                }
                else
                {
                    _regs_prev.value = mem_get_byte(_regs_curr.iy.word + offset_value);
                    mem_set_byte(_regs_curr.iy.word + offset_value, _regs_curr.iy.h);
                    _regs_curr.value = mem_get_byte(_regs_curr.iy.word + offset_value);
                }
                break;
            case 0x75:
                offset_value = ip_get_byte_signed();
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = mem_get_byte(_regs_curr.ix.word + offset_value);
                    mem_set_byte(_regs_curr.ix.word + offset_value, _regs_curr.hl.l);
                    _regs_curr.value = mem_get_byte(_regs_curr.ix.word + offset_value);
                }
                else
                {
                    _regs_prev.value = mem_get_byte(_regs_curr.iy.word + offset_value);
                    mem_set_byte(_regs_curr.iy.word + offset_value, _regs_curr.hl.l);
                    _regs_curr.value = mem_get_byte(_regs_curr.iy.word + offset_value);
                }
                break;
            case 0x76:
                _regs_curr.ip -= 2; // Halt infinity loop
                break;
            case 0x77:
                offset_value = ip_get_byte_signed();
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = mem_get_byte(_regs_curr.ix.word + offset_value);
                    mem_set_byte(_regs_curr.ix.word + offset_value, _regs_curr.af.a);
                    _regs_curr.value = mem_get_byte(_regs_curr.ix.word + offset_value);
                }
                else
                {
                    _regs_prev.value = mem_get_byte(_regs_curr.iy.word + offset_value);
                    mem_set_byte(_regs_curr.iy.word + offset_value, _regs_curr.af.a);
                    _regs_curr.value = mem_get_byte(_regs_curr.iy.word + offset_value);
                }
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
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = _regs_curr.af.a;
                    _regs_curr.af.a = _regs_curr.ix.h;
                    _regs_curr.value = _regs_curr.af.a;
                }
                else
                {
                    _regs_prev.value = _regs_curr.af.a;
                    _regs_curr.af.a = _regs_curr.iy.h;
                    _regs_curr.value = _regs_curr.af.a;
                }
                break;
            case 0x7d:
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = _regs_curr.af.a;
                    _regs_curr.af.a = _regs_curr.ix.l;
                    _regs_curr.value = _regs_curr.af.a;
                }
                else
                {
                    _regs_prev.value = _regs_curr.af.a;
                    _regs_curr.af.a = _regs_curr.iy.l;
                    _regs_curr.value = _regs_curr.af.a;
                }
                break;
            case 0x7e:
                offset_value = ip_get_byte_signed();
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = _regs_curr.af.a;
                    _regs_curr.af.a = mem_get_byte(_regs_curr.ix.word + offset_value);
                    _regs_curr.value = _regs_curr.af.a;
                }
                else
                {
                    _regs_prev.value = _regs_curr.af.a;
                    _regs_curr.af.a = mem_get_byte(_regs_curr.iy.word + offset_value);
                    _regs_curr.value = _regs_curr.af.a;
                }
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
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = _regs_curr.af.a;
                    _regs_curr.af.a = alu_add_byte(_regs_curr.af.a, _regs_curr.ix.h);
                    _regs_curr.value = _regs_curr.af.a;
                }
                else
                {
                    _regs_prev.value = _regs_curr.af.a;
                    _regs_curr.af.a = alu_add_byte(_regs_curr.af.a, _regs_curr.iy.h);
                    _regs_curr.value = _regs_curr.af.a;
                }
                break;
            case 0x85:
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = _regs_curr.af.a;
                    _regs_curr.af.a = alu_add_byte(_regs_curr.af.a, _regs_curr.ix.l);
                    _regs_curr.value = _regs_curr.af.a;
                }
                else
                {
                    _regs_prev.value = _regs_curr.af.a;
                    _regs_curr.af.a = alu_add_byte(_regs_curr.af.a, _regs_curr.iy.l);
                    _regs_curr.value = _regs_curr.af.a;
                }
                break;
            case 0x86:
                offset_value = ip_get_byte_signed();
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = _regs_curr.af.a;
                    _regs_curr.af.a = alu_add_byte(_regs_curr.af.a, mem_get_byte(_regs_curr.ix.word + offset_value));
                    _regs_curr.value = _regs_curr.af.a;
                }
                else
                {
                    _regs_prev.value = _regs_curr.af.a;
                    _regs_curr.af.a = alu_add_byte(_regs_curr.af.a, mem_get_byte(_regs_curr.iy.word + offset_value));
                    _regs_curr.value = _regs_curr.af.a;
                }
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
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = _regs_curr.af.a;
                    _regs_curr.af.a = alu_adc_byte(_regs_curr.af.a, _regs_curr.ix.h);
                    _regs_curr.value = _regs_curr.af.a;
                }
                else
                {
                    _regs_prev.value = _regs_curr.af.a;
                    _regs_curr.af.a = alu_adc_byte(_regs_curr.af.a, _regs_curr.iy.h);
                    _regs_curr.value = _regs_curr.af.a;
                }
                break;
            case 0x8d:
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = _regs_curr.af.a;
                    _regs_curr.af.a = alu_adc_byte(_regs_curr.af.a, _regs_curr.ix.l);
                    _regs_curr.value = _regs_curr.af.a;
                }
                else
                {
                    _regs_prev.value = _regs_curr.af.a;
                    _regs_curr.af.a = alu_adc_byte(_regs_curr.af.a, _regs_curr.iy.l);
                    _regs_curr.value = _regs_curr.af.a;
                }
                break;
            case 0x8e:
                offset_value = ip_get_byte_signed();
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = _regs_curr.af.a;
                    _regs_curr.af.a = alu_adc_byte(_regs_curr.af.a, mem_get_byte(_regs_curr.ix.word + offset_value));
                    _regs_curr.value = _regs_curr.af.a;
                }
                else
                {
                    _regs_prev.value = _regs_curr.af.a;
                    _regs_curr.af.a = alu_adc_byte(_regs_curr.af.a, mem_get_byte(_regs_curr.iy.word + offset_value));
                    _regs_curr.value = _regs_curr.af.a;
                }
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
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = _regs_curr.af.a;
                    _regs_curr.af.a = alu_sub_byte(_regs_curr.af.a, _regs_curr.ix.h);
                    _regs_curr.value = _regs_curr.af.a;
                }
                else
                {
                    _regs_prev.value = _regs_curr.af.a;
                    _regs_curr.af.a = alu_sub_byte(_regs_curr.af.a, _regs_curr.iy.h);
                    _regs_curr.value = _regs_curr.af.a;
                }
                break;
            case 0x95:
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = _regs_curr.af.a;
                    _regs_curr.af.a = alu_sub_byte(_regs_curr.af.a, _regs_curr.ix.l);
                    _regs_curr.value = _regs_curr.af.a;
                }
                else
                {
                    _regs_prev.value = _regs_curr.af.a;
                    _regs_curr.af.a = alu_sub_byte(_regs_curr.af.a, _regs_curr.iy.l);
                    _regs_curr.value = _regs_curr.af.a;
                }
                break;
            case 0x96:
                offset_value = ip_get_byte_signed();
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = _regs_curr.af.a;
                    _regs_curr.af.a = alu_sub_byte(_regs_curr.af.a, mem_get_byte(_regs_curr.ix.word + offset_value));
                    _regs_curr.value = _regs_curr.af.a;
                }
                else
                {
                    _regs_prev.value = _regs_curr.af.a;
                    _regs_curr.af.a = alu_sub_byte(_regs_curr.af.a, mem_get_byte(_regs_curr.iy.word + offset_value));
                    _regs_curr.value = _regs_curr.af.a;
                }
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
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = _regs_curr.af.a;
                    _regs_curr.af.a = alu_sbc_byte(_regs_curr.af.a, _regs_curr.ix.h);
                    _regs_curr.value = _regs_curr.af.a;
                }
                else
                {
                    _regs_prev.value = _regs_curr.af.a;
                    _regs_curr.af.a = alu_sbc_byte(_regs_curr.af.a, _regs_curr.iy.h);
                    _regs_curr.value = _regs_curr.af.a;
                }
                break;
            case 0x9d:
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = _regs_curr.af.a;
                    _regs_curr.af.a = alu_sbc_byte(_regs_curr.af.a, _regs_curr.ix.l);
                    _regs_curr.value = _regs_curr.af.a;
                }
                else
                {
                    _regs_prev.value = _regs_curr.af.a;
                    _regs_curr.af.a = alu_sbc_byte(_regs_curr.af.a, _regs_curr.iy.l);
                    _regs_curr.value = _regs_curr.af.a;
                }
                break;
            case 0x9e:
                offset_value = ip_get_byte_signed();
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = _regs_curr.af.a;
                    _regs_curr.af.a = alu_sbc_byte(_regs_curr.af.a, mem_get_byte(_regs_curr.ix.word + offset_value));
                    _regs_curr.value = _regs_curr.af.a;
                }
                else
                {
                    _regs_prev.value = _regs_curr.af.a;
                    _regs_curr.af.a = alu_sbc_byte(_regs_curr.af.a, mem_get_byte(_regs_curr.iy.word + offset_value));
                    _regs_curr.value = _regs_curr.af.a;
                }
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
                if(_regs_curr.prefix_dd)
                {
                _regs_prev.value = _regs_curr.af.a;
                _regs_curr.af.a = alu_and_byte(_regs_curr.af.a, _regs_curr.hl.h);
                _regs_curr.value = _regs_curr.af.a;
                }
                break;
            case 0xa5:
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = _regs_curr.af.a;
                    _regs_curr.af.a = alu_and_byte(_regs_curr.af.a, _regs_curr.ix.l);
                    _regs_curr.value = _regs_curr.af.a;
                }
                else
                {
                    _regs_prev.value = _regs_curr.af.a;
                    _regs_curr.af.a = alu_and_byte(_regs_curr.af.a, _regs_curr.iy.l);
                    _regs_curr.value = _regs_curr.af.a;
                }
                break;
            case 0xa6:
                offset_value = ip_get_byte_signed();
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = _regs_curr.af.a;
                    _regs_curr.af.a = alu_and_byte(_regs_curr.af.a, mem_get_byte(_regs_curr.ix.word));
                    _regs_curr.value = _regs_curr.af.a;
                }
                else
                {
                    _regs_prev.value = _regs_curr.af.a;
                    _regs_curr.af.a = alu_and_byte(_regs_curr.af.a, mem_get_byte(_regs_curr.iy.word));
                    _regs_curr.value = _regs_curr.af.a;
                }
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
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = _regs_curr.af.a;
                    _regs_curr.af.a = alu_xor_byte(_regs_curr.af.a, _regs_curr.ix.h);
                    _regs_curr.value = _regs_curr.af.a;
                }
                else
                {
                    _regs_prev.value = _regs_curr.af.a;
                    _regs_curr.af.a = alu_xor_byte(_regs_curr.af.a, _regs_curr.iy.h);
                    _regs_curr.value = _regs_curr.af.a;
                }
                break;
            case 0xad:
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = _regs_curr.af.a;
                    _regs_curr.af.a = alu_xor_byte(_regs_curr.af.a, _regs_curr.ix.l);
                    _regs_curr.value = _regs_curr.af.a;
                }
                else
                {
                    _regs_prev.value = _regs_curr.af.a;
                    _regs_curr.af.a = alu_xor_byte(_regs_curr.af.a, _regs_curr.iy.l);
                    _regs_curr.value = _regs_curr.af.a;
                }
                break;
            case 0xae:
                offset_value = ip_get_byte_signed();
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = _regs_curr.af.a;
                    _regs_curr.af.a = alu_xor_byte(_regs_curr.af.a, mem_get_byte(_regs_curr.ix.word + offset_value));
                    _regs_curr.value = _regs_curr.af.a;
                }
                else
                {
                    _regs_prev.value = _regs_curr.af.a;
                    _regs_curr.af.a = alu_xor_byte(_regs_curr.af.a, mem_get_byte(_regs_curr.iy.word + offset_value));
                    _regs_curr.value = _regs_curr.af.a;
                }
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
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = _regs_curr.af.a;
                    _regs_curr.af.a = alu_or_byte(_regs_curr.af.a, _regs_curr.ix.h);
                    _regs_curr.value = _regs_curr.af.a;
                }
                else
                {
                    _regs_prev.value = _regs_curr.af.a;
                    _regs_curr.af.a = alu_or_byte(_regs_curr.af.a, _regs_curr.iy.h);
                    _regs_curr.value = _regs_curr.af.a;
                }
                break;
            case 0xb5:
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = _regs_curr.af.a;
                    _regs_curr.af.a = alu_or_byte(_regs_curr.af.a, _regs_curr.ix.l);
                    _regs_curr.value = _regs_curr.af.a;
                }
                else
                {
                    _regs_prev.value = _regs_curr.af.a;
                    _regs_curr.af.a = alu_or_byte(_regs_curr.af.a, _regs_curr.iy.l);
                    _regs_curr.value = _regs_curr.af.a;
                }
                break;
            case 0xb6:
                offset_value = ip_get_byte_signed();
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = _regs_curr.af.a;
                    _regs_curr.af.a = alu_or_byte(_regs_curr.af.a, mem_get_byte(_regs_curr.ix.word + offset_value));
                    _regs_curr.value = _regs_curr.af.a;
                }
                else
                {
                    _regs_prev.value = _regs_curr.af.a;
                    _regs_curr.af.a = alu_or_byte(_regs_curr.af.a, mem_get_byte(_regs_curr.iy.word + offset_value));
                    _regs_curr.value = _regs_curr.af.a;
                }
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
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = _regs_curr.af.a;
                    alu_sub_byte(_regs_curr.af.a, _regs_curr.ix.h);
                    _regs_curr.value = _regs_curr.af.a;
                }
                else
                {
                    _regs_prev.value = _regs_curr.af.a;
                    alu_sub_byte(_regs_curr.af.a, _regs_curr.iy.h);
                    _regs_curr.value = _regs_curr.af.a;
                }
                break;
            case 0xbd:
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = _regs_curr.af.a;
                    alu_sub_byte(_regs_curr.af.a, _regs_curr.ix.l);
                    _regs_curr.value = _regs_curr.af.a;
                }
                else
                {
                    _regs_prev.value = _regs_curr.af.a;
                    alu_sub_byte(_regs_curr.af.a, _regs_curr.iy.l);
                    _regs_curr.value = _regs_curr.af.a;
                }
                break;
            case 0xbe:
                offset_value = ip_get_byte_signed();
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = _regs_curr.af.a;
                    alu_sub_byte(_regs_curr.af.a, mem_get_byte(_regs_curr.ix.word + offset_value));
                    _regs_curr.value = _regs_curr.af.a;
                }
                else
                {
                    _regs_prev.value = _regs_curr.af.a;
                    alu_sub_byte(_regs_curr.af.a, mem_get_byte(_regs_curr.iy.word + offset_value));
                    _regs_curr.value = _regs_curr.af.a;
                }
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
                reset_prefixes = false;
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
                reset_prefixes = false;
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
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = _regs_curr.ix.word;
                    _regs_curr.ix.word = mem_get_word(_regs_curr.sp);
                    _regs_curr.sp+=2;
                    _regs_curr.value = _regs_curr.ix.word;
                }
                else
                {
                    _regs_prev.value = _regs_curr.iy.word;
                    _regs_curr.iy.word = mem_get_word(_regs_curr.sp);
                    _regs_curr.sp+=2;
                    _regs_curr.value = _regs_curr.iy.word;
                }
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
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = _regs_curr.ix.word;
                    tmp = mem_get_word(_regs_curr.sp);
                    mem_set_word(_regs_curr.sp, _regs_curr.ix.word);
                    _regs_curr.ix.word = tmp;
                    _regs_curr.value = _regs_curr.ix.word;
                }
                else
                {
                    _regs_prev.value = _regs_curr.iy.word;
                    tmp = mem_get_word(_regs_curr.sp);
                    mem_set_word(_regs_curr.sp, _regs_curr.iy.word);
                    _regs_curr.iy.word = tmp;
                    _regs_curr.value = _regs_curr.iy.word;
                }
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
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = _regs_curr.sp;
                    _regs_curr.sp-=2;
                    mem_set_word(_regs_curr.sp, _regs_curr.ix.word);
                    _regs_curr.value = _regs_curr.sp;
                }
                else
                {
                    _regs_prev.value = _regs_curr.sp;
                    _regs_curr.sp-=2;
                    mem_set_word(_regs_curr.sp, _regs_curr.iy.word);
                    _regs_curr.value = _regs_curr.sp;
                }
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
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = _regs_curr.ip;
                    _regs_curr.ip = mem_get_word(_regs_curr.ix.word);
                    _regs_curr.value = _regs_curr.ip;
                }
                else
                {
                    _regs_prev.value = _regs_curr.ip;
                    _regs_curr.ip = mem_get_word(_regs_curr.iy.word);
                    _regs_curr.value = _regs_curr.ip;
                }
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
                reset_prefixes = false;
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
                if(_regs_curr.prefix_dd)
                {
                    _regs_prev.value = _regs_curr.sp;
                    _regs_curr.sp = _regs_curr.ix.word;
                    _regs_curr.value = _regs_curr.sp;
                }
                else
                {
                    _regs_prev.value = _regs_curr.sp;
                    _regs_curr.sp = _regs_curr.iy.word;
                    _regs_curr.value = _regs_curr.sp;
                }
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
                reset_prefixes = false;
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

    }
    if(reset_prefixes)
    {
        _regs_curr.prefix_cb = false;
        _regs_curr.prefix_dd = false;
        _regs_curr.prefix_ed = false;
        _regs_curr.prefix_fd = false;
    }
    _regs_curr.r++;
}


void exec()
{
    memset(&_regs_curr, 0, sizeof(z80_regs_t));
    _regs_curr.ip = 0x100;
    _regs_curr.sp = 0xf000;
    keyb_init();
    while(_executing)
    {
        if(_debug && _skip_call_step && _regs_curr.ip == _skip_call_step_address)
        {
            _skip_call_step = false;
            _skip_call_step_address = -1;
        }
        keyb_process();
        if(_debug) screen_draw();
        else screen_draw_if_changed();
        if(_debug && !_skip_call_step)
        {
            while(!_next_step && _executing && _debug)
            {
                keyb_process();
            }
            _next_step = false;
        }
        exec_step();
        if(_skip_call_step && _skip_call_step_address < 0) _skip_call_step = false;
    }
    keyb_exit();
}