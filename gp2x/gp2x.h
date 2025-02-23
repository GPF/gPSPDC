#ifndef __GPSP_GP2X_H__
#define __GPSP_GP2X_H__

extern u32   gp2x_dev;
extern volatile u16 *gp2x_memregs;
extern volatile u32  *gp2x_memregl;

extern void gp2x_quit(void);
extern void cpuctrl_init(void); /* call this at first */
extern void save_system_regs(void); /* save some registers */
extern void cpuctrl_deinit(void);
extern void set_display_clock_div(unsigned div);
extern void set_FCLK(unsigned MHZ); /* adjust the clock frequency (in Mhz units) */
extern void set_920_Div(unsigned short div); /* 0 to 7 divider (freq=FCLK/(1+div)) */
extern void set_DCLK_Div(unsigned short div); /* 0 to 7 divider (freq=FCLK/(1+div)) */
extern void Disable_940(void); /* 940t down */
extern void gp2x_video_wait_vsync(void);
extern unsigned short get_920_Div();
extern void set_940_Div(unsigned short div);

#endif
