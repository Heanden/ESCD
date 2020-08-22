#include "main.h"
#include "ascii.h"
#include "pic.h"
#include <math.h>

#define GPF0CON (*(volatile unsigned long *)0xE0200120)
#define GPF1CON (*(volatile unsigned long *)0xE0200140)
#define GPF2CON (*(volatile unsigned long *)0xE0200160)
#define GPF3CON (*(volatile unsigned long *)0xE0200180)

#define GPD0CON (*(volatile unsigned long *)0xE02000A0)
#define GPD0DAT (*(volatile unsigned long *)0xE02000A4)

#define CLK_SRC1 (*(volatile unsigned long *)0xe0100204)
#define CLK_DIV1 (*(volatile unsigned long *)0xe0100304)
#define DISPLAY_CONTROL (*(volatile unsigned long *)0xe0107008)

#define VIDCON0 (*(volatile unsigned long *)0xF8000000)
#define VIDCON1 (*(volatile unsigned long *)0xF8000004)
#define VIDTCON2 (*(volatile unsigned long *)0xF8000018)
#define WINCON0 (*(volatile unsigned long *)0xF8000020)
#define WINCON2 (*(volatile unsigned long *)0xF8000028)
#define SHADOWCON (*(volatile unsigned long *)0xF8000034)
#define VIDOSD0A (*(volatile unsigned long *)0xF8000040)
#define VIDOSD0B (*(volatile unsigned long *)0xF8000044)
#define VIDOSD0C (*(volatile unsigned long *)0xF8000048)

#define VIDW00ADD0B0 (*(volatile unsigned long *)0xF80000A0)
#define VIDW00ADD1B0 (*(volatile unsigned long *)0xF80000D0)

#define VIDTCON0 (*(volatile unsigned long *)0xF8000010)
#define VIDTCON1 (*(volatile unsigned long *)0xF8000014)

#define HSPW (40)	   // 1~40
#define HBPD (10 - 1)  // 46
#define HFPD (240 - 1) // 16 210 354
#define VSPW (20)	   // 1~20
#define VBPD (10 - 1)  // 23
#define VFPD (30 - 1)  // 7 22 147

// FB地址
#define FB_ADDR (0x23000000)
#define ROW (600)
#define COL (1024)
#define HOZVAL (COL - 1)
#define LINEVAL (ROW - 1)

#define XSIZE COL
#define YSIZE ROW

u32 *pfb = (u32 *)FB_ADDR;

// 常用颜色定义
#define BLUE 0x0000FF
#define RED 0xFF0000
#define GREEN 0x00FF00
#define WHITE 0xFFFFFF
#define BLACK 0x000000

int raise(int a)
{
	return 0;
}
void lcd_init(void)
{
	// 配置引脚用于LCD功能
	GPF0CON = 0x22222222;
	GPF1CON = 0x22222222;
	GPF2CON = 0x22222222;
	GPF3CON = 0x22222222;

	// 打开背光	GPD0_0（PWMTOUT0）
	GPD0CON &= ~(0xf << 0);
	GPD0CON |= (1 << 0);  // output mode
	GPD0DAT &= ~(1 << 0); // output 0 to enable backlight

	// 10: RGB=FIMD I80=FIMD ITU=FIMD
	DISPLAY_CONTROL = 2 << 0;

	// bit[26~28]:使用RGB接口
	// bit[18]:RGB 并行
	// bit[2]:选择时钟源为HCLK_DSYS=166MHz
	VIDCON0 &= ~((3 << 26) | (1 << 18) | (1 << 2));

	// bit[1]:使能lcd控制器
	// bit[0]:当前帧结束后使能lcd控制器
	VIDCON0 |= ((1 << 0) | (1 << 1));

	// bit[6]:选择需要分频
	// bit[6~13]:分频系数为5，即VCLK = 166M/(4+1) = 33M
	VIDCON0 |= 4 << 6 | 1 << 4;

	// H43-HSD043I9W1.pdf(p13) 时序图：VSYNC和HSYNC都是低脉冲
	// s5pv210芯片手册(p1207) 时序图：VSYNC和HSYNC都是高脉冲有效，所以需要反转
	VIDCON1 |= 1 << 5 | 1 << 6;

	// 设置时序
	VIDTCON0 = VBPD << 16 | VFPD << 8 | VSPW << 0;
	VIDTCON1 = HBPD << 16 | HFPD << 8 | HSPW << 0;
	// 设置长宽(物理屏幕)
	VIDTCON2 = (LINEVAL << 11) | (HOZVAL << 0);

	// 设置window0
	// bit[0]:使能
	// bit[2~5]:24bpp（RGB888）
	WINCON0 |= 1 << 0;
	WINCON0 &= ~(0xf << 2);
	WINCON0 |= (0xB << 2) | (1 << 15);

#define LeftTopX 0
#define LeftTopY 0
#define RightBotX 1023
#define RightBotY 599

	// 设置window0的上下左右
	// 设置的是显存空间的大小
	VIDOSD0A = (LeftTopX << 11) | (LeftTopY << 0);
	VIDOSD0B = (RightBotX << 11) | (RightBotY << 0);
	VIDOSD0C = (LINEVAL + 1) * (HOZVAL + 1);

	// 设置fb的地址
	VIDW00ADD0B0 = FB_ADDR;
	VIDW00ADD1B0 = (((HOZVAL + 1) * 4 + 0) * (LINEVAL + 1)) & (0xffffff);

	// 使能channel 0传输数据
	SHADOWCON = 0x1;
}

