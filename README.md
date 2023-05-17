# uf2conv
 Microsoft UF2 & BIN format convert tools

# example
 uf2conv firmware.bin [firmware.uf2]
 uf2conv -d firmware.uf2 [firmware.bin]

# options
 dump (-d)      Dump UF2 payload to binary;
 flags (-f)     Set UF2 block flags, default is 0x00000000;
 address (-a)   Set target address, default is 0x00000000;
 identify (-i)  Set family identify, default is 0x00000000;
 size (-s)      Set UF2 block payload size, default is 256 bytes;
 fixed (-F)     fixed UF2 block payload size if remaining valid data less then expected;
 help (-h)      Print usage massages;
