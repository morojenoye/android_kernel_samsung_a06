/*
 * Copyright (C) 2016 MediaTek Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See http://www.gnu.org/licenses/gpl-2.0.html for more details.
 */

/*****************************************************************************
 *
 * Filename:
 * ---------
 *	   ov05a10txdpd2216frontmipiraw_Sensor.c
 *
 * Project:
 * --------
 *	   ALPS
 *
 * Description:
 * ------------
 *	   Source code of Sensor driver
 *
 *
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/

#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/atomic.h>
#include <linux/types.h>
//#include "kd_camera_typedef.h"
//#include "kd_camera_hw.h"
//#include "kd_imgsensor.h"
//#include "kd_imgsensor_define.h"
//#include "kd_imgsensor_errcode.h"
#include "o2102_ov05a10hlt_front_mipi_raw.h"
/****************************Modify Following Strings for Debug****************************/
#define PFX "o2102_ov05a10hlt_front_mipi_raw"
#define LOG_1 LOG_INF("OV05A10TXDPD2216FRONT,MIPI 2LANE\n")
#define LOG_2 LOG_INF("preview 2608*1960@30fps,420Mbps/lane; video 2608*1960@30fps,420Mbps/lane; capture 5M@15fps,420Mbps/lane\n")
/****************************	Modify end	  *******************************************/
//#define LOG_INF(format, args...)	  xlog_printk(ANDROID_LOG_INFO	 , PFX, "[%s] " format, __FUNCTION__, ##args)
#define LOG_INF(format, args...) printk(PFX "[%s] " format, __FUNCTION__, ##args)
/*hs04 code for DEVAL6398A-462 by jianghongyan at 2022/09/08 start */
#define VBLANK_BASE 1993
/*hs04 code for DEVAL6398A-462 by jianghongyan at 2022/09/08 end */
static DEFINE_SPINLOCK(imgsensor_drv_lock);
/* hs04 code for SR-AL6398A-01-12 by liluling at 2022/7/22 begin */
/* hs04 code for SR-AL6398A-01-12 by liluling at 2022/7/20 begin */
//static kal_uint32 shutter_time = 10;
/*
static void custom1_setting(void);
static void custom2_setting(void);
static void custom3_setting(void);
static void custom4_setting(void);
static void custom5_setting(void);
*/
/*  hs04 code for DEVAL6398A-462 by liluling at 2022/08/11 start */
/*  hs04 code for DEVAL6398A-462 by liluling at 2022/08/26 start */
static struct imgsensor_info_struct imgsensor_info = {
	.sensor_id = O2102_OV05A10HLT_FRONT_SENSOR_ID,

	.checksum_value = 0x8e8fb430,
	.pre = {
		.pclk = 96000000,              //record different mode's pclk
		.linelength = 1562,             //record different mode's linelength
		.framelength = 2048,            //record different mode's framelength
		.startx = 0,                    //record different mode's startx of grabwindow
		.starty = 0,                    //record different mode's starty of grabwindow
		.grabwindow_width = 2592,        //record different mode's width of grabwindow
		.grabwindow_height = 1944,        //record different mode's height of grabwindow
		/*     following for MIPIDataLowPwr2HighSpeedSettleDelayCount by different scenario    */
		.mipi_data_lp2hs_settle_dc = 23,//unit , ns
		/*     following for GetDefaultFramerateByScenario()    */
		.max_framerate = 300,
		.mipi_pixel_rate = 192000000

	},
	.cap = {
		.pclk = 96000000,
		.linelength = 1562,
		.framelength = 2048,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 2592,
		.grabwindow_height = 1944,
		.mipi_data_lp2hs_settle_dc = 23,//unit , ns
		.max_framerate = 300,
		.mipi_pixel_rate = 192000000
	},
/*
	.cap1 = {
		.pclk = 93600000,
		.linelength = 1565,
		.framelength = 3984,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 2592,
		.grabwindow_height = 1944,
		.mipi_data_lp2hs_settle_dc = 23,//unit , ns
		.max_framerate = 300,
	},
*/
	.normal_video = {
		.pclk = 96000000,              //record different mode's pclk
		.linelength = 1562,             //record different mode's linelength
		.framelength =2048,            //record different mode's framelength
		.startx = 0,                    //record different mode's startx of grabwindow
		.starty = 0,                    //record different mode's starty of grabwindow
		.grabwindow_width = 2592,        //record different mode's width of grabwindow
		.grabwindow_height = 1944,        //record different mode's height of grabwindow
		/*     following for MIPIDataLowPwr2HighSpeedSettleDelayCount by different scenario    */
		.mipi_data_lp2hs_settle_dc = 23,//unit , ns
		/*     following for GetDefaultFramerateByScenario()    */
		.max_framerate = 300,
		.mipi_pixel_rate = 192000000
	},
	.hs_video = {
		.pclk = 96000000,
		.linelength = 1562,
		.framelength = 2048,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 2592,
		.grabwindow_height = 1944,
		.mipi_data_lp2hs_settle_dc = 23,//unit , ns
		.max_framerate = 300,
		.mipi_pixel_rate = 192000000
	},
	.slim_video = {
		.pclk = 96000000,              //record different mode's pclk
		.linelength = 1562,             //record different mode's linelength
		.framelength = 2048,            //record different mode's framelength
		.startx = 0,                    //record different mode's startx of grabwindow
		.starty = 0,                    //record different mode's starty of grabwindow
		.grabwindow_width = 2592,        //record different mode's width of grabwindow
		.grabwindow_height = 1944,        //record different mode's height of grabwindow
		.mipi_data_lp2hs_settle_dc = 23,//unit , ns
		.max_framerate = 300,
		.mipi_pixel_rate = 192000000
	},
/*	.custom1 = {
		.pclk = 48000000,              //record different mode's pclk
		.linelength = 1562,             //record different mode's linelength
		.framelength = 998,            //record different mode's framelength
		.startx = 0,                    //record different mode's startx of grabwindow
		.starty = 0,                    //record different mode's starty of grabwindow
		.grabwindow_width = 1280,        //record different mode's width of grabwindow
		.grabwindow_height = 960,        //record different mode's height of grabwindow
		.mipi_data_lp2hs_settle_dc = 23,//unit , ns
		.max_framerate = 300,
		.mipi_pixel_rate = 48000000
	},
	.custom2 = {
		.pclk = 48000000,              //record different mode's pclk
		.linelength = 1562,             //record different mode's linelength
		.framelength = 998,            //record different mode's framelength
		.startx = 0,                    //record different mode's startx of grabwindow
		.starty = 0,                    //record different mode's starty of grabwindow
		.grabwindow_width = 1280,        //record different mode's width of grabwindow
		.grabwindow_height = 960,        //record different mode's height of grabwindow
		.mipi_data_lp2hs_settle_dc = 23,//unit , ns
		.max_framerate = 300,
		.mipi_pixel_rate = 48000000
	},
	.custom3 = {
		.pclk = 38400000,
		.linelength = 1562,
		.framelength = 1024,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 1280,
		.grabwindow_height = 960,
		.mipi_data_lp2hs_settle_dc = 23,//unit , ns
		.max_framerate = 240,
		.mipi_pixel_rate = 38400000
	},
	.custom4 = {
		.pclk = 32000000,
		.linelength = 1561,
		.framelength = 998,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 1280,
		.grabwindow_height = 960,
		.mipi_data_lp2hs_settle_dc = 23,//unit , ns
		.max_framerate = 200,
		.mipi_pixel_rate = 32000000
	},
	.custom5 = {
		.pclk = 24000000,
		.linelength = 1562,
		.framelength = 998,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 640,
		.grabwindow_height = 480,
		.mipi_data_lp2hs_settle_dc = 23,//unit , ns
		.max_framerate = 150,
		.mipi_pixel_rate = 24000000
	},
*/
	.margin = 13,            //sensor framelength &amp; shutter margin
	.min_shutter = 4,        //min shutter
	.max_frame_length = 0x7fff,//max framelength by sensor register's limitation
	.ae_shut_delay_frame = 0,    //shutter delay frame for AE cycle, 2 frame with ispGain_delay-shut_delay=2-0=2
	.ae_sensor_gain_delay_frame = 0,//sensor gain delay frame for AE cycle,2 frame with ispGain_delay-sensor_gain_delay=2-0=2
	.ae_ispGain_delay_frame = 2,//isp gain delay frame for AE cycle
	.ihdr_support = 0,      //1, support; 0,not support
	.ihdr_le_firstline = 0,  //1,le first ; 0, se first
	.sensor_mode_num = 10,      //support sensor mode num

	.cap_delay_frame = 3,
	.pre_delay_frame = 3,
	.video_delay_frame = 3,
	.hs_video_delay_frame = 3,    //enter high speed video  delay frame num
	.slim_video_delay_frame = 3,//enter slim video delay frame num
	/*
	.custom1_delay_frame = 3,
	.custom2_delay_frame = 3,
	.custom3_delay_frame = 3,
	.custom4_delay_frame = 3,
	.custom5_delay_frame = 3,
    */
	.isp_driving_current = ISP_DRIVING_8MA, //mclk driving current
	.sensor_interface_type = SENSOR_INTERFACE_TYPE_MIPI,//sensor_interface_type
	.mipi_sensor_type = MIPI_OPHY_NCSI2, //0,MIPI_OPHY_NCSI2;  1,MIPI_OPHY_CSI2
	.mipi_settle_delay_mode = MIPI_SETTLEDELAY_AUTO,//0,MIPI_SETTLEDELAY_AUTO; 1,MIPI_SETTLEDELAY_MANNUAL
/*hs04 code for DEVAL6398A-24 by xutengtao at 2022/07/28 start */
	.sensor_output_dataformat = SENSOR_OUTPUT_FORMAT_RAW_Gb,//sensor output first pixel color
/*hs04 code for DEVAL6398A-24 by xutengtao at 2022/07/28 end */
	.mclk = 24,//mclk value, suggest 24 or 26 for 24Mhz or 26Mhz
	.mipi_lane_num = SENSOR_MIPI_2_LANE,//mipi lane num
	.i2c_addr_table = {0x78, 0xff}, //5409H---0x78 /5409---0x7a  zhanghua
	.i2c_speed = 1000,
};


static struct imgsensor_struct imgsensor = {
	.mirror = IMAGE_NORMAL,				   //mirrorflip information
	.sensor_mode = IMGSENSOR_MODE_INIT, //IMGSENSOR_MODE enum value,record current sensor mode,such as: INIT, Preview, Capture, Video,High Speed Video, Slim Video
	.shutter = 0x3D0,					 //current shutter
	.gain = 0x100,						  //current gain
	.dummy_pixel = 0,					 //current dummypixel
	.dummy_line = 0,					//current dummyline
	.current_fps = 300,	 //full size current fps : 24fps for PIP, 30fps for Normal or ZSD
	.autoflicker_en = KAL_FALSE,  //auto flicker enable: KAL_FALSE for disable auto flicker, KAL_TRUE for enable auto flicker
	.test_pattern = KAL_FALSE,		  //test pattern mode or not. KAL_FALSE for in test pattern mode, KAL_TRUE for normal output
	.current_scenario_id = MSDK_SCENARIO_ID_CAMERA_PREVIEW,//current scenario id
	.ihdr_en = 0, //sensor need support LE, SE with HDR feature
	.i2c_write_id = 0x78,//record current sensor's i2c write id //5409H---0x78 /5409/5409V---0x7a	zhanghua
};

/*add rawinfo start*/
/*
static struct  SENSOR_RAWINFO_STRUCT imgsensor_raw_info = {
	2560,//raw_weight
	1920,//raw_height
	2,//unpack raw byte,raw10 packed by 16bit(2byte)
	BAYER_GRBG,//raw_colorFilterValue
	64,//raw_blackLevel
	76.9,//raw_viewAngle
	10,//raw_bitWidth
	16//raw_maxSensorGain 64x
};
*/
/*add rawinfo end*/
/* Sensor output window information */
static struct SENSOR_WINSIZE_INFO_STRUCT imgsensor_winsize_info[10]= {
 { 2608, 1960, 8, 8, 2592, 1944, 2592, 1944, 0000, 0000, 2592, 1944, 0000, 0000, 2592,  1944}, // Preview 2112*1558
 { 2608, 1960, 8, 8, 2592, 1944, 2592, 1944, 0000, 0000, 2592, 1944, 0000, 0000, 2592,  1944}, // capture 4206*3128
 { 2608, 1960, 8, 8, 2592, 1944, 2592, 1944, 0000, 0000, 2592, 1944, 0000, 0000, 2592,  1944}, // video
 { 2608, 1960, 24, 20, 2560, 1920, 2560, 1920, 0000, 0000,  640,  480, 0000, 0000,  640,  480}, //hight speed video
 { 2608, 1960, 24, 20, 2560, 1920, 2560, 1920, 0000, 0000, 1280,  960, 0000, 0000, 1280,  960},// slim video
};


static kal_uint16 read_cmos_sensor(kal_uint32 addr)
{
	kal_uint16 get_byte = 0;
	char pu_send_cmd[1] = { (char)(addr & 0xFF) } ;

	// kdSetI2CSpeed(imgsensor_info.i2c_speed); // Add this func to set i2c speed by each sensor

	iReadRegI2C(pu_send_cmd, 1, (u8 *)&get_byte, 1, imgsensor.i2c_write_id);

	return get_byte;
}


