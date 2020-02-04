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


/*显示16*16汉字*/
void Show_Hz16(unsigned int x,unsigned int y,unsigned char *p,unsigned int WordColor,unsigned int BackColor)
{
	unsigned int i=0,j=0,k=0;

	while(*p != '\0')//当前索引汉字不为空时
	{
		LCD_Set_Window(x,y,16,16);//设置16*16大小窗口
		LCD_SetCursor(x,y);//设置坐标
		LCD_WriteRAM_Prepare();//开始写GRAM
		for(i=0;i<10;i++)//在所有的汉字结构体数组中查找，i最大为结构体数组成员的个数
		{
			if((*p==GB16_Code[i].Index[0]) && (*(p+1)==GB16_Code[i].Index[1]))//索引汉字成功
			{ 
				for(j=0;j<32;j++)//写入数据
				{
					unsigned short word=GB16_Code[i].Msk[j];
					for(k=0;k<8;k++)//循环8次移位
					{
						if((word&0x80)==0x80)
						{
							//LCD_Fast_DrawPoint(x,y,WordColor);
							LCD_WR_DATA(WordColor);//写入字体颜色
						}else {
							//LCD_Fast_DrawPoint(x,y,BackColor);
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

extern const unsigned char asc2_1206[95][12];
extern const unsigned char asc2_2412[95][36];
extern const unsigned char asc2_1608[95][16];

void ShowString(u16 x,u16 y,u8 *str, u8 size,u16 wColor,u16 bColor)
{
	u8 num,t,temp,j;
	u8 csize=(size/8+((size%8)?1:0))*(size/2);//得到字体对应的字节数
	while(*str)
	{
		LCD_Set_Window(x,y,16,16);//设置16*16大小窗口
		LCD_SetCursor(x,y);//设置坐标
		LCD_WriteRAM_Prepare();//开始写GRAM
		num=*str-' ';//取出当前字符的位置
		for(t=0;t<csize;t++)
		{
			if(size==12)temp=asc2_1206[num][t]; 	 	//调用1206字体
			else if(size==16)temp=asc2_1608[num][t];	//调用1608字体
			else if(size==24)temp=asc2_2412[num][t];	//调用2412字体
			else return;								//没有的字库		
			for(j=0;j<8;j++)//开始画点
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
		str++;//取下一个字符
		if(size==12)x+=6; //位置递增
		else if(size==16)x+=8;
		else if(size==24)x+=12;
	}
}



