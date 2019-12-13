
#include "C8051F020.h"
#include "absacc.h"
#include "data_define.c"
#include "Init_Device.c"

#define C11 XBYTE[0x0000] //LED1��һ�������
#define C12 XBYTE[0x0001] //LED2�ڶ��������
#define C13 XBYTE[0x0002] //LED3�����������
#define C14 XBYTE[0x0003] //LED4���ĸ������

#define C3 XBYTE[0x4000] //D/Aת����  DAC
#define C2 XBYTE[0x2000] //A/Dת����  CS1

static unsigned int DataT = 0;   //16���Ƶ�A/D����
static int DataTO = 0;           //10���Ƶ��¶�����
static unsigned int DataTI = 0;  //10���Ƶ�Ŀ���¶ȣ���ʱ��
static unsigned int TargetT = 0; //10���Ƶ�Ŀ���¶�
static char tp = 0;              //Ŀ���¶Ⱥ͵�ǰ�¶ȵĲ�ֵ
static unsigned int sign = 0;    //��ʾtp��������
static unsigned char Switch = 0; //���ؼ��Ƿ��µı�־
static unsigned int Input = 0;   //����

unsigned char num[] = {0xC0, 0xF9, 0xA4, 0xB0, 0x99,
                       0x92, 0x82, 0xF8, 0x80, 0x90}; //��������
unsigned char key_table[4][5] = {{0, 1, 2, 3, 20},
                                 {4, 5, 6, 7, 21},
                                 {8, 9, 10, 11, 22},
                                 {12, 13, 14, 15, 23}}; //�������

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
        ReadInput(); //��ȡ����
        Task();      //���¶Ƚ��е�����������
    }
    return 0;
}

unsigned int Convert(const unsigned int a)
{

    return (int)99 * a / 0xff; //����ֵ=��������*95/255 ��ȡ������ ����ֵ��Χ��0~95��
}

unsigned int ReadT(void)
{
    C2 = 0;    //��A/Dת��������ָ��
    delay();   //�ȴ��������
    return C2; //���ز�����������0-255
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
    C14 = num[i]; //��4������ܣ���ʾ�ڶ������ֵĸ�λ
		}
    C13 = num[z % 10]; //��3������ܣ���ʾ�ڶ������ֵ�ʮλ
				j=x / 10;
		if(j==0)
		{
			C12 = 0xff;
		}
		else
		{
    C12 = num[j]; //��2������ܣ���ʾ��һ�����ֵĸ�λ
		}
    C11 = num[x % 10]; //��1������ܣ���ʾ��һ�����ֵ�ʮλ
		delay2(20);
}

void delay(void) //��ʱ�ӳ���
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
        if (key_val == 12) //����C������
        {
            // DAC = 128;
            // delay(100);
            return 12;
        }
        if (key_val == 15) //����F������
        {
            return 15;
        }
    }

    return 255; //�ް�������
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
        if (key_val < 10) //ȷ�����µ�Ϊ���ּ�
        {
            return (key_val);
        }
        else
        {
            return 255; //�԰��µķ����ּ��������봦��
        }
    }

    return 255; //�ް�������

    //LED4=num[key_table[x][y]];

    //if (n==2)
    //  LED3=num[key_table[x][y]];
}

// unsigned int Decode(unsigned int a) //�Լ�ֵ����
// {
//     return 0; //���ؼ�ֵ
// }

