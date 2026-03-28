#include "emu.h"
#include <stdio.h>

char *_disk_default = NULL;

void abi_dos_call_5()
{
    if(!_disk_default) _disk_default = _disk_a_path;
    uint16_t ptr;
    char path[FILENAME_MAX];
    switch(_regs_curr.bc.c)
    {
        case 0x00: // Program terminate
            _executing = false;
            break;
        case 0x01: // Console input
        case 0x03: // Auxiliary input
        case 0x07: // Direct console input 
            _regs_curr.hl.l = keyb_wait_pop();
            _regs_curr.af.a = _regs_curr.hl.l;
            screen_put_char(_regs_curr.af.a);
            break;
        case 0x02: // Console output
        case 0x04: // Auxiliary output
            screen_put_char(_regs_curr.de.e);
            break;
        case 0x05: // Disk Reset
            _disk_transferr_address = 0x80;
            break;
        case 0x06: // Direct Console IO
            if(_regs_curr.de.e == 0xff)
            {
                _regs_curr.hl.l = keyb_wait_pop();
                _regs_curr.af.a = _regs_curr.hl.l;
            }
            else
            {
                screen_put_char(_regs_curr.de.e);
            }
            break;
        case 0x08: // Console input without echo
            _regs_curr.hl.l = keyb_wait_pop();
            _regs_curr.af.a = _regs_curr.hl.l;
            break;
        case 0x09: // String output
            ptr = _regs_curr.de.word;
            while(mem_get_byte(ptr) != '$')
            {
                screen_put_char(mem_get_byte(ptr++));
            }
            break;
        case 0x0a: // Buffered Line Input
            ptr = _regs_curr.de.word;
            {
                char c;
                mem_set_byte(ptr + 1, 0);
                do
                {
                    c = keyb_wait_pop();
                    if(mem_get_byte(ptr + 1) < mem_get_byte(ptr))
                    {
                        if(c == 0x8)
                        {
                            screen_put_char(0x8);
                            screen_put_char(' ');
                            screen_put_char(0x8);
                            mem_set_byte(ptr + 1, mem_get_byte(ptr+1) - 1);
                        }
                        else if(c != '\n' && c != '\r')
                        {
                            mem_set_byte(ptr + 2 + mem_get_byte(ptr+1), c);
                            mem_set_byte(ptr + 1, mem_get_byte(ptr+1) + 1);
                            screen_put_char(c);
                        }
                    }
                    screen_draw_if_changed();
                } while(c != '\n' && c != '\r');
                mem_set_byte(ptr + 2 + mem_get_byte(ptr), '\r');
            }
            break;
        case 0x0b: // Console Status
            _regs_curr.af.a = keyb_avail() ? 0xff : 0x00;
            _regs_curr.hl.l = _regs_curr.af.a;
            break;
        case 0x0c: // Return Version Number (CP/M)
            _regs_curr.af.a = 0x22;
            _regs_curr.hl.l = 0x22;
            _regs_curr.bc.b = 0x00;
            _regs_curr.hl.h = 0x00;
            break;
        case 0x0e: // Select Disk
            _regs_curr.af.a = 2;
            _regs_curr.hl.l = 2;
            switch (_regs_curr.de.e)
            {
                case 0:
                    _disk_default = _disk_a_path;
                    break;
                case 1:
                    _disk_default = _disk_b_path;
                    break;
                default:
                    break;
            }
            break;
        case 0x0f: // Open file FCB
        case 0x16: // Create file FCB
        case 0x23: // Get file size
            {
                abi_dos_fcb_t *fcb = (abi_dos_fcb_t*)&_memory[_regs_curr.de.word];
                if(fcb->drive == 0)strcpy(path, _disk_default);
                else if(fcb->drive == 2) strcpy(path, _disk_b_path);
                else strcpy(path, _disk_a_path);
                char c;
                bool inject_dot = false;
                for(int i = 0; i < 8; i++)
                {
                    if(fcb->name[i] == ' ') break;
                    c = tolower(fcb->name[i]);
                    strncat(path, &c, 1);
                    inject_dot = true;
                }
                if(inject_dot) strcat(path, ".");
                for(int i = 0; i < 3; i++)
                {
                    if(fcb->ext[i] == ' ') break;
                    c = tolower(fcb->ext[i]);
                    strncat(path, &c, 1);
                }
                FILE *file = fopen(path, _regs_curr.bc.c == 0x0f ? "rb" : "wb");
                if(!file)
                {
                    fprintf(stderr, "\n\n[ERROR: FILE NOT FOUND: %s]", path);
                    _regs_curr.af.a = 0xff;
                }
                else
                {
                    _regs_curr.af.a = 0x00;
                }
                _regs_curr.hl.l = _regs_curr.af.a;
                if(_regs_curr.bc.c == 0x23)
                {
                    fseek(file, 0, SEEK_END);
                    fcb->file_size = ftell(file);
                    fseek(file, 0, SEEK_SET);
                    fclose(file);
                }
                else
                {
                    fcb->file_size = 0;
                    memcpy(fcb->internal, &file, sizeof(size_t));
                }
            }
            break;
        case 0x10: // Close FCB
            {
                abi_dos_fcb_t *fcb = (abi_dos_fcb_t*)&_memory[_regs_curr.de.word];
                FILE *file;
                memcpy(&file, fcb->internal, sizeof(size_t));
                if(file)
                {
                    fclose(file);
                    _regs_curr.af.a = 0x00;
                }
                else
                {
                    _regs_curr.af.a = 0xff;
                }
                _regs_curr.hl.l = _regs_curr.af.a;
            }
            break;
        case 0x14: // Sequential Read FCB
            {
                abi_dos_fcb_t *fcb = (abi_dos_fcb_t*)&_memory[_regs_curr.de.word];
                FILE *file;
                memcpy(&file, fcb->internal, sizeof(size_t));
                ptr = _disk_transferr_address;
                if(file)
                {
                    if(feof(file))
                    {
                        _regs_curr.af.a = 0x01;
                    }
                    else
                    {
                        for (size_t i = 0; i < 128; i++)
                        {
                            char tmp;
                            if(fread(&tmp, 1, 1, file) <= 0)
                            {
                                tmp = 0;
                            }
                            mem_set_byte(ptr++, tmp);
                        }
                        _regs_curr.af.a = 0x00;
                    }
                }
                else
                {
                    _regs_curr.af.a = 0xff;
                }
                _regs_curr.hl.l = _regs_curr.af.a;
                
            }
            break;
        case 0x15: // Sequential Write FCB
            {
                abi_dos_fcb_t *fcb = (abi_dos_fcb_t*)&_memory[_regs_curr.de.word];
                FILE *file;
                memcpy(&file, fcb->internal, sizeof(size_t));
                ptr = _disk_transferr_address;
                if(file)
                {
                    if(feof(file))
                    {
                        _regs_curr.af.a = 0x01;
                    }
                    else
                    {
                        _regs_curr.af.a = 0x00;
                        for (size_t i = 0; i < 128; i++)
                        {
                            char tmp = mem_get_byte(ptr++);
                            if(fwrite(&tmp, 1, 1, file) <= 0)
                            {
                                _regs_curr.af.a = 0x01;
                                break;
                            }
                        }
                    }
                }
                else
                {
                    _regs_curr.af.a = 0xff;
                }
                _regs_curr.hl.l = _regs_curr.af.a;
                
            }
            break;
        case 0x17: // Rename file
            _regs_curr.af.a = 0xff;
            _regs_curr.hl.l = 0xff;
            break;
        case 0x19: // Rename file
            _regs_curr.af.a = _disk_default == _disk_a_path ? 0 : 1;
            _regs_curr.hl.l = _regs_curr.af.a;
            break;
        case 0x1a: // Set Disk Transfer Address
            _disk_transferr_address = _regs_curr.de.word;
            break;
        case 0x21: // Random Read FCB
            {
                abi_dos_fcb_t *fcb = (abi_dos_fcb_t*)&_memory[_regs_curr.de.word];
                FILE *file;
                memcpy(&file, fcb->internal, sizeof(size_t));
                ptr = _disk_transferr_address;
                if(file)
                {
                    fseek(file, fcb->random_record_number * 128, SEEK_SET);
                    if(feof(file))
                    {
                        _regs_curr.af.a = 0x01;
                    }
                    else
                    {
                        for (size_t i = 0; i < 128; i++)
                        {
                            char tmp;
                            if(fread(&tmp, 1, 1, file) <= 0)
                            {
                                tmp = 0;
                            }
                            mem_set_byte(ptr++, tmp);
                        }
                        _regs_curr.af.a = 0x00;
                    }
                }
                else
                {
                    _regs_curr.af.a = 0xff;
                }
                _regs_curr.hl.l = _regs_curr.af.a;
                
            }
            break;
        case 0x22: // Random Write FCB
            {
                abi_dos_fcb_t *fcb = (abi_dos_fcb_t*)&_memory[_regs_curr.de.word];
                FILE *file;
                memcpy(&file, fcb->internal, sizeof(size_t));
                ptr = _disk_transferr_address;
                if(file)
                {
                    fseek(file, fcb->random_record_number * 128, SEEK_SET);
                    if(feof(file))
                    {
                        _regs_curr.af.a = 0x01;
                    }
                    else
                    {
                        _regs_curr.af.a = 0x00;
                        for (size_t i = 0; i < 128; i++)
                        {
                            char tmp = mem_get_byte(ptr++);
                            if(fwrite(&tmp, 1, 1, file) <= 0)
                            {
                                _regs_curr.af.a = 0x01;
                                break;
                            }
                        }
                    }
                }
                else
                {
                    _regs_curr.af.a = 0xff;
                }
                _regs_curr.hl.l = _regs_curr.af.a;
                
            }
            break;
        case 0x24: // Set Random Record
            {
                abi_dos_fcb_t *fcb = (abi_dos_fcb_t*)&_memory[_regs_curr.de.word];
                FILE *file;
                memcpy(&file, fcb->internal, sizeof(size_t));
                if(file)
                {
                    fcb->random_record_number = fcb->current_record + (fcb->extent_low << 8) + (fcb->extent_high << 16);
                }
                
            }
            break;
        case 0x26: // Random Block Read FCB
            {
                abi_dos_fcb_t *fcb = (abi_dos_fcb_t*)&_memory[_regs_curr.de.word];
                FILE *file;
                memcpy(&file, fcb->internal, sizeof(size_t));
                ptr = _disk_transferr_address;
                uint16_t block_size = fcb->extent_high + (fcb->record_count << 8);
                uint16_t block_count = _regs_curr.hl.word;
                _regs_curr.hl.word = 0;
                if(file)
                {
                    fseek(file, fcb->random_record_number * block_size, SEEK_SET);
                    if(feof(file))
                    {
                        _regs_curr.af.a = 0x01;
                    }
                    else
                    {
                        for (size_t i = 0; i < (block_size * block_count); i++)
                        {
                            char tmp;
                            if(fread(&tmp, 1, 1, file) <= 0)
                            {
                                tmp = 0;
                            }
                            mem_set_byte(ptr++, tmp);
                            _regs_curr.hl.word = (uint16_t)((size_t)i / (size_t)block_size);
                        }
                        _regs_curr.af.a = 0x00;
                    }
                }
                else
                {
                    _regs_curr.af.a = 0xff;
                }
                _regs_curr.hl.l = _regs_curr.af.a;
                
            }
            break;
        case 0x27: // Random Block Write FCB
            {
                abi_dos_fcb_t *fcb = (abi_dos_fcb_t*)&_memory[_regs_curr.de.word];
                FILE *file;
                memcpy(&file, fcb->internal, sizeof(size_t));
                ptr = _disk_transferr_address;
                uint16_t block_size = fcb->extent_high + (fcb->record_count << 8);
                uint16_t block_count = _regs_curr.hl.word;
                if(file)
                {
                    fseek(file, fcb->random_record_number * block_size, SEEK_SET);
                    if(feof(file))
                    {
                        _regs_curr.af.a = 0x01;
                    }
                    else
                    {
                        _regs_curr.af.a = 0x00;
                        for (size_t i = 0; i < (block_size * block_count); i++)
                        {
                            char tmp = mem_get_byte(ptr++);
                            if(fwrite(&tmp, 1, 1, file) <= 0)
                            {
                                _regs_curr.af.a = 0x01;
                                break;
                            }
                        }
                    }
                }
                else
                {
                    _regs_curr.af.a = 0xff;
                }
                _regs_curr.hl.l = _regs_curr.af.a;
                
            }
            break;
        case 0x28: // Random Write fill Zero fill FCB
            {
                abi_dos_fcb_t *fcb = (abi_dos_fcb_t*)&_memory[_regs_curr.de.word];
                FILE *file;
                memcpy(&file, fcb->internal, sizeof(size_t));
                ptr = _disk_transferr_address;
                if(file)
                {
                    fseek(file, fcb->random_record_number * 128, SEEK_SET);
                    if(feof(file))
                    {
                        _regs_curr.af.a = 0x01;
                    }
                    else
                    {
                        _regs_curr.af.a = 0x00;
                        for (size_t i = 0; i < 128; i++)
                        {
                            char tmp = 0;
                            if(fwrite(&tmp, 1, 1, file) <= 0)
                            {
                                _regs_curr.af.a = 0x01;
                                break;
                            }
                        }
                    }
                }
                else
                {
                    _regs_curr.af.a = 0xff;
                }
                _regs_curr.hl.l = _regs_curr.af.a;
                
            }
            break;
        case 0x2a: // Get Data
            {
                time_t t = time(NULL);
                struct tm *tm = localtime(&t);
                _regs_curr.hl.word = tm->tm_year + 1900;
                _regs_curr.de.d = tm->tm_mon + 1;
                _regs_curr.de.e = tm->tm_mday;
                _regs_curr.af.a = tm->tm_wday;
            }
            break;
        case 0x2b: // Set Data
            _regs_curr.af.a = 0xff;
            break;
        case 0x2c: // Get Time
            {
                time_t t = time(NULL);
                struct tm *tm = localtime(&t);
                _regs_curr.hl.h = tm->tm_hour;
                _regs_curr.hl.l = tm->tm_min;
                _regs_curr.de.d = tm->tm_sec;
                _regs_curr.de.e = 0;
            }
            break;
        case 0x2d: // Set Time
            _regs_curr.af.a = 0xff;
            break;
        case 0x2f: // Absolute Sector Read
            _regs_curr.af.a = 0xff;
            break;
        case 0x30: // Absolute Sector Write
            _regs_curr.af.a = 0xff;
            break;
        case 0x31: // Get Disk Parameters
            _regs_curr.af.a = 0xff;
            break;
        case 0x62: // Terminate with error code
            _executing = false;
            _return_code = _regs_curr.bc.b;
            break;
        case 0x6f: // Get MSX-DOS Version Number
            _regs_curr.af.a = 0;
            _regs_curr.bc.word = 0x0100;
            _regs_curr.de.word = 0x0000;
        default:
            fprintf(stderr, "\n\n[ERROR: NOT IMPLEMENTED COMMAND %i OF CALL 5 ABI]", _regs_curr.bc.c);
            exit(1);
            break;
    }

    _regs_curr.ip = mem_get_word(_regs_curr.sp);
    _regs_curr.sp += 2;
}