static void write_cmos_sensor(kal_uint32 addr, kal_uint32 para)
{
	char pu_send_cmd[2] = { (char)(addr & 0xFF), (char)(para & 0xFF)} ;

	// kdSetI2CSpeed(imgsensor_info.i2c_speed); // Add this func to set i2c speed by each sensor

	iWriteRegI2C(pu_send_cmd, 2, imgsensor.i2c_write_id);
}
static void set_dummy(void)
{
    LOG_INF("dummyline = %d, dummypixels = %d \n", imgsensor.dummy_line, imgsensor.dummy_pixel);
	/* you can set dummy by imgsensor.dummy_line and imgsensor.dummy_pixel, or you can set dummy by imgsensor.frame_length and imgsensor.line_length */
	write_cmos_sensor(0xfd, 0x01);
	/*hs04 code for DEVAL6398A-462 by jianghongyan at 2022/09/08 start */
	write_cmos_sensor(0x05, ((imgsensor.frame_length-VBLANK_BASE)>> 8) & 0xFF);
    write_cmos_sensor(0x06, (imgsensor.frame_length-VBLANK_BASE) & 0xFF);
	/*hs04 code for DEVAL6398A-462 by jianghongyan at 2022/09/08 end */
	write_cmos_sensor(0x01, 0x01);

} 	 /*	   set_dummy  */
static kal_uint32 return_sensor_id(void)
{
	write_cmos_sensor(0xfd, 0x00);
	return ((read_cmos_sensor(0x02) << 8) | read_cmos_sensor(0x03));
}
static void set_max_framerate(UINT16 framerate, kal_bool min_framelength_en)
{
	kal_uint32 frame_length = imgsensor.frame_length;

   // LOG_INF("framerate = %d, min framelength should enable? \n", framerate,min_framelength_en);

	frame_length = imgsensor.pclk / framerate * 10 / imgsensor.line_length;

	spin_lock(&imgsensor_drv_lock);
	imgsensor.frame_length = (frame_length > imgsensor.min_frame_length) ? frame_length : imgsensor.min_frame_length;
	imgsensor.dummy_line = imgsensor.frame_length - imgsensor.min_frame_length;

	if (imgsensor.frame_length > imgsensor_info.max_frame_length) {
		imgsensor.frame_length = imgsensor_info.max_frame_length;
		imgsensor.dummy_line = imgsensor.frame_length - imgsensor.min_frame_length;
	}
    if (min_framelength_en)
		imgsensor.min_frame_length = imgsensor.frame_length;
	spin_unlock(&imgsensor_drv_lock);

	set_dummy();
} 	 /*	   set_max_framerate  */




static kal_uint32 streaming_control(kal_bool enable)
{
	LOG_INF("streaming_enable(0=Sw Standby,1=streaming): %d\n", enable);
	if ( enable ) {
		write_cmos_sensor(0xfd, 0x01);
		write_cmos_sensor(0xa0, 0x01);  //stream on
	} else {
		write_cmos_sensor(0xfd, 0x01);
		write_cmos_sensor(0xa0, 0x00);  //stream off
	}
	return ERROR_NONE;
}

static void write_shutter(kal_uint16 shutter)
{
	  kal_uint16 realtime_fps = 0;

	  spin_lock(&imgsensor_drv_lock);
    if (shutter > imgsensor.min_frame_length - imgsensor_info.margin) {
        imgsensor.frame_length = shutter + imgsensor_info.margin;
    } else {
        imgsensor.frame_length = imgsensor.min_frame_length;
    }
    if (imgsensor.frame_length > imgsensor_info.max_frame_length) {
        imgsensor.frame_length = imgsensor_info.max_frame_length;
    }
	spin_unlock(&imgsensor_drv_lock);
	shutter = (shutter < imgsensor_info.min_shutter) ? imgsensor_info.min_shutter : shutter;
	shutter = (shutter > (imgsensor_info.max_frame_length - imgsensor_info.margin)) ? (imgsensor_info.max_frame_length - imgsensor_info.margin) : shutter;

	shutter = (shutter >> 1) << 1;
	imgsensor.frame_length = (imgsensor.frame_length >> 1) << 1;
	if (imgsensor.autoflicker_en) {
		realtime_fps = imgsensor.pclk / imgsensor.line_length * 10 / imgsensor.frame_length;
		if (realtime_fps >= 297 && realtime_fps <= 305) {
			set_max_framerate(296, 0);
		} else if (realtime_fps >= 147 && realtime_fps <= 150) {
			set_max_framerate(146, 0);
		} else {
            set_max_framerate(realtime_fps,0);
		}
	}

		LOG_INF("huazai_shutter = %d == %x\n", shutter, shutter);
	// Update Shutter
		write_cmos_sensor(0xfd, 0x01);
		write_cmos_sensor(0x01, 0x00);
		write_cmos_sensor(0x03, (shutter >> 8) & 0xFF);
		write_cmos_sensor(0x04, shutter  & 0xFF);
		write_cmos_sensor(0x01, 0x01);


	LOG_INF("huazai_read 0x03 = 0x%x,0x04 == 0x%x\n", read_cmos_sensor(0x03), read_cmos_sensor(0x04));
	//LOG_INF("shutter = %d, framelength = %d\n", shutter, imgsensor.frame_length);

} /*write_shutter  */


/*************************************************************************
* FUNCTION
*	 set_shutter
*
* DESCRIPTION
*	 This function set e-shutter of sensor to change exposure time.
*
* PARAMETERS
*	 iShutter : exposured lines
*
* RETURNS
*	 None
*
* GLOBALS AFFECTED
*
*************************************************************************/
static void set_shutter(kal_uint16 shutter)
{
	unsigned long flags;

	spin_lock_irqsave(&imgsensor_drv_lock, flags);
	imgsensor.shutter = shutter;
	spin_unlock_irqrestore(&imgsensor_drv_lock, flags);

	write_shutter(shutter);
}



/*************************************************************************
* FUNCTION
*	 set_gain
*
* DESCRIPTION
*	 This function is to set global gain to sensor.
*
* PARAMETERS
*	 iGain : sensor global gain(base: 0x40)
*
* RETURNS
*	 the actually gain set to sensor.
*
* GLOBALS AFFECTED
*
*************************************************************************/
static kal_uint16 set_gain(kal_uint16 gain)
{
	kal_uint16 reg_gain = 0x0000;
	/*hs04 code for DEAL6398A-595 by hudongdong at 2022/09/09 start */
	if(gain < BASEGAIN)
	{
		gain = BASEGAIN;
	}
	else if(gain > (15.5*BASEGAIN))
	{
		gain = 15.5*BASEGAIN;
	}

	reg_gain = 0x10 * gain/BASEGAIN ;		 //change mtk gain base to aptina gain base

	if (reg_gain <= 0x10) {
			write_cmos_sensor(0xfd, 0x01);
			write_cmos_sensor(0x01, 0x00);
			write_cmos_sensor(0x24, 0x10);
			write_cmos_sensor(0x01, 0x01);
			//write_cmos_sensor(0x55, 0x24);
			LOG_INF("OV05A_test reg_gain = 0x%0x,  Page1_0x55 = %d", read_cmos_sensor(0x24), read_cmos_sensor(0x55));
	} else if (0x10 < reg_gain && reg_gain <= 0x18) {
			write_cmos_sensor(0xfd, 0x01);
			write_cmos_sensor(0x01, 0x00);
			write_cmos_sensor(0x24, (kal_uint8)reg_gain);
			write_cmos_sensor(0x01, 0x01);
			//write_cmos_sensor(0x55, 0x24);
			LOG_INF("OV05A_test reg_gain = 0x%0x,  Page1_0x55 = %d", read_cmos_sensor(0x24), read_cmos_sensor(0x55));
	} else if (0x18 < reg_gain && reg_gain <= 0x30) {
			write_cmos_sensor(0xfd, 0x01);
			write_cmos_sensor(0x01, 0x00);
			write_cmos_sensor(0x24, (kal_uint8)reg_gain);
			write_cmos_sensor(0x01, 0x01);
			//write_cmos_sensor(0x55, 0x1e);
			LOG_INF("OV05A_test reg_gain = 0x%0x,  Page1_0x55 = %d", read_cmos_sensor(0x24), read_cmos_sensor(0x55));
	} else if (0x30 < reg_gain && reg_gain <= 0xf8) {
			write_cmos_sensor(0xfd, 0x01);
			write_cmos_sensor(0x01, 0x00);
			write_cmos_sensor(0x24, (kal_uint8)reg_gain);
			write_cmos_sensor(0x01, 0x01);
			//write_cmos_sensor(0x55, 0x14);
			LOG_INF("OV05A_test reg_gain = 0x%0x,  Page1_0x55 = %d", read_cmos_sensor(0x24), read_cmos_sensor(0x55));
	} else if (reg_gain > 0xf8)  {
			write_cmos_sensor(0xfd, 0x01);
			write_cmos_sensor(0x01, 0x00);
			write_cmos_sensor(0x24, 0xf8);
			write_cmos_sensor(0x01, 0x01);
			//write_cmos_sensor(0x55, 0x14);
			LOG_INF("OV05A_test reg_gain = 0x%0x,  Page1_0x55 = %d", read_cmos_sensor(0x24), read_cmos_sensor(0x55));
	}
	/*hs04 code for DEAL6398A-595 by hudongdong at 2022/09/09 end */
	return gain;
} 	 /*	   set_gain	 */

// static void ihdr_write_shutter_gain(kal_uint16 le, kal_uint16 se, kal_uint16 gain)
// {
// 	//not support HDR
// 	//LOG_INF("le:0x%x, se:0x%x, gain:0x%x\n", le, se, gain);
// }



/*************************************************************************
* FUNCTION
*	 night_mode
*
* DESCRIPTION
*	 This function night mode of sensor.
*
* PARAMETERS
*	 bEnable: KAL_TRUE -> enable night mode, otherwise, disable night mode
*
* RETURNS
*	 None
*
* GLOBALS AFFECTED
*
*************************************************************************/
static void night_mode(kal_bool enable)
{
/*No Need to implement this function*/
} 	 /*	   night_mode	 */

static void sensor_init(void)
	/*	  MIPI_sensor_Init	*/
{


}    /*    MIPI_sensor_Init  */
/*hs04 code for DEVAL6398A-24 by xutengtao at 2022/07/28 start */
/* hs04 code for DEAL6398A-1806 by liluling at 2022/11/1 start */
static void preview_setting(void)
{
			write_cmos_sensor(0xfd, 0x00);
			write_cmos_sensor(0x20, 0x01); //add for software reset
			write_cmos_sensor(0x20, 0x00);
			mdelay(5);
			write_cmos_sensor(0xfd, 0x00);
			write_cmos_sensor(0x2e, 0x4d); //24
			write_cmos_sensor(0x2f, 0x03);
			write_cmos_sensor(0xfd, 0x01);
			write_cmos_sensor(0x03, 0x06);
			write_cmos_sensor(0x04, 0x36);
			write_cmos_sensor(0x06, 0x37);
			write_cmos_sensor(0x24, 0xff);
			write_cmos_sensor(0x39, 0x0d);
			write_cmos_sensor(0x3f, 0x01);
			write_cmos_sensor(0x31, 0x00);
			write_cmos_sensor(0x01, 0x01);
			write_cmos_sensor(0x11, 0x60);
			write_cmos_sensor(0x33, 0xb0);
			write_cmos_sensor(0x12, 0x03);
			write_cmos_sensor(0x13, 0xd0);
			write_cmos_sensor(0x45, 0x1e);
			write_cmos_sensor(0x16, 0xea);
			write_cmos_sensor(0x19, 0xf7);
			write_cmos_sensor(0x1a, 0x5f);
			write_cmos_sensor(0x1c, 0x0c);
			write_cmos_sensor(0x1d, 0x06);
			write_cmos_sensor(0x1e, 0x09);
			write_cmos_sensor(0x20, 0x07);
			write_cmos_sensor(0x2a, 0x0f);
			write_cmos_sensor(0x2c, 0x10);
			write_cmos_sensor(0x25, 0x0d);
			write_cmos_sensor(0x26, 0x0d);
			write_cmos_sensor(0x27, 0x08);
			write_cmos_sensor(0x29, 0x01);
			write_cmos_sensor(0x2d, 0x06);
			write_cmos_sensor(0x55, 0x14);
			write_cmos_sensor(0x56, 0x00);
			write_cmos_sensor(0x57, 0x17);
			write_cmos_sensor(0x59, 0x00);
			write_cmos_sensor(0x5a, 0x04);
			write_cmos_sensor(0x50, 0x10);
			write_cmos_sensor(0x53, 0x0e);
			write_cmos_sensor(0x6b, 0x10);
			write_cmos_sensor(0x5c, 0x20);
			write_cmos_sensor(0x5d, 0x00);
			write_cmos_sensor(0x5e, 0x06);
			write_cmos_sensor(0x66, 0x38);
			write_cmos_sensor(0x68, 0x30);
			write_cmos_sensor(0x71, 0x3f);
			write_cmos_sensor(0x72, 0x05);
			write_cmos_sensor(0x73, 0x3e);
			write_cmos_sensor(0x81, 0x22);
			write_cmos_sensor(0x8a, 0x66);
			write_cmos_sensor(0x8b, 0x66);
			write_cmos_sensor(0xc0, 0x02);
			write_cmos_sensor(0xc1, 0x02);
			write_cmos_sensor(0xc2, 0x02);
			write_cmos_sensor(0xc3, 0x02);
			write_cmos_sensor(0xc4, 0x82);
			write_cmos_sensor(0xc5, 0x82);
			write_cmos_sensor(0xc6, 0x82);
			write_cmos_sensor(0xc7, 0x82);
			write_cmos_sensor(0xfb, 0x4b);
			write_cmos_sensor(0xf0, 0x27);
			write_cmos_sensor(0xf1, 0x27);
			write_cmos_sensor(0xf2, 0x27);
			write_cmos_sensor(0xf3, 0x27);
			write_cmos_sensor(0xb1, 0xad);
			write_cmos_sensor(0xb6, 0x42);
			write_cmos_sensor(0xa1, 0x04);
			//write_cmos_sensor(0xa0, 0x01);
			write_cmos_sensor(0xfd, 0x02);
			write_cmos_sensor(0x34, 0xc8);
			write_cmos_sensor(0x60, 0x99);
			write_cmos_sensor(0x93, 0x03);
			write_cmos_sensor(0x14, 0x04);
			write_cmos_sensor(0x15, 0x60);
			write_cmos_sensor(0x16, 0x04);
			write_cmos_sensor(0x17, 0x60);
			write_cmos_sensor(0x18, 0xe0);
			write_cmos_sensor(0x19, 0xa0);
			write_cmos_sensor(0xfd, 0x04);
			write_cmos_sensor(0x31, 0x4b);
			write_cmos_sensor(0x32, 0x4b);
			write_cmos_sensor(0xfd, 0x03);
			write_cmos_sensor(0xc0, 0x00);
			write_cmos_sensor(0xfd, 0x01);
			write_cmos_sensor(0x8e, 0x0a);
			write_cmos_sensor(0x8f, 0x20);
			write_cmos_sensor(0x90, 0x07);
			write_cmos_sensor(0x91, 0x98);
			write_cmos_sensor(0xfd, 0x02);
			write_cmos_sensor(0xa0, 0x00);
			write_cmos_sensor(0xa1, 0x08);
			write_cmos_sensor(0xa2, 0x07);
			write_cmos_sensor(0xa3, 0x98);
			write_cmos_sensor(0xa4, 0x00);
			write_cmos_sensor(0xa5, 0x08);
			write_cmos_sensor(0xa6, 0x0a);
			write_cmos_sensor(0xa7, 0x20);
			write_cmos_sensor(0xfd, 0x01);
			write_cmos_sensor(0xa0, 0x01);
			write_cmos_sensor(0xfd, 0x01);


			// write_cmos_sensor(0xfd,0x01);
			// write_cmos_sensor(0xa0,0x01);
}    /*    preview_setting  */

