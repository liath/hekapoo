#!/usr/bin/env bash

# Run from powershell in the root of the repo with:
# nodemon -e "c,h,a51,sh" -x bash ./build.sh

KEIL_PATH="C:\Keil_v5"
# Idk if this is needed tbh but w/e
SILABS_PATH="C:\SiLabs\MCU\INC"

# http://www.keil.com/support/man/docs/c51/c51_cm_directives.htm
COMPILER_FLAGS="DEBUG OBJECTEXTEND BROWSE NOAREGS NOINTPROMOTE CODE SYMBOLS INCDIR(..\src;..\..\common\src;$SILABS_PATH)"
ASSEMBLER_FLAGS="NOMOD51 DEBUG XREF EP INCDIR(..\src;..\..\common\src;$SILABS_PATH)"
LINKER_FLAGS="PL (68) PW (78) IXREF RS (256) CODE (0x800-0X107F ) XDATA (0X80-0X800) STACK (?STACK (0x90))"

echo ">>> Cleaning up any old build artifacts"
find firmware -type f | grep -E '(OBJ|LST|M51|cyglink.txt|tmp.out)' | xargs rm

echo ">>> Compiling..."
cd firmware/hekapoo/src
printf "hekapoo.c\t\t" && cmd.exe /C "$KEIL_PATH\C51\BIN\C51.exe hekapoo.c $COMPILER_FLAGS" | tail -n+5 | sed -r 's|.*COMPLETE\.\s+||g'
printf "keyfob_startup.a51\t" && cmd.exe /C "$KEIL_PATH\C51\BIN\A51.EXE keyfob_startup.a51 $ASSEMBLER_FLAGS" | tail -n+5 | sed -r 's|.*COMPLETE\.\s+||g'

cd ../../common/src
printf "si4010_link.c\t\t" && cmd.exe /C "$KEIL_PATH\C51\BIN\C51.exe si4010_link.c $COMPILER_FLAGS" | tail -n+5 | sed -r 's|.*COMPLETE\.\s+||g'
printf "si4010_data.c\t\t" && cmd.exe /C "$KEIL_PATH\C51\BIN\C51.exe si4010_data.c $COMPILER_FLAGS" | tail -n+5 | sed -r 's|.*COMPLETE\.\s+||g'
printf "si4010_rom_keil.a51\t" && cmd.exe /C "$KEIL_PATH\C51\BIN\A51.EXE si4010_rom_keil.a51 $ASSEMBLER_FLAGS" | tail -n+5 | sed -r 's|.*COMPLETE\.\s+||g'
cd ../../../

echo && echo ">>> Linking..."
WIN_DIR=$(cmd.exe /C "echo %cd%" | sed -r "s|\x0d$||") # CRLF :|
cat << HERE > firmware/hekapoo/out/cyglink.txt
"$WIN_DIR\firmware\common\src\si4010_link.OBJ",
"$WIN_DIR\firmware\common\src\si4010_rom_keil.OBJ",
"$WIN_DIR\firmware\common\src\si4010_data.OBJ",
"$WIN_DIR\firmware\hekapoo\src\keyfob_startup.OBJ",
"$WIN_DIR\firmware\hekapoo\src\hekapoo.OBJ" TO "$WIN_DIR\firmware\hekapoo\out\hekapoo.omf" $LINKER_FLAGS
HERE
cmd.exe /C "$KEIL_PATH\C51\BIN\BL51.EXE @$WIN_DIR\firmware\hekapoo\out\cyglink.txt"
cmd.exe /C "$KEIL_PATH\C51\BIN\OH51.EXE $WIN_DIR\firmware\hekapoo\out\hekapoo.omf"
