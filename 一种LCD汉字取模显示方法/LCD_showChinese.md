### 一种单片机LCD汉字取模显示(不需要字库)

最近在做一个小项目的时候需要用到LCD显示中文汉字，用到的字数不多，如果使用字库占的内存大，而且本人不太熟悉。上网看了一下别人的方法，移植了一下，觉得这个方法不错。后期还可以移植到OLED的显示上，直接可以像显示英文字符串的方法一样显示中文，非常方便：		

```c
		show_Hz32(60,160,"武汉加油",BLACK,WHITE);
		Show_Hz16(60,200,"武汉加油",BLACK,WHITE);
		LCD_ShowString(20,90,100,40,16,"China come on!");
```

下面以正点原子的mini开发板和2.8寸液晶LCD为例，记录一下这种显示方法的实现。

我们知道LCD上显示汉字一般都需要加上字库，因为编码方式与英文的方式不同，汉字或者说GBK各种国标码一般都是采用两个字节进行编码。暂且叫做汉字内码吧，详细的编码方式可以百度一下。

#### **关键点：**

**一个汉字占的空间是2个字节(Byte)，也就是说`"哈"`这样一个汉字占用两个字节的内存空间。**

我们封装一种结构体类型**typFNT_GB16**：

```c
typedef struct 
{
	unsigned char Index[2];
	unsigned short Msk[16];
}typFNT_GB16;
```

Index用作汉字占用的2Byte，后面我会说到为什么。Msk则是这个汉字取出的字模占用的空间。16*16的汉字就是32个字节：			
$$
16*2Byte=32Byte
$$
接下来是对汉字取模，使用下面这款软件的话：

​	LCD字体大小跟取模的关系是：

​														**LCD32x32字体对应软件24号字体(宋体)也是小一**

​														**LCD16x16字体对应软件12号字体(宋体)也是小四**

![image-20200202202448927](C:\Users\ACER\AppData\Roaming\Typora\typora-user-images\image-20200202202448927.png)

16x16字体取模设置如下：

![image-20200202202844943](C:\Users\ACER\AppData\Roaming\Typora\typora-user-images\image-20200202202844943.png)

![image-20200202203108900](C:\Users\ACER\AppData\Roaming\Typora\typora-user-images\image-20200202203108900.png)

32x32字体的设置类似，但是使用24号字体取出来回多出四个字节的数据，一般都是0，我们删除即可：

![image-20200202203340546](C:\Users\ACER\AppData\Roaming\Typora\typora-user-images\image-20200202203340546.png)



然后在字库头文件中，创建结构体**typFNT_GB16**类型的数组：**GB16_Code[]**，把我们刚刚取得的字模数据放入数组，同时在每个汉字的字模数据前面 加上一个`"x"`，这里的x就是对应的汉字，当然还有逗号。

```c
const typFNT_GB16 GB16_Code[] = 
{
/*--  文字:  武  --*/
/*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
"武",
0x00,0x40,0x00,0x50,0x3F,0x48,0x00,0x48,0x00,0x40,0xFF,0xFE,0x00,0x40,0x04,0x40,
0x04,0x40,0x27,0x40,0x24,0x20,0x24,0x22,0x24,0x12,0x27,0x8A,0xF8,0x06,0x40,0x02,

/*--  文字:  汉  --*/
/*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
"汉",
0x00,0x00,0x27,0xF8,0x12,0x08,0x12,0x08,0x82,0x08,0x41,0x10,0x49,0x10,0x09,0x10,
0x10,0xA0,0x10,0xA0,0xE0,0x40,0x20,0x40,0x20,0xA0,0x21,0x10,0x22,0x08,0x0C,0x06,
};
```

**这样的话，刚刚我们封装结构体类型中Index就会对应到我们的汉字的两个字节空间。**

然后我们就可以通过判断字符串中汉字占用的两个字节来判断是哪个汉字，并且把点阵数据写入LCD，最后就可以显示了。16x16汉字显示的函数如下，32x32的函数也是类似的：

```c
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
		for(i=0;i<2;i++)//在所有的汉字结构体数组中查找，i最大为结构体数组成员的个数
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
```



#### **总结：**

巧妙的地方就在汉字的2Byte的内码上，通过这个方式来判断汉字后再写入数据就ok了。

参考博客：

[]: https://blog.csdn.net/weilexuexi12/article/details/69890328
[]: https://blog.csdn.net/qq_40987215/article/details/89391441
[]: https://blog.csdn.net/nana1108/article/details/5645958

