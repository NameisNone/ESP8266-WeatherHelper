#ifndef __WEATHER_H
#define __WEATHER_H
#include "sys.h"

//天气信息结构体
typedef struct 
{
	/*指针需要分配内存，否则sprintf会死机*/
	unsigned char *city;//城市city[10]
	unsigned char *text;//现象text[30]
	char code;//天气现象代码
	char temp;//温度
	
}WeatherData;

/*天气现象共33种，这里只列出了（code:0~20）21种，详情如下：
* https://docs.seniverse.com/api/start/code.html
*
*/
/*
typedef enum
{
	Sunny = 0,//晴（国内城市白天晴）
	Clear,//晴（国内城市夜晚晴）
	Fair,//晴（国外城市白天晴）
	Fair1,//晴（国外城市夜晚晴）
	Cloundy,//多云
	Partly_Cloudy,//晴间多云
	Partly_Cloudy1,//晴间多云
	Mostly_Cloudy,//大部多云
	Mostly_Cloudy1,//大部多云
	Overcast,//阴
	Shower,//阵雨
	Thundershower,//雷阵雨
	Thundershower_with_Hail,//雷阵雨伴有冰雹
	Light_Rain,//小雨
	Moderate_Rain,//中雨
	Heavy_Rain,//大雨
	Storm,//暴雨
	Heavy_Storm,//大暴雨
	Severe_Storm,//特大暴雨
	Ice_Rain,//冻雨
	Sleet//雨夹雪  20
}Phenomenon;
*/

#endif