void lcd_draw_pixel(int x, int y, int color)
{
	unsigned long *pixel = (unsigned long *)FB_ADDR;
	*(pixel + y * COL + x) = color;
	return;
}

static void lcd_draw_background(u32 color)
{
	u32 i, j;

	for (j = 0; j < ROW; j++)
	{
		for (i = 0; i < COL; i++)
		{
			lcd_draw_pixel(i, j, color);
		}
	}
}

static void lcd_draw_hline(u32 x1, u32 x2, u32 y, u32 color)
{
	u32 x;

	for (x = x1; x < x2; x++)
	{
		lcd_draw_pixel(x, y, color);
	}
}

void glib_line(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, unsigned int color)
{
	int dx, dy, e;
	dx = x2 - x1;
	dy = y2 - y1;

	if (dx >= 0)
	{
		if (dy >= 0) // dy>=0
		{
			if (dx >= dy) // 1/8 octant
			{
				e = dy - dx / 2;
				while (x1 <= x2)
				{
					lcd_draw_pixel(x1, y1, color);
					if (e > 0)
					{
						y1 += 1;
						e -= dx;
					}
					x1 += 1;
					e += dy;
				}
			}
			else // 2/8 octant
			{
				e = dx - dy / 2;
				while (y1 <= y2)
				{
					lcd_draw_pixel(x1, y1, color);
					if (e > 0)
					{
						x1 += 1;
						e -= dy;
					}
					y1 += 1;
					e += dx;
				}
			}
		}
		else // dy<0
		{
			dy = -dy; // dy=(dy)

			if (dx >= dy) // 8/8 octant
			{
				e = dy - dx / 2;
				while (x1 <= x2)
				{
					lcd_draw_pixel(x1, y1, color);
					if (e > 0)
					{
						y1 -= 1;
						e -= dx;
					}
					x1 += 1;
					e += dy;
				}
			}
			else // 7/8 octant
			{
				e = dx - dy / 2;
				while (y1 >= y2)
				{
					lcd_draw_pixel(x1, y1, color);
					if (e > 0)
					{
						x1 += 1;
						e -= dy;
					}
					y1 -= 1;
					e += dx;
				}
			}
		}
	}
	else //dx<0
	{
		dx = -dx;	 //dx=(dx)
		if (dy >= 0) // dy>=0
		{
			if (dx >= dy) // 4/8 octant
			{
				e = dy - dx / 2;
				while (x1 >= x2)
				{
					lcd_draw_pixel(x1, y1, color);
					if (e > 0)
					{
						y1 += 1;
						e -= dx;
					}
					x1 -= 1;
					e += dy;
				}
			}
			else // 3/8 octant
			{
				e = dx - dy / 2;
				while (y1 <= y2)
				{
					lcd_draw_pixel(x1, y1, color);
					if (e > 0)
					{
						x1 -= 1;
						e -= dy;
					}
					y1 += 1;
					e += dx;
				}
			}
		}
		else // dy<0
		{
			dy = -dy; // dy=(dy)

			if (dx >= dy) // 5/8 octant
			{
				e = dy - dx / 2;
				while (x1 >= x2)
				{
					lcd_draw_pixel(x1, y1, color);
					if (e > 0)
					{
						y1 -= 1;
						e -= dx;
					}
					x1 -= 1;
					e += dy;
				}
			}
			else // 6/8 octant
			{
				e = dx - dy / 2;
				while (y1 >= y2)
				{
					lcd_draw_pixel(x1, y1, color);
					if (e > 0)
					{
						x1 -= 1;
						e -= dy;
					}
					y1 -= 1;
					e += dx;
				}
			}
		}
	}
}