static void capture_setting(void)
{
			write_cmos_sensor(0xfd, 0x00);
			write_cmos_sensor(0x20, 0x01); //add for software reset
			write_cmos_sensor(0x20, 0x00);
			mdelay(5);
			write_cmos_sensor(0xfd, 0x00);
			write_cmos_sensor(0x2e, 0x4d); //24
			write_cmos_sensor(0x2f, 0x03);
			write_cmos_sensor(0xfd, 0x01);
			write_cmos_sensor(0x03, 0x06);
			write_cmos_sensor(0x04, 0x36);
			write_cmos_sensor(0x06, 0x37);
			write_cmos_sensor(0x24, 0xff);
			write_cmos_sensor(0x39, 0x0d);
			write_cmos_sensor(0x3f, 0x01);
			write_cmos_sensor(0x31, 0x00);
			write_cmos_sensor(0x01, 0x01);
			write_cmos_sensor(0x11, 0x60);
			write_cmos_sensor(0x33, 0xb0);
			write_cmos_sensor(0x12, 0x03);
			write_cmos_sensor(0x13, 0xd0);
			write_cmos_sensor(0x45, 0x1e);
			write_cmos_sensor(0x16, 0xea);
			write_cmos_sensor(0x19, 0xf7);
			write_cmos_sensor(0x1a, 0x5f);
			write_cmos_sensor(0x1c, 0x0c);
			write_cmos_sensor(0x1d, 0x06);
			write_cmos_sensor(0x1e, 0x09);
			write_cmos_sensor(0x20, 0x07);
			write_cmos_sensor(0x2a, 0x0f);
			write_cmos_sensor(0x2c, 0x10);
			write_cmos_sensor(0x25, 0x0d);
			write_cmos_sensor(0x26, 0x0d);
			write_cmos_sensor(0x27, 0x08);
			write_cmos_sensor(0x29, 0x01);
			write_cmos_sensor(0x2d, 0x06);
			write_cmos_sensor(0x55, 0x14);
			write_cmos_sensor(0x56, 0x00);
			write_cmos_sensor(0x57, 0x17);
			write_cmos_sensor(0x59, 0x00);
			write_cmos_sensor(0x5a, 0x04);
			write_cmos_sensor(0x50, 0x10);
			write_cmos_sensor(0x53, 0x0e);
			write_cmos_sensor(0x6b, 0x10);
			write_cmos_sensor(0x5c, 0x20);
			write_cmos_sensor(0x5d, 0x00);
			write_cmos_sensor(0x5e, 0x06);
			write_cmos_sensor(0x66, 0x38);
			write_cmos_sensor(0x68, 0x30);
			write_cmos_sensor(0x71, 0x3f);
			write_cmos_sensor(0x72, 0x05);
			write_cmos_sensor(0x73, 0x3e);
			write_cmos_sensor(0x81, 0x22);
			write_cmos_sensor(0x8a, 0x66);
			write_cmos_sensor(0x8b, 0x66);
			write_cmos_sensor(0xc0, 0x02);
			write_cmos_sensor(0xc1, 0x02);
			write_cmos_sensor(0xc2, 0x02);
			write_cmos_sensor(0xc3, 0x02);
			write_cmos_sensor(0xc4, 0x82);
			write_cmos_sensor(0xc5, 0x82);
			write_cmos_sensor(0xc6, 0x82);
			write_cmos_sensor(0xc7, 0x82);
			write_cmos_sensor(0xfb, 0x4b);
			write_cmos_sensor(0xf0, 0x27);
			write_cmos_sensor(0xf1, 0x27);
			write_cmos_sensor(0xf2, 0x27);
			write_cmos_sensor(0xf3, 0x27);
			write_cmos_sensor(0xb1, 0xad);
			write_cmos_sensor(0xb6, 0x42);
			write_cmos_sensor(0xa1, 0x04);
			//write_cmos_sensor(0xa0, 0x01);
			write_cmos_sensor(0xfd, 0x02);
			write_cmos_sensor(0x34, 0xc8);
			write_cmos_sensor(0x60, 0x99);
			write_cmos_sensor(0x93, 0x03);
			write_cmos_sensor(0x14, 0x04);
			write_cmos_sensor(0x15, 0x60);
			write_cmos_sensor(0x16, 0x04);
			write_cmos_sensor(0x17, 0x60);
			write_cmos_sensor(0x18, 0xe0);
			write_cmos_sensor(0x19, 0xa0);
			write_cmos_sensor(0xfd, 0x04);
			write_cmos_sensor(0x31, 0x4b);
			write_cmos_sensor(0x32, 0x4b);
			write_cmos_sensor(0xfd, 0x03);
			write_cmos_sensor(0xc0, 0x00);
			write_cmos_sensor(0xfd, 0x01);
			write_cmos_sensor(0x8e, 0x0a);
			write_cmos_sensor(0x8f, 0x20);
			write_cmos_sensor(0x90, 0x07);
			write_cmos_sensor(0x91, 0x98);
			write_cmos_sensor(0xfd, 0x02);
			write_cmos_sensor(0xa0, 0x00);
			write_cmos_sensor(0xa1, 0x08);
			write_cmos_sensor(0xa2, 0x07);
			write_cmos_sensor(0xa3, 0x98);
			write_cmos_sensor(0xa4, 0x00);
			write_cmos_sensor(0xa5, 0x08);
			write_cmos_sensor(0xa6, 0x0a);
			write_cmos_sensor(0xa7, 0x20);
			write_cmos_sensor(0xfd, 0x01);
			write_cmos_sensor(0xa0, 0x01);
			write_cmos_sensor(0xfd, 0x01);


			// write_cmos_sensor(0xfd,0x01);
			// write_cmos_sensor(0xa0,0x01);
}    /*    capture_setting  */

static void normal_video_setting(void)
{
	preview_setting();
}    /*    preview_setting  */

