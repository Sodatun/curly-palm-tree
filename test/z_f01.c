
#include "C8051F020.h"
#include "absacc.h"
#include "data_define.c"
#include "Init_Device.c"

#define C11 XBYTE[0x0000] //LED1第一个数码管
#define C12 XBYTE[0x0001] //LED2第二个数码管
#define C13 XBYTE[0x0002] //LED3第三个数码管
#define C14 XBYTE[0x0003] //LED4第四个数码管

#define C3 XBYTE[0x4000] //D/A转换器  DAC
#define C2 XBYTE[0x2000] //A/D转换器  CS1

static unsigned int DataT = 0;   //16进制的A/D数据
static int DataTO = 0;           //10进制的温度数据
static unsigned int DataTI = 0;  //10进制的目标温度（临时）
static unsigned int TargetT = 0; //10进制的目标温度
static char tp = 0;              //目标温度和当前温度的差值
static unsigned int sign = 0;    //表示tp的正负号
static unsigned char Switch = 0; //开关键是否按下的标志
static unsigned int Input = 0;   //输入

unsigned char num[] = {0xC0, 0xF9, 0xA4, 0xB0, 0x99,
                       0x92, 0x82, 0xF8, 0x80, 0x90}; //数码管码表
unsigned char key_table[4][5] = {{0, 1, 2, 3, 20},
                                 {4, 5, 6, 7, 21},
                                 {8, 9, 10, 11, 22},
                                 {12, 13, 14, 15, 23}}; //按键码表

void delay(void);
void delay1(void);
void delay2(unsigned int x);
void Display(const unsigned int, const unsigned int);
unsigned int Convert(const unsigned int);
unsigned int ReadT(void);
unsigned int ReadKey(void);
unsigned int ReadKeyS(void);
// unsigned int Decode(unsigned int);
unsigned int ReadInput(void);
void Task(void);
void Movinglight(void);

int main(void)
{
    Init_Device();

    while (1)
    {
        ReadInput(); //读取键盘
        Task();      //对温度进行调整（主任务）
    }
    return 0;
}

unsigned int Convert(const unsigned int a)
{

    return (int)99 * a / 0xff; //返回值=输入数据*95/255 （取整数） 返回值范围（0~95）
}

unsigned int ReadT(void)
{
    C2 = 0;    //向A/D转换器发出指令
    delay();   //等待采样完成
    return C2; //返回采样数据数据0-255
}

void Display(const unsigned int x, const unsigned int z)
{
		unsigned int i,j;
		i=z / 10;
		if(z==0)
		{
			C14 = 0xff;
		}
		else
		{
    C14 = num[i]; //第4个数码管，显示第二个数字的个位
		}
    C13 = num[z % 10]; //第3个数码管，显示第二个数字的十位
				j=x / 10;
		if(j==0)
		{
			C12 = 0xff;
		}
		else
		{
    C12 = num[j]; //第2个数码管，显示第一个数字的个位
		}
    C11 = num[x % 10]; //第1个数码管，显示第一个数字的十位
		delay2(20);
}

void delay(void) //延时子程序
{
    long int i;
    for (i = 0; i < 0x1f0; ++i)
        ;
}
void delay1(void)
{
    long int i;
    for (i = 0; i < 0x81f0; ++i)
        ;
}

void delay2(unsigned int x)
{
    unsigned long i;
    for (i = 0; i < 1000 * x; ++i)
        // i = i;
        ;
}

unsigned int ReadKeyS(void)
{
    unsigned char x, y, n;
    unsigned char i, j, a, temp, key_val;
    n = 1;
    x = 10;
    y = 10;

    for (i = 4; i < 8; i++)
    {
        a = XBYTE[i];
        a = ~a & 0x1f;
        for (j = 0, temp = 1; j < 5; j++)
        {
            if (a & temp)
            {
                x = i - 4;
                y = j;
                //LED4=num[j+1];
                //LED3=num[i-3];
            }
            temp = temp << 1;
        }
    }
    if (x < 4 && y < 5)
    {
        key_val = key_table[x][y];
        if (key_val == 12) //按键C被按下
        {
            // DAC = 128;
            // delay(100);
            return 12;
        }
        if (key_val == 15) //按键F被按下
        {
            return 15;
        }
    }

    return 255; //无按键按下
}

