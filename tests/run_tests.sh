#!/bin/bash
# HC SDK Test Runner
# Usage: ./run_tests.sh [z80|8080|8085|8086|all]
# Each test is a .b or .s file with an optional .expect file

set -e
cd "$(dirname "$0")"

SDK="../bin"
LIBS="../libs"
TARGET="${1:-all}"
PASS=0
FAIL=0

# Emulators
EMU_Z80="$SDK/msxdosemu"
EMU_86="/usr/local/bin/emu2"
[ -x "$EMU_86" ] || EMU_86=""

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'

run_test() {
    local name="$1" kind="$2" source="$3" expect="$4"
    local ok=0
    local base="/tmp/hctest_$$_$(echo "$name" | tr '/ ()' '____')"
    
    case "$kind" in
        asm_z80)
            $SDK/hcasm-z80 -o /dev/null "$source" 2>/dev/null && ok=1
            ;;
        asm_8080)
            $SDK/hcasm-8080 -o /dev/null "$source" 2>/dev/null && ok=1
            ;;
        asm_8085)
            $SDK/hcasm-8085 -o /dev/null "$source" 2>/dev/null && ok=1
            ;;
        asm_8086)
            $SDK/hcasm-8086 -o /dev/null "$source" 2>/dev/null && ok=1
            ;;
        b_z80)
            $SDK/hcbcomp-z80 -o "$base.s" "$source" 2>/dev/null && \
            $SDK/hcasm-z80 -o "$base.obj" "$base.s" 2>/dev/null && \
            $SDK/hclink-bin -text 0x100 -o "$base.com" "$base.obj" "$LIBS/z80-cpm-b.lib" 2>/dev/null && \
            ok=1
            if [ $ok -eq 1 ] && [ -n "$expect" ]; then
                local out=$(echo "" | $EMU_Z80 "$base.com" 2>&1 | tr -d '\r')
                [ "$out" = "$(cat "$expect")" ] || ok=0
            fi
            rm -f "$base".* 2>/dev/null
            ;;
        b_8080)
            $SDK/hcbcomp-8080 -o "$base.s" "$source" 2>/dev/null && \
            $SDK/hcasm-8080 -o "$base.obj" "$base.s" 2>/dev/null && \
            $SDK/hclink-bin -text 0x100 -o "$base.com" "$base.obj" "$LIBS/8080-cpm-b.lib" 2>/dev/null && \
            ok=1
            if [ $ok -eq 1 ] && [ -n "$expect" ]; then
                local out=$(echo "" | $EMU_Z80 "$base.com" 2>&1 | tr -d '\r')
                [ "$out" = "$(cat "$expect")" ] || ok=0
            fi
            rm -f "$base".* 2>/dev/null
            ;;
        b_8086)
            $SDK/hcbcomp-8086 -o "$base.s" "$source" 2>/dev/null && \
            $SDK/hcasm-8086 -o "$base.obj" "$base.s" 2>/dev/null && \
            $SDK/hclink-bin -text 0x100 -o "$base.com" "$base.obj" "$LIBS/8086-msdos-b.lib" 2>/dev/null && \
            ok=1
            if [ $ok -eq 1 ] && [ -n "$expect" ] && [ -n "$EMU_86" ]; then
                local out=$($EMU_86 "$base.com" 2>&1 | tr -d '\r\n')
                [ "$out" = "$(tr -d '\n' < "$expect")" ] || ok=0
            fi
            rm -f "$base".* 2>/dev/null
            ;;
        b_8086exe)
            $SDK/hcbcomp-8086exe -o "$base.s" "$source" 2>/dev/null && \
            $SDK/hcasm-8086 -o "$base.obj" "$base.s" 2>/dev/null && \
            $SDK/hclink-mz -stack 1024 -o "$base.exe" "$base.obj" "$LIBS/8086-msdos-exe-b.lib" 2>/dev/null && \
            ok=1
            if [ $ok -eq 1 ] && [ -n "$expect" ] && [ -n "$EMU_86" ]; then
                local out=$($EMU_86 "$base.exe" 2>&1 | tr -d '\r\n')
                [ "$out" = "$(tr -d '\n' < "$expect")" ] || ok=0
            fi
            rm -f "$base".* 2>/dev/null
            ;;
    esac
    
    if [ $ok -eq 1 ]; then
        echo -e "  ${GREEN}PASS${NC} $name"
        PASS=$((PASS + 1))
    else
        echo -e "  ${RED}FAIL${NC} $name"
        FAIL=$((FAIL + 1))
    fi
}

# ── Assembler tests ─────────────────────────────────────────────

do_asm() {
    local arch="$1" kind="$2"
    [ "$TARGET" != "all" ] && [ "$TARGET" != "$arch" ] && return
    echo ""
    echo "── Assembler $arch ──"
    for f in asm/$arch/*.s; do
        [ -f "$f" ] || continue
        local name=$(basename "$f" .s)
        run_test "asm/$arch/$name" "$kind" "$f" ""
    done
}

do_asm "z80"   "asm_z80"
do_asm "8080"  "asm_8080"
do_asm "8085"  "asm_8085"
do_asm "8086"  "asm_8086"

# ── B language tests ────────────────────────────────────────────

do_b() {
    local arch="$1" kind="$2"
    [ "$TARGET" != "all" ] && [ "$TARGET" != "$arch" ] && return
    echo ""
    echo "── B language ($arch) ──"
    for f in b/*.b; do
        [ -f "$f" ] || continue
        local name=$(basename "$f" .b)
        local expect="b/$name.expect"
        [ -f "$expect" ] || expect=""
        run_test "b/$name ($arch)" "$kind" "$f" "$expect"
    done
}

do_b "z80"     "b_z80"
do_b "8080"    "b_8080"
do_b "8086"    "b_8086"
do_b "8086exe" "b_8086exe"

echo ""
echo "=========================================="
echo -e "  ${GREEN}Passed: $PASS${NC}  ${RED}Failed: $FAIL${NC}"
echo "=========================================="
[ $FAIL -eq 0 ] || exit 1
