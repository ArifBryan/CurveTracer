#pragma once

#include <inttypes.h>

const uint8_t tempWarningBitmap [] = {
	0xff, 0xff, 0xff, 0x80, 0xff, 0xf3, 0xff, 0x80, 0xff, 0xe3, 0xff, 0x80, 0xff, 0xc9, 0xff, 0x80, 
	0xff, 0xc9, 0xff, 0x80, 0xff, 0x9c, 0xff, 0x80, 0xff, 0xbe, 0xff, 0x80, 0xff, 0x3e, 0x7f, 0x80, 
	0xfe, 0x7f, 0x3f, 0x80, 0xfe, 0x77, 0x3f, 0x80, 0xfc, 0xf7, 0x9f, 0x80, 0xfc, 0xf7, 0x9f, 0x80, 
	0xf9, 0xf7, 0xcf, 0x80, 0xf9, 0xf7, 0xcf, 0x80, 0xf3, 0xf7, 0xe7, 0x80, 0xe7, 0xf7, 0xf3, 0x80, 
	0xe7, 0xe3, 0xf3, 0x80, 0xcf, 0xe3, 0xf9, 0x80, 0xcf, 0xe3, 0xf9, 0x80, 0x9f, 0xf7, 0xfc, 0x80, 
	0x80, 0x00, 0x00, 0x80, 0x80, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00
};

const uint8_t pcBitmap[] = {
	0x80, 0x00, 0x00, 0x80, 0x3f, 0xff, 0xfe, 0x00, 0x7f, 0xff, 0xff, 0x00, 0x7f, 0xff, 0xff, 0x00, 
	0x7f, 0xff, 0xff, 0x00, 0x7f, 0xff, 0xff, 0x00, 0x7f, 0xff, 0xff, 0x00, 0x7f, 0xff, 0xff, 0x00, 
	0x7f, 0xff, 0xff, 0x00, 0x7f, 0xff, 0xff, 0x00, 0x7f, 0xff, 0xff, 0x00, 0x7f, 0xff, 0xff, 0x00, 
	0x7f, 0xff, 0xff, 0x00, 0x7f, 0xff, 0xff, 0x00, 0x3f, 0xff, 0xfe, 0x00, 0x80, 0x00, 0x00, 0x80, 
	0xff, 0xdd, 0xff, 0x80, 0xff, 0xdd, 0xff, 0x80, 0xff, 0xdd, 0xff, 0x80, 0xff, 0x00, 0x3f, 0x80
};