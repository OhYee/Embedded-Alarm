#include <reg52.h>

// 定义所有管脚
sbit PORT_1_0 = P1 ^ 0;
sbit PORT_1_1 = P1 ^ 1;
sbit PORT_1_2 = P1 ^ 2;
sbit PORT_1_3 = P1 ^ 3;
sbit PORT_1_4 = P1 ^ 4;
sbit PORT_1_5 = P1 ^ 5;
sbit PORT_1_6 = P1 ^ 6;
sbit PORT_1_7 = P1 ^ 7;

// 数码管显示
unsigned char code table[] = {
    0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07,
    0x7f, 0x6f, 0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71
};

// 延时程序
void DelayMs(unsigned long ms) {
    unsigned int t;
    while (ms--)
        for (t = 0; t < 120; t++)
            ;
}

// 获取P1管脚指定位的值
bit getStatus(unsigned int num) {
    bit ret;
    switch (num) {
        case 0:
            ret = PORT_1_0;
            break;
        case 1:
            ret = PORT_1_1;
            break;
        case 2:
            ret = PORT_1_2;
            break;
        case 3:
            ret = PORT_1_3;
            break;
        case 4:
            ret = PORT_1_4;
            break;
        case 5:
            ret = PORT_1_5;
            break;
        case 6:
            ret = PORT_1_6;
            break;
        case 7:
            ret = PORT_1_7;
            break;
    }
    return ret;
}

// 设置P1管脚指定位的值
void setStatus(unsigned int num, bit value) {
    switch (num) {
    case 0:
        PORT_1_0 = value;
        break;
    case 1:
        PORT_1_1 = value;
        break;
    case 2:
        PORT_1_2 = value;
        break;
    case 3:
        PORT_1_3 = value;
        break;
    case 4:
        PORT_1_4 = value;
        break;
    case 5:
        PORT_1_5 = value;
        break;
    case 6:
        PORT_1_6 = value;
        break;
    case 7:
        PORT_1_7 = value;
        break;
    }
}

// 判断按键是否按下，按下后触发
int keyPressing[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
int checkKey(unsigned int num) {
    int keyDown = 0;
    if (getStatus(num) == 1 && keyPressing[num] == 1)
        keyPressing[num] = 0;
    if (getStatus(num) == 0 && keyPressing[num] == 0) {
        DelayMs(10);
        if (getStatus(num) == 0) {
            keyDown = 1;
            keyPressing[num] = 1;
        }
    }
    return keyDown;
}

// 串口通信：发送数据
void send(unsigned char sendData) {
    SBUF = sendData;
    while (!TI);
    TI = 0;
}

// 串口通信：接受数据
void reciveWorker(unsigned char);
void reciveListener() interrupt 4 {
    RI = 0;
    reciveWorker(SBUF);
}

/***************/
int blind = 1;	// 闪烁灯状态

// 设置数码管显示
void setSeg(int num, int idx){
		P0 = (num & 0x0f)|(idx<<4)|(blind<<7);
}

// 时间结构体定义
typedef struct{
		int hour,minute,second;
		int showHour, showMinute, showSecond;
} Time;

// 设定时间
void setTime(Time *time, int hour, int minute, int second) {
		time->hour = hour;
		time->minute = minute;
		time->second = second;
}

// 从另一个时间结构体设定时间
void setTimeFromTime(Time *time, Time *time2) {
		time->hour = time2->hour;
		time->minute = time2->minute;
		time->second = time2->second;
}

// 设定时间显示状态
void setTimeDisplay(Time *time, int hour, int minute, int second) {
		time->showHour = hour;
		time->showMinute = minute;
		time->showSecond = second;
}

//　判断时间是否相等
int equalTime(Time time1, Time time2) {
		if (time1.hour == time2.hour && time1.minute == time2.minute && time1.second == time2.second)
				return 1;
		else
				return 0;
}

// 小时自加
void addTimeHour(Time *time) {
		time->hour++;
		if(time->hour >= 24){
				time->hour = 0;
		}
}

// 分钟自加
void addTimeMinute(Time *time) {
		time->minute++;
		if(time->minute >= 60){
				time->minute = 0;
				addTimeHour(time);
		}
}

// 秒钟自加
void addTimeSecond(Time *time) {
		time->second++;
		if(time->second >= 60){
				time->second = 0;
				addTimeMinute(time);
		}
}

// 显示时间
void displayTime(Time time) {
		if(time.showHour)
			setSeg(time.hour/10,0);
		DelayMs(5);
		if(time.showHour)
				setSeg(time.hour%10,1);
		DelayMs(5);
		if(time.showMinute)
				setSeg(time.minute/10,2);
		DelayMs(5);
		if(time.showMinute)
				setSeg(time.minute%10,3);
		DelayMs(5);
		if(time.showSecond)
				setSeg(time.second/10,4);
		DelayMs(5);
		if(time.showSecond)
				setSeg(time.second%10,5);
		DelayMs(5);
}


Time nowTime, alarmTime;	// 当前时间和闹钟时间
Time *timePtr;				// 要显示的时间指针

// 将时间文本转换成整数
int time2Int(int idx){
		return (__TIME__[idx*3]-'0')*10+(__TIME__[idx*3+1]-'0');
}

void init(){
		int i;
	
    P0 = 0x00;
    SCON = 0x50;    // communication
    TMOD = 0x20;    // timer
    TH1 = 0xfd;
    TL1 = 0xfd;
    TR1 = 1;
    EA = 1;
    ES = 1;
	
	setTime(&nowTime,time2Int(0),time2Int(1),time2Int(2));
	setTimeDisplay(&nowTime,1,1,1);
	setTimeFromTime(&alarmTime,&nowTime);
	setTimeDisplay(&alarmTime,1,1,1);
	for(i=0;i<10;++i)
			addTimeSecond(&alarmTime);

	timePtr = &nowTime;
}

// solve the recive data
void reciveWorker(unsigned char reciveData) {
}



// 时间自加，每秒执行一次该函数
void timeInc(){
	if(equalTime(nowTime,alarmTime))
		send(1);
	addTimeSecond(&nowTime);
	blind = ~blind;
}


bit alarm = 0;
bit edit = 0;
int editStatus = 4;

// 时间显示状态更新
void timeDisplayUpdate(){
	if(edit) {
		setTimeDisplay(timePtr,editStatus&1,editStatus&2,editStatus&4);
	} else {
		setTimeDisplay(timePtr,1,1,1);
	}
}

// 按键按下
void key0click() {
	alarm=~alarm;
	if(alarm)
		timePtr = &alarmTime;
	else
		timePtr = &nowTime;
	timeDisplayUpdate();
	
	setStatus(7,~alarm);
}

void key1click(){
	edit=~edit;
	editStatus=4;
	timeDisplayUpdate();
	setStatus(6,~edit);
}

void key2click(){
	editStatus >>= 1;
	if(editStatus < 1)
		editStatus = 4;
	timeDisplayUpdate();
}

void key3click(){
	editStatus <<= 1;
	if(editStatus > 4)
		editStatus = 1;
	timeDisplayUpdate();
}

void key4click(){
	switch(editStatus){
		case 1:{
			addTimeHour(timePtr);
			break;
		}
		case 2:{
			addTimeMinute(timePtr);
			break;
		}
		case 4:{
			addTimeSecond(timePtr);
			break;
		}
	}
}

void key5click(){
	switch(editStatus){
		case 1:{
			timePtr->hour--;
			if(timePtr->hour <= -1)
				timePtr->hour = 23;
			break;
		}
		case 2:{
			timePtr->minute--;
			if(timePtr->minute <= -1)
					timePtr->minute = 59;
			break;
		}
		case 4:{
			timePtr->second--;
			if(timePtr->second <= -1)
					timePtr->second = 59;
			break;
		}
	}
}

void checkAllKey(){
	if(checkKey(0))
		key0click();
	if(checkKey(1))
		key1click();
	if(checkKey(2))
		key2click();
	if(checkKey(3))
		key3click();
	if(checkKey(4))
		key4click();
	if(checkKey(5))
		key5click();
}

int main() {
	int i;
    init();
    while(1) {
		DelayMs(10);
		timeInc();
		
		for(i=0;i<(990/30);++i){
			checkAllKey();
			displayTime(*timePtr);
		}
    }
    return 0;
}

