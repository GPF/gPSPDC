/*  Parts used from cpuctrl */
/*  cpuctrl for GP2X
    Copyright (C) 2005  Hermes/PS2Reality 

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

*/


#include <sys/mman.h>
#include "gp2x.h"
#include "../common.h"

u32   gp2x_dev = 0;
volatile u16 *gp2x_memregs;
volatile u32  *gp2x_memregl;

/* system registers */
static struct 
{
	unsigned short SYSCLKENREG,SYSCSETREG,FPLLVSETREG,DUALINT920,DUALINT940,DUALCTRL940;
}
system_reg;

static unsigned short dispclockdiv;

static volatile unsigned short *MEM_REG;

#define SYS_CLK_FREQ 7372800

enum  { GP2X_UP=0x1,         GP2X_LEFT=0x4,         GP2X_DOWN=0x10,    GP2X_RIGHT=0x40,
        GP2X_START=(1<<8),   GP2X_SELECT=1<<9,      GP2X_L=(1<<10),    GP2X_R=(1<<11),
        GP2X_A=1<<12,        GP2X_B=1<<13,          GP2X_X=1<<14,      GP2X_Y=1<<15,
        GP2X_VOL_UP=1<<23,   GP2X_VOL_DOWN=(1<<22), GP2X_PUSH=1<<27 };

void cpuctrl_init(void)
{
	MEM_REG=&gp2x_memregs[0];
	system_reg.SYSCSETREG=MEM_REG[0x91c>>1];
	system_reg.FPLLVSETREG=MEM_REG[0x912>>1];
	system_reg.SYSCLKENREG=MEM_REG[0x904>>1];
	system_reg.DUALINT920=MEM_REG[0x3B40>>1];
	system_reg.DUALINT940=MEM_REG[0x3B42>>1];
	system_reg.DUALCTRL940=MEM_REG[0x3B48>>1];
	dispclockdiv=MEM_REG[0x924>>1];
}


void cpuctrl_deinit(void)
{
	MEM_REG[0x91c>>1]=system_reg.SYSCSETREG;
	MEM_REG[0x910>>1]=system_reg.FPLLVSETREG;
	MEM_REG[0x3B40>>1]=system_reg.DUALINT920;
	MEM_REG[0x3B42>>1]=system_reg.DUALINT940;
	MEM_REG[0x3B48>>1]=system_reg.DUALCTRL940;
	MEM_REG[0x904>>1]=system_reg.SYSCLKENREG;
	MEM_REG[0x924>>1]=dispclockdiv;
}


void set_display_clock_div(unsigned div)
{
	div=((div & 63) | 64)<<8;
	MEM_REG[0x924>>1]=(MEM_REG[0x924>>1] & ~(255<<8)) | div;
}


void set_FCLK(unsigned MHZ)
{
	unsigned v;
	unsigned mdiv,pdiv=3,scale=0;
	MHZ*=1000000;
	mdiv=(MHZ*pdiv)/SYS_CLK_FREQ;
	mdiv=((mdiv-8)<<8) & 0xff00;
	pdiv=((pdiv-2)<<2) & 0xfc;
	scale&=3;
	v=mdiv | pdiv | scale;
	MEM_REG[0x910>>1]=v;
}

unsigned short get_920_Div()
{
	return (MEM_REG[0x91c>>1] & 0x7); 
}

void set_920_Div(unsigned short div)
{
	unsigned short v;
	v = MEM_REG[0x91c>>1] & (~0x3);
	MEM_REG[0x91c>>1] = (div & 0x7) | v; 
}


void set_DCLK_Div( unsigned short div )
{
	unsigned short v;
	v = (unsigned short)( MEM_REG[0x91c>>1] & (~(0x7 << 6)) );
	MEM_REG[0x91c>>1] = ((div & 0x7) << 6) | v; 
}

void set_940_Div(unsigned short div)
{	
	unsigned short v;
	v = (unsigned short)( MEM_REG[0x91c>>1] & (~(0x7 << 3)));
	MEM_REG[0x91c>>1] = ((div & 0x7) << 3) | v; 
}

void Disable_940(void)
{
	MEM_REG[0x3B42>>1];
	MEM_REG[0x3B42>>1]=0;
	MEM_REG[0x3B46>>1]=0xffff;	
	MEM_REG[0x3B48>>1]|= (1 << 7);
	MEM_REG[0x904>>1]&=0xfffe;
}

void gp2x_video_wait_vsync(void)
{
	MEM_REG[0x2846>>1]=(MEM_REG[0x2846>>1] | 0x20) & ~2;
	while(!(MEM_REG[0x2846>>1] & 2));
}


void gp2x_overclock(void)
{
    int frag = 0x80000|7;
	unsigned lfreq[25]={60,66,80,100,120,133,150,166,200,210,220,240,250,266,270,275,280,285,290,295,300,305,310,315,320};
    int cur_freq = 8;
	char ocstr[256];

	print_string("USE THE L AND R TRIGGERS TO SELECT CPU SPEED", 0xFFFF, 0x0000, 0, 0);
	print_string("PRESS START TO SET CLOCKSPEED AND CONTINUE", 0xFFFF, 0x0000, 0, 10);
	flip_screen();

	cpuctrl_init();

	for(;;)
	{
		unsigned long keys = gp2x_joystick_read();
		sprintf(ocstr, "CPU WILL BE SET TO: %3u ", lfreq[cur_freq]);
		print_string(ocstr, 0xFFFF, 0x0000, 0, 20);
		flip_screen();

		if( keys & GP2X_L )
		{
			if( cur_freq > 0 ) cur_freq--;
		}
		if( keys & GP2X_R )
		{
			if( cur_freq < 24 ) cur_freq++;
		}
		if( keys & GP2X_START )
		{
			int m;
			m=get_920_Div();
			set_FCLK(lfreq[cur_freq]);
			set_920_Div(m);
			set_940_Div(m);
			set_DCLK_Div(m);
			break;
		}

		delay_us(100000);
	}
}

void gp2x_quit(void)
{
  cpuctrl_deinit();
  munmap((void *)gp2x_memregl,      0x10000);
  close(gp2x_dev);
  chdir("/usr/gp2x");
  execl("gp2xmenu","gp2xmenu",NULL);
}