unsigned int ReadKey(void)
{
    unsigned char x, y, n;
    unsigned char i, j, a, temp, key_val;
    n = 1;
    x = 10;
    y = 10;

    for (i = 4; i < 8; i++)
    {
        a = XBYTE[i];
        a = ~a & 0x1f;
        for (j = 0, temp = 1; j < 5; j++)
        {
            if (a & temp)
            {
                x = i - 4;
                y = j;
                // LED2=num[x];
                // LED1=num[y];
                // delay(200);
            }
            temp = temp << 1;
        }
    }

    if (x < 4 && y < 5)
    {
        // LED4=0xff;
        key_val = key_table[x][y];
        if (key_val < 10) //确保按下的为数字键
        {
            return (key_val);
        }
        else
        {
            return 255; //对按下的非数字键做无输入处理
        }
    }

    return 255; //无按键按下

    //LED4=num[key_table[x][y]];

    //if (n==2)
    //  LED3=num[key_table[x][y]];
}

// unsigned int Decode(unsigned int a) //对键值解码
// {
//     return 0; //返回键值
// }

unsigned int ReadInput(void)
{
    unsigned int key = 0;
    unsigned int i = 0;
    // unsigned int temp = 0;
    key = ReadKeyS(); //读取开关键是否按下  Kx
    // i = Decode(ReadKeyS()); //对读取的键值解码    0-9
    i = key; //对读取的键值解码    0-9

    if (i != 12) //如果不等于开关键（12为开关键C对应的值），则不读数据，退出
    {
        return 0xff;
    }
    Movinglight();
    Display(0, DataTO); //数码管的输入区清零
    //delay1();
    while (1)
    {
        key = ReadKey(); //读取第一个键值（十位）
        i = key;         //解码
        if (i < 10)      //如果是0-9对应的键值，则更新DataTI（临时的输入数据）
        {
            DataTI = i;              //更新DataTI
            Display(DataTI, DataTO); //更新数码管，显示输入的数字
            break;
        }
        //Task(); //和main()的那个Task()相同，目的是防止输入数据的时候单片机失去控温能力
    }
    delay();
    while (1)
    {
        key = ReadKey(); //读取键值
        if (key == 255)  //等键盘松开
        {
            break;
        }

        Task();
    }

    while (1)
    {
        key = ReadKey(); //读取第二个键值（个位）
        i = key;         //解码
        if (i < 10)      //如果是0-9对应的键值，则更新DataTI（临时的输入数据）
        {
            DataTI = DataTI * 10 + i; //更新DateTI数据
            Display(DataTI, DataTO);  //更新数码管，显示输入的数字
            TargetT = DataTI;         //更新目标温度（正式）
            break;
        }
        Task();
    }

    while (1)
    {
        key = ReadKeyS(); //读取开关键是否按下  Kx
        // i = Decode(ReadKeyS()); //对读取的键值解码    0-9
        i = key;     //对读取的键值解码    0-9
        if (i == 15) //如果等于开关键（15为开关键F对应的值），则执行
        {
            break;
        }
       // Task();
    }

    return DataTI; //返回输入的数据
}
void Task(void)
{

    DataT = ReadT();         //读取A/D的数据
    DataTO = Convert(DataT); //把A/D数据转化为10进制的温度数据
    if (DataTI > 95)         //如果温度大于95度，就修正为96度（95度时温度变送器为5V）
    {
        DataTI = 95;
    }
    tp = TargetT - DataTO; //目标温度和目标温度的差值
    if (tp < 0)            //如果为负
    {
        tp = ~tp + 1; //求差值绝对值
        sign = 1;     //符号位标记为1（1为负数，0为正数）
    }
    else
    {
        sign = 0; //符号位标记为0（1为负数，0为正数）
    }
    //分三段控制，0~30度，31~70度,71~95度
    if (DataTO < 31) //第一段
    {
        while (1)
        {

            if (tp < 1) //如果差值为0，则令驱动器空载（0V)
            {
                C3 = 128;
                break;
            }
            if (tp < 4 && sign == 0) //如果差值小于4且为正数，令驱动器加热（但不是满载）
            {
                C3 = 170;
                break;
            }

            if (tp < 4 && sign == 1) //如果差值小于4且为负数，令驱动器冷却（但不是满载）
            {
                C3 = 30;
                break;
            }

            if (sign == 1) //如果差值大于4且为负数，令驱动器冷却（满载）
            {
                C3 = 0;
                break;
            }
            if (sign == 0) //如果差值大于4且为正数，令驱动器加热（满载）
            {
                C3 = 255;
                break;
            }

            break;
        }
    }
    if (DataTO < 71 && DataTO > 30) //第二段
    {
        while (1)
        {

            if (tp < 1) //如果差值为0，则令驱动器空载（0V)
            {
                C3 = 128;
                break;
            }
            if (tp < 4 && sign == 0) //如果差值小于4且为正数，令驱动器加热（但不是满载）
            {
                C3 = 220;
                break;
            }

            if (tp < 4 && sign == 1) //如果差值小于4且为负数，令驱动器冷却（但不是满载）
            {
                C3 = 75;
                break;
            }

            if (sign == 1) //如果差值大于4且为负数，令驱动器冷却（满载）
            {
                C3 = 0;
                break;
            }
            if (sign == 0) //如果差值大于4且为正数，令驱动器加热（满载）
            {
                C3 = 255;
                break;
            }

            break;
        }
    }
    if (DataTO < 96 && DataTO > 70) //第三段
    {
        while (1)
        {

            if (tp < 1) //如果差值为0，则令驱动器空载（0V)
            {
                C3 = 128;
                break;
            }
            if (tp < 4 && sign == 0) //如果差值小于4且为正数，令驱动器加热（但不是满载）
            {
                C3 = 255;
                break;
            }

            if (tp < 4 && sign == 1) //如果差值小于4且为负数，令驱动器冷却（但不是满载）
            {
                C3 = 100;
                break;
            }

            if (sign == 1) //如果差值大于4且为负数，令驱动器冷却（满载）
            {
                C3 = 0;
                break;
            }
            if (sign == 0) //如果差值大于4且为正数，令驱动器加热（满载）
            {
                C3 = 255;
                break;
            }

            break;
        }
    }

    Display(DataTI, DataTO); //更新数码管显示
    return;
}

