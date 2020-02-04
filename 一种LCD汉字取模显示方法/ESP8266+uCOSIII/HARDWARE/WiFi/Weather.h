#ifndef __WEATHER_H
#define __WEATHER_H
#include "sys.h"

//������Ϣ�ṹ��
typedef struct 
{
	/*д�˸�BUG��ʹ��ָ��������⣬sprintf����*/
	char city[10];//����
	char text[30];//����
	char code;//�����������
	char temp;//�¶�
	char humity;//ʪ��
	
}WeatherData;

/*��������33�֣�����ֻ�г��ˣ�code:0~20��21�֣��������£�
* https://docs.seniverse.com/api/start/code.html
*
*/
typedef enum
{
	Sunny = 0,//�磨���ڳ��а����磩
	Clear,//�磨���ڳ���ҹ���磩
	Fair,//�磨������а����磩
	Fair1,//�磨�������ҹ���磩
	Cloundy,//����
	Partly_Cloudy,//������
	Partly_Cloudy1,//������
	Mostly_Cloudy,//�󲿶���
	Mostly_Cloudy1,//�󲿶���
	Overcast,//��
	Shower,//����
	Thundershower,//������
	Thundershower_with_Hail,//��������б���
	Light_Rain,//С��
	Moderate_Rain,//����
	Heavy_Rain,//����
	Storm,//����
	Heavy_Storm,//����
	Severe_Storm,//�ش���
	Ice_Rain,//����
	Sleet//���ѩ  20
}Phenomenon;


#endif

