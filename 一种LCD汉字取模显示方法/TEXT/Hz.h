#ifndef __HZ_H
#define __HZ_H

typedef struct 
{
	unsigned char Index[2];
	unsigned char Msk[128];
}typFNT_GB32;

//16*16×ÖÌåÏÔÊ¾
typedef struct 
{
	unsigned char Index[2];
	unsigned char Msk[32];
}typFNT_GB16;



void PutHZ3232(u16 x, u16 y, u8 c[2], u16 dcolor,u16 bgcolor);
void show_Hz32(unsigned int x1,unsigned int y1,unsigned char *str,unsigned int dcolor,unsigned int bgcolor);
void Show_Hz16(unsigned int x,unsigned int y,unsigned char *p,unsigned int WordColor,unsigned int BackColor);
#endif