void Movinglight(void)
{
		unsigned char i;
    for (i = 0; i < 8; i++)
    {
        switch (i)
        {
        case 0:
            C11 = 0xfe;
            C12 = 0xfe;
           // C13 = 0xfe;
           // C14 = 0xfe;
            delay2(10); //默认为50（1s）
            break;

        case 1:
            C11 = 0xfd;
            C12 = 0xfd;
           // C13 = 0xfd;
           // C14 = 0xfd;
            delay2(10); //默认为50（1s）
            break;

        case 2:
            C11 = 0xfb;
            C12 = 0xfb;
           // C13 = 0xfb;
           // C14 = 0xfb;
            delay2(10); //默认为50（1s）
            break;

        case 3:
            C11 = 0xf7;
            C12 = 0xf7;
           // C13 = 0xf7;
           // C14 = 0xf7;
            delay2(10); //默认为50（1s）
            break;

        case 4:
            C11 = 0xef;
            C12 = 0xef;
           // C13 = 0xef;
            //C14 = 0xef;
            delay2(10); //默认为50（1s）
            break;

        case 5:
            C11 = 0xdf;
            C12 = 0xdf;
           // C13 = 0xdf;
           // C14 = 0xdf;
            delay2(10); //默认为50（1s）
            break;

        case 6:
            C11 = 0xbf;
            C12 = 0xbf;
          //  C13 = 0xbf;
          //  C14 = 0xbf;
            delay2(10); //默认为50（1s）
            break;

        case 7:
            C11 = 0x7f;
            C12 = 0x7f;
          //  C13 = 0x7f;
            //C14 = 0x7f;
            delay2(10); //默认为50（1s）
            break;
        }
    }
    return;
}