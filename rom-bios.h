// Common BIOS code between PC/XT/AT and ELKS targets


// BIOS main data area (BDA)
// Starts @ 40:0h
// Usually 256 bytes long

#define BDA_VIDEO_MODE 0x0449


// Generic BIOS

void rom_init_0 (void);
void rom_term_0 (void);
