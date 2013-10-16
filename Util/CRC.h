#pragma once

unsigned short getCrc16(unsigned char* Source, int len);
unsigned int getCrc32(unsigned char* Source, int len);
unsigned int getChecksum(unsigned char* Source, int len);