static void show_8_16(unsigned int x, unsigned int y, unsigned int color, unsigned char *data)
{
	// count记录当前正在绘制的像素的次序
	int i, j, count = 0;

	for (j = y; j < (y + 16); j++)
	{
		for (i = x; i < (x + 8); i++)
		{
			if (i < XSIZE && j < YSIZE)
			{
				// 在坐标(i, j)这个像素处判断是0还是1，如果是1写color；如果是0直接跳过
				if (data[count / 8] & (1 << (count % 8)))
					lcd_draw_pixel(i, j, color);
			}
			count++;
		}
	}
}

static void show_16_16(unsigned int x, unsigned int y, unsigned int color, unsigned char *data)
{
	// count记录当前正在绘制的像素的次序
	int i, j, count = 0;

	for (j = y; j < (y + 16); j++)
	{
		for (i = x; i < (x + 16); i++)
		{
			if (i < XSIZE && j < YSIZE)
			{
				// 在坐标(i, j)这个像素处判断是0还是1，如果是1写color；如果是0直接跳过
				if (data[count / 8] & (1 << (count % 8)))
					lcd_draw_pixel(i, j, color);
			}
			count++;
		}
	}
}

static void show_32_32(unsigned int x, unsigned int y, unsigned int color, unsigned char *data)
{
	// count记录当前正在绘制的像素的次序
	int i, j, count = 0;

	for (j = y; j < (y + 32); j++)
	{
		for (i = x; i < (x + 32); i++)
		{
			if (i < XSIZE && j < YSIZE)
			{
				// 在坐标(i, j)这个像素处判断是0还是1，如果是1写color；如果是0直接跳过
				if (data[count / 8] & (1 << (count % 8)))
					lcd_draw_pixel(i, j, color);
			}
			count++;
		}
	}
}

static void lcd_draw_vline(u32 x, u32 y1, u32 y2, u32 color)
{
	u32 y;

	for (y = y1; y < y2; y++)
	{
		lcd_draw_pixel(x, y, color);
	}
}

void draw_circular(unsigned int centerX, unsigned int centerY, unsigned int radius, unsigned int color)
{
	int x, y;
	int tempX, tempY;
	;
	int SquareOfR = radius * radius;

	for (y = 0; y < XSIZE; y++)
	{
		for (x = 0; x < YSIZE; x++)
		{
			if (y <= centerY && x <= centerX)
			{
				tempY = centerY - y;
				tempX = centerX - x;
			}
			else if (y <= centerY && x >= centerX)
			{
				tempY = centerY - y;
				tempX = x - centerX;
			}
			else if (y >= centerY && x <= centerX)
			{
				tempY = y - centerY;
				tempX = centerX - x;
			}
			else
			{
				tempY = y - centerY;
				tempX = x - centerX;
			}
			if ((tempY * tempY + tempX * tempX) <= SquareOfR)
				lcd_draw_pixel(x, y, color);
		}
	}
}

void draw_ascii_ok32(unsigned int x, unsigned int y, unsigned int color, unsigned char *str)
{
	int i;
	unsigned char *ch;
	for (i = 0; str[i] != '\0'; i++)
	{
		ch = (unsigned char *)jmu_32_32[((unsigned char)str[i] - 97) * 8];
		show_32_32(x, y, color, ch);

		x += 32;
		if (x >= XSIZE)
		{
			x -= XSIZE; // 回车
			y += 32;	// 换行
		}
	}
}

void lcd_draw_picture(const unsigned char *pData)
{
	u32 x, y, color, p = 0, pco = 0;
	for (x = 0; x < COL; x++)
	{
		for (y = 0; y < ROW; y++)
		{
			if (pco == 2)
				color = (pData[p] & (2 ^ pco)) ? 0x0000C0 : WHITE;
			else
				color = (pData[p] & (2 ^ pco)) ? 0x0000C0 : WHITE;
			lcd_draw_pixel(x, y, color);
			if (pco > 7)
			{
				pco = 0;
				p++;
			}
			pco++;
		}
	}
}

void draw_ascii_ok16(unsigned int x, unsigned int y, unsigned int color, unsigned char *str)
{
	int i;
	unsigned char *ch;
	for (i = 0; str[i] != '\0'; i++)
	{
		ch = (unsigned char *)ascii_16_16[((unsigned char)str[i] - 97) * 2];
		show_16_16(x, y, color, ch);

		x += 20;
		if (x >= XSIZE)
		{
			x -= XSIZE; // 回车
			y += 16;	// 换行
		}
	}
}

