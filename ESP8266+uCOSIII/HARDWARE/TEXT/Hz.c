#include "stm32f10x.h"
#include "lcd.h"
#include "Hz.h"
#include "HzFont.h"

//extern struct typFNT_HZ32 codeHZ_32[];
//32X32汉字
void PutHZ3232(u16 x, u16 y, u8 c[2], u16 dcolor,u16 bgcolor)
{
	unsigned int i,j,k;
 
	//TFT_SetWindow(x,y,x+32-1, y+32-1);     //选择坐标位置
	LCD_Set_Window(x,y,32,32);
	LCD_SetCursor(x,y);
	LCD_WriteRAM_Prepare();
	for (k=0;k<40;k++) { //15标示自建汉字库中的个数，循环查询内码
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


/*显示16*16汉字*/
void Show_Hz16(unsigned int x,unsigned int y,unsigned char *p,unsigned int WordColor,unsigned int BackColor)
{
	unsigned int i=0,j=0,k=0;
	//unsigned short word=0;

	while(*p != '\0')//当前索引汉字不为空时
	{
		LCD_Set_Window(x,y,16,16);//设置16*16大小窗口
		LCD_SetCursor(x,y);//设置坐标
		LCD_WriteRAM_Prepare();//开始写GRAM
		for(i=0;i<10 ;i++)//在所有的汉字结构体数组中查找，i最大为结构体数组成员的个数
		{
			if((*p==GB16_Code[i].Index[0]) && (*(p+1)==GB16_Code[i].Index[1]))//索引汉字成功
			{ //i=i+34;
				for(j=0;j<32;j++)//写入数据
				{
					unsigned short word=GB16_Code[i].Msk[j];
					for(k=0;k<8;k++)//循环8次移位
					{
						if((word&0x80)==0x80)
						{
							LCD_WR_DATA(WordColor);//写入字体颜色
						}else {
							LCD_WR_DATA(BackColor);//写入字体背景色
						}
						word<<=1;//往前移位
					}		
				}
			}
		}
		p+=2;
		x+=16;
	}
}



