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
				LCD_WR_DATA(dcolor);
				}
			else {
			    LCD_WR_DATA(bgcolor);
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
	//unsigned short word=0;

	while(*p != '\0')//��ǰ�������ֲ�Ϊ��ʱ
	{
		LCD_Set_Window(x,y,16,16);//����16*16��С����
		LCD_SetCursor(x,y);//��������
		LCD_WriteRAM_Prepare();//��ʼдGRAM
		for(i=0;i<10 ;i++)//�����еĺ��ֽṹ�������в��ң�i���Ϊ�ṹ�������Ա�ĸ���
		{
			if((*p==GB16_Code[i].Index[0]) && (*(p+1)==GB16_Code[i].Index[1]))//�������ֳɹ�
			{ //i=i+34;
				for(j=0;j<32;j++)//д������
				{
					unsigned short word=GB16_Code[i].Msk[j];
					for(k=0;k<8;k++)//ѭ��8����λ
					{
						if((word&0x80)==0x80)
						{
							LCD_WR_DATA(WordColor);//д��������ɫ
						}else {
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



