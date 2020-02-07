#include "stm32f10x.h"
#include "lcd.h"
#include "Hz.h"
#include "HzFont.h"

//extern struct typFNT_HZ32 codeHZ_32[];
//32X32����
void PutHZ3232(u16 x, u16 y, u8 c[2], u16 dcolor,u16 bgcolor)
{
	unsigned int i,j,k;
 
	//TFT_SetWindow(x,y,x+32-1, y+32-1);     //ѡ������λ��
	LCD_Set_Window(x,y,32,32);
	LCD_SetCursor(x,y);
	LCD_WriteRAM_Prepare();
	for (k=0;k<40;k++) { //15��ʾ�Խ����ֿ��еĸ�����ѭ����ѯ����
	  if ((GB32_Code[k].Index[0]==c[0])&&(GB32_Code[k].Index[1]==c[1])){ 
    	for(i=0;i<128;i++) {
		  unsigned short m=GB32_Code[k].Msk[i];
		  for(j=0;j<8;j++) {
			if((m&0x80)==0x80) {
				LCD_Fast_DrawPoint(x,y,dcolor);
				//LCD_WR_DATAX(dcolor);
				}
			else {
				LCD_Fast_DrawPoint(x,y,bgcolor);
			    //LCD_WR_DATAX(bgcolor);
				}
			m<<=1;
			}    
		  }
		}  
	}	
}
 
void show_Hz32(unsigned int x1,unsigned int y1,unsigned char *str,unsigned int dcolor,unsigned int bgcolor)	 
{  unsigned char l=0;
        while(*str)
           {
     	   PutHZ3232(x1+l*16,y1,(unsigned char*)str,dcolor, bgcolor);
			str+=2;l+=2;
      }
}


/*��ʾ16*16����*/
void Show_Hz16(unsigned int x,unsigned int y,unsigned char *p,unsigned int WordColor,unsigned int BackColor)
{
	unsigned int i=0,j=0,k=0;

	while(*p != '\0')//��ǰ�������ֲ�Ϊ��ʱ
	{
		LCD_Set_Window(x,y,16,16);//����16*16��С����
		LCD_SetCursor(x,y);//��������
		LCD_WriteRAM_Prepare();//��ʼдGRAM
		for(i=0;i<10;i++)//�����еĺ��ֽṹ�������в��ң�i���Ϊ�ṹ�������Ա�ĸ���
		{
			if((*p==GB16_Code[i].Index[0]) && (*(p+1)==GB16_Code[i].Index[1]))//�������ֳɹ�
			{ 
				for(j=0;j<32;j++)//д������
				{
					unsigned short word=GB16_Code[i].Msk[j];
					for(k=0;k<8;k++)//ѭ��8����λ
					{
						if((word&0x80)==0x80)
						{
							//LCD_Fast_DrawPoint(x,y,WordColor);
							LCD_WR_DATA(WordColor);//д��������ɫ
						}else {
							//LCD_Fast_DrawPoint(x,y,BackColor);
							LCD_WR_DATA(BackColor);//д�����屳��ɫ
						}
						word<<=1;//��ǰ��λ
					}		
				}
			}
		}
		p+=2;
		x+=16;
	}
}

extern const unsigned char asc2_1206[95][12];
extern const unsigned char asc2_2412[95][36];
extern const unsigned char asc2_1608[95][16];

void ShowString(u16 x,u16 y,u8 *str, u8 size,u16 wColor,u16 bColor)
{
	u8 num,t,temp,j;
	u8 csize=(size/8+((size%8)?1:0))*(size/2);//�õ������Ӧ���ֽ���
	while(*str)
	{
		LCD_Set_Window(x,y,16,16);//����16*16��С����
		LCD_SetCursor(x,y);//��������
		LCD_WriteRAM_Prepare();//��ʼдGRAM
		num=*str-' ';//ȡ����ǰ�ַ���λ��
		for(t=0;t<csize;t++)
		{
			if(size==12)temp=asc2_1206[num][t]; 	 	//����1206����
			else if(size==16)temp=asc2_1608[num][t];	//����1608����
			else if(size==24)temp=asc2_2412[num][t];	//����2412����
			else return;								//û�е��ֿ�		
			for(j=0;j<8;j++)//��ʼ����
			{
				if(temp&0x80)
				{
					LCD_WR_DATA(wColor);
				}
				else 
				{
					LCD_WR_DATA(bColor);
				}
				temp<<=1;
			}
		}
		str++;//ȡ��һ���ַ�
		if(size==12)x+=6; //λ�õ���
		else if(size==16)x+=8;
		else if(size==24)x+=12;
	}
}



