sh-elf-objcopy -R .stack -O binary gdC.elf gdC.bin
rm cd/1ST_READ.BIN
$KOS_BASE/utils/scramble/scramble gdC.bin 1ST_READ.BIN
cp 1ST_READ.BIN cd/
mkdcdisc  -n gbapspDC -N -f cd/gba_bios.bin -d cd/gbaDC -e gdC.elf -o gbapspDC.cdi 


