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

void init() {
    P0 = 0x00;
    SCON = 0x50; // communication
    TMOD = 0x21; // timer

    TH1 = 0xfd;
    TL1 = 0xfd;

    TR1 = 1;
    TR0 = 0;

    EA = 1;
    ES = 1;

    IE = 0xF3;
}

bit ring = 0;
bit temp = 0;

// 音阶
unsigned int code Tone[] = {
    63628, 63835, 64021, 64103, 64260, 64400, 64524,
    64580, 64684, 64777, 64820, 64898, 64968, 65030,
    65058, 65110, 65157, 65178, 65217, 65252, 65283
};

// -1 -2 -3 -4 -5 -6 -7
// 0, 1, 2, 3, 4, 5, 6

// 1  2  3  4  5  6  7
// 7, 8, 9, 10,11,12,13

// +1 +2 +3 +4 +5 +6 +7
// 14,15,16,17,18,19,20

// 曲谱
int code melody[3][50] = {
    { -1,
        4, 7, 9, 9,
        11, 8, 9, 7, -1,
        7, 7, 6, 5, 5,
        6, 7, 8, 9, 4, -1,
        4, 7, 7, 8, 9,
        8, 8, 9, 10, 11, -1,
        9, 9, 12, 11, 11,
        4, 9, 8, 9, 7,
        -1, 0xFF },
    { -1,
        9, 12, 14, 13, 12, 16, 15, 13, 12, 14, 13, 11, 13, 9, 9, 9, 12, 14, 13,
        12, 16, 18, 18, 17, 15, 17, 16, 15, 8, 14, 12, //12,14,16,14,16,14,
        -1, 0xFF },
    { -1,
        7, 8, 9, 11, 9, 8, 7,
        7, 9, 12, 14, 13, 11, 9, 8,
        -1, 0xFF }
};

// 延时
unsigned int code tick[3][100] = {
    { 1,
        1, 1, 2, 4,
        2, 4, 2, 4, 2,
        2, 2, 1, 1, 2,
        2, 2, 1, 1, 5, 1,
        2, 2, 1, 1, 2,
        2, 2, 1, 1, 5, 1,
        2, 2, 1, 1, 2,
        2, 2, 1, 1, 4,
        6, 0xFF },
    { 1,
        1, 2, 1, 1, 2, 1, 3, 3, 1, 1, 1, 1, 2, 1, 3, 2, 1, 1, 1, 1,
        2, 1, 2, 1, 2, 1, 1, 1, 1, 2, 1, 4, //2,1,2,1,2,1,
        6, 0xFF },
    { 1,
        1, 1, 2, 4, 1, 1, 4,
        1, 1, 2, 1, 2, 1, 2, 4,
        6, 0xFF }
};

int songPos = 0;        // 歌曲播放位置
int musicIter = 0;      // 歌曲编号
sbit BEEP = P1 ^ 7;     //　连接发声器的管脚

// 开始播放音乐
void startMusic() {
    if (ring == 0)
        songPos = 0;
    ring = 1;
}

// 停止播放音乐
void stopMusic() {
    ring = 0;
    songPos = 0;
    setStatus(0, 1);
}

// 歌曲延时
void songDelay(unsigned int ms) {
    while (ms--)
        DelayMs(200);
}

// 中断1 定时器触发播放歌曲
void playMusic() interrupt 1 {
    if (melody[musicIter][songPos] != -1) {
        TH0 = Tone[melody[musicIter][songPos]] / 256;
        TL0 = Tone[melody[musicIter][songPos]] % 256;
        BEEP = ~BEEP;
    }
}

// 中断0 检测到按键
void keyClick() interrupt 0 {
    if (getStatus(1) == 0) {
        stopMusic();
    }
    if (getStatus(2) == 0) {
        startMusic();
    }
    if (getStatus(3) == 0) {
        while (~getStatus(3));  // 等待按键结束
        musicIter = (musicIter - 1 + 3) % 3;
        songPos = 0;
    }
    if (getStatus(4) == 0) {
        while (~getStatus(4));  // 等待案件结束
        musicIter = (musicIter + 1) % 3;
        songPos = 0;
    }
}

// 数据接受监听器
void reciveWorker(unsigned char reciveData) {
    if (reciveData == 1) {
        startMusic();
    }
}

// 音乐控制部分
void makeSong() {
    if (melody[musicIter][songPos] == 0xFF) {
        songPos = 0;
    }
    TR0 = 1;
    songDelay(tick[musicIter][songPos]);
    TR0 = 0;
    //BEEP = 0;
    songPos++;
}

// 主循环的工作
void doWork() {
    setStatus(0, ~ring);
    P0 = table[musicIter];
    if (ring) {
        makeSong();
    }
}

int main() {
    init();
    while (1) {
        //DelayMs(50);
        doWork();
    }
    return 0;
}