static void hs_video_setting(void)
{
			write_cmos_sensor(0xfd, 0x00);
			write_cmos_sensor(0x20, 0x01); //add for software reset
			write_cmos_sensor(0x20, 0x00);
			mdelay(5);
			write_cmos_sensor(0xfd, 0x00);
			write_cmos_sensor(0x2e, 0x4d);  //24
			write_cmos_sensor(0x2f, 0x03);
			write_cmos_sensor(0xfd, 0x01);
			write_cmos_sensor(0x03, 0x06);
			write_cmos_sensor(0x04, 0x36);
			write_cmos_sensor(0x06, 0x37);
			write_cmos_sensor(0x24, 0xff);
			write_cmos_sensor(0x39, 0x0d);
			write_cmos_sensor(0x3f, 0x01);
			write_cmos_sensor(0x31, 0x00);
			write_cmos_sensor(0x01, 0x01);
			write_cmos_sensor(0x11, 0x60);
			write_cmos_sensor(0x33, 0xb0);
			write_cmos_sensor(0x12, 0x03);
			write_cmos_sensor(0x13, 0xd0);
			write_cmos_sensor(0x45, 0x1e);
			write_cmos_sensor(0x16, 0xea);
			write_cmos_sensor(0x19, 0xf7);
			write_cmos_sensor(0x1a, 0x5f);
			write_cmos_sensor(0x1c, 0x0c);
			write_cmos_sensor(0x1d, 0x06);
			write_cmos_sensor(0x1e, 0x09);
			write_cmos_sensor(0x20, 0x07);
			write_cmos_sensor(0x2a, 0x0f);
			write_cmos_sensor(0x2c, 0x10);
			write_cmos_sensor(0x25, 0x0d);
			write_cmos_sensor(0x26, 0x0d);
			write_cmos_sensor(0x27, 0x08);
			write_cmos_sensor(0x29, 0x01);
			write_cmos_sensor(0x2d, 0x06);
			write_cmos_sensor(0x55, 0x14);
			write_cmos_sensor(0x56, 0x00);
			write_cmos_sensor(0x57, 0x17);
			write_cmos_sensor(0x59, 0x00);
			write_cmos_sensor(0x5a, 0x04);
			write_cmos_sensor(0x50, 0x10);
			write_cmos_sensor(0x53, 0x0e);
			write_cmos_sensor(0x6b, 0x10);
			write_cmos_sensor(0x5c, 0x20);
			write_cmos_sensor(0x5d, 0x00);
			write_cmos_sensor(0x5e, 0x06);
			write_cmos_sensor(0x66, 0x38);
			write_cmos_sensor(0x68, 0x30);
			write_cmos_sensor(0x71, 0x3f);
			write_cmos_sensor(0x72, 0x05);
			write_cmos_sensor(0x73, 0x3e);
			write_cmos_sensor(0x81, 0x22);
			write_cmos_sensor(0x8a, 0x66);
			write_cmos_sensor(0x8b, 0x66);
			write_cmos_sensor(0xc0, 0x02);
			write_cmos_sensor(0xc1, 0x02);
			write_cmos_sensor(0xc2, 0x02);
			write_cmos_sensor(0xc3, 0x02);
			write_cmos_sensor(0xc4, 0x82);
			write_cmos_sensor(0xc5, 0x82);
			write_cmos_sensor(0xc6, 0x82);
			write_cmos_sensor(0xc7, 0x82);
			write_cmos_sensor(0xfb, 0x4b);
			write_cmos_sensor(0xf0, 0x27);
			write_cmos_sensor(0xf1, 0x27);
			write_cmos_sensor(0xf2, 0x27);
			write_cmos_sensor(0xf3, 0x27);
			write_cmos_sensor(0xb1, 0xad);
			write_cmos_sensor(0xb6, 0x42);
			write_cmos_sensor(0xa1, 0x04);
			//write_cmos_sensor(0xa0, 0x01);
			write_cmos_sensor(0xfd, 0x02);
			write_cmos_sensor(0x34, 0xc8);
			write_cmos_sensor(0x60, 0x99);
			write_cmos_sensor(0x93, 0x03);
			write_cmos_sensor(0x14, 0x04);
			write_cmos_sensor(0x15, 0x60);
			write_cmos_sensor(0x16, 0x04);
			write_cmos_sensor(0x17, 0x60);
			write_cmos_sensor(0x18, 0xe0);
			write_cmos_sensor(0x19, 0xa0);
			write_cmos_sensor(0xfd, 0x04);
			write_cmos_sensor(0x31, 0x4b);
			write_cmos_sensor(0x32, 0x4b);
			write_cmos_sensor(0xfd, 0x03);
			write_cmos_sensor(0xc0, 0x00);
			write_cmos_sensor(0xfd, 0x01);
			write_cmos_sensor(0x8e, 0x0a);
			write_cmos_sensor(0x8f, 0x20);
			write_cmos_sensor(0x90, 0x07);
			write_cmos_sensor(0x91, 0x98);
			write_cmos_sensor(0xfd, 0x02);
			write_cmos_sensor(0xa0, 0x00);
			write_cmos_sensor(0xa1, 0x08);
			write_cmos_sensor(0xa2, 0x07);
			write_cmos_sensor(0xa3, 0x98);
			write_cmos_sensor(0xa4, 0x00);
			write_cmos_sensor(0xa5, 0x08);
			write_cmos_sensor(0xa6, 0x0a);
			write_cmos_sensor(0xa7, 0x20);
			write_cmos_sensor(0xfd, 0x01);
			write_cmos_sensor(0xa0, 0x01);
			write_cmos_sensor(0xfd, 0x01);

			// write_cmos_sensor(0xfd,0x01);
			// write_cmos_sensor(0xa0,0x01);

}
/* hs04 code for DEAL6398A-1806 by liluling at 2022/11/1 end */
/*hs04 code for DEVAL6398A-24 by xutengtao at 2022/07/28 end */
static void slim_video_setting(void)
{
	preview_setting();
}
/*
static void custom1_setting(void)
{
	preview_setting();
}

static void custom2_setting(void)
{
	preview_setting();
}

static void custom3_setting(void)
{
			write_cmos_sensor(0xfd, 0x00);
			write_cmos_sensor(0x20, 0x01); //add for software reset
			write_cmos_sensor(0x20, 0x00);
			mdelay(5); 
			write_cmos_sensor(0xfd, 0x00);
			write_cmos_sensor(0x2e, 0x0d);
			write_cmos_sensor(0x2f, 0x01);
			write_cmos_sensor(0x30, 0x13);
			write_cmos_sensor(0x34, 0x02);
			write_cmos_sensor(0xfd, 0x01);
			write_cmos_sensor(0x03, 0x02);
			write_cmos_sensor(0x04, 0xd7);
			write_cmos_sensor(0x06, 0x1A);
			write_cmos_sensor(0x24, 0x80);
			write_cmos_sensor(0x39, 0x0d);
			write_cmos_sensor(0x3f, 0x02);
			write_cmos_sensor(0x31, 0x04);
			write_cmos_sensor(0x01, 0x01);
			write_cmos_sensor(0x11, 0x60);
			write_cmos_sensor(0x33, 0xb0);
			write_cmos_sensor(0x12, 0x03);
			write_cmos_sensor(0x13, 0xd0);
			write_cmos_sensor(0x45, 0x1e);
			write_cmos_sensor(0x16, 0xea);
			write_cmos_sensor(0x19, 0xf7);
			write_cmos_sensor(0x1a, 0x5f);
			write_cmos_sensor(0x1c, 0x0c);
			write_cmos_sensor(0x1d, 0x06);
			write_cmos_sensor(0x1e, 0x09);
			write_cmos_sensor(0x1f, 0x0b);
			write_cmos_sensor(0x20, 0x07);
			write_cmos_sensor(0x2a, 0x0f);
			write_cmos_sensor(0x2c, 0x10);
			write_cmos_sensor(0x25, 0x0d);
			write_cmos_sensor(0x26, 0x0d);
			write_cmos_sensor(0x27, 0x08);
			write_cmos_sensor(0x29, 0x01);
			write_cmos_sensor(0x2d, 0x06);
			write_cmos_sensor(0x55, 0x14);
			write_cmos_sensor(0x56, 0x00);
			write_cmos_sensor(0x57, 0x17);
			write_cmos_sensor(0x59, 0x00);
			write_cmos_sensor(0x5a, 0x04);
			write_cmos_sensor(0x50, 0x10);
			write_cmos_sensor(0x53, 0x0e);
			write_cmos_sensor(0x6b, 0x10);
			write_cmos_sensor(0x5c, 0x20);
			write_cmos_sensor(0x5d, 0x00);
			write_cmos_sensor(0x5e, 0x06);
			write_cmos_sensor(0x66, 0x38);
			write_cmos_sensor(0x68, 0x30);
			write_cmos_sensor(0x71, 0x3f);
			write_cmos_sensor(0x72, 0x05);
			write_cmos_sensor(0x73, 0x3e);
			write_cmos_sensor(0x81, 0x22);
			write_cmos_sensor(0x8a, 0x66);
			write_cmos_sensor(0x8b, 0x66);
			write_cmos_sensor(0xc0, 0x02);
			write_cmos_sensor(0xc1, 0x02);
			write_cmos_sensor(0xc2, 0x02);
			write_cmos_sensor(0xc3, 0x02);
			write_cmos_sensor(0xc4, 0x82);
			write_cmos_sensor(0xc5, 0x82);
			write_cmos_sensor(0xc6, 0x82);
			write_cmos_sensor(0xc7, 0x82);
			write_cmos_sensor(0xfb, 0x43);
			write_cmos_sensor(0xf0, 0x27);
			write_cmos_sensor(0xf1, 0x27);
			write_cmos_sensor(0xf2, 0x27);
			write_cmos_sensor(0xf3, 0x27);
			write_cmos_sensor(0xb1, 0xad);
			write_cmos_sensor(0xb6, 0x42);
			write_cmos_sensor(0xa1, 0x01);
			write_cmos_sensor(0xfd, 0x02);
			write_cmos_sensor(0x34, 0xc8);
			write_cmos_sensor(0x60, 0x99);
			write_cmos_sensor(0x93, 0x03);
			write_cmos_sensor(0x14, 0x01);
			write_cmos_sensor(0x15, 0xbc);
			write_cmos_sensor(0x16, 0x01);
			write_cmos_sensor(0x17, 0xbc);
			write_cmos_sensor(0x18, 0xe0);
			write_cmos_sensor(0x19, 0xa0);
			write_cmos_sensor(0xfd, 0x04);
			write_cmos_sensor(0x31, 0x4b);
			write_cmos_sensor(0x32, 0x4b);
			write_cmos_sensor(0xfd, 0x03);
			write_cmos_sensor(0xc0, 0x00);
			write_cmos_sensor(0xfd, 0x02);
			write_cmos_sensor(0xa0, 0x00);
			write_cmos_sensor(0xa1, 0x0a);
			write_cmos_sensor(0xa2, 0x03);
			write_cmos_sensor(0xa3, 0xc0);
			write_cmos_sensor(0xa4, 0x00);
			write_cmos_sensor(0xa5, 0x0c);
			write_cmos_sensor(0xa6, 0x05);
			write_cmos_sensor(0xa7, 0x00);
			write_cmos_sensor(0xfd, 0x01);
			write_cmos_sensor(0x8e, 0x05);
			write_cmos_sensor(0x8f, 0x00);
			write_cmos_sensor(0x90, 0x03);
			write_cmos_sensor(0x91, 0xc0);
			write_cmos_sensor(0xfd, 0x01);

			// write_cmos_sensor(0xfd,0x01);
			// write_cmos_sensor(0xa0,0x01);
}

static void custom4_setting(void)
{
			write_cmos_sensor(0xfd, 0x00);
			write_cmos_sensor(0x20, 0x01); //add for software reset
			write_cmos_sensor(0x20, 0x00);
			mdelay(5); 
			write_cmos_sensor(0xfd, 0x00);
			write_cmos_sensor(0x2e, 0x11);
			write_cmos_sensor(0x2f, 0x02);
			write_cmos_sensor(0x30, 0x13);
			write_cmos_sensor(0x34, 0x02);
			write_cmos_sensor(0xfd, 0x01);
			write_cmos_sensor(0x03, 0x02);
			write_cmos_sensor(0x04, 0xe0);
			write_cmos_sensor(0x06, 0x01);
			write_cmos_sensor(0x24, 0x80);
			write_cmos_sensor(0x39, 0x0d);
			write_cmos_sensor(0x3f, 0x02);
			write_cmos_sensor(0x31, 0x04);
			write_cmos_sensor(0x01, 0x01);
			write_cmos_sensor(0x11, 0x60);
			write_cmos_sensor(0x33, 0xb0);
			write_cmos_sensor(0x12, 0x03);
			write_cmos_sensor(0x13, 0xd0);
			write_cmos_sensor(0x45, 0x1e);
			write_cmos_sensor(0x16, 0xea);
			write_cmos_sensor(0x19, 0xf7);
			write_cmos_sensor(0x1a, 0x5f);
			write_cmos_sensor(0x1c, 0x0c);
			write_cmos_sensor(0x1d, 0x06);
			write_cmos_sensor(0x1e, 0x09);
			write_cmos_sensor(0x1f, 0x0b);
			write_cmos_sensor(0x20, 0x07);
			write_cmos_sensor(0x2a, 0x0f);
			write_cmos_sensor(0x2c, 0x10);
			write_cmos_sensor(0x25, 0x0d);
			write_cmos_sensor(0x26, 0x0d);
			write_cmos_sensor(0x27, 0x08);
			write_cmos_sensor(0x29, 0x01);
			write_cmos_sensor(0x2d, 0x06);
			write_cmos_sensor(0x55, 0x14);
			write_cmos_sensor(0x56, 0x00);
			write_cmos_sensor(0x57, 0x17);
			write_cmos_sensor(0x59, 0x00);
			write_cmos_sensor(0x5a, 0x04);
			write_cmos_sensor(0x50, 0x10);
			write_cmos_sensor(0x53, 0x0e);
			write_cmos_sensor(0x6b, 0x10);
			write_cmos_sensor(0x5c, 0x20);
			write_cmos_sensor(0x5d, 0x00);
			write_cmos_sensor(0x5e, 0x06);
			write_cmos_sensor(0x66, 0x38);
			write_cmos_sensor(0x68, 0x30);
			write_cmos_sensor(0x71, 0x3f);
			write_cmos_sensor(0x72, 0x05);
			write_cmos_sensor(0x73, 0x3e);
			write_cmos_sensor(0x81, 0x22);
			write_cmos_sensor(0x8a, 0x66);
			write_cmos_sensor(0x8b, 0x66);
			write_cmos_sensor(0xc0, 0x02);
			write_cmos_sensor(0xc1, 0x02);
			write_cmos_sensor(0xc2, 0x02);
			write_cmos_sensor(0xc3, 0x02);
			write_cmos_sensor(0xc4, 0x82);
			write_cmos_sensor(0xc5, 0x82);
			write_cmos_sensor(0xc6, 0x82);
			write_cmos_sensor(0xc7, 0x82);
			write_cmos_sensor(0xfb, 0x43);
			write_cmos_sensor(0xf0, 0x27);
			write_cmos_sensor(0xf1, 0x27);
			write_cmos_sensor(0xf2, 0x27);
			write_cmos_sensor(0xf3, 0x27);
			write_cmos_sensor(0xb1, 0xad);
			write_cmos_sensor(0xb6, 0x42);
			write_cmos_sensor(0xa1, 0x01);
			write_cmos_sensor(0xfd, 0x02);
			write_cmos_sensor(0x34, 0xc8);
			write_cmos_sensor(0x60, 0x99);
			write_cmos_sensor(0x93, 0x03);
			write_cmos_sensor(0x14, 0x01);
			write_cmos_sensor(0x15, 0x7d);
			write_cmos_sensor(0x16, 0x01);
			write_cmos_sensor(0x17, 0x7d);
			write_cmos_sensor(0x18, 0xe0);
			write_cmos_sensor(0x19, 0xa0);
			write_cmos_sensor(0xfd, 0x04);
			write_cmos_sensor(0x31, 0x4b);
			write_cmos_sensor(0x32, 0x4b);
			write_cmos_sensor(0xfd, 0x03);
			write_cmos_sensor(0xc0, 0x00);
			write_cmos_sensor(0xfd, 0x02);
			write_cmos_sensor(0xa0, 0x00);
			write_cmos_sensor(0xa1, 0x0a);
			write_cmos_sensor(0xa2, 0x03);
			write_cmos_sensor(0xa3, 0xc0);
			write_cmos_sensor(0xa4, 0x00);
			write_cmos_sensor(0xa5, 0x0c);
			write_cmos_sensor(0xa6, 0x05);
			write_cmos_sensor(0xa7, 0x00);
			write_cmos_sensor(0xfd, 0x01);
			write_cmos_sensor(0x8e, 0x05);
			write_cmos_sensor(0x8f, 0x00);
			write_cmos_sensor(0x90, 0x03);
			write_cmos_sensor(0x91, 0xc0);
			write_cmos_sensor(0xfd, 0x01);

			// write_cmos_sensor(0xfd,0x01);
			// write_cmos_sensor(0xa0,0x01);
}

static void custom5_setting(void)
{
			write_cmos_sensor(0xfd, 0x00);
			write_cmos_sensor(0x20, 0x01); //add for software reset
			write_cmos_sensor(0x20, 0x00);
			mdelay(5); 
			write_cmos_sensor(0xfd, 0x00);
			write_cmos_sensor(0x2e, 0x11);
			write_cmos_sensor(0x2f, 0x03);
			write_cmos_sensor(0x30, 0x13);
			write_cmos_sensor(0x34, 0x02);
			write_cmos_sensor(0xfd, 0x01);
			write_cmos_sensor(0x03, 0x02);
			write_cmos_sensor(0x04, 0xe0);
			write_cmos_sensor(0x06, 0x01);
			write_cmos_sensor(0x24, 0x80);
			write_cmos_sensor(0x39, 0x0d);
			write_cmos_sensor(0x3f, 0x02);
			write_cmos_sensor(0x31, 0x04);
			write_cmos_sensor(0x01, 0x01);
			write_cmos_sensor(0x11, 0x60);
			write_cmos_sensor(0x33, 0xb0);
			write_cmos_sensor(0x12, 0x03);
			write_cmos_sensor(0x13, 0xd0);
			write_cmos_sensor(0x45, 0x1e);
			write_cmos_sensor(0x16, 0xea);
			write_cmos_sensor(0x19, 0xf7);
			write_cmos_sensor(0x1a, 0x5f);
			write_cmos_sensor(0x1c, 0x0c);
			write_cmos_sensor(0x1d, 0x06);
			write_cmos_sensor(0x1e, 0x09);
			write_cmos_sensor(0x1f, 0x0b);
			write_cmos_sensor(0x20, 0x07);
			write_cmos_sensor(0x2a, 0x0f);
			write_cmos_sensor(0x2c, 0x10);
			write_cmos_sensor(0x25, 0x0d);
			write_cmos_sensor(0x26, 0x0d);
			write_cmos_sensor(0x27, 0x08);
			write_cmos_sensor(0x29, 0x01);
			write_cmos_sensor(0x2d, 0x06);
			write_cmos_sensor(0x55, 0x14);
			write_cmos_sensor(0x56, 0x00);
			write_cmos_sensor(0x57, 0x17);
			write_cmos_sensor(0x59, 0x00);
			write_cmos_sensor(0x5a, 0x04);
			write_cmos_sensor(0x50, 0x10);
			write_cmos_sensor(0x53, 0x0e);
			write_cmos_sensor(0x6b, 0x10);
			write_cmos_sensor(0x5c, 0x20);
			write_cmos_sensor(0x5d, 0x00);
			write_cmos_sensor(0x5e, 0x06);
			write_cmos_sensor(0x66, 0x38);
			write_cmos_sensor(0x68, 0x30);
			write_cmos_sensor(0x71, 0x3f);
			write_cmos_sensor(0x72, 0x05);
			write_cmos_sensor(0x73, 0x3e);
			write_cmos_sensor(0x81, 0x22);
			write_cmos_sensor(0x8a, 0x66);
			write_cmos_sensor(0x8b, 0x66);
			write_cmos_sensor(0xc0, 0x02);
			write_cmos_sensor(0xc1, 0x02);
			write_cmos_sensor(0xc2, 0x02);
			write_cmos_sensor(0xc3, 0x02);
			write_cmos_sensor(0xc4, 0x82);
			write_cmos_sensor(0xc5, 0x82);
			write_cmos_sensor(0xc6, 0x82);
			write_cmos_sensor(0xc7, 0x82);
			write_cmos_sensor(0xfb, 0x43);
			write_cmos_sensor(0xf0, 0x27);
			write_cmos_sensor(0xf1, 0x27);
			write_cmos_sensor(0xf2, 0x27);
			write_cmos_sensor(0xf3, 0x27);
			write_cmos_sensor(0xb1, 0xad);
			write_cmos_sensor(0xb6, 0x42);
			write_cmos_sensor(0xa1, 0x01);
			write_cmos_sensor(0xfd, 0x02);
			write_cmos_sensor(0x34, 0xc8);
			write_cmos_sensor(0x60, 0x99);
			write_cmos_sensor(0x93, 0x03);
			write_cmos_sensor(0x14, 0x01);
			write_cmos_sensor(0x15, 0x1e);
			write_cmos_sensor(0x16, 0x01);
			write_cmos_sensor(0x17, 0x1e);
			write_cmos_sensor(0x18, 0xe0);
			write_cmos_sensor(0x19, 0xa0);
			write_cmos_sensor(0xfd, 0x04);
			write_cmos_sensor(0x31, 0x4b);
			write_cmos_sensor(0x32, 0x4b);
			write_cmos_sensor(0xfd, 0x03);
			write_cmos_sensor(0xc0, 0x00);
			write_cmos_sensor(0xfd, 0x02);
			write_cmos_sensor(0xa0, 0x00);
			write_cmos_sensor(0xa1, 0xfa);
			write_cmos_sensor(0xa2, 0x01);
			write_cmos_sensor(0xa3, 0xe0);
			write_cmos_sensor(0xa4, 0x01);
			write_cmos_sensor(0xa5, 0x4c);
			write_cmos_sensor(0xa6, 0x02);
			write_cmos_sensor(0xa7, 0x80);
			write_cmos_sensor(0xfd, 0x01);
			write_cmos_sensor(0x8e, 0x02);
			write_cmos_sensor(0x8f, 0x80);
			write_cmos_sensor(0x90, 0x01);
			write_cmos_sensor(0x91, 0xe0);
			write_cmos_sensor(0xfd, 0x01);

			// write_cmos_sensor(0xfd,0x01);
			// write_cmos_sensor(0xa0,0x01);
}
*/
/*************************************************************************
* FUNCTION
*	 get_imgsensor_id
*
* DESCRIPTION
*	 This function get the sensor ID
*
* PARAMETERS
*	 *sensorID : return the sensor ID
*
* RETURNS
*	 None
*
* GLOBALS AFFECTED
*
*************************************************************************/