void draw_ascii_ok8(unsigned int x, unsigned int y, unsigned int color, unsigned char *str)
{
	int i;
	unsigned char *ch;
	for (i = 0; str[i] != '\0'; i++)
	{
		ch = (unsigned char *)ascii_8_16[(unsigned char)str[i] - 0x20];
		show_8_16(x, y, color, ch);

		x += 8;
		if (x >= XSIZE)
		{
			x -= XSIZE; // 回车
			y += 16;	// 换行
		}
	}
}

void draw_triangle(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, unsigned int x3, unsigned int y3, unsigned int color)
{
	int temp;
	if (x1 > x2)
	{
		temp = x1;
		x1 = x2;
		x2 = temp;

		temp = y1;
		y1 = y2;
		y2 = temp;
	}

	if (x1 > x3)
	{
		temp = x1;
		x1 = x3;
		x3 = temp;

		temp = y1;
		y1 = y3;
		y3 = temp;
	}

	if (x2 > x3)
	{
		temp = x2;
		x2 = x3;
		x3 = temp;

		temp = y2;
		y2 = y3;
		y3 = temp;
	}
	int x, y;
	for (x = 0; x < XSIZE; x++)
	{
		for (y = 0; y < YSIZE; y++)
		{
			if (((y1 >= y2) & (y2 >= y3)) | ((y1 <= y2) & (y2 <= y3)))
			{
				if (((x1 - x3) * y2) <= ((y1 - y3) * x2))
				{
					if (((x1 - x2) * y) >= ((y1 - y2) * x) & ((x2 - x3) * y) >= ((y2 - y3) * x) & ((y1 - y3) * x) >= ((x1 - x3) * y))
					{
						lcd_draw_pixel(x, y, color);
					}
				}
				else
				{
					if (((x1 - x2) * y) <= ((y1 - y2) * x) & ((x2 - x3) * y) <= ((y2 - y3) * x) & ((y1 - y3) * x) <= ((x1 - x3) * y))
					{
						lcd_draw_pixel(x, y, color);
					}
				}
			}

			else
			{
				if (((x1 - x3) * y2) <= ((y1 - y3) * x2))
				{
					if (((x1 - x2) * y) <= ((y1 - y2) * x) & ((x2 - x3) * y) >= ((y2 - y3) * x) & ((y1 - y3) * x) >= ((x1 - x3) * y))
					{
						lcd_draw_pixel(x, y, color);
					}
				}
				else
				{
					if (((x1 - x2) * y) <= ((y1 - y2) * x) & ((x2 - x3) * y) >= ((y2 - y3) * x) & ((y1 - y3) * x) <= ((x1 - x3) * y))
					{
						lcd_draw_pixel(x, y, color);
					}
				}
			}
		}
	}
}

void lcd_test(void)
{
	lcd_init(); //1、LCD控制器初始化
	lcd_draw_background(WHITE);

	//2、实现屏幕颜色自动切换（红-绿-蓝-黑-白及多种灰色）。
	/*
	lcd_draw_background(RED);
	delay();
	lcd_draw_background(GREEN);
	delay();
	lcd_draw_background(BLUE);
	delay();
	*/
	//3、几何图形的显示（矩形、三角形、五角星、椭圆）。
	draw_triangle(400, 400, 500, 200, 600, 400, RED);

	//5、画图（小组成员合照一张，要求能辨别出人，单色显示，分辨率不宜太大，编译后的bin文件不大于16K）。
	//lcd_draw_picture(gImage_pic);

	//4、显示不同大小的英文、中文字符（学号、姓名、姓名拼音、集美大学诚毅学院）。
	/*
	draw_ascii_ok32(950, 50, RED, "a");//jmu，cycu
	draw_ascii_ok32(950, 80, RED, "b");
	draw_ascii_ok32(950, 120, RED, "c");
	draw_ascii_ok32(950, 160, RED, "d");
	draw_ascii_ok32(950, 200, RED, "e");
	draw_ascii_ok32(950, 240, RED, "f");
	draw_ascii_ok32(950, 280, RED, "g");
	draw_ascii_ok32(950, 320, RED, "h");

	draw_ascii_ok16(100, 10, BLUE, "ijk"); //FY
	draw_ascii_ok8(160, 10, BLUE, "Chen Feiyuan");
	draw_ascii_ok8(100, 30, BLUE, "201741053072");

	draw_ascii_ok16(370, 10, BLUE, "lmn"); //JM
	draw_ascii_ok8(430, 10, BLUE, "Lin Junming");
	draw_ascii_ok8(370, 30, BLUE, "201741053057");

	draw_ascii_ok16(680, 10, BLUE, "opq"); //ZH
	draw_ascii_ok8(740, 10, BLUE, "Wu Zhenhang");
	draw_ascii_ok8(680, 30, BLUE, "201741053075");
	*/
}