unsigned int ReadInput(void)
{
    unsigned int key = 0;
    unsigned int i = 0;
    // unsigned int temp = 0;
    key = ReadKeyS(); //��ȡ���ؼ��Ƿ���  Kx
    // i = Decode(ReadKeyS()); //�Զ�ȡ�ļ�ֵ����    0-9
    i = key; //�Զ�ȡ�ļ�ֵ����    0-9

    if (i != 12) //��������ڿ��ؼ���12Ϊ���ؼ�C��Ӧ��ֵ�����򲻶����ݣ��˳�
    {
        return 0xff;
    }
    Movinglight();
    Display(0, DataTO); //����ܵ�����������
    //delay1();
    while (1)
    {
        key = ReadKey(); //��ȡ��һ����ֵ��ʮλ��
        i = key;         //����
        if (i < 10)      //�����0-9��Ӧ�ļ�ֵ�������DataTI����ʱ���������ݣ�
        {
            DataTI = i;              //����DataTI
            Display(DataTI, DataTO); //��������ܣ���ʾ���������
            break;
        }
        //Task(); //��main()���Ǹ�Task()��ͬ��Ŀ���Ƿ�ֹ�������ݵ�ʱ��Ƭ��ʧȥ��������
    }
    delay();
    while (1)
    {
        key = ReadKey(); //��ȡ��ֵ
        if (key == 255)  //�ȼ����ɿ�
        {
            break;
        }

        Task();
    }

    while (1)
    {
        key = ReadKey(); //��ȡ�ڶ�����ֵ����λ��
        i = key;         //����
        if (i < 10)      //�����0-9��Ӧ�ļ�ֵ�������DataTI����ʱ���������ݣ�
        {
            DataTI = DataTI * 10 + i; //����DateTI����
            Display(DataTI, DataTO);  //��������ܣ���ʾ���������
            TargetT = DataTI;         //����Ŀ���¶ȣ���ʽ��
            break;
        }
        Task();
    }

    while (1)
    {
        key = ReadKeyS(); //��ȡ���ؼ��Ƿ���  Kx
        // i = Decode(ReadKeyS()); //�Զ�ȡ�ļ�ֵ����    0-9
        i = key;     //�Զ�ȡ�ļ�ֵ����    0-9
        if (i == 15) //������ڿ��ؼ���15Ϊ���ؼ�F��Ӧ��ֵ������ִ��
        {
            break;
        }
       // Task();
    }

    return DataTI; //�������������
}
void Task(void)
{

    DataT = ReadT();         //��ȡA/D������
    DataTO = Convert(DataT); //��A/D����ת��Ϊ10���Ƶ��¶�����
    if (DataTI > 95)         //����¶ȴ���95�ȣ�������Ϊ96�ȣ�95��ʱ�¶ȱ�����Ϊ5V��
    {
        DataTI = 95;
    }
    tp = TargetT - DataTO; //Ŀ���¶Ⱥ�Ŀ���¶ȵĲ�ֵ
    if (tp < 0)            //���Ϊ��
    {
        tp = ~tp + 1; //���ֵ����ֵ
        sign = 1;     //����λ���Ϊ1��1Ϊ������0Ϊ������
    }
    else
    {
        sign = 0; //����λ���Ϊ0��1Ϊ������0Ϊ������
    }
    //�����ο��ƣ�0~30�ȣ�31~70��,71~95��
    if (DataTO < 31) //��һ��
    {
        while (1)
        {

            if (tp < 1) //�����ֵΪ0���������������أ�0V)
            {
                C3 = 128;
                break;
            }
            if (tp < 4 && sign == 0) //�����ֵС��4��Ϊ�����������������ȣ����������أ�
            {
                C3 = 170;
                break;
            }

            if (tp < 4 && sign == 1) //�����ֵС��4��Ϊ����������������ȴ�����������أ�
            {
                C3 = 30;
                break;
            }

            if (sign == 1) //�����ֵ����4��Ϊ����������������ȴ�����أ�
            {
                C3 = 0;
                break;
            }
            if (sign == 0) //�����ֵ����4��Ϊ�����������������ȣ����أ�
            {
                C3 = 255;
                break;
            }

            break;
        }
    }
    if (DataTO < 71 && DataTO > 30) //�ڶ���
    {
        while (1)
        {

            if (tp < 1) //�����ֵΪ0���������������أ�0V)
            {
                C3 = 128;
                break;
            }
            if (tp < 4 && sign == 0) //�����ֵС��4��Ϊ�����������������ȣ����������أ�
            {
                C3 = 220;
                break;
            }

            if (tp < 4 && sign == 1) //�����ֵС��4��Ϊ����������������ȴ�����������أ�
            {
                C3 = 75;
                break;
            }

            if (sign == 1) //�����ֵ����4��Ϊ����������������ȴ�����أ�
            {
                C3 = 0;
                break;
            }
            if (sign == 0) //�����ֵ����4��Ϊ�����������������ȣ����أ�
            {
                C3 = 255;
                break;
            }

            break;
        }
    }
    if (DataTO < 96 && DataTO > 70) //������
    {
        while (1)
        {

            if (tp < 1) //�����ֵΪ0���������������أ�0V)
            {
                C3 = 128;
                break;
            }
            if (tp < 4 && sign == 0) //�����ֵС��4��Ϊ�����������������ȣ����������أ�
            {
                C3 = 255;
                break;
            }

            if (tp < 4 && sign == 1) //�����ֵС��4��Ϊ����������������ȴ�����������أ�
            {
                C3 = 100;
                break;
            }

            if (sign == 1) //�����ֵ����4��Ϊ����������������ȴ�����أ�
            {
                C3 = 0;
                break;
            }
            if (sign == 0) //�����ֵ����4��Ϊ�����������������ȣ����أ�
            {
                C3 = 255;
                break;
            }

            break;
        }
    }

    Display(DataTI, DataTO); //�����������ʾ
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
            delay2(10); //Ĭ��Ϊ50��1s��
            break;

        case 1:
            C11 = 0xfd;
            C12 = 0xfd;
           // C13 = 0xfd;
           // C14 = 0xfd;
            delay2(10); //Ĭ��Ϊ50��1s��
            break;

        case 2:
            C11 = 0xfb;
            C12 = 0xfb;
           // C13 = 0xfb;
           // C14 = 0xfb;
            delay2(10); //Ĭ��Ϊ50��1s��
            break;

        case 3:
            C11 = 0xf7;
            C12 = 0xf7;
           // C13 = 0xf7;
           // C14 = 0xf7;
            delay2(10); //Ĭ��Ϊ50��1s��
            break;

        case 4:
            C11 = 0xef;
            C12 = 0xef;
           // C13 = 0xef;
            //C14 = 0xef;
            delay2(10); //Ĭ��Ϊ50��1s��
            break;

        case 5:
            C11 = 0xdf;
            C12 = 0xdf;
           // C13 = 0xdf;
           // C14 = 0xdf;
            delay2(10); //Ĭ��Ϊ50��1s��
            break;

        case 6:
            C11 = 0xbf;
            C12 = 0xbf;
          //  C13 = 0xbf;
          //  C14 = 0xbf;
            delay2(10); //Ĭ��Ϊ50��1s��
            break;

        case 7:
            C11 = 0x7f;
            C12 = 0x7f;
          //  C13 = 0x7f;
            //C14 = 0x7f;
            delay2(10); //Ĭ��Ϊ50��1s��
            break;
        }
    }
    return;
}