static kal_uint32 get_imgsensor_id(UINT32 *sensor_id)
{

	kal_uint8 i = 0;
	kal_uint8 retry = 2;
	LOG_1;
	LOG_2;
	//sensor have two i2c address 0x78 & 0x7a, we should detect the module used i2c address
	while (imgsensor_info.i2c_addr_table[i] != 0xff) {
		spin_lock(&imgsensor_drv_lock);
		imgsensor.i2c_write_id = imgsensor_info.i2c_addr_table[i];
		spin_unlock(&imgsensor_drv_lock);
		do {
			*sensor_id = return_sensor_id();
			if (*sensor_id == imgsensor_info.sensor_id) {
				pr_debug(" superpix i2c write id: 0x%x, sensor id: 0x%x\n", imgsensor.i2c_write_id, *sensor_id);
				return ERROR_NONE;
			}
			pr_debug(" superpix Read sensor id fail, i2c_write_id: 0x%x, id: 0x%x\n", imgsensor.i2c_write_id, *sensor_id);
			retry--;
		}  while (retry > 0);
		i++;
		retry = 2;
	}
	if (*sensor_id != imgsensor_info.sensor_id) {
		// if Sensor ID is not correct, Must set *sensor_id to 0xFFFFFFFF
		*sensor_id = 0xFFFFFFFF;
		return ERROR_SENSOR_CONNECT_FAIL;
	}
	return ERROR_NONE;
}


/*************************************************************************
* FUNCTION
*	 open
*
* DESCRIPTION
*	 This function initialize the registers of CMOS sensor
*
* PARAMETERS
*	 None
*
* RETURNS
*	 None
*
* GLOBALS AFFECTED
*
*************************************************************************/
static kal_uint32 open(void)
{
	kal_uint8 i = 0;
	kal_uint8 retry = 2;
	kal_uint32 sensor_id = 0;
	LOG_1;
	LOG_2;

	//sensor have two i2c address 0x6c 0x6d & 0x21 0x20, we should detect the module used i2c address
	while (imgsensor_info.i2c_addr_table[i] != 0xff) {
		spin_lock(&imgsensor_drv_lock);
		imgsensor.i2c_write_id = imgsensor_info.i2c_addr_table[i];
		spin_unlock(&imgsensor_drv_lock);
		do {
			sensor_id = return_sensor_id();
			if (sensor_id == imgsensor_info.sensor_id) {
				pr_debug("superpix i2c write id: 0x%x, sensor id: 0x%x\n", imgsensor.i2c_write_id, sensor_id);
				break;
			}
			pr_debug("superpix Read sensor id fail, i2c_write_id:0x%x, id: 0x%x\n", imgsensor.i2c_write_id, sensor_id);
			retry--;
		}  while (retry > 0);
		i++;
		if (sensor_id == imgsensor_info.sensor_id) {
			break;
		}
		retry = 2;
	}
	if (imgsensor_info.sensor_id != sensor_id) {
		return ERROR_SENSOR_CONNECT_FAIL;
	}
	// #ifdef DEBUG_SENSOR	 //
	//   	// struct file *fp;
	// 	 mm_segment_t fs;
	// 	//  loff_t pos = 0;
	// 	//  static char buf[10*1024] ;

	// 	struct file *fp = filp_open("/mnt/sdcard/OV05A10TXDPD2216FRONT_sd", O_RDONLY, 0);
	// 	  if (IS_ERR(fp)) {
	// 		 fromsd = 0;
	// 		 printk("gpww open file error\n");
	// 		 //return 0;
	// 	} else {
	// 		fromsd = 1;
	// 		  //OV05A10TXDPD2216FRONTMIPI_Initialize_from_T_Flash();
	// 		  printk("gpww read ok!\n");

	// 		   filp_close(fp, NULL);
	// 			  set_fs(fs);
	// 	    }
	// #endif

	/* initail sequence write in  */
	sensor_init();

	spin_lock(&imgsensor_drv_lock);

	imgsensor.autoflicker_en = KAL_TRUE;
	imgsensor.sensor_mode = IMGSENSOR_MODE_INIT;
	imgsensor.pclk = imgsensor_info.pre.pclk;
	imgsensor.frame_length = imgsensor_info.pre.framelength;
	imgsensor.line_length = imgsensor_info.pre.linelength;
	imgsensor.min_frame_length = imgsensor_info.pre.framelength;
	imgsensor.dummy_pixel = 0;
	imgsensor.dummy_line = 0;
	imgsensor.ihdr_en = 0;
	imgsensor.test_pattern = KAL_FALSE;
	imgsensor.current_fps = imgsensor_info.pre.max_framerate;
	spin_unlock(&imgsensor_drv_lock);

	return ERROR_NONE;
} 	 /*	   open	 */



/*************************************************************************
* FUNCTION
*	 close
*
* DESCRIPTION
*
*
* PARAMETERS
*	 None
*
* RETURNS
*	 None
*
* GLOBALS AFFECTED
*
*************************************************************************/
static kal_uint32 close(void)
{
	LOG_INF("E\n");

	/*No Need to implement this function*/

	return ERROR_NONE;
} 	 /*	   close  */

// static void set_mirror_flip(kal_uint8 image_mirror)
// {
// 	LOG_INF("image_mirror = %d\n", image_mirror);

// 	switch (image_mirror) {
// 	case IMAGE_NORMAL: //off off
// 		write_cmos_sensor(0x3f, 0x00);
// 		break;
// 	case IMAGE_H_MIRROR: //on off
// 		write_cmos_sensor(0x3f, 0x01);
// 		break;
// 	case IMAGE_V_MIRROR: //off on
// 		write_cmos_sensor(0x3f, 0x02);
// 		break;
// 	case IMAGE_HV_MIRROR: //on on
// 		write_cmos_sensor(0x3f, 0x03);
// 		break;
// 	default:
// 		LOG_INF("Error image_mirror setting");
// 	}

// }

/*************************************************************************
* FUNCTION
* preview
*
* DESCRIPTION
*	 This function start the sensor preview.
*
* PARAMETERS
*	 *image_window : address pointer of pixel numbers in one period of HSYNC
*  *sensor_config_data : address pointer of line numbers in one period of VSYNC
*
* RETURNS
*	 None
*
* GLOBALS AFFECTED
*
*************************************************************************/
static kal_uint32 preview(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
					  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("E\n");

	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_PREVIEW;
	imgsensor.pclk = imgsensor_info.pre.pclk;
	//imgsensor.video_mode = KAL_FALSE;
	imgsensor.line_length = imgsensor_info.pre.linelength;
	imgsensor.frame_length = imgsensor_info.pre.framelength;
	imgsensor.min_frame_length = imgsensor_info.pre.framelength;
	imgsensor.current_fps = imgsensor_info.pre.max_framerate;
	imgsensor.autoflicker_en = KAL_TRUE;
	imgsensor.dummy_line = 0;
	imgsensor.dummy_pixel = 0;
	spin_unlock(&imgsensor_drv_lock);	
	preview_setting();
	//set_mirror_flip(sensor_config_data->SensorImageMirror);
	return ERROR_NONE;
} 	 /*	   preview	 */

/*************************************************************************
* FUNCTION
*	 capture
*
* DESCRIPTION
*	 This function setup the CMOS sensor in capture MY_OUTPUT mode
*
* PARAMETERS
*
* RETURNS
*	 None
*
* GLOBALS AFFECTED
*
*************************************************************************/
static kal_uint32 capture(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
						  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("E\n");

	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_CAPTURE;
	imgsensor.pclk = imgsensor_info.cap.pclk;
	imgsensor.line_length = imgsensor_info.cap.linelength;
	imgsensor.frame_length = imgsensor_info.cap.framelength;
	imgsensor.min_frame_length = imgsensor_info.cap.framelength;
	imgsensor.current_fps = imgsensor_info.cap.max_framerate;

	imgsensor.autoflicker_en = KAL_TRUE;
	imgsensor.dummy_line = 0;
	imgsensor.dummy_pixel = 0;
	spin_unlock(&imgsensor_drv_lock);	

	capture_setting();
	//set_mirror_flip(sensor_config_data->SensorImageMirror);
	return ERROR_NONE;
} 	 /* capture() */
static kal_uint32 normal_video(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
					  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("E\n");

	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_VIDEO;
	imgsensor.pclk = imgsensor_info.normal_video.pclk;
	imgsensor.line_length = imgsensor_info.normal_video.linelength;
	imgsensor.frame_length = imgsensor_info.normal_video.framelength;
	imgsensor.min_frame_length = imgsensor_info.normal_video.framelength;
	imgsensor.current_fps = imgsensor_info.normal_video.max_framerate;
	imgsensor.dummy_line = 0;
	imgsensor.dummy_pixel = 0;
	imgsensor.autoflicker_en = KAL_TRUE;
	spin_unlock(&imgsensor_drv_lock);
	normal_video_setting();
	//set_mirror_flip(sensor_config_data->SensorImageMirror);
	return ERROR_NONE;
} 	 /*	   normal_video	  */

static kal_uint32 hs_video(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
					  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("E\n");

	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_HIGH_SPEED_VIDEO;
	imgsensor.pclk = imgsensor_info.hs_video.pclk;
	//imgsensor.video_mode = KAL_TRUE;
	imgsensor.line_length = imgsensor_info.hs_video.linelength;
	imgsensor.frame_length = imgsensor_info.hs_video.framelength;
	imgsensor.min_frame_length = imgsensor_info.hs_video.framelength;
	imgsensor.current_fps = imgsensor_info.hs_video.max_framerate;
	imgsensor.dummy_line = 0;
	imgsensor.dummy_pixel = 0;
	imgsensor.autoflicker_en = KAL_TRUE;
	spin_unlock(&imgsensor_drv_lock);
	hs_video_setting();
	//set_mirror_flip(sensor_config_data->SensorImageMirror);
	return ERROR_NONE;
} 	 /*	   hs_video	  */

static kal_uint32 slim_video(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
					  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("E\n");

	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_SLIM_VIDEO;
	imgsensor.pclk = imgsensor_info.slim_video.pclk;
	imgsensor.line_length = imgsensor_info.slim_video.linelength;
	imgsensor.frame_length = imgsensor_info.slim_video.framelength;
	imgsensor.min_frame_length = imgsensor_info.slim_video.framelength;
	imgsensor.current_fps = imgsensor_info.slim_video.max_framerate;
	imgsensor.dummy_line = 0;
	imgsensor.dummy_pixel = 0;
	imgsensor.autoflicker_en = KAL_TRUE;
	spin_unlock(&imgsensor_drv_lock);
	slim_video_setting();
	// set_mirror_flip(sensor_config_data->SensorImageMirror);

	return ERROR_NONE;
} 	 /*	   slim_video	  */
/*
static kal_uint32 custom1(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
	MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("E\n");

	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_CUSTOM1;
	imgsensor.pclk = imgsensor_info.custom1.pclk;
	imgsensor.line_length = imgsensor_info.custom1.linelength;
	imgsensor.frame_length = imgsensor_info.custom1.framelength;
	imgsensor.min_frame_length = imgsensor_info.custom1.framelength;
	imgsensor.current_fps = imgsensor_info.custom1.max_framerate;
	imgsensor.dummy_line = 0;
	imgsensor.dummy_pixel = 0;
	imgsensor.autoflicker_en = KAL_TRUE;
	spin_unlock(&imgsensor_drv_lock);
	custom1_setting();
	return ERROR_NONE;
}
static kal_uint32 custom2(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
	MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("E\n");

	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_CUSTOM2;
	imgsensor.pclk = imgsensor_info.custom2.pclk;
	imgsensor.line_length = imgsensor_info.custom2.linelength;
	imgsensor.frame_length = imgsensor_info.custom2.framelength;
	imgsensor.min_frame_length = imgsensor_info.custom2.framelength;
	imgsensor.current_fps = imgsensor_info.custom2.max_framerate;
	imgsensor.dummy_line = 0;
	imgsensor.dummy_pixel = 0;
	imgsensor.autoflicker_en = KAL_TRUE;
	spin_unlock(&imgsensor_drv_lock);
	custom2_setting();
	return ERROR_NONE;
}
static kal_uint32 custom3(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
	MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("E\n");

	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_CUSTOM3;
	imgsensor.pclk = imgsensor_info.custom3.pclk;
	imgsensor.line_length = imgsensor_info.custom3.linelength;
	imgsensor.frame_length = imgsensor_info.custom3.framelength;
	imgsensor.min_frame_length = imgsensor_info.custom3.framelength;
	imgsensor.current_fps = imgsensor_info.custom3.max_framerate;
	imgsensor.dummy_line = 0;
	imgsensor.dummy_pixel = 0;
	imgsensor.autoflicker_en = KAL_TRUE;
	spin_unlock(&imgsensor_drv_lock);
	custom3_setting();
	return ERROR_NONE;
}
static kal_uint32 custom4(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
	MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("E\n");

	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_CUSTOM4;
	imgsensor.pclk = imgsensor_info.custom4.pclk;
	imgsensor.line_length = imgsensor_info.custom4.linelength;
	imgsensor.frame_length = imgsensor_info.custom4.framelength;
	imgsensor.min_frame_length = imgsensor_info.custom4.framelength;
	imgsensor.current_fps = imgsensor_info.custom4.max_framerate;
	imgsensor.dummy_line = 0;
	imgsensor.dummy_pixel = 0;
	imgsensor.autoflicker_en = KAL_TRUE;
	spin_unlock(&imgsensor_drv_lock);
	custom4_setting();
	return ERROR_NONE;
}
static kal_uint32 custom5(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
	MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("E\n");

	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_CUSTOM5;
	imgsensor.pclk = imgsensor_info.custom5.pclk;
	imgsensor.line_length = imgsensor_info.custom5.linelength;
	imgsensor.frame_length = imgsensor_info.custom5.framelength;
	imgsensor.min_frame_length = imgsensor_info.custom5.framelength;
	imgsensor.current_fps = imgsensor_info.custom5.max_framerate;
	imgsensor.dummy_line = 0;
	imgsensor.dummy_pixel = 0;
	imgsensor.autoflicker_en = KAL_TRUE;
	spin_unlock(&imgsensor_drv_lock);
	custom5_setting();
	return ERROR_NONE;
}
*/
static kal_uint32 get_resolution(MSDK_SENSOR_RESOLUTION_INFO_STRUCT *sensor_resolution)
{
	LOG_INF("E\n");
	sensor_resolution->SensorFullWidth = imgsensor_info.cap.grabwindow_width;
	sensor_resolution->SensorFullHeight = imgsensor_info.cap.grabwindow_height;

	sensor_resolution->SensorPreviewWidth = imgsensor_info.pre.grabwindow_width;
	sensor_resolution->SensorPreviewHeight = imgsensor_info.pre.grabwindow_height;

	sensor_resolution->SensorVideoWidth = imgsensor_info.normal_video.grabwindow_width;
	sensor_resolution->SensorVideoHeight = imgsensor_info.normal_video.grabwindow_height;


	sensor_resolution->SensorHighSpeedVideoWidth = imgsensor_info.hs_video.grabwindow_width;
	sensor_resolution->SensorHighSpeedVideoHeight = imgsensor_info.hs_video.grabwindow_height;

	sensor_resolution->SensorSlimVideoWidth = imgsensor_info.slim_video.grabwindow_width;
	sensor_resolution->SensorSlimVideoHeight = imgsensor_info.slim_video.grabwindow_height;
/*
	sensor_resolution->SensorCustom1Width = imgsensor_info.custom1.grabwindow_width;
	sensor_resolution->SensorCustom1Height = imgsensor_info.custom1.grabwindow_height;

	sensor_resolution->SensorCustom2Width = imgsensor_info.custom2.grabwindow_width;
	sensor_resolution->SensorCustom2Height = imgsensor_info.custom2.grabwindow_height;

	sensor_resolution->SensorCustom3Width = imgsensor_info.custom3.grabwindow_width;
	sensor_resolution->SensorCustom3Height = imgsensor_info.custom3.grabwindow_height;

	sensor_resolution->SensorCustom4Width = imgsensor_info.custom4.grabwindow_width;
	sensor_resolution->SensorCustom4Height = imgsensor_info.custom4.grabwindow_height;

	sensor_resolution->SensorCustom5Width = imgsensor_info.custom5.grabwindow_width;
	sensor_resolution->SensorCustom5Height = imgsensor_info.custom5.grabwindow_height;
*/
	return ERROR_NONE;
}    /*    get_resolution    */

static kal_uint32 get_info(enum MSDK_SCENARIO_ID_ENUM scenario_id,
					  MSDK_SENSOR_INFO_STRUCT *sensor_info,
					  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("scenario_id = %d\n", scenario_id);


	//sensor_info->SensorVideoFrameRate = imgsensor_info.normal_video.max_framerate/10; /* not use */
	//sensor_info->SensorStillCaptureFrameRate= imgsensor_info.cap.max_framerate/10; /* not use */
	//imgsensor_info->SensorWebCamCaptureFrameRate= imgsensor_info.v.max_framerate; /* not use */

	sensor_info->SensorClockPolarity = SENSOR_CLOCK_POLARITY_LOW;
	sensor_info->SensorClockFallingPolarity = SENSOR_CLOCK_POLARITY_LOW; /* not use */
	sensor_info->SensorHsyncPolarity = SENSOR_CLOCK_POLARITY_LOW; // inverse with datasheet
	sensor_info->SensorVsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
	sensor_info->SensorInterruptDelayLines = 4; /* not use */
	sensor_info->SensorResetActiveHigh = FALSE; /* not use */
	sensor_info->SensorResetDelayCount = 5; /* not use */

	sensor_info->SensroInterfaceType = imgsensor_info.sensor_interface_type;
	sensor_info->MIPIsensorType = imgsensor_info.mipi_sensor_type;
	sensor_info->SettleDelayMode = imgsensor_info.mipi_settle_delay_mode;
	sensor_info->SensorOutputDataFormat = imgsensor_info.sensor_output_dataformat;

	sensor_info->CaptureDelayFrame = imgsensor_info.cap_delay_frame;
	sensor_info->PreviewDelayFrame = imgsensor_info.pre_delay_frame;
	sensor_info->VideoDelayFrame = imgsensor_info.video_delay_frame;
	sensor_info->HighSpeedVideoDelayFrame = imgsensor_info.hs_video_delay_frame;
	sensor_info->SlimVideoDelayFrame = imgsensor_info.slim_video_delay_frame;
/*
	sensor_info->Custom1DelayFrame = imgsensor_info.custom1_delay_frame;
	sensor_info->Custom2DelayFrame = imgsensor_info.custom2_delay_frame;
	sensor_info->Custom3DelayFrame = imgsensor_info.custom3_delay_frame;
	sensor_info->Custom4DelayFrame = imgsensor_info.custom4_delay_frame;
	sensor_info->Custom5DelayFrame = imgsensor_info.custom5_delay_frame;
*/
	sensor_info->SensorMasterClockSwitch = 0; /* not use */
	sensor_info->SensorDrivingCurrent = imgsensor_info.isp_driving_current;

	sensor_info->AEShutDelayFrame = imgsensor_info.ae_shut_delay_frame;			 /* The frame of setting shutter default 0 for TG int */
	sensor_info->AESensorGainDelayFrame = imgsensor_info.ae_sensor_gain_delay_frame;	/* The frame of setting sensor gain */
	sensor_info->AEISPGainDelayFrame = imgsensor_info.ae_ispGain_delay_frame;
	sensor_info->IHDR_Support = imgsensor_info.ihdr_support;
	sensor_info->IHDR_LE_FirstLine = imgsensor_info.ihdr_le_firstline;
	sensor_info->SensorModeNum = imgsensor_info.sensor_mode_num;

	sensor_info->SensorMIPILaneNumber = imgsensor_info.mipi_lane_num;
	sensor_info->SensorClockFreq = imgsensor_info.mclk;
	sensor_info->SensorClockDividCount = 3; /* not use */
	sensor_info->SensorClockRisingCount = 0;
	sensor_info->SensorClockFallingCount = 2; /* not use */
	sensor_info->SensorPixelClockCount = 3; /* not use */
	sensor_info->SensorDataLatchCount = 2; /* not use */

	sensor_info->MIPIDataLowPwr2HighSpeedTermDelayCount = 0;
	sensor_info->MIPICLKLowPwr2HighSpeedTermDelayCount = 0;
	sensor_info->SensorWidthSampling = 0;  // 0 is default 1x
	sensor_info->SensorHightSampling = 0;	 // 0 is default 1x
	sensor_info->SensorPacketECCOrder = 1;

	switch (scenario_id) {
	case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
			sensor_info->SensorGrabStartX = imgsensor_info.pre.startx;
			sensor_info->SensorGrabStartY = imgsensor_info.pre.starty;

			sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount = imgsensor_info.pre.mipi_data_lp2hs_settle_dc;

			break;
	case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			sensor_info->SensorGrabStartX = imgsensor_info.cap.startx;
			sensor_info->SensorGrabStartY = imgsensor_info.cap.starty;

			sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount = imgsensor_info.cap.mipi_data_lp2hs_settle_dc;

			break;
	case MSDK_SCENARIO_ID_VIDEO_PREVIEW:

			sensor_info->SensorGrabStartX = imgsensor_info.normal_video.startx;
			sensor_info->SensorGrabStartY = imgsensor_info.normal_video.starty;

			sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount = imgsensor_info.normal_video.mipi_data_lp2hs_settle_dc;

			break;
	case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
			sensor_info->SensorGrabStartX = imgsensor_info.hs_video.startx;
			sensor_info->SensorGrabStartY = imgsensor_info.hs_video.starty;

			sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount = imgsensor_info.hs_video.mipi_data_lp2hs_settle_dc;

			break;
	case MSDK_SCENARIO_ID_SLIM_VIDEO:
			sensor_info->SensorGrabStartX = imgsensor_info.slim_video.startx;
			sensor_info->SensorGrabStartY = imgsensor_info.slim_video.starty;

			sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount = imgsensor_info.slim_video.mipi_data_lp2hs_settle_dc;

			break;
/*
	case MSDK_SCENARIO_ID_CUSTOM1:
	    	sensor_info->SensorGrabStartX = imgsensor_info.custom1.startx;
	    	sensor_info->SensorGrabStartY = imgsensor_info.custom1.starty;

	    	sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount = imgsensor_info.custom1.mipi_data_lp2hs_settle_dc;

			break;
	case MSDK_SCENARIO_ID_CUSTOM2:
	    sensor_info->SensorGrabStartX = imgsensor_info.custom2.startx;
	    sensor_info->SensorGrabStartY = imgsensor_info.custom2.starty;

	    sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount = imgsensor_info.custom2.mipi_data_lp2hs_settle_dc;

			break;
	case MSDK_SCENARIO_ID_CUSTOM3:
	    	sensor_info->SensorGrabStartX = imgsensor_info.custom3.startx;
	    	sensor_info->SensorGrabStartY = imgsensor_info.custom3.starty;

	    	sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount = imgsensor_info.custom3.mipi_data_lp2hs_settle_dc;

			break;
	case MSDK_SCENARIO_ID_CUSTOM4:
	    	sensor_info->SensorGrabStartX = imgsensor_info.custom4.startx;
	    	sensor_info->SensorGrabStartY = imgsensor_info.custom4.starty;

	    	sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount = imgsensor_info.custom4.mipi_data_lp2hs_settle_dc;

			break;
	case MSDK_SCENARIO_ID_CUSTOM5:
	    	sensor_info->SensorGrabStartX = imgsensor_info.custom5.startx;
	    	sensor_info->SensorGrabStartY = imgsensor_info.custom5.starty;

	    	sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount = imgsensor_info.custom5.mipi_data_lp2hs_settle_dc;

			break;
*/
	default:
			sensor_info->SensorGrabStartX = imgsensor_info.pre.startx;
			sensor_info->SensorGrabStartY = imgsensor_info.pre.starty;

			sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount = imgsensor_info.pre.mipi_data_lp2hs_settle_dc;
			break;
	}

	return ERROR_NONE;
} 	 /*	   get_info	 */


static kal_uint32 control(enum MSDK_SCENARIO_ID_ENUM scenario_id, MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
					  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("scenario_id = %d\n", scenario_id);
	spin_lock(&imgsensor_drv_lock);
	imgsensor.current_scenario_id = scenario_id;
	spin_unlock(&imgsensor_drv_lock);
	switch (scenario_id) {
	case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
			preview(image_window, sensor_config_data);
			break;
	case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			capture(image_window, sensor_config_data);
			break;
	case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			normal_video(image_window, sensor_config_data);
			break;
	case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
			hs_video(image_window, sensor_config_data);
			break;
	case MSDK_SCENARIO_ID_SLIM_VIDEO:
			slim_video(image_window, sensor_config_data);
			break;
/*
	case MSDK_SCENARIO_ID_CUSTOM1:
			custom1(image_window, sensor_config_data);
			break;
	case MSDK_SCENARIO_ID_CUSTOM2:
			custom2(image_window, sensor_config_data);
			break;
	case MSDK_SCENARIO_ID_CUSTOM3:
			custom3(image_window, sensor_config_data);
			break;
	case MSDK_SCENARIO_ID_CUSTOM4:
			custom4(image_window, sensor_config_data);
			break;
	case MSDK_SCENARIO_ID_CUSTOM5:
			custom5(image_window, sensor_config_data);
			break;
*/
	default:
			LOG_INF("Error ScenarioId setting");
			preview(image_window, sensor_config_data);
			return ERROR_INVALID_SCENARIO_ID;
	}
	return ERROR_NONE;
} 	 /* control() */



static kal_uint32 set_video_mode(UINT16 framerate)
{//This Function not used after ROME
	LOG_INF("framerate = %d\n ", framerate);
	// SetVideoMode Function should fix framerate
	if (framerate == 0) {
		// Dynamic frame rate
		return ERROR_NONE;
	}
	spin_lock(&imgsensor_drv_lock);
	if ((framerate == 300) && (imgsensor.autoflicker_en == KAL_TRUE)) {
		imgsensor.current_fps = 296;
	} else if ((framerate == 150) && (imgsensor.autoflicker_en == KAL_TRUE)) {
		imgsensor.current_fps = 146;
	} else {
		imgsensor.current_fps = framerate;
	}
	spin_unlock(&imgsensor_drv_lock);
	set_max_framerate(imgsensor.current_fps, 1);

	return ERROR_NONE;
}

static kal_uint32 set_auto_flicker_mode(kal_bool enable, UINT16 framerate)
{
	LOG_INF("enable = %d, framerate = %d \n", enable, framerate);
	spin_lock(&imgsensor_drv_lock);
	if (enable) { //enable auto flicker
		imgsensor.autoflicker_en = KAL_TRUE;
	} else { //Cancel Auto flick
		imgsensor.autoflicker_en = KAL_FALSE;
	}
	spin_unlock(&imgsensor_drv_lock);
	return ERROR_NONE;
}


static kal_uint32 set_max_framerate_by_scenario(enum MSDK_SCENARIO_ID_ENUM scenario_id, MUINT32 framerate)
{
	kal_uint32 frame_length;

	LOG_INF("scenario_id = %d, framerate = %d\n", scenario_id, framerate);

	switch (scenario_id) {
	case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
			frame_length = imgsensor_info.pre.pclk / framerate * 10 / imgsensor_info.pre.linelength;
			spin_lock(&imgsensor_drv_lock);
			imgsensor.dummy_line = (frame_length > imgsensor_info.pre.framelength) ? (frame_length - imgsensor_info.pre.framelength) : 0;
			imgsensor.frame_length = imgsensor_info.pre.framelength + imgsensor.dummy_line;
			imgsensor.min_frame_length = imgsensor.frame_length;
			spin_unlock(&imgsensor_drv_lock);
			set_dummy();
			break;
	case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			if (framerate == 0) {
				return ERROR_NONE;
			}
			frame_length = imgsensor_info.normal_video.pclk / framerate * 10 / imgsensor_info.normal_video.linelength;
			spin_lock(&imgsensor_drv_lock);
			imgsensor.dummy_line = (frame_length > imgsensor_info.normal_video.framelength) ? (frame_length - imgsensor_info.normal_video.framelength) : 0;
			imgsensor.frame_length = imgsensor_info.normal_video.framelength + imgsensor.dummy_line;
			imgsensor.min_frame_length = imgsensor.frame_length;
			spin_unlock(&imgsensor_drv_lock);
			set_dummy();
			break;
	case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			frame_length = imgsensor_info.cap.pclk / framerate * 10 / imgsensor_info.cap.linelength;
			spin_lock(&imgsensor_drv_lock);
			imgsensor.dummy_line = (frame_length > imgsensor_info.cap.framelength) ? (frame_length - imgsensor_info.cap.framelength) : 0;
			imgsensor.frame_length = imgsensor_info.cap.framelength + imgsensor.dummy_line;
			imgsensor.min_frame_length = imgsensor.frame_length;
			spin_unlock(&imgsensor_drv_lock);
			set_dummy();
			break;
	case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
			frame_length = imgsensor_info.hs_video.pclk / framerate * 10 / imgsensor_info.hs_video.linelength;
			spin_lock(&imgsensor_drv_lock);
			imgsensor.dummy_line = (frame_length > imgsensor_info.hs_video.framelength) ? (frame_length - imgsensor_info.hs_video.framelength) : 0;
			imgsensor.frame_length = imgsensor_info.hs_video.framelength + imgsensor.dummy_line;
			imgsensor.min_frame_length = imgsensor.frame_length;
			spin_unlock(&imgsensor_drv_lock);
			set_dummy();
			break;
	case MSDK_SCENARIO_ID_SLIM_VIDEO:
			frame_length = imgsensor_info.slim_video.pclk / framerate * 10 / imgsensor_info.slim_video.linelength;
			spin_lock(&imgsensor_drv_lock);
			imgsensor.dummy_line = (frame_length > imgsensor_info.slim_video.framelength) ? (frame_length - imgsensor_info.slim_video.framelength) : 0;
			imgsensor.frame_length = imgsensor_info.slim_video.framelength + imgsensor.dummy_line;
			imgsensor.min_frame_length = imgsensor.frame_length;
			spin_unlock(&imgsensor_drv_lock);
			set_dummy();
			break;
/*
	case MSDK_SCENARIO_ID_CUSTOM1:
			frame_length = imgsensor_info.custom1.pclk / framerate * 10 / imgsensor_info.custom1.linelength;
			spin_lock(&imgsensor_drv_lock);
			imgsensor.dummy_line = (frame_length > imgsensor_info.custom1.framelength) ? (frame_length - imgsensor_info.custom1.framelength) : 0;
			imgsensor.frame_length = imgsensor_info.custom1.framelength + imgsensor.dummy_line;
			imgsensor.min_frame_length = imgsensor.frame_length;
			spin_unlock(&imgsensor_drv_lock);
			set_dummy();
			break;
	case MSDK_SCENARIO_ID_CUSTOM2:
			frame_length = imgsensor_info.custom2.pclk / framerate * 10 / imgsensor_info.custom2.linelength;
			spin_lock(&imgsensor_drv_lock);
			imgsensor.dummy_line = (frame_length > imgsensor_info.custom2.framelength) ? (frame_length - imgsensor_info.custom2.framelength) : 0;
			imgsensor.frame_length = imgsensor_info.custom2.framelength + imgsensor.dummy_line;
			imgsensor.min_frame_length = imgsensor.frame_length;
			spin_unlock(&imgsensor_drv_lock);
			set_dummy();
			break;
	case MSDK_SCENARIO_ID_CUSTOM3:
			frame_length = imgsensor_info.custom3.pclk / framerate * 10 / imgsensor_info.custom3.linelength;
			spin_lock(&imgsensor_drv_lock);
			imgsensor.dummy_line = (frame_length > imgsensor_info.custom3.framelength) ? (frame_length - imgsensor_info.custom3.framelength) : 0;
			imgsensor.frame_length = imgsensor_info.custom3.framelength + imgsensor.dummy_line;
			imgsensor.min_frame_length = imgsensor.frame_length;
			spin_unlock(&imgsensor_drv_lock);
			set_dummy();
			break;
	case MSDK_SCENARIO_ID_CUSTOM4:
			frame_length = imgsensor_info.custom4.pclk / framerate * 10 / imgsensor_info.custom4.linelength;
			spin_lock(&imgsensor_drv_lock);
			imgsensor.dummy_line = (frame_length > imgsensor_info.custom4.framelength) ? (frame_length - imgsensor_info.custom4.framelength) : 0;
			imgsensor.frame_length = imgsensor_info.custom4.framelength + imgsensor.dummy_line;
			imgsensor.min_frame_length = imgsensor.frame_length;
			spin_unlock(&imgsensor_drv_lock);
			set_dummy();
			break;
	case MSDK_SCENARIO_ID_CUSTOM5:
			frame_length = imgsensor_info.custom5.pclk / framerate * 10 / imgsensor_info.custom5.linelength;
			spin_lock(&imgsensor_drv_lock);
			imgsensor.dummy_line = (frame_length > imgsensor_info.custom5.framelength) ? (frame_length - imgsensor_info.custom5.framelength) : 0;
			imgsensor.frame_length = imgsensor_info.custom5.framelength + imgsensor.dummy_line;
			imgsensor.min_frame_length = imgsensor.frame_length;
			spin_unlock(&imgsensor_drv_lock);
			set_dummy();
			break;
*/
	default:  //coding with	 preview scenario by default
			frame_length = imgsensor_info.pre.pclk / framerate * 10 / imgsensor_info.pre.linelength;
			spin_lock(&imgsensor_drv_lock);
			imgsensor.dummy_line = (frame_length > imgsensor_info.pre.framelength) ? (frame_length - imgsensor_info.pre.framelength) : 0;
			imgsensor.frame_length = imgsensor_info.pre.framelength + imgsensor.dummy_line;
			imgsensor.min_frame_length = imgsensor.frame_length;
			spin_unlock(&imgsensor_drv_lock);
			set_dummy();
			LOG_INF("error scenario_id = %d, we use preview scenario \n", scenario_id);
			break;
	}
	return ERROR_NONE;
}


static kal_uint32 get_default_framerate_by_scenario(enum MSDK_SCENARIO_ID_ENUM scenario_id, MUINT32 *framerate)
{
	LOG_INF("scenario_id = %d\n", scenario_id);

	switch (scenario_id) {
	case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
			*framerate = imgsensor_info.pre.max_framerate;
			break;
	case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			*framerate = imgsensor_info.normal_video.max_framerate;
			break;
	case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			*framerate = imgsensor_info.cap.max_framerate;
			break;
	case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
			*framerate = imgsensor_info.hs_video.max_framerate;
			break;
	case MSDK_SCENARIO_ID_SLIM_VIDEO:
			*framerate = imgsensor_info.slim_video.max_framerate;
			break;
/*
	case MSDK_SCENARIO_ID_CUSTOM1:
			*framerate = imgsensor_info.custom1.max_framerate;
			break;
	case MSDK_SCENARIO_ID_CUSTOM2:
			*framerate = imgsensor_info.custom2.max_framerate;
			break;
	case MSDK_SCENARIO_ID_CUSTOM3:
			*framerate = imgsensor_info.custom3.max_framerate;
			break;
	case MSDK_SCENARIO_ID_CUSTOM4:
			*framerate = imgsensor_info.custom4.max_framerate;
			break;
	case MSDK_SCENARIO_ID_CUSTOM5:
			*framerate = imgsensor_info.custom5.max_framerate;
			break;
*/
	default:
			break;
	}

	return ERROR_NONE;
}



static kal_uint32 set_test_pattern_mode(kal_bool enable)
{
	LOG_INF("enable: %d\n", enable);

	if (enable) {
		write_cmos_sensor(0xfd, 0x01);
		write_cmos_sensor(0xb6, 0x43);
	} else {
		write_cmos_sensor(0xfd, 0x01);
		write_cmos_sensor(0xb6, 0x42);
	}

		spin_lock(&imgsensor_drv_lock);
		imgsensor.test_pattern = enable;
		spin_unlock(&imgsensor_drv_lock);
		return ERROR_NONE;
}


static kal_uint32 feature_control(MSDK_SENSOR_FEATURE_ENUM feature_id,
							 UINT8 *feature_para, UINT32 *feature_para_len)
{
	UINT16 *feature_return_para_16 = (UINT16 *) feature_para;
	UINT16 *feature_data_16 = (UINT16 *) feature_para;
	UINT32 *feature_return_para_32 = (UINT32 *) feature_para;
	UINT32 *feature_data_32 = (UINT32 *) feature_para;
	unsigned long long *feature_data = (unsigned long long *) feature_para;
	//unsigned long long *feature_return_para = (unsigned long long *) feature_para;

/*add rawinfo start*/
/*	struct SENSOR_RAWINFO_STRUCT *rawinfo;*/
/*add rawinfo end*/

	struct SENSOR_WINSIZE_INFO_STRUCT *wininfo;
	MSDK_SENSOR_REG_INFO_STRUCT *sensor_reg_data = (MSDK_SENSOR_REG_INFO_STRUCT *) feature_para;

	//printk("feature_id = %d\n", feature_id);
	switch (feature_id) {
	case SENSOR_FEATURE_GET_PERIOD:
			*feature_return_para_16++ = imgsensor.line_length;
			*feature_return_para_16 = imgsensor.frame_length;
			*feature_para_len = 4;
			break;
	case SENSOR_FEATURE_GET_PIXEL_RATE:
		switch (*feature_data) {
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) =
			(imgsensor_info.cap.pclk /
			(imgsensor_info.cap.linelength - 80))*
			imgsensor_info.cap.grabwindow_width;

			break;
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) =
			(imgsensor_info.normal_video.pclk /
			(imgsensor_info.normal_video.linelength - 80))*
			imgsensor_info.normal_video.grabwindow_width;

			break;
		case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) =
			(imgsensor_info.hs_video.pclk /
			(imgsensor_info.hs_video.linelength - 80))*
			imgsensor_info.hs_video.grabwindow_width;

			break;
		case MSDK_SCENARIO_ID_SLIM_VIDEO:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) =
			(imgsensor_info.slim_video.pclk /
			(imgsensor_info.slim_video.linelength - 80))*
			imgsensor_info.slim_video.grabwindow_width;

			break;
/*
		case MSDK_SCENARIO_ID_CUSTOM1:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) =
			(imgsensor_info.custom1.pclk /
			(imgsensor_info.custom1.linelength - 80))*
			imgsensor_info.custom1.grabwindow_width;

			break;
		case MSDK_SCENARIO_ID_CUSTOM2:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) =
			(imgsensor_info.custom2.pclk /
			(imgsensor_info.custom2.linelength - 80))*
			imgsensor_info.custom2.grabwindow_width;

			break;
		case MSDK_SCENARIO_ID_CUSTOM3:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) =
			(imgsensor_info.custom3.pclk /
			(imgsensor_info.custom3.linelength - 80))*
			imgsensor_info.custom3.grabwindow_width;

			break;
		case MSDK_SCENARIO_ID_CUSTOM4:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) =
			(imgsensor_info.custom4.pclk /
			(imgsensor_info.custom4.linelength - 80))*
			imgsensor_info.custom4.grabwindow_width;

			break;
		case MSDK_SCENARIO_ID_CUSTOM5:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) =
			(imgsensor_info.custom5.pclk /
			(imgsensor_info.custom5.linelength - 80))*
			imgsensor_info.custom5.grabwindow_width;

			break;
*/
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		default:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) =
			(imgsensor_info.pre.pclk /
			(imgsensor_info.pre.linelength - 80))*
			imgsensor_info.pre.grabwindow_width;
			break;
		}
		break;

	case SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ:
			LOG_INF("feature_Control imgsensor.pclk = %d, imgsensor.current_fps = %d\n", imgsensor.pclk, imgsensor.current_fps);
			*feature_return_para_32 = imgsensor.pclk;
			*feature_para_len = 4;
			break;
	case SENSOR_FEATURE_SET_ESHUTTER:
			set_shutter(*feature_data);
			break;
	case SENSOR_FEATURE_SET_NIGHTMODE:
			night_mode((BOOL)*feature_data);
			break;
	case SENSOR_FEATURE_SET_GAIN:
			set_gain((UINT16)*feature_data);
			break;
	case SENSOR_FEATURE_SET_FLASHLIGHT:
			break;
	case SENSOR_FEATURE_SET_ISP_MASTER_CLOCK_FREQ:
			break;
	case SENSOR_FEATURE_SET_REGISTER:
			write_cmos_sensor(sensor_reg_data->RegAddr, sensor_reg_data->RegData);
			break;
	case SENSOR_FEATURE_GET_REGISTER:
			sensor_reg_data->RegData = read_cmos_sensor(sensor_reg_data->RegAddr);
			break;
	case SENSOR_FEATURE_GET_LENS_DRIVER_ID:
			// get the lens driver ID from EEPROM or just return LENS_DRIVER_ID_DO_NOT_CARE
			// if EEPROM does not exist in camera module.
			*feature_return_para_32 = LENS_DRIVER_ID_DO_NOT_CARE;
			*feature_para_len = 4;
			break;
	case SENSOR_FEATURE_SET_VIDEO_MODE:
			set_video_mode(*feature_data);
			break;
	case SENSOR_FEATURE_CHECK_SENSOR_ID:
			get_imgsensor_id(feature_return_para_32);
			break;
	case SENSOR_FEATURE_SET_AUTO_FLICKER_MODE:
			set_auto_flicker_mode((BOOL)*feature_data_16, *(feature_data_16+1));
			break;
	case SENSOR_FEATURE_SET_MAX_FRAME_RATE_BY_SCENARIO:
			set_max_framerate_by_scenario((enum MSDK_SCENARIO_ID_ENUM)*feature_data, *(feature_data+1));
			break;
	case SENSOR_FEATURE_GET_DEFAULT_FRAME_RATE_BY_SCENARIO:
			get_default_framerate_by_scenario((enum MSDK_SCENARIO_ID_ENUM)*(feature_data), (MUINT32 *)(uintptr_t)(*(feature_data+1)));
			break;
	case SENSOR_FEATURE_SET_TEST_PATTERN:
			set_test_pattern_mode((BOOL)*feature_data);
			break;
	case SENSOR_FEATURE_GET_TEST_PATTERN_CHECKSUM_VALUE: //for factory mode auto testing
			*feature_return_para_32 = imgsensor_info.checksum_value;
			*feature_para_len = 4;
			break;
	case SENSOR_FEATURE_SET_FRAMERATE:
			LOG_INF("current fps :%d\n", (UINT32)*feature_data);
			spin_lock(&imgsensor_drv_lock);
			imgsensor.current_fps = *feature_data;
			spin_unlock(&imgsensor_drv_lock);
			break;
	case SENSOR_FEATURE_SET_HDR:
			LOG_INF("ihdr enable :%d\n", (BOOL)*feature_data);
			spin_lock(&imgsensor_drv_lock);
			imgsensor.ihdr_en = (BOOL)*feature_data;
			spin_unlock(&imgsensor_drv_lock);
			break;
/*add rawinfo start*/
/*
	case SENSOR_FEATURE_GET_RAW_INFO:
			pr_debug("SENSOR_FEATURE_GET_RAW_INFO scenarioId:%d\n", (UINT32) *feature_data);
			rawinfo = (struct SENSOR_RAWINFO_STRUCT *) (uintptr_t) (*(feature_data + 1));
			memcpy((void *)rawinfo, (void *)&imgsensor_raw_info, sizeof(struct SENSOR_RAWINFO_STRUCT));
			break;
*/
/*add rawinfo end*/
	case SENSOR_FEATURE_GET_CROP_INFO:
			LOG_INF("SENSOR_FEATURE_GET_CROP_INFO scenarioId:%d\n", (UINT32)*feature_data);

			wininfo = (struct SENSOR_WINSIZE_INFO_STRUCT *)(uintptr_t)(*(feature_data+1));

			switch (*feature_data_32) {
			case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
					memcpy((void *)wininfo, (void *)&imgsensor_winsize_info[1], sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
					break;
			case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
					memcpy((void *)wininfo, (void *)&imgsensor_winsize_info[2], sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
					break;
			case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
					memcpy((void *)wininfo, (void *)&imgsensor_winsize_info[3], sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
					break;
			case MSDK_SCENARIO_ID_SLIM_VIDEO:
					memcpy((void *)wininfo, (void *)&imgsensor_winsize_info[4], sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
					break;
/*
			case MSDK_SCENARIO_ID_CUSTOM1:
					memcpy((void *)wininfo, (void *)&imgsensor_winsize_info[5], sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
					break;
			case MSDK_SCENARIO_ID_CUSTOM2:
					memcpy((void *)wininfo, (void *)&imgsensor_winsize_info[6], sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
					break;
			case MSDK_SCENARIO_ID_CUSTOM3:
					memcpy((void *)wininfo, (void *)&imgsensor_winsize_info[7], sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
					break;
			case MSDK_SCENARIO_ID_CUSTOM4:
					memcpy((void *)wininfo, (void *)&imgsensor_winsize_info[8], sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
					break;
			case MSDK_SCENARIO_ID_CUSTOM5:
					memcpy((void *)wininfo, (void *)&imgsensor_winsize_info[9], sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
					break;
*/
			case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
			default:
					memcpy((void *)wininfo, (void *)&imgsensor_winsize_info[0], sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
					break;
			}
	case SENSOR_FEATURE_SET_IHDR_SHUTTER_GAIN:
		//LOG_INF("SENSOR_SET_SENSOR_IHDR LE=%d, SE=%d, Gain=%d\n", (UINT16)*feature_data, (UINT16)*(feature_data+1), (UINT16)*(feature_data+2));
		//ihdr_write_shutter_gain((UINT16)*feature_data,(UINT16)*(feature_data+1),(UINT16)*(feature_data+2));
		break;


	case SENSOR_FEATURE_SET_STREAMING_SUSPEND:
		LOG_INF("SENSOR_FEATURE_SET_STREAMING_SUSPEND\n");
		streaming_control(KAL_FALSE);
		break;
	case SENSOR_FEATURE_SET_STREAMING_RESUME:
		LOG_INF("SENSOR_FEATURE_SET_STREAMING_RESUME, shutter:%llu\n", *feature_data);
		if (*feature_data != 0)
			set_shutter(*feature_data);
		streaming_control(KAL_TRUE);
		break;
	case SENSOR_FEATURE_GET_BINNING_TYPE:
		switch (*(feature_data + 1)) {
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
		case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
		case MSDK_SCENARIO_ID_SLIM_VIDEO:
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
/*
		case MSDK_SCENARIO_ID_CUSTOM1:
		case MSDK_SCENARIO_ID_CUSTOM2:
		case MSDK_SCENARIO_ID_CUSTOM3:
		case MSDK_SCENARIO_ID_CUSTOM4:
		case MSDK_SCENARIO_ID_CUSTOM5:
*/
		default:
			*feature_return_para_32 = 1; /*BINNING_AVERAGED*/
			break;
		}
		pr_debug("SENSOR_FEATURE_GET_BINNING_TYPE AE_binning_type:%d,\n",
			*feature_return_para_32);
		*feature_para_len = 4;

		break;
	case SENSOR_FEATURE_GET_MIPI_PIXEL_RATE:
		{
		kal_uint32 rate;

		switch (*feature_data) {
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			rate = imgsensor_info.cap.mipi_pixel_rate;
			break;
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			rate = imgsensor_info.normal_video.mipi_pixel_rate;
			break;
		case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
			rate = imgsensor_info.hs_video.mipi_pixel_rate;
			break;
		case MSDK_SCENARIO_ID_SLIM_VIDEO:
			rate = imgsensor_info.slim_video.mipi_pixel_rate;
			break;
/*
		case MSDK_SCENARIO_ID_CUSTOM1:
			rate = imgsensor_info.custom1.mipi_pixel_rate;
			break;
		case MSDK_SCENARIO_ID_CUSTOM2:
			rate = imgsensor_info.custom2.mipi_pixel_rate;
			break;
		case MSDK_SCENARIO_ID_CUSTOM3:
			rate = imgsensor_info.custom3.mipi_pixel_rate;
			break;
		case MSDK_SCENARIO_ID_CUSTOM4:
			rate = imgsensor_info.custom4.mipi_pixel_rate;
			break;
		case MSDK_SCENARIO_ID_CUSTOM5:
			rate = imgsensor_info.custom5.mipi_pixel_rate;
			break;
*/
/* hs04 code for SR-AL6398A-01-12 by liluling at 2022/7/20 end */
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
			rate = imgsensor_info.pre.mipi_pixel_rate;
			break;
		default:
			rate = 0;
			break;
		}
		*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) = rate;
		}
		break;

	default:
		break;
	}
	return ERROR_NONE;
}    /*    feature_control()  */

static struct SENSOR_FUNCTION_STRUCT sensor_func = {
	open,
	get_info,
	get_resolution,
	feature_control,
	control,
	close
};

UINT32 O2102_OV5A10HLT_FRONT_MIPI_RAW_SensorInit(struct SENSOR_FUNCTION_STRUCT **pfFunc)
{
	/* To Do : Check Sensor status here */
	if (pfFunc != NULL)
		*pfFunc = &sensor_func;
	return ERROR_NONE;
} 	 /*	   OV05A10TXDPD2216FRONTMIPISensorInit	*/
/* hs04 code for SR-AL6398A-01-12 by liluling at 2022/7/22 end */
/*  hs04 code for DEVAL6398A-462 by liluling at 2022/08/26 end */
