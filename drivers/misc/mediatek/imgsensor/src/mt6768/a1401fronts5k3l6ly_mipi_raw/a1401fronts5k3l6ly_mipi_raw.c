/*****************************************************************************
 *
 * Filename:
 * ---------
 *	 S5K3L6mipiraw_sensor.c
 *
 * Project:
 * --------
 *	 ALPS MT6735
 *
 * Description:
 * ------------
 *	 Source code of Sensor driver
 *
 *	PengtaoFan
 *  20150624: the first driver from ov8858
 *  20150706: add pip 15fps setting
 *  20150720: use non - continue mode
 *  15072011511229: add pdaf, the pdaf old has be delete by recovery
 *  15072209190629: non - continue mode bandwith limited , has <tiaowen> , modify to continue mode
 *  15072209201129: modify not enter init_setting bug
 *  15072718000000: crc addd 0x49c09f86
 *  15072718000001: MODIFY LOG SWITCH
 *  15072811330000: ADD NON-CONTIUE MODE ,PREVIEW 29FPS,CAPTURE 29FPS
 					([TODO]REG0304 0786->0780  PREVEIW INCREASE TO 30FPS)
 *  15072813000000: modify a wrong setting at pip reg030e 0x119->0xc8
 *  15080409230000: pass!
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/
/*hs14 code for SR-AL6528A-01-88 by pengxutao at 2022-11-1 start*/
#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <asm/atomic.h>
//#include <asm/system.h>
//#include <linux/xlog.h>
#include <linux/types.h>
//ludesuo add for D41 camera performance begin
// #ifdef CONFIG_RLK_CAM_PERFORMANCE_IMPROVE
// #include <linux/dma-mapping.h>
// #endif
//ludesuo add for D41 camera performance end

#include "kd_camera_typedef.h"

//#include "kd_camera_hw.h"
#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_imgsensor_errcode.h"

#include "a1401fronts5k3l6ly_mipi_raw.h"

/*===FEATURE SWITH===*/
// #define FANPENGTAO   //for debug log
// #define LOG_INF LOG_INF_LOD
//#define NONCONTINUEMODE
/*===FEATURE SWITH===*/

/****************************Modify Following Strings for Debug****************************/
#define PFX "S5K3L6"
#define LOG_INF_NEW(format, args...)                                           \
	pr_debug(PFX "[%s] " format, __func__, ##args)
#define LOG_INF printk

#define LOG_1 LOG_INF("S5K3L6,MIPI 4LANE\n")
#define SENSORDB LOG_INF
/****************************   Modify end    *******************************************/

static DEFINE_SPINLOCK(imgsensor_drv_lock);

static imgsensor_info_struct imgsensor_info = {
	.sensor_id = A1401FRONTS5K3L6LY_SENSOR_ID,		//Sensor ID Value: 0x30C8//record sensor id defined in Kd_imgsensor.h

	.checksum_value = 0x6ed0b036,		//checksum value for Camera Auto Test

	.pre = {
		.pclk = 476000000,
		.linelength = 4896,
		.framelength = 3260,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 4128,
		.grabwindow_height = 3096,
		.mipi_data_lp2hs_settle_dc = 85,
		.max_framerate = 300,
	},

	.cap = {
		.pclk = 476000000,
		.linelength = 4896,
		.framelength = 3260,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 4128,
		.grabwindow_height = 3096,
		.mipi_data_lp2hs_settle_dc = 85,
		.max_framerate = 300,
	},
	.normal_video = {
		.pclk = 476000000,
		.linelength = 4896,
		.framelength = 3260,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 4128,
		.grabwindow_height = 3096,
		.mipi_data_lp2hs_settle_dc = 85,
		.max_framerate = 300,
	},
	.hs_video = {
		.pclk = 476000000,
		.linelength = 4896,
		.framelength = 3268,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 3712,
		.grabwindow_height = 2556,
		.mipi_data_lp2hs_settle_dc = 85,
		.max_framerate = 300,
	},
	.slim_video = {
		.pclk = 476000000,
		.linelength = 5248,
		.framelength = 3049,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 3408,
		.grabwindow_height = 2556,
		.mipi_data_lp2hs_settle_dc = 85,
		.max_framerate = 300,
	},
	.custom1 = {
		.pclk = 476000000,
		.linelength = 4896,
		.framelength = 3260,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 1696,
		.grabwindow_height = 1272,
		.mipi_data_lp2hs_settle_dc = 85,
		.max_framerate = 300,
	},
	.custom2 = {
		.pclk = 476000000,
		.linelength = 4896,
		.framelength = 3260,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 1856,
		.grabwindow_height = 1044,
		.mipi_data_lp2hs_settle_dc = 85,
		.max_framerate = 300,
	},
	.custom3 = {
		.pclk = 476000000,
		.linelength = 4896,
		.framelength = 3260,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 2064,
		.grabwindow_height = 1160,
		.mipi_data_lp2hs_settle_dc = 85,
		.max_framerate = 300,
	},
	.custom4 = {
		.pclk = 476000000,
		.linelength = 4896,
		.framelength = 3260,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 2064,
		.grabwindow_height = 1548,
		.mipi_data_lp2hs_settle_dc = 85,
		.max_framerate = 300,
	},
	.custom5 = {
		.pclk = 476000000,
		.linelength  = 4896,
		.framelength = 3260,
		.startx = 0,
		.starty = 0,
		.grabwindow_width  = 4128,
		.grabwindow_height = 2324,
		.mipi_data_lp2hs_settle_dc = 85,
		.max_framerate = 300,
	},


	.margin = 5,			//sensor framelength & shutter margin
	.min_shutter = 4,		//min shutter
	.max_frame_length = 0xFFFF,//REG0x0202 <=REG0x0340-5//max framelength by sensor register's limitation
	.ae_shut_delay_frame = 0,	//shutter delay frame for AE cycle, 2 frame with ispGain_delay-shut_delay=2-0=2
	.ae_sensor_gain_delay_frame = 0,//sensor gain delay frame for AE cycle,2 frame with ispGain_delay-sensor_gain_delay=2-0=2
	.ae_ispGain_delay_frame = 2,//isp gain delay frame for AE cycle
	.ihdr_support = 0,	  //1, support; 0,not support
	.ihdr_le_firstline = 0,  //1,le first ; 0, se first
	.sensor_mode_num = 10,	  //support sensor mode num ,don't support Slow motion
//ludesuo add for D41 to develop camera performance on
// #ifdef CONFIG_RLK_CAM_PERFORMANCE_IMPROVE //def X555_H538
// 	.cap_delay_frame = 2,		//enter capture delay frame num
// 	.pre_delay_frame = 2, 		//enter preview delay frame num
// 	.video_delay_frame = 2,		//enter video delay frame num
// 	.hs_video_delay_frame = 2,	//enter high speed video  delay frame num
// 	.slim_video_delay_frame = 2,//enter slim video delay frame num
// #else
/*hs14 code for SR-AL6528A-01-20 by renxinglin at 2022/11/17 start*/
	.cap_delay_frame = 2,		//enter capture delay frame num
	.pre_delay_frame = 2, 		//enter preview delay frame num
	.video_delay_frame = 2,		//enter video delay frame num
	.hs_video_delay_frame = 2,	//enter high speed video  delay frame num
	.slim_video_delay_frame = 2,//enter slim video delay frame num
	.custom1_delay_frame = 2,
	.custom2_delay_frame = 2,
	.custom3_delay_frame = 2,
	.custom4_delay_frame = 2,
	.custom5_delay_frame = 2,
/*hs14 code for SR-AL6528A-01-20 by renxinglin at 2022/11/17 end*/
// #endif

	.isp_driving_current = ISP_DRIVING_4MA, //mclk driving current
	.sensor_interface_type = SENSOR_INTERFACE_TYPE_MIPI,//sensor_interface_type
    .mipi_sensor_type = MIPI_OPHY_NCSI2, //0,MIPI_OPHY_NCSI2;  1,MIPI_OPHY_CSI2
    .mipi_settle_delay_mode = 1,//0,MIPI_SETTLEDELAY_AUTO; 1,MIPI_SETTLEDELAY_MANNUAL
	.sensor_output_dataformat = SENSOR_OUTPUT_FORMAT_RAW_Gr,//sensor output first pixel color
	.mclk = 24,//mclk value, suggest 24 or 26 for 24Mhz or 26Mhz
	.mipi_lane_num = SENSOR_MIPI_4_LANE,//mipi lane num
	.i2c_addr_table = {0x5a, 0x20, 0xff},//record sensor support all write id addr, only supprt 4must end with 0xff
//ludesuo add for D41 to develop camera performance on
// #ifdef CONFIG_RLK_CAM_PERFORMANCE_IMPROVE //def X555_H538
    .i2c_speed = 400, // i2c read/write speed
// #endif
//ludesuo add for D41 to develop camera performance off
};

static imgsensor_struct imgsensor = {
	.mirror = IMAGE_NORMAL, //mirrorflip information
	.sensor_mode =
		IMGSENSOR_MODE_INIT, //IMGSENSOR_MODE enum value,record current sensor mode,such as: INIT, Preview, Capture, Video,High Speed Video, Slim Video
	.shutter = 0x200, //current shutter
	.gain = 0x200, //current gain
	.dummy_pixel = 0, //current dummypixel
	.dummy_line = 0, //current dummyline
	.current_fps =
		0, //full size current fps : 24fps for PIP, 30fps for Normal or ZSD
	.autoflicker_en =
		KAL_FALSE, //auto flicker enable: KAL_FALSE for disable auto flicker, KAL_TRUE for enable auto flicker
	.test_pattern =
		KAL_FALSE, //test pattern mode or not. KAL_FALSE for in test pattern mode, KAL_TRUE for normal output
	.current_scenario_id =
		MSDK_SCENARIO_ID_CAMERA_PREVIEW, //current scenario id
	.ihdr_en = KAL_FALSE, //sensor need support LE, SE with HDR feature
	.i2c_write_id = 0x20, //record current sensor's i2c write id
};

/* Sensor output window information*/
static struct SENSOR_WINSIZE_INFO_STRUCT imgsensor_winsize_info[10] = {
	{ 4208, 3120, 40, 12, 4128, 3096, 4128, 3096, 0, 0, 4128, 3096, 0, 0,
	  4128, 3096 }, // Preview
	{ 4208, 3120, 40, 12, 4128, 3096, 4128, 3096, 0, 0, 4128, 3096, 0, 0,
	  4128, 3096 }, // capture
	{ 4208, 3120, 40, 12, 4128, 3096, 4128, 3096, 0, 0, 4128, 3096, 0, 0,
	  4128, 3096 }, // video
	{ 4208, 3120, 248, 282, 3712, 2556, 3712, 2556, 0, 0, 3712, 2556, 0, 0,
	  3712, 2556 }, //hight speed video
	{ 4208, 3120, 400, 282, 3408, 2556, 3408, 2556, 0, 0, 3408, 2556, 0, 0,
	  3408, 2556 }, // slim video
	{ 4208, 3120, 408, 288, 3392, 2544, 1696, 1272, 0, 0, 1696, 1272, 0, 0,
	  1696, 1272 }, // custom1
	{ 4208, 3120, 248, 516, 3712, 2088, 1856, 1044, 0, 0, 1856, 1044, 0, 0,
	  1856, 1044 }, // custom2
	{ 4208, 3120, 40, 400, 4128, 2320, 2064, 1160, 0, 0, 2064, 1160, 0, 0,
	  2064, 1160 }, // custom3
	{ 4208, 3120, 40, 12, 4128, 3096, 2064, 1548, 0, 0, 2064, 1548, 0, 0,
	  2064, 1548 }, // custom4
	{ 4208, 3120, 40, 398, 4128, 2324, 4128, 2324, 0, 0, 4128, 2324, 0, 0,
	  4128, 2324 },
};

static kal_uint16 read_cmos_sensor_byte(kal_uint16 addr)
{
	kal_uint16 get_byte = 0;
	char pu_send_cmd[2] = { (char)(addr >> 8), (char)(addr & 0xFF) };


	iReadRegI2C(
		pu_send_cmd, 2, (u8 *) &get_byte, 1, imgsensor.i2c_write_id);
	return get_byte;
}

static kal_uint16 read_cmos_sensor(kal_uint32 addr)
{
	kal_uint16 get_byte = 0;
	char pu_send_cmd[2] = { (char)(addr >> 8), (char)(addr & 0xFF) };


	iReadRegI2C(
		pu_send_cmd, 2, (u8 *) &get_byte, 1, imgsensor.i2c_write_id);
	return get_byte;
}

static void write_cmos_sensor_byte(kal_uint32 addr, kal_uint32 para)
{
	char pu_send_cmd[3] = {
		(char)(addr >> 8), (char)(addr & 0xFF), (char)(para & 0xFF) };


	iWriteRegI2C(pu_send_cmd, 3, imgsensor.i2c_write_id);
}

static void write_cmos_sensor(kal_uint16 addr, kal_uint16 para)
{
	char pusendcmd[4] = {
			(char)(addr >> 8), (char)(addr & 0xFF),
			(char)(para >> 8), (char)(para & 0xFF)};


	iWriteRegI2C(pusendcmd, 4, imgsensor.i2c_write_id);
}

static void set_dummy(void)
{
	LOG_INF("dummyline = %d, dummypixels = %d \n", imgsensor.dummy_line,
		imgsensor.dummy_pixel);
	/* you can set dummy by imgsensor.dummy_line and imgsensor.dummy_pixel, or you can set dummy by imgsensor.frame_length and imgsensor.line_length */
	write_cmos_sensor(0x0340, imgsensor.frame_length & 0xFFFF);
	write_cmos_sensor(0x0342, imgsensor.line_length & 0xFFFF);
} /*	set_dummy  */

static void set_max_framerate(UINT16 framerate, kal_bool min_framelength_en)
{
	kal_uint32 frame_length = imgsensor.frame_length;

	LOG_INF("framerate = %d, min framelength should enable(%d) \n",
		framerate, min_framelength_en);

	frame_length = imgsensor.pclk / framerate * 10 / imgsensor.line_length;
	spin_lock(&imgsensor_drv_lock);
	imgsensor.frame_length = (frame_length > imgsensor.min_frame_length) ?
					 frame_length :
					 imgsensor.min_frame_length;
	imgsensor.dummy_line =
		imgsensor.frame_length - imgsensor.min_frame_length;
	//dummy_line = frame_length - imgsensor.min_frame_length;
	//if (dummy_line < 0)
	//imgsensor.dummy_line = 0;
	//else
	//imgsensor.dummy_line = dummy_line;
	//imgsensor.frame_length = frame_length + imgsensor.dummy_line;
	if (imgsensor.frame_length > imgsensor_info.max_frame_length) {
		imgsensor.frame_length = imgsensor_info.max_frame_length;
		imgsensor.dummy_line =
			imgsensor.frame_length - imgsensor.min_frame_length;
	}
	if (min_framelength_en)
		imgsensor.min_frame_length = imgsensor.frame_length;
	spin_unlock(&imgsensor_drv_lock);
	set_dummy();
} /*	set_max_framerate  */

/*static void write_shutter(kal_uint16 shutter)
{
	kal_uint16 realtime_fps = 0;

	 //0x3500, 0x3501, 0x3502 will increase VBLANK to get exposure larger than frame exposure
	 //AE doesn't update sensor gain at capture mode, thus extra exposure lines must be updated here.

	// OV Recommend Solution
	// if shutter bigger than frame_length, should extend frame length first
	spin_lock(&imgsensor_drv_lock);
	if (shutter > imgsensor.min_frame_length - imgsensor_info.margin)
		imgsensor.frame_length = shutter + imgsensor_info.margin;
	else
		imgsensor.frame_length = imgsensor.min_frame_length;
	if (imgsensor.frame_length > imgsensor_info.max_frame_length)
		imgsensor.frame_length = imgsensor_info.max_frame_length;
	spin_unlock(&imgsensor_drv_lock);
	shutter = (shutter < imgsensor_info.min_shutter) ? imgsensor_info.min_shutter : shutter;
	shutter = (shutter > (imgsensor_info.max_frame_length - imgsensor_info.margin)) ? (imgsensor_info.max_frame_length - imgsensor_info.margin) : shutter;

	if (imgsensor.autoflicker_en) {
		realtime_fps = imgsensor.pclk / imgsensor.line_length * 10 / imgsensor.frame_length;
		if(realtime_fps >= 297 && realtime_fps <= 305)
			set_max_framerate(296,0);
		else if(realtime_fps >= 147 && realtime_fps <= 150)
			set_max_framerate(146,0);
	} else {
		// Extend frame length
		write_cmos_sensor(0x0340, imgsensor.frame_length & 0xFFFF);
	}

	// Update Shutter
	write_cmos_sensor(0x0202, (shutter) & 0xFFFF);
	LOG_INF("Exit! shutter =%d, framelength =%d\n", shutter,imgsensor.frame_length);

	//LOG_INF("frame_length = %d ", frame_length);

}*/	/*	write_shutter  */

/*************************************************************************
* FUNCTION
*	set_shutter
*
* DESCRIPTION
*	This function set e-shutter of sensor to change exposure time.
*
* PARAMETERS
*	iShutter : exposured lines
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
static void set_shutter(kal_uint16 shutter)
{
	unsigned long flags;
	kal_uint16 realtime_fps = 0;
	//kal_uint32 frame_length = 0;
	spin_lock_irqsave(&imgsensor_drv_lock, flags);
	imgsensor.shutter = shutter;
	spin_unlock_irqrestore(&imgsensor_drv_lock, flags);

	//write_shutter(shutter);
	/* 0x3500, 0x3501, 0x3502 will increase VBLANK to get exposure larger than frame exposure */
	/* AE doesn't update sensor gain at capture mode, thus extra exposure lines must be updated here. */

	// OV Recommend Solution
	// if shutter bigger than frame_length, should extend frame length first
	spin_lock(&imgsensor_drv_lock);
	if (shutter > imgsensor.min_frame_length - imgsensor_info.margin)
		imgsensor.frame_length = shutter + imgsensor_info.margin;
	else
		imgsensor.frame_length = imgsensor.min_frame_length;
	if (imgsensor.frame_length > imgsensor_info.max_frame_length)
		imgsensor.frame_length = imgsensor_info.max_frame_length;
	spin_unlock(&imgsensor_drv_lock);
	shutter = (shutter < imgsensor_info.min_shutter) ?
			  imgsensor_info.min_shutter :
			  shutter;
	shutter = (shutter >
		   (imgsensor_info.max_frame_length - imgsensor_info.margin)) ?
			  (imgsensor_info.max_frame_length -
			   imgsensor_info.margin) :
			  shutter;

	if (imgsensor.autoflicker_en) {
		realtime_fps = imgsensor.pclk / imgsensor.line_length * 10 /
			       imgsensor.frame_length;
		if (realtime_fps >= 297 && realtime_fps <= 305)
			set_max_framerate(296, 0);
		else if (realtime_fps >= 147 && realtime_fps <= 150)
			set_max_framerate(146, 0);
		else {
			// Extend frame length
			write_cmos_sensor(0x0340,
					  imgsensor.frame_length & 0xFFFF);
		}
	} else {
		// Extend frame length
		write_cmos_sensor(0x0340, imgsensor.frame_length & 0xFFFF);
	}

	// Update Shutter
	write_cmos_sensor(0X0202, shutter & 0xFFFF);
	LOG_INF("Exit! shutter =%d, framelength =%d\n", shutter,
		imgsensor.frame_length);
}

static kal_uint16 gain2reg(const kal_uint16 gain)
{
	kal_uint16 reg_gain = 0x0000;
	//gain = 64 = 1x real gain.
	reg_gain = gain / 2;
	//reg_gain = reg_gain & 0xFFFF;
	return (kal_uint16)reg_gain;
}

/*************************************************************************
* FUNCTION
*	set_gain
*
* DESCRIPTION
*	This function is to set global gain to sensor.
*
* PARAMETERS
*	iGain : sensor global gain(base: 0x40)
*
* RETURNS
*	the actually gain set to sensor.
*
* GLOBALS AFFECTED
*
*************************************************************************/
static kal_uint16 set_gain(kal_uint16 gain)
{
	//gain = 64 = 1x real gain.
	kal_uint16 reg_gain;
	LOG_INF("set_gain %d \n", gain);
	if (gain < BASEGAIN || gain > 16 * BASEGAIN) {
		LOG_INF("Error gain setting");
		if (gain < BASEGAIN)
			gain = BASEGAIN;
		else if (gain > 16 * BASEGAIN)
			gain = 16 * BASEGAIN;
	}

	reg_gain = gain2reg(gain);
	spin_lock(&imgsensor_drv_lock);
	imgsensor.gain = reg_gain;
	spin_unlock(&imgsensor_drv_lock);
	LOG_INF("gain = %d , reg_gain = 0x%x\n ", gain, reg_gain);

	write_cmos_sensor(0x0204, (reg_gain & 0xFFFF));
	return gain;
} /*	set_gain  */

//ihdr_write_shutter_gain not support for s5k3l6
static void ihdr_write_shutter_gain(kal_uint16 le, kal_uint16 se,
				    kal_uint16 gain)
{
	LOG_INF("le:0x%x, se:0x%x, gain:0x%x\n", le, se, gain);
	if (imgsensor.ihdr_en) {
		spin_lock(&imgsensor_drv_lock);
		if (le > imgsensor.min_frame_length - imgsensor_info.margin)
			imgsensor.frame_length = le + imgsensor_info.margin;
		else
			imgsensor.frame_length = imgsensor.min_frame_length;
		if (imgsensor.frame_length > imgsensor_info.max_frame_length)
			imgsensor.frame_length =
				imgsensor_info.max_frame_length;
		spin_unlock(&imgsensor_drv_lock);
		if (le < imgsensor_info.min_shutter)
			le = imgsensor_info.min_shutter;
		if (se < imgsensor_info.min_shutter)
			se = imgsensor_info.min_shutter;

		// Extend frame length first
		//

		set_gain(gain);
	}
}

static void set_mirror_flip(kal_uint8 image_mirror)
{
	LOG_INF("image_mirror = %d\n", image_mirror);

	/********************************************************
	   *
	   *   0x3820[2] ISP Vertical flip
	   *   0x3820[1] Sensor Vertical flip
	   *
	   *   0x3821[2] ISP Horizontal mirror
	   *   0x3821[1] Sensor Horizontal mirror
	   *
	   *   ISP and Sensor flip or mirror register bit should be the same!!
	   *
	   ********************************************************/
	spin_lock(&imgsensor_drv_lock);
	imgsensor.mirror = image_mirror;
	spin_unlock(&imgsensor_drv_lock);
	switch (image_mirror) {
	case IMAGE_NORMAL:
		write_cmos_sensor(0x0101, 0X00); //GR
		break;
	case IMAGE_H_MIRROR:
		write_cmos_sensor(0x0101, 0X01); //R
		break;
	case IMAGE_V_MIRROR:
		write_cmos_sensor(0x0101, 0X02); //B
		break;
	case IMAGE_HV_MIRROR:
		write_cmos_sensor(0x0101, 0X03); //GB
		break;
	default:
		LOG_INF("Error image_mirror setting\n");
	}
}

/*************************************************************************
* FUNCTION
*	night_mode
*
* DESCRIPTION
*	This function night mode of sensor.
*
* PARAMETERS
*	bEnable: KAL_TRUE -> enable night mode, otherwise, disable night mode
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
static void night_mode(kal_bool enable)
{
	/*No Need to implement this function*/
} /*	night_mode	*/

static void sensor_init(void)
{
	LOG_INF("E\n");

	write_cmos_sensor(0x306A, 0x2F4C);
	write_cmos_sensor(0x306C, 0xCA01);
	write_cmos_sensor(0x307A, 0x0D20);
	write_cmos_sensor(0x309E, 0x002D);
	write_cmos_sensor(0x3072, 0x0013);
	write_cmos_sensor(0x3074, 0x0977);
	write_cmos_sensor(0x3076, 0x9411);
	write_cmos_sensor(0x3024, 0x0016);
	write_cmos_sensor(0x3002, 0x0E00);
	write_cmos_sensor(0x3006, 0x1000);
	write_cmos_sensor(0x300A, 0x0C00);
	write_cmos_sensor(0x3018, 0xC500);
	write_cmos_sensor(0x303A, 0x0204);
	write_cmos_sensor(0x3266, 0x0001);
	write_cmos_sensor(0x38DA, 0x000A);
	write_cmos_sensor(0x38DC, 0x000B);
	write_cmos_sensor(0x38D6, 0x000A);
	write_cmos_sensor(0x3070, 0x3D00);
	write_cmos_sensor(0x3084, 0x1314);
	write_cmos_sensor(0x3086, 0x0CE7);
	write_cmos_sensor(0x3004, 0x0800);
	write_cmos_sensor(0x3C08, 0xFFFF);

} /*	sensor_init  */

static void preview_setting(void)
{
	int i = 0;
	int framecnt = 0;
	write_cmos_sensor_byte(0x0100, 0x00);

	for (i = 0; i < 100; i++) {
		framecnt = read_cmos_sensor_byte(
			0x0005); // waiting for sensor to  stop output  then  set the  setting
		if (framecnt == 0xFF) {
			break;
		} else {
			mdelay(1);
		}
	}

	LOG_INF("E\n");
	//$MV1[MCLK:24,Width:2104,Height:1560,Format:MIPI_Raw10,mipi_lane:4,mipi_datarate:568,pvi_pclk_inverse:0]
	write_cmos_sensor(0x314A, 0x5F00);
	write_cmos_sensor(0x3064, 0xFFCF);
	write_cmos_sensor(0x3066, 0x7E00);
	write_cmos_sensor(0x309C, 0x0640);
	write_cmos_sensor(0x380C, 0x001A);
	write_cmos_sensor(0x32B2, 0x0000);
	write_cmos_sensor(0x32B4, 0x0000);
	write_cmos_sensor(0x32B6, 0x0000);
	write_cmos_sensor(0x32B8, 0x0000);
	write_cmos_sensor(0x3090, 0x8800);
	write_cmos_sensor(0x3238, 0x000C);
	write_cmos_sensor(0x0100, 0x0000);
	write_cmos_sensor(0x0344, 0x0030);
	write_cmos_sensor(0x0348, 0x104F);
	write_cmos_sensor(0x0346, 0x0014);
	write_cmos_sensor(0x034A, 0x0C2B);
	write_cmos_sensor(0x034C, 0x1020);
	write_cmos_sensor(0x034E, 0x0C18);
	write_cmos_sensor(0x0202, 0x03DE);
	write_cmos_sensor(0x3400, 0x0000);
	write_cmos_sensor(0x3402, 0x4E46);
	write_cmos_sensor(0x0136, 0x1800);
	write_cmos_sensor(0x0304, 0x0004);
	write_cmos_sensor(0x0306, 0x0077);
	write_cmos_sensor(0x030C, 0x0004);
	write_cmos_sensor(0x030E, 0x0064);
	write_cmos_sensor(0x3C16, 0x0000);
	write_cmos_sensor(0x0342, 0x1320);
	write_cmos_sensor(0x0340, 0x0CC4);
	write_cmos_sensor(0x0900, 0x0000);
	write_cmos_sensor(0x0386, 0x0001);
	write_cmos_sensor(0x3452, 0x0000);
	write_cmos_sensor(0x345A, 0x0000);
	write_cmos_sensor(0x345C, 0x0000);
	write_cmos_sensor(0x345E, 0x0000);
	write_cmos_sensor(0x3460, 0x0000);
	write_cmos_sensor(0x38C4, 0x0009);
	write_cmos_sensor(0x38D8, 0x002A);
	write_cmos_sensor(0x38DA, 0x000A);
	write_cmos_sensor(0x38DC, 0x000B);
	write_cmos_sensor(0x38C2, 0x0009);
	write_cmos_sensor(0x38C0, 0x000E);
	write_cmos_sensor(0x38D6, 0x000A);
	write_cmos_sensor(0x38D4, 0x0008);
	write_cmos_sensor(0x38B0, 0x000E);
	write_cmos_sensor(0x3932, 0x1000);
	write_cmos_sensor(0x0820, 0x04B0);
	write_cmos_sensor(0x3C34, 0x0008);
	write_cmos_sensor(0x3C36, 0x3800);
	write_cmos_sensor(0x3C38, 0x0020);
	write_cmos_sensor(0x393E, 0x4000);
	write_cmos_sensor(0x3892, 0x2EE0);
	write_cmos_sensor(0x3C1E, 0x0100);
	write_cmos_sensor(0x0100, 0x0100);
	write_cmos_sensor(0x3C1E, 0x0000);

} /*	preview_setting  */

static void capture_setting(void)
{
	int i = 0;
	int framecnt = 0;
	write_cmos_sensor_byte(0x0100, 0x00);

	for (i = 0; i < 100; i++) {
		framecnt = read_cmos_sensor_byte(
			0x0005); // waiting for sensor to  stop output  then  set the  setting
		if (framecnt == 0xFF) {
			break;
		} else {
			mdelay(1);
		}
	}

	//$MV1[MCLK:24,Width:4208,Height:3120,Format:MIPI_Raw10,mipi_lane:4,mipi_datarate:1200,pvi_pclk_inverse:0]

	write_cmos_sensor(0x314A, 0x5F00);
	write_cmos_sensor(0x3064, 0xFFCF);
	write_cmos_sensor(0x3066, 0x7E00);
	write_cmos_sensor(0x309C, 0x0640);
	write_cmos_sensor(0x380C, 0x001A);
	write_cmos_sensor(0x32B2, 0x0000);
	write_cmos_sensor(0x32B4, 0x0000);
	write_cmos_sensor(0x32B6, 0x0000);
	write_cmos_sensor(0x32B8, 0x0000);
	write_cmos_sensor(0x3090, 0x8800);
	write_cmos_sensor(0x3238, 0x000C);
	write_cmos_sensor(0x0100, 0x0000);
	write_cmos_sensor(0x0344, 0x0030);
	write_cmos_sensor(0x0348, 0x104F);
	write_cmos_sensor(0x0346, 0x0014);
	write_cmos_sensor(0x034A, 0x0C2B);
	write_cmos_sensor(0x034C, 0x1020);
	write_cmos_sensor(0x034E, 0x0C18);
	write_cmos_sensor(0x0202, 0x03DE);
	write_cmos_sensor(0x3400, 0x0000);
	write_cmos_sensor(0x3402, 0x4E46);
	write_cmos_sensor(0x0136, 0x1800);
	write_cmos_sensor(0x0304, 0x0004);
	write_cmos_sensor(0x0306, 0x0077);
	write_cmos_sensor(0x030C, 0x0004);
	write_cmos_sensor(0x030E, 0x0064);
	write_cmos_sensor(0x3C16, 0x0000);
	write_cmos_sensor(0x0342, 0x1320);
	write_cmos_sensor(0x0340, 0x0CC4);
	write_cmos_sensor(0x0900, 0x0000);
	write_cmos_sensor(0x0386, 0x0001);
	write_cmos_sensor(0x3452, 0x0000);
	write_cmos_sensor(0x345A, 0x0000);
	write_cmos_sensor(0x345C, 0x0000);
	write_cmos_sensor(0x345E, 0x0000);
	write_cmos_sensor(0x3460, 0x0000);
	write_cmos_sensor(0x38C4, 0x0009);
	write_cmos_sensor(0x38D8, 0x002A);
	write_cmos_sensor(0x38DA, 0x000A);
	write_cmos_sensor(0x38DC, 0x000B);
	write_cmos_sensor(0x38C2, 0x0009);
	write_cmos_sensor(0x38C0, 0x000E);
	write_cmos_sensor(0x38D6, 0x000A);
	write_cmos_sensor(0x38D4, 0x0008);
	write_cmos_sensor(0x38B0, 0x000E);
	write_cmos_sensor(0x3932, 0x1000);
	write_cmos_sensor(0x0820, 0x04B0);
	write_cmos_sensor(0x3C34, 0x0008);
	write_cmos_sensor(0x3C36, 0x3800);
	write_cmos_sensor(0x3C38, 0x0020);
	write_cmos_sensor(0x393E, 0x4000);
	write_cmos_sensor(0x3892, 0x2EE0);
	write_cmos_sensor(0x3C1E, 0x0100);
	write_cmos_sensor(0x0100, 0x0100);
	write_cmos_sensor(0x3C1E, 0x0000);
}

static void normal_video_setting(void)
{
	int i = 0;
	int framecnt = 0;
	write_cmos_sensor_byte(0x0100, 0x00);

	for (i = 0; i < 100; i++) {
		framecnt = read_cmos_sensor_byte(
			0x0005); // waiting for sensor to  stop output  then  set the  setting
		if (framecnt == 0xFF) {
			break;
		} else {
			mdelay(1);
		}
	}
	write_cmos_sensor(0x314A, 0x5F00);
	write_cmos_sensor(0x3064, 0xFFCF);
	write_cmos_sensor(0x3066, 0x7E00);
	write_cmos_sensor(0x309C, 0x0640);
	write_cmos_sensor(0x380C, 0x001A);
	write_cmos_sensor(0x32B2, 0x0000);
	write_cmos_sensor(0x32B4, 0x0000);
	write_cmos_sensor(0x32B6, 0x0000);
	write_cmos_sensor(0x32B8, 0x0000);
	write_cmos_sensor(0x3090, 0x8800);
	write_cmos_sensor(0x3238, 0x000C);
	write_cmos_sensor(0x0100, 0x0000);
	write_cmos_sensor(0x0344, 0x0030);
	write_cmos_sensor(0x0348, 0x104F);
	write_cmos_sensor(0x0346, 0x0014);
	write_cmos_sensor(0x034A, 0x0C2B);
	write_cmos_sensor(0x034C, 0x1020);
	write_cmos_sensor(0x034E, 0x0C18);
	write_cmos_sensor(0x0202, 0x03DE);
	write_cmos_sensor(0x3400, 0x0000);
	write_cmos_sensor(0x3402, 0x4E46);
	write_cmos_sensor(0x0136, 0x1800);
	write_cmos_sensor(0x0304, 0x0004);
	write_cmos_sensor(0x0306, 0x0077);
	write_cmos_sensor(0x030C, 0x0004);
	write_cmos_sensor(0x030E, 0x0064);
	write_cmos_sensor(0x3C16, 0x0000);
	write_cmos_sensor(0x0342, 0x1320);
	write_cmos_sensor(0x0340, 0x0CC4);
	write_cmos_sensor(0x0900, 0x0000);
	write_cmos_sensor(0x0386, 0x0001);
	write_cmos_sensor(0x3452, 0x0000);
	write_cmos_sensor(0x345A, 0x0000);
	write_cmos_sensor(0x345C, 0x0000);
	write_cmos_sensor(0x345E, 0x0000);
	write_cmos_sensor(0x3460, 0x0000);
	write_cmos_sensor(0x38C4, 0x0009);
	write_cmos_sensor(0x38D8, 0x002A);
	write_cmos_sensor(0x38DA, 0x000A);
	write_cmos_sensor(0x38DC, 0x000B);
	write_cmos_sensor(0x38C2, 0x0009);
	write_cmos_sensor(0x38C0, 0x000E);
	write_cmos_sensor(0x38D6, 0x000A);
	write_cmos_sensor(0x38D4, 0x0008);
	write_cmos_sensor(0x38B0, 0x000E);
	write_cmos_sensor(0x3932, 0x1000);
	write_cmos_sensor(0x0820, 0x04B0);
	write_cmos_sensor(0x3C34, 0x0008);
	write_cmos_sensor(0x3C36, 0x3800);
	write_cmos_sensor(0x3C38, 0x0020);
	write_cmos_sensor(0x393E, 0x4000);
	write_cmos_sensor(0x3892, 0x2EE0);
	write_cmos_sensor(0x3C1E, 0x0100);
	write_cmos_sensor(0x0100, 0x0100);
	write_cmos_sensor(0x3C1E, 0x0000);
}

static void hs_video_setting(void)
{
	int i = 0;
	int framecnt = 0;
	write_cmos_sensor_byte(0x0100, 0x00);

	for (i = 0; i < 100; i++) {
		framecnt = read_cmos_sensor_byte(
			0x0005); // waiting for sensor to  stop output  then  set the  setting
		if (framecnt == 0xFF) {
			break;
		} else {
			mdelay(1);
		}
	}

	LOG_INF("E\n");
	//$MV1[MCLK:24,Width:640,Height:480,Format:MIPI_Raw10,mipi_lane:4,mipi_datarate:186,pvi_pclk_inverse:0]
	write_cmos_sensor(0x0344, 0x0100);
	write_cmos_sensor(0x0346, 0x0122);
	write_cmos_sensor(0x0348, 0x0F7F);
	write_cmos_sensor(0x034A, 0x0B1D);
	write_cmos_sensor(0x034C, 0x0E80);
	write_cmos_sensor(0x034E, 0x09FC);
	write_cmos_sensor(0x0900, 0x0000);
	write_cmos_sensor(0x0380, 0x0001);
	write_cmos_sensor(0x0382, 0x0001);
	write_cmos_sensor(0x0384, 0x0001);
	write_cmos_sensor(0x0386, 0x0001);
	write_cmos_sensor(0x0114, 0x0330);
	write_cmos_sensor(0x0110, 0x0002);
	write_cmos_sensor(0x0136, 0x1800);
	write_cmos_sensor(0x0304, 0x0004);
	write_cmos_sensor(0x0306, 0x0077);
	write_cmos_sensor(0x3C1E, 0x0000);
	write_cmos_sensor(0x030C, 0x0002);
	write_cmos_sensor(0x030E, 0x004B);
	write_cmos_sensor(0x3C16, 0x0001);
	write_cmos_sensor(0x0300, 0x0006);
	write_cmos_sensor(0x0342, 0x1600);
	write_cmos_sensor(0x0340, 0x0DDF);
	write_cmos_sensor(0x38C4, 0x0007);
	write_cmos_sensor(0x38D8, 0x001E);
	write_cmos_sensor(0x38DA, 0x0008);
	write_cmos_sensor(0x38DC, 0x0008);
	write_cmos_sensor(0x38C2, 0x0007);
	write_cmos_sensor(0x38C0, 0x000A);
	write_cmos_sensor(0x38D6, 0x0007);
	write_cmos_sensor(0x38D4, 0x0006);
	write_cmos_sensor(0x38B0, 0x000B);
	write_cmos_sensor(0x3932, 0x1000);
	write_cmos_sensor(0x3938, 0x000C);
	write_cmos_sensor(0x0820, 0x0384);
	write_cmos_sensor(0x380C, 0x00D5);
	write_cmos_sensor(0x3064, 0xFFCF);
	write_cmos_sensor(0x309C, 0x0640);
	write_cmos_sensor(0x3090, 0x8800);
	write_cmos_sensor(0x3238, 0x000C);
	write_cmos_sensor(0x314A, 0x5F00);
	write_cmos_sensor(0x3300, 0x0000);
	write_cmos_sensor(0x3400, 0x0000);
	write_cmos_sensor(0x3402, 0x4E42);
	write_cmos_sensor(0x32B2, 0x0008);
	write_cmos_sensor(0x32B4, 0x0008);
	write_cmos_sensor(0x32B6, 0x0008);
	write_cmos_sensor(0x32B8, 0x0008);
	write_cmos_sensor(0x3C34, 0x0048);
	write_cmos_sensor(0x3C36, 0x2000);
	write_cmos_sensor(0x3C38, 0x0030);
	write_cmos_sensor(0x393E, 0x4000);
	write_cmos_sensor(0x3C1E, 0x0100);
	write_cmos_sensor(0x0100, 0x0100);
	write_cmos_sensor(0x3C1E, 0x0000);
}

static void slim_video_setting(void)
{
	int i = 0;
	int framecnt = 0;
	write_cmos_sensor_byte(0x0100, 0x00);

	for (i = 0; i < 100; i++) {
		framecnt = read_cmos_sensor_byte(
			0x0005); // waiting for sensor to  stop output  then  set the  setting
		if (framecnt == 0xFF) {
			break;
		} else {
			mdelay(1);
		}
	}
	LOG_INF("E\n");
	//$MV1[MCLK:24,Width:1920,Height:1080,Format:MIPI_Raw10,mipi_lane:4,mipi_datarate:520,pvi_pclk_inverse:0]
	write_cmos_sensor(0x0344, 0x0198);
	write_cmos_sensor(0x0346, 0x0122);
	write_cmos_sensor(0x0348, 0x0EE7);
	write_cmos_sensor(0x034A, 0x0B1D);
	write_cmos_sensor(0x034C, 0x0D50);
	write_cmos_sensor(0x034E, 0x09FC);
	write_cmos_sensor(0x0900, 0x0000);
	write_cmos_sensor(0x0380, 0x0001);
	write_cmos_sensor(0x0382, 0x0001);
	write_cmos_sensor(0x0384, 0x0001);
	write_cmos_sensor(0x0386, 0x0001);
	write_cmos_sensor(0x0114, 0x0330);
	write_cmos_sensor(0x0110, 0x0002);
	write_cmos_sensor(0x0136, 0x1800);
	write_cmos_sensor(0x0304, 0x0004);
	write_cmos_sensor(0x0306, 0x0077);
	write_cmos_sensor(0x3C1E, 0x0000);
	write_cmos_sensor(0x030C, 0x0003);
	write_cmos_sensor(0x030E, 0x004B);
	write_cmos_sensor(0x3C16, 0x0000);
	write_cmos_sensor(0x0300, 0x0006);
	write_cmos_sensor(0x0342, 0x1320);
	write_cmos_sensor(0x0340, 0x0CBC);
	write_cmos_sensor(0x38C4, 0x0009);
	write_cmos_sensor(0x38D8, 0x002A);
	write_cmos_sensor(0x38DA, 0x000A);
	write_cmos_sensor(0x38DC, 0x000B);
	write_cmos_sensor(0x38C2, 0x000A);
	write_cmos_sensor(0x38C0, 0x000F);
	write_cmos_sensor(0x38D6, 0x000A);
	write_cmos_sensor(0x38D4, 0x0009);
	write_cmos_sensor(0x38B0, 0x000F);
	write_cmos_sensor(0x3932, 0x1800);
	write_cmos_sensor(0x3938, 0x000C);
	write_cmos_sensor(0x0820, 0x04b0);
	write_cmos_sensor(0x380C, 0x0090);
	write_cmos_sensor(0x3064, 0xFFCF);
	write_cmos_sensor(0x309C, 0x0640);
	write_cmos_sensor(0x3090, 0x8800);
	write_cmos_sensor(0x3238, 0x000C);
	write_cmos_sensor(0x314A, 0x5F00);
	write_cmos_sensor(0x3300, 0x0000);
	write_cmos_sensor(0x3400, 0x0000);
	write_cmos_sensor(0x3402, 0x4E42);
	write_cmos_sensor(0x32B2, 0x0006);
	write_cmos_sensor(0x32B4, 0x0006);
	write_cmos_sensor(0x32B6, 0x0006);
	write_cmos_sensor(0x32B8, 0x0006);
	write_cmos_sensor(0x3C34, 0x0048);
	write_cmos_sensor(0x3C36, 0x3000);
	write_cmos_sensor(0x3C38, 0x0020);
	write_cmos_sensor(0x393E, 0x4000);
	write_cmos_sensor(0x303A, 0x0204);
	write_cmos_sensor(0x3034, 0x4B01);
	write_cmos_sensor(0x3036, 0x0029);
	write_cmos_sensor(0x3032, 0x4800);
	write_cmos_sensor(0x320E, 0x049E);
	write_cmos_sensor(0x3C1E, 0x0100);
	write_cmos_sensor(0x0100, 0x0100);
	write_cmos_sensor(0x3C1E, 0x0000);
}
static void custom1_setting(void)
{
	int i = 0;
	int framecnt = 0;
	write_cmos_sensor_byte(0x0100, 0x00);

	for (i = 0; i < 100; i++) {
		framecnt = read_cmos_sensor_byte(
			0x0005); // waiting for sensor to  stop output  then  set the  setting
		if (framecnt == 0xFF) {
			break;
		} else {
			mdelay(1);
		}
	}
	LOG_INF("E\n");
	//$MV1[MCLK:24,Width:1920,Height:1080,Format:MIPI_Raw10,mipi_lane:4,mipi_datarate:520,pvi_pclk_inverse:0]
	write_cmos_sensor(0x0344, 0x01A0);
	write_cmos_sensor(0x0346, 0x0128);
	write_cmos_sensor(0x0348, 0x0EDF);
	write_cmos_sensor(0x034A, 0x0B17);
	write_cmos_sensor(0x034C, 0x06A0);
	write_cmos_sensor(0x034E, 0x04F8);
	write_cmos_sensor(0x0900, 0x0122);
	write_cmos_sensor(0x0380, 0x0001);
	write_cmos_sensor(0x0382, 0x0001);
	write_cmos_sensor(0x0384, 0x0001);
	write_cmos_sensor(0x0386, 0x0003);
	write_cmos_sensor(0x0114, 0x0330);
	write_cmos_sensor(0x0110, 0x0002);
	write_cmos_sensor(0x0136, 0x1800);
	write_cmos_sensor(0x0304, 0x0004);
	write_cmos_sensor(0x0306, 0x0077);
	write_cmos_sensor(0x3C1E, 0x0000);
	write_cmos_sensor(0x030C, 0x0003);
	write_cmos_sensor(0x030E, 0x0047);
	write_cmos_sensor(0x3C16, 0x0001);
	write_cmos_sensor(0x0300, 0x0006);
	write_cmos_sensor(0x0342, 0x1320);
	write_cmos_sensor(0x0340, 0x0CBC);
	write_cmos_sensor(0x38C4, 0x0004);
	write_cmos_sensor(0x38D8, 0x0011);
	write_cmos_sensor(0x38DA, 0x0005);
	write_cmos_sensor(0x38DC, 0x0005);
	write_cmos_sensor(0x38C2, 0x0005);
	write_cmos_sensor(0x38C0, 0x0004);
	write_cmos_sensor(0x38D6, 0x0004);
	write_cmos_sensor(0x38D4, 0x0004);
	write_cmos_sensor(0x38B0, 0x0007);
	write_cmos_sensor(0x3932, 0x1000);
	write_cmos_sensor(0x3938, 0x000C);
	write_cmos_sensor(0x0820, 0x0238);
	write_cmos_sensor(0x380C, 0x0049);
	write_cmos_sensor(0x3064, 0xFFCF);
	write_cmos_sensor(0x309C, 0x0640);
	write_cmos_sensor(0x3090, 0x8000);
	write_cmos_sensor(0x3238, 0x000B);
	write_cmos_sensor(0x314A, 0x5F02);
	write_cmos_sensor(0x3300, 0x0000);
	write_cmos_sensor(0x3400, 0x0000);
	write_cmos_sensor(0x3402, 0x4E46);
	write_cmos_sensor(0x32B2, 0x0008);
	write_cmos_sensor(0x32B4, 0x0008);
	write_cmos_sensor(0x32B6, 0x0008);
	write_cmos_sensor(0x32B8, 0x0008);
	write_cmos_sensor(0x3C34, 0x0048);
	write_cmos_sensor(0x3C36, 0x3000);
	write_cmos_sensor(0x3C38, 0x0020);
	write_cmos_sensor(0x393E, 0x4000);
	write_cmos_sensor(0x303A, 0x0204);
	write_cmos_sensor(0x3034, 0x4B01);
	write_cmos_sensor(0x3036, 0x0029);
	write_cmos_sensor(0x3032, 0x4800);
	write_cmos_sensor(0x320E, 0x049E);
	write_cmos_sensor(0x3C1E, 0x0100);
	write_cmos_sensor(0x0100, 0x0100);
	write_cmos_sensor(0x3C1E, 0x0000);
}

static void custom2_setting(void)
{
	int i = 0;
	int framecnt = 0;
	write_cmos_sensor_byte(0x0100, 0x00);

	for (i = 0; i < 100; i++) {
		framecnt = read_cmos_sensor_byte(
			0x0005); // waiting for sensor to  stop output  then  set the  setting
		if (framecnt == 0xFF) {
			break;
		} else {
			mdelay(1);
		}
	}
	LOG_INF("E\n");
	//$MV1[MCLK:24,Width:1920,Height:1080,Format:MIPI_Raw10,mipi_lane:4,mipi_datarate:520,pvi_pclk_inverse:0]
	write_cmos_sensor(0x314A, 0x5F00);
	write_cmos_sensor(0x3064, 0xFFCF);
	write_cmos_sensor(0x3066, 0x7E00);
	write_cmos_sensor(0x309C, 0x0640);
	write_cmos_sensor(0x380C, 0x004A);
	write_cmos_sensor(0x32B2, 0x0000);
	write_cmos_sensor(0x32B4, 0x0000);
	write_cmos_sensor(0x32B6, 0x0000);
	write_cmos_sensor(0x32B8, 0x0000);
	write_cmos_sensor(0x3090, 0x8800);
	write_cmos_sensor(0x3238, 0x000C);
	write_cmos_sensor(0x0100, 0x0000);
	write_cmos_sensor(0x0344, 0x0100);
	write_cmos_sensor(0x0348, 0x0F7F);
	write_cmos_sensor(0x0346, 0x020C);
	write_cmos_sensor(0x034A, 0x0A33);
	write_cmos_sensor(0x034C, 0x0740);
	write_cmos_sensor(0x034E, 0x0414);
	write_cmos_sensor(0x0202, 0x03DE);
	write_cmos_sensor(0x3400, 0x0000);
	write_cmos_sensor(0x3402, 0x4E46);
	write_cmos_sensor(0x0136, 0x1800);
	write_cmos_sensor(0x0304, 0x0004);
	write_cmos_sensor(0x0306, 0x0077);
	write_cmos_sensor(0x030C, 0x0004);
	write_cmos_sensor(0x030E, 0x0064);
	write_cmos_sensor(0x3C16, 0x0000);
	write_cmos_sensor(0x0342, 0x1320);
	write_cmos_sensor(0x0340, 0x0CC4);
	write_cmos_sensor(0x0900, 0x0122);
	write_cmos_sensor(0x0386, 0x0003);
	write_cmos_sensor(0x3452, 0x0000);
	write_cmos_sensor(0x345A, 0x0000);
	write_cmos_sensor(0x345C, 0x0000);
	write_cmos_sensor(0x345E, 0x0000);
	write_cmos_sensor(0x3460, 0x0000);
	write_cmos_sensor(0x38C4, 0x0009);
	write_cmos_sensor(0x38D8, 0x002A);
	write_cmos_sensor(0x38DA, 0x000A);
	write_cmos_sensor(0x38DC, 0x000B);
	write_cmos_sensor(0x38C2, 0x0009);
	write_cmos_sensor(0x38C0, 0x000E);
	write_cmos_sensor(0x38D6, 0x000A);
	write_cmos_sensor(0x38D4, 0x0008);
	write_cmos_sensor(0x38B0, 0x000E);
	write_cmos_sensor(0x3932, 0x1000);
	write_cmos_sensor(0x0820, 0x04B0);
	write_cmos_sensor(0x3C34, 0x0008);
	write_cmos_sensor(0x3C36, 0x3800);
	write_cmos_sensor(0x3C38, 0x0020);
	write_cmos_sensor(0x393E, 0x4000);
	write_cmos_sensor(0x3892, 0x2EE0);
	write_cmos_sensor(0x3C1E, 0x0100);
	write_cmos_sensor(0x0100, 0x0100);
	write_cmos_sensor(0x3C1E, 0x0000);
}
/*hs14 code for P221122-04485 by jianghongyan at 2022-11-25 start*/
static void custom3_setting(void)
{
	int i = 0;
	int framecnt = 0;
	write_cmos_sensor_byte(0x0100, 0x00);

	for (i = 0; i < 100; i++) {
		framecnt = read_cmos_sensor_byte(
			0x0005); // waiting for sensor to  stop output  then  set the  setting
		if (framecnt == 0xFF) {
			break;
		} else {
			mdelay(1);
		}
	}
	LOG_INF("E\n");
	//$MV1[MCLK:24,Width:1920,Height:1080,Format:MIPI_Raw10,mipi_lane:4,mipi_datarate:520,pvi_pclk_inverse:0]

	write_cmos_sensor(0x0344, 0x0030);
	write_cmos_sensor(0x0346, 0x0198);
	write_cmos_sensor(0x0348, 0x104F);
	write_cmos_sensor(0x034A, 0x0AA7);
	write_cmos_sensor(0x034C, 0x0810);
	write_cmos_sensor(0x034E, 0x0488);
	write_cmos_sensor(0x0900, 0x0122);
	write_cmos_sensor(0x0380, 0x0001);
	write_cmos_sensor(0x0382, 0x0001);
	write_cmos_sensor(0x0384, 0x0001);
	write_cmos_sensor(0x0386, 0x0003);
	write_cmos_sensor(0x0114, 0x0330);
	write_cmos_sensor(0x0110, 0x0002);
	write_cmos_sensor(0x0136, 0x1800);
	write_cmos_sensor(0x0304, 0x0004);
	write_cmos_sensor(0x0306, 0x0077);
	write_cmos_sensor(0x3C1E, 0x0000);
	write_cmos_sensor(0x030C, 0x0003);
	write_cmos_sensor(0x030E, 0x0047);
	write_cmos_sensor(0x3C16, 0x0001);
	write_cmos_sensor(0x0300, 0x0006);
	write_cmos_sensor(0x0342, 0x1320);
	write_cmos_sensor(0x0340, 0x0CBC);
	write_cmos_sensor(0x38C4, 0x0004);
	write_cmos_sensor(0x38D8, 0x0011);
	write_cmos_sensor(0x38DA, 0x0005);
	write_cmos_sensor(0x38DC, 0x0005);
	write_cmos_sensor(0x38C2, 0x0005);
	write_cmos_sensor(0x38C0, 0x0004);
	write_cmos_sensor(0x38D6, 0x0004);
	write_cmos_sensor(0x38D4, 0x0004);
	write_cmos_sensor(0x38B0, 0x0007);
	write_cmos_sensor(0x3932, 0x1000);
	write_cmos_sensor(0x3938, 0x000C);
	write_cmos_sensor(0x0820, 0x0238);
	write_cmos_sensor(0x380C, 0x0049);
	write_cmos_sensor(0x3064, 0xFFCF);
	write_cmos_sensor(0x309C, 0x0640);
	write_cmos_sensor(0x3090, 0x8000);
	write_cmos_sensor(0x3238, 0x000B);
	write_cmos_sensor(0x314A, 0x5F02);
	write_cmos_sensor(0x3300, 0x0000);
	write_cmos_sensor(0x3400, 0x0000);
	write_cmos_sensor(0x3402, 0x4E46);
	write_cmos_sensor(0x32B2, 0x0008);
	write_cmos_sensor(0x32B4, 0x0008);
	write_cmos_sensor(0x32B6, 0x0008);
	write_cmos_sensor(0x32B8, 0x0008);
	write_cmos_sensor(0x3C34, 0x0048);
	write_cmos_sensor(0x3C36, 0x3000);
	write_cmos_sensor(0x3C38, 0x0020);
	write_cmos_sensor(0x393E, 0x4000);
	write_cmos_sensor(0x303A, 0x0204);
	write_cmos_sensor(0x3034, 0x4B01);
	write_cmos_sensor(0x3036, 0x0029);
	write_cmos_sensor(0x3032, 0x4800);
	write_cmos_sensor(0x320E, 0x049E);
	write_cmos_sensor(0x3C1E, 0x0100);
	write_cmos_sensor(0x0100, 0x0100);
	write_cmos_sensor(0x3C1E, 0x0000);
}
/*hs14 code for P221122-04485 by jianghongyan at 2022-11-25 end*/
static void custom4_setting(void)
{
	int i = 0;
	int framecnt = 0;
	write_cmos_sensor_byte(0x0100, 0x00);

	for (i = 0; i < 100; i++) {
		framecnt = read_cmos_sensor_byte(
			0x0005); // waiting for sensor to  stop output  then  set the  setting
		if (framecnt == 0xFF) {
			break;
		} else {
			mdelay(1);
		}
	}
	LOG_INF("E\n");
	//$MV1[MCLK:24,Width:1920,Height:1080,Format:MIPI_Raw10,mipi_lane:4,mipi_datarate:520,pvi_pclk_inverse:0]
	write_cmos_sensor(0x314A, 0x5F02);
	write_cmos_sensor(0x3064, 0xFFCF);
	write_cmos_sensor(0x3066, 0x7E00);
	write_cmos_sensor(0x309C, 0x0640);
	write_cmos_sensor(0x380C, 0x0090);
	write_cmos_sensor(0x32B2, 0x0003);
	write_cmos_sensor(0x32B4, 0x0003);
	write_cmos_sensor(0x32B6, 0x0003);
	write_cmos_sensor(0x32B8, 0x0003);
	write_cmos_sensor(0x3090, 0x8000);
	write_cmos_sensor(0x3238, 0x000B);
	write_cmos_sensor(0x0100, 0x0000);
	write_cmos_sensor(0x0344, 0x0030);
	write_cmos_sensor(0x0348, 0x104F);
	write_cmos_sensor(0x0346, 0x0014);
	write_cmos_sensor(0x034A, 0x0C2B);
	write_cmos_sensor(0x034C, 0x0810);
	write_cmos_sensor(0x034E, 0x060C);
	write_cmos_sensor(0x0202, 0x0656);
	write_cmos_sensor(0x3400, 0x0000);
	write_cmos_sensor(0x3402, 0x4E46);
	write_cmos_sensor(0x0136, 0x1800);
	write_cmos_sensor(0x0304, 0x0004);
	write_cmos_sensor(0x0306, 0x0077);
	write_cmos_sensor(0x030C, 0x0004);
	write_cmos_sensor(0x030E, 0x0064);
	write_cmos_sensor(0x3C16, 0x0000);
	write_cmos_sensor(0x0342, 0x1320);
	write_cmos_sensor(0x0340, 0x0CC4);
	write_cmos_sensor(0x0900, 0x0122);
	write_cmos_sensor(0x0386, 0x0003);
	write_cmos_sensor(0x3452, 0x0000);
	write_cmos_sensor(0x345A, 0x0000);
	write_cmos_sensor(0x345C, 0x0000);
	write_cmos_sensor(0x345E, 0x0000);
	write_cmos_sensor(0x3460, 0x0000);
	write_cmos_sensor(0x38C4, 0x0009);
	write_cmos_sensor(0x38D8, 0x002A);
	write_cmos_sensor(0x38DA, 0x000A);
	write_cmos_sensor(0x38DC, 0x000B);
	write_cmos_sensor(0x38C2, 0x0009);
	write_cmos_sensor(0x38C0, 0x000E);
	write_cmos_sensor(0x38D6, 0x000A);
	write_cmos_sensor(0x38D4, 0x0008);
	write_cmos_sensor(0x38B0, 0x000E);
	write_cmos_sensor(0x3932, 0x1000);
	write_cmos_sensor(0x0820, 0x04B0);
	write_cmos_sensor(0x3C34, 0x0008);
	write_cmos_sensor(0x3C36, 0x3800);
	write_cmos_sensor(0x3C38, 0x0020);
	write_cmos_sensor(0x393E, 0x4000);
	write_cmos_sensor(0x3892, 0x2EE0);
	write_cmos_sensor(0x3C1E, 0x0100);
	write_cmos_sensor(0x0100, 0x0100);
	write_cmos_sensor(0x3C1E, 0x0000);
}
static void custom5_setting(void)
{
	int i = 0;
	int framecnt = 0;
	write_cmos_sensor_byte(0x0100, 0x00);

	for (i = 0; i < 100; i++) {
		framecnt = read_cmos_sensor_byte(
			0x0005); // waiting for sensor to  stop output  then  set the  setting
		if (framecnt == 0xFF) {
			break;
		} else {
			mdelay(1);
		}
	}
	LOG_INF("E\n");
	//$MV1[MCLK:24,Width:1920,Height:1080,Format:MIPI_Raw10,mipi_lane:4,mipi_datarate:520,pvi_pclk_inverse:0]
	write_cmos_sensor(0x314A, 0x5F00);
	write_cmos_sensor(0x3064, 0xFFCF);
	write_cmos_sensor(0x3066, 0x7E00);
	write_cmos_sensor(0x309C, 0x0640);
	write_cmos_sensor(0x380C, 0x001A);
	write_cmos_sensor(0x32B2, 0x0000);
	write_cmos_sensor(0x32B4, 0x0000);
	write_cmos_sensor(0x32B6, 0x0000);
	write_cmos_sensor(0x32B8, 0x0000);
	write_cmos_sensor(0x3090, 0x8800);
	write_cmos_sensor(0x3238, 0x000C);
	write_cmos_sensor(0x0100, 0x0000);
	write_cmos_sensor(0x0344, 0x0030);
	write_cmos_sensor(0x0348, 0x104F);
	write_cmos_sensor(0x0346, 0x0196);
	write_cmos_sensor(0x034A, 0x0AA9);
	write_cmos_sensor(0x034C, 0x1020);
	write_cmos_sensor(0x034E, 0x0914);
	write_cmos_sensor(0x0202, 0x03DE);
	write_cmos_sensor(0x3400, 0x0000);
	write_cmos_sensor(0x3402, 0x4E46);
	write_cmos_sensor(0x0136, 0x1800);
	write_cmos_sensor(0x0304, 0x0004);
	write_cmos_sensor(0x0306, 0x0077);
	write_cmos_sensor(0x030C, 0x0004);
	write_cmos_sensor(0x030E, 0x0064);
	write_cmos_sensor(0x3C16, 0x0000);
	write_cmos_sensor(0x0342, 0x1320);
	write_cmos_sensor(0x0340, 0x0CC4);
	write_cmos_sensor(0x0900, 0x0000);
	write_cmos_sensor(0x0386, 0x0001);
	write_cmos_sensor(0x3452, 0x0000);
	write_cmos_sensor(0x345A, 0x0000);
	write_cmos_sensor(0x345C, 0x0000);
	write_cmos_sensor(0x345E, 0x0000);
	write_cmos_sensor(0x3460, 0x0000);
	write_cmos_sensor(0x38C4, 0x0009);
	write_cmos_sensor(0x38D8, 0x002A);
	write_cmos_sensor(0x38DA, 0x000A);
	write_cmos_sensor(0x38DC, 0x000B);
	write_cmos_sensor(0x38C2, 0x0009);
	write_cmos_sensor(0x38C0, 0x000E);
	write_cmos_sensor(0x38D6, 0x000A);
	write_cmos_sensor(0x38D4, 0x0008);
	write_cmos_sensor(0x38B0, 0x000E);
	write_cmos_sensor(0x3932, 0x1000);
	write_cmos_sensor(0x0820, 0x04B0);
	write_cmos_sensor(0x3C34, 0x0008);
	write_cmos_sensor(0x3C36, 0x3800);
	write_cmos_sensor(0x3C38, 0x0020);
	write_cmos_sensor(0x393E, 0x4000);
	write_cmos_sensor(0x3892, 0x2EE0);
	write_cmos_sensor(0x3C1E, 0x0100);
	write_cmos_sensor(0x0100, 0x0100);
	write_cmos_sensor(0x3C1E, 0x0000);
}
static kal_uint32 return_sensor_id(void)
{
	return ((read_cmos_sensor_byte(0x0000) << 8) |
		read_cmos_sensor_byte(0x0001));
}

/*************************************************************************
* FUNCTION
*	get_imgsensor_id
*
* DESCRIPTION
*	This function get the sensor ID
*
* PARAMETERS
*	*sensorID : return the sensor ID
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/

static kal_uint32 get_imgsensor_id(UINT32 *sensor_id)
{
	kal_uint8 i = 0;
	kal_uint8 retry = 2;
	//sensor have two i2c address 0x5b 0x5a & 0x21 0x20, we should detect the module used i2c address
	while (imgsensor_info.i2c_addr_table[i] != 0xff) {
		spin_lock(&imgsensor_drv_lock);
		imgsensor.i2c_write_id = imgsensor_info.i2c_addr_table[i];
		spin_unlock(&imgsensor_drv_lock);
		do {
			*sensor_id = return_sensor_id();
			if (*sensor_id == imgsensor_info.sensor_id) {
				LOG_INF("i2c write id: 0x%x, ReadOut sensor id: 0x%x, imgsensor_info.sensor_id:0x%x.\n",
					imgsensor.i2c_write_id, *sensor_id,
					imgsensor_info.sensor_id);
				return ERROR_NONE;
			}
			LOG_INF("Read sensor id fail, i2c write id: 0x%x, ReadOut sensor id: 0x%x, imgsensor_info.sensor_id:0x%x.\n",
				imgsensor.i2c_write_id, *sensor_id,
				imgsensor_info.sensor_id);
			retry--;
		} while (retry > 0);
		i++;
		retry = 1;
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
*	open
*
* DESCRIPTION
*	This function initialize the registers of CMOS sensor
*
* PARAMETERS
*	None
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
static kal_uint32 open(void)
{
	//const kal_uint8 i2c_addr[] = {IMGSENSOR_WRITE_ID_1, IMGSENSOR_WRITE_ID_2};
	kal_uint8 i = 0;
	kal_uint8 retry = 2;
	kal_uint32 sensor_id = 0;
	LOG_1;
	//sensor have two i2c address 0x6c 0x6d & 0x21 0x20, we should detect the module used i2c address
	while (imgsensor_info.i2c_addr_table[i] != 0xff) {
		spin_lock(&imgsensor_drv_lock);
		imgsensor.i2c_write_id = imgsensor_info.i2c_addr_table[i];
		spin_unlock(&imgsensor_drv_lock);
		do {
			sensor_id = return_sensor_id();
			if (sensor_id == imgsensor_info.sensor_id) {
				LOG_INF("i2c write id: 0x%x, sensor id: 0x%x\n",
					imgsensor.i2c_write_id, sensor_id);
				break;
			}
			LOG_INF("Read sensor id fail, id: 0x%x, sensor id: 0x%x\n",
				imgsensor.i2c_write_id, sensor_id);
			retry--;
		} while (retry > 0);
		i++;
		if (sensor_id == imgsensor_info.sensor_id)
			break;
		retry = 2;
	}
	if (imgsensor_info.sensor_id != sensor_id)
		return ERROR_SENSOR_CONNECT_FAIL;

	/* initail sequence write in  */
	sensor_init();

	spin_lock(&imgsensor_drv_lock);

	imgsensor.autoflicker_en = KAL_FALSE;
	imgsensor.sensor_mode = IMGSENSOR_MODE_INIT;
	imgsensor.pclk = imgsensor_info.pre.pclk;
	imgsensor.frame_length = imgsensor_info.pre.framelength;
	imgsensor.line_length = imgsensor_info.pre.linelength;
	imgsensor.min_frame_length = imgsensor_info.pre.framelength;
	imgsensor.dummy_pixel = 0;
	imgsensor.dummy_line = 0;
	imgsensor.ihdr_en = KAL_FALSE;
	imgsensor.test_pattern = KAL_FALSE;
	imgsensor.current_fps = imgsensor_info.pre.max_framerate;
	spin_unlock(&imgsensor_drv_lock);

	return ERROR_NONE;
} /*  open  */

/*************************************************************************
* FUNCTION
*	close
*
* DESCRIPTION
*
*
* PARAMETERS
*	None
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
static kal_uint32 close(void)
{
	LOG_INF("E\n");

	/*No Need to implement this function*/

	return ERROR_NONE;
} /*	close  */

/*************************************************************************
* FUNCTION
* preview
*
* DESCRIPTION
*	This function start the sensor preview.
*
* PARAMETERS
*	*image_window : address pointer of pixel numbers in one period of HSYNC
*  *sensor_config_data : address pointer of line numbers in one period of VSYNC
*
* RETURNS
*	None
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
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	preview_setting();
	set_mirror_flip(IMAGE_NORMAL);
	mdelay(2);
	/*
	#ifdef FANPENGTAO
	int i=0;
	for(i=0; i<10; i++){
		LOG_INF("delay time = %d, the frame no = %d\n", i*10, read_cmos_sensor(0x0005));
		mdelay(10);
	}
	#endif
	*/
	return ERROR_NONE;
} /*	preview   */

/*************************************************************************
* FUNCTION
*	capture
*
* DESCRIPTION
*	This function setup the CMOS sensor in capture MY_OUTPUT mode
*
* PARAMETERS
*
* RETURNS
*	None
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
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	capture_setting();
	set_mirror_flip(IMAGE_NORMAL);
	mdelay(2);

#if 0
	if(imgsensor.test_pattern == KAL_TRUE)
	{
		//write_cmos_sensor(0x5002,0x00);
  }
#endif

	return ERROR_NONE;
} /* capture() */

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
	//imgsensor.current_fps = 300;
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	normal_video_setting();
	set_mirror_flip(IMAGE_NORMAL);

	return ERROR_NONE;
} /*	normal_video   */

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
	imgsensor.dummy_line = 0;
	imgsensor.dummy_pixel = 0;
	//imgsensor.current_fps = 300;
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	hs_video_setting();
	set_mirror_flip(IMAGE_NORMAL);

	return ERROR_NONE;
} /*	hs_video   */

static kal_uint32 slim_video(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
			     MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("E\n");

	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_SLIM_VIDEO;
	imgsensor.pclk = imgsensor_info.slim_video.pclk;
	//imgsensor.video_mode = KAL_TRUE;
	imgsensor.line_length = imgsensor_info.slim_video.linelength;
	imgsensor.frame_length = imgsensor_info.slim_video.framelength;
	imgsensor.min_frame_length = imgsensor_info.slim_video.framelength;
	imgsensor.dummy_line = 0;
	imgsensor.dummy_pixel = 0;
	//imgsensor.current_fps = 300;
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	slim_video_setting();
	set_mirror_flip(IMAGE_NORMAL);
	return ERROR_NONE;
}
static kal_uint32 custom1(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
			  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("E\n");

	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_CUSTOM1;
	imgsensor.pclk = imgsensor_info.custom1.pclk;
	//imgsensor.video_mode = KAL_TRUE;
	imgsensor.line_length = imgsensor_info.custom1.linelength;
	imgsensor.frame_length = imgsensor_info.custom1.framelength;
	imgsensor.min_frame_length = imgsensor_info.custom1.framelength;
	imgsensor.dummy_line = 0;
	imgsensor.dummy_pixel = 0;
	//imgsensor.current_fps = 300;
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	custom1_setting();
	set_mirror_flip(IMAGE_NORMAL);
	return ERROR_NONE;
}
static kal_uint32 custom2(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
			  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("E\n");

	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_CUSTOM2;
	imgsensor.pclk = imgsensor_info.custom2.pclk;
	//imgsensor.video_mode = KAL_TRUE;
	imgsensor.line_length = imgsensor_info.custom2.linelength;
	imgsensor.frame_length = imgsensor_info.custom2.framelength;
	imgsensor.min_frame_length = imgsensor_info.custom2.framelength;
	imgsensor.dummy_line = 0;
	imgsensor.dummy_pixel = 0;
	//imgsensor.current_fps = 300;
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	custom2_setting();
	set_mirror_flip(IMAGE_NORMAL);
	return ERROR_NONE;
}
static kal_uint32 custom3(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
			  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("E\n");

	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_CUSTOM3;
	imgsensor.pclk = imgsensor_info.custom3.pclk;
	//imgsensor.video_mode = KAL_TRUE;
	imgsensor.line_length = imgsensor_info.custom3.linelength;
	imgsensor.frame_length = imgsensor_info.custom3.framelength;
	imgsensor.min_frame_length = imgsensor_info.custom3.framelength;
	imgsensor.dummy_line = 0;
	imgsensor.dummy_pixel = 0;
	//imgsensor.current_fps = 300;
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	custom3_setting();
	set_mirror_flip(IMAGE_NORMAL);
	return ERROR_NONE;
}
static kal_uint32 custom4(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
			  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("E\n");

	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_CUSTOM4;
	imgsensor.pclk = imgsensor_info.custom4.pclk;
	//imgsensor.video_mode = KAL_TRUE;
	imgsensor.line_length = imgsensor_info.custom4.linelength;
	imgsensor.frame_length = imgsensor_info.custom4.framelength;
	imgsensor.min_frame_length = imgsensor_info.custom4.framelength;
	imgsensor.dummy_line = 0;
	imgsensor.dummy_pixel = 0;
	//imgsensor.current_fps = 300;
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	custom4_setting();
	set_mirror_flip(IMAGE_NORMAL);
	return ERROR_NONE;
}
static kal_uint32 custom5(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
			  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("E\n");

	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_CUSTOM5;
	imgsensor.pclk = imgsensor_info.custom5.pclk;
	//imgsensor.video_mode = KAL_TRUE;
	imgsensor.line_length = imgsensor_info.custom5.linelength;
	imgsensor.frame_length = imgsensor_info.custom5.framelength;
	imgsensor.min_frame_length = imgsensor_info.custom5.framelength;
	imgsensor.dummy_line = 0;
	imgsensor.dummy_pixel = 0;
	//imgsensor.current_fps = 300;
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	custom5_setting();
	set_mirror_flip(IMAGE_NORMAL);
	return ERROR_NONE;
}

static kal_uint32
get_resolution(MSDK_SENSOR_RESOLUTION_INFO_STRUCT *sensor_resolution)
{
	LOG_INF("E\n");
	sensor_resolution->SensorFullWidth =
		imgsensor_info.cap.grabwindow_width;
	sensor_resolution->SensorFullHeight =
		imgsensor_info.cap.grabwindow_height;

	sensor_resolution->SensorPreviewWidth =
		imgsensor_info.pre.grabwindow_width;
	sensor_resolution->SensorPreviewHeight =
		imgsensor_info.pre.grabwindow_height;

	sensor_resolution->SensorVideoWidth =
		imgsensor_info.normal_video.grabwindow_width;
	sensor_resolution->SensorVideoHeight =
		imgsensor_info.normal_video.grabwindow_height;

	sensor_resolution->SensorHighSpeedVideoWidth =
		imgsensor_info.hs_video.grabwindow_width;
	sensor_resolution->SensorHighSpeedVideoHeight =
		imgsensor_info.hs_video.grabwindow_height;

	sensor_resolution->SensorSlimVideoWidth =
		imgsensor_info.slim_video.grabwindow_width;
	sensor_resolution->SensorSlimVideoHeight =
		imgsensor_info.slim_video.grabwindow_height;
	sensor_resolution->SensorCustom1Width =
		imgsensor_info.custom1.grabwindow_width;
	sensor_resolution->SensorCustom1Height =
		imgsensor_info.custom1.grabwindow_height;
	sensor_resolution->SensorCustom2Width =
		imgsensor_info.custom2.grabwindow_width;
	sensor_resolution->SensorCustom2Height =
		imgsensor_info.custom2.grabwindow_height;
	sensor_resolution->SensorCustom3Width =
		imgsensor_info.custom3.grabwindow_width;
	sensor_resolution->SensorCustom3Height =
		imgsensor_info.custom3.grabwindow_height;
	sensor_resolution->SensorCustom4Width =
		imgsensor_info.custom4.grabwindow_width;
	sensor_resolution->SensorCustom4Height =
		imgsensor_info.custom4.grabwindow_height;
	sensor_resolution->SensorCustom5Width =
		imgsensor_info.custom5.grabwindow_width;
	sensor_resolution->SensorCustom5Height =
		imgsensor_info.custom5.grabwindow_height;
	return ERROR_NONE;
} /*	get_resolution	*/

static kal_uint32 get_info(enum MSDK_SCENARIO_ID_ENUM scenario_id,
			   MSDK_SENSOR_INFO_STRUCT *sensor_info,
			   MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("scenario_id = %d\n", scenario_id);

	//sensor_info->SensorVideoFrameRate = imgsensor_info.normal_video.max_framerate/10; /* not use */
	//sensor_info->SensorStillCaptureFrameRate= imgsensor_info.cap.max_framerate/10; /* not use */
	//imgsensor_info->SensorWebCamCaptureFrameRate= imgsensor_info.v.max_framerate; /* not use */

	sensor_info->SensorClockPolarity = SENSOR_CLOCK_POLARITY_LOW;
	sensor_info->SensorClockFallingPolarity =
		SENSOR_CLOCK_POLARITY_LOW; /* not use */
	sensor_info->SensorHsyncPolarity =
		SENSOR_CLOCK_POLARITY_LOW; // inverse with datasheet
	sensor_info->SensorVsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
	sensor_info->SensorInterruptDelayLines = 4; /* not use */
	sensor_info->SensorResetActiveHigh = FALSE; /* not use */
	sensor_info->SensorResetDelayCount = 5; /* not use */

	sensor_info->SensroInterfaceType = imgsensor_info.sensor_interface_type;
	sensor_info->MIPIsensorType = imgsensor_info.mipi_sensor_type;
	sensor_info->SettleDelayMode = imgsensor_info.mipi_settle_delay_mode;
	sensor_info->SensorOutputDataFormat =
		imgsensor_info.sensor_output_dataformat;

	sensor_info->CaptureDelayFrame = imgsensor_info.cap_delay_frame;
	sensor_info->PreviewDelayFrame = imgsensor_info.pre_delay_frame;
	sensor_info->VideoDelayFrame = imgsensor_info.video_delay_frame;
	sensor_info->HighSpeedVideoDelayFrame = imgsensor_info.hs_video_delay_frame;
	sensor_info->SlimVideoDelayFrame = imgsensor_info.slim_video_delay_frame;
	sensor_info->Custom1DelayFrame = imgsensor_info.custom1_delay_frame;
	sensor_info->Custom2DelayFrame = imgsensor_info.custom2_delay_frame;
	sensor_info->Custom3DelayFrame = imgsensor_info.custom3_delay_frame;
	sensor_info->Custom4DelayFrame = imgsensor_info.custom4_delay_frame;
	sensor_info->Custom5DelayFrame = imgsensor_info.custom5_delay_frame;

	sensor_info->SensorMasterClockSwitch = 0; /* not use */
	sensor_info->SensorDrivingCurrent = imgsensor_info.isp_driving_current;

	sensor_info->AEShutDelayFrame =
		imgsensor_info
			.ae_shut_delay_frame; /* The frame of setting shutter default 0 for TG int */
	sensor_info->AESensorGainDelayFrame =
		imgsensor_info
			.ae_sensor_gain_delay_frame; /* The frame of setting sensor gain */
	sensor_info->AEISPGainDelayFrame =
		imgsensor_info.ae_ispGain_delay_frame;
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
	sensor_info->SensorWidthSampling = 0; // 0 is default 1x
	sensor_info->SensorHightSampling = 0; // 0 is default 1x
	sensor_info->SensorPacketECCOrder = 1;

	switch (scenario_id) {
	case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		sensor_info->SensorGrabStartX = imgsensor_info.pre.startx;
		sensor_info->SensorGrabStartY = imgsensor_info.pre.starty;

		sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount =
			imgsensor_info.pre.mipi_data_lp2hs_settle_dc;

		break;
	case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		sensor_info->SensorGrabStartX = imgsensor_info.cap.startx;
		sensor_info->SensorGrabStartY = imgsensor_info.cap.starty;

		sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount =
			imgsensor_info.cap.mipi_data_lp2hs_settle_dc;

		break;
	case MSDK_SCENARIO_ID_VIDEO_PREVIEW:

		sensor_info->SensorGrabStartX =
			imgsensor_info.normal_video.startx;
		sensor_info->SensorGrabStartY =
			imgsensor_info.normal_video.starty;

		sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount =
			imgsensor_info.normal_video.mipi_data_lp2hs_settle_dc;

		break;
	case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
		sensor_info->SensorGrabStartX = imgsensor_info.hs_video.startx;
		sensor_info->SensorGrabStartY = imgsensor_info.hs_video.starty;

		sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount =
			imgsensor_info.hs_video.mipi_data_lp2hs_settle_dc;

		break;
	case MSDK_SCENARIO_ID_SLIM_VIDEO:
		sensor_info->SensorGrabStartX =
			imgsensor_info.slim_video.startx;
		sensor_info->SensorGrabStartY =
			imgsensor_info.slim_video.starty;

		sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount =
			imgsensor_info.slim_video.mipi_data_lp2hs_settle_dc;

		break;
	case MSDK_SCENARIO_ID_CUSTOM1:
		sensor_info->SensorGrabStartX = imgsensor_info.custom1.startx;
		sensor_info->SensorGrabStartY = imgsensor_info.custom1.starty;

		sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount =
			imgsensor_info.custom1.mipi_data_lp2hs_settle_dc;

		break;
	case MSDK_SCENARIO_ID_CUSTOM2:
		sensor_info->SensorGrabStartX = imgsensor_info.custom2.startx;
		sensor_info->SensorGrabStartY = imgsensor_info.custom2.starty;

		sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount =
			imgsensor_info.custom2.mipi_data_lp2hs_settle_dc;

		break;
	case MSDK_SCENARIO_ID_CUSTOM3:
		sensor_info->SensorGrabStartX = imgsensor_info.custom3.startx;
		sensor_info->SensorGrabStartY = imgsensor_info.custom3.starty;

		sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount =
			imgsensor_info.custom3.mipi_data_lp2hs_settle_dc;

		break;
	case MSDK_SCENARIO_ID_CUSTOM4:
		sensor_info->SensorGrabStartX = imgsensor_info.custom4.startx;
		sensor_info->SensorGrabStartY = imgsensor_info.custom4.starty;

		sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount =
			imgsensor_info.custom4.mipi_data_lp2hs_settle_dc;

		break;
	case MSDK_SCENARIO_ID_CUSTOM5:
		sensor_info->SensorGrabStartX = imgsensor_info.custom5.startx;
		sensor_info->SensorGrabStartY = imgsensor_info.custom5.starty;

		sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount =
			imgsensor_info.custom5.mipi_data_lp2hs_settle_dc;

		break;
	default:
		sensor_info->SensorGrabStartX = imgsensor_info.pre.startx;
		sensor_info->SensorGrabStartY = imgsensor_info.pre.starty;

		sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount =
			imgsensor_info.pre.mipi_data_lp2hs_settle_dc;
		break;
	}

	return ERROR_NONE;
} /*	get_info  */

static kal_uint32 control(enum MSDK_SCENARIO_ID_ENUM scenario_id,
			  MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
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
	default:
		LOG_INF("Error ScenarioId setting");
		preview(image_window, sensor_config_data);
		return ERROR_INVALID_SCENARIO_ID;
	}
	return ERROR_NONE;
} /* control() */

static kal_uint32 set_video_mode(UINT16 framerate)
{
	LOG_INF("framerate = %d\n ", framerate);
	// SetVideoMode Function should fix framerate
	if (framerate == 0)
		// Dynamic frame rate
		return ERROR_NONE;
	spin_lock(&imgsensor_drv_lock);
	if ((framerate == 300) && (imgsensor.autoflicker_en == KAL_TRUE))
		imgsensor.current_fps = 296;
	else if ((framerate == 150) && (imgsensor.autoflicker_en == KAL_TRUE))
		imgsensor.current_fps = 146;
	else
		imgsensor.current_fps = framerate;
	spin_unlock(&imgsensor_drv_lock);
	set_max_framerate(imgsensor.current_fps, 1);

	return ERROR_NONE;
}

static kal_uint32 set_auto_flicker_mode(kal_bool enable, UINT16 framerate)
{
	LOG_INF("enable = %d, framerate = %d \n", enable, framerate);
	spin_lock(&imgsensor_drv_lock);
	if (enable) //enable auto flicker
		imgsensor.autoflicker_en = KAL_TRUE;
	else //Cancel Auto flick
		imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	return ERROR_NONE;
}

static kal_uint32
set_max_framerate_by_scenario(enum MSDK_SCENARIO_ID_ENUM scenario_id,
			      MUINT32 framerate)
{
	kal_uint32 frame_length;

	LOG_INF("scenario_id = %d, framerate = %d\n", scenario_id, framerate);

	switch (scenario_id) {
	case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		frame_length = imgsensor_info.pre.pclk / framerate * 10 /
			       imgsensor_info.pre.linelength;
		spin_lock(&imgsensor_drv_lock);
		imgsensor.dummy_line =
			(frame_length > imgsensor_info.pre.framelength) ?
				(frame_length -
				 imgsensor_info.pre.framelength) :
				0;
		imgsensor.frame_length =
			imgsensor_info.pre.framelength + imgsensor.dummy_line;
		imgsensor.min_frame_length = imgsensor.frame_length;
		spin_unlock(&imgsensor_drv_lock);
		if (imgsensor.frame_length > imgsensor.shutter)
			set_dummy();
		break;
	case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
		if (framerate == 0)
			return ERROR_NONE;
		frame_length = imgsensor_info.normal_video.pclk / framerate *
			       10 / imgsensor_info.normal_video.linelength;
		spin_lock(&imgsensor_drv_lock);
		imgsensor.dummy_line =
			(frame_length >
			 imgsensor_info.normal_video.framelength) ?
				(frame_length -
				 imgsensor_info.normal_video.framelength) :
				0;
		imgsensor.frame_length =
			imgsensor_info.normal_video.framelength +
			imgsensor.dummy_line;
		imgsensor.min_frame_length = imgsensor.frame_length;
		spin_unlock(&imgsensor_drv_lock);
		if (imgsensor.frame_length > imgsensor.shutter)
			set_dummy();
		break;
	case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		frame_length = imgsensor_info.cap.pclk / framerate * 10 /
			       imgsensor_info.cap.linelength;
		spin_lock(&imgsensor_drv_lock);
		imgsensor.dummy_line =
			(frame_length > imgsensor_info.cap.framelength) ?
				(frame_length -
				 imgsensor_info.cap.framelength) :
				0;
		imgsensor.frame_length =
			imgsensor_info.cap.framelength + imgsensor.dummy_line;
		imgsensor.min_frame_length = imgsensor.frame_length;
		spin_unlock(&imgsensor_drv_lock);
		if (imgsensor.frame_length > imgsensor.shutter)
			set_dummy();
		break;
	case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
		frame_length = imgsensor_info.hs_video.pclk / framerate * 10 /
			       imgsensor_info.hs_video.linelength;
		spin_lock(&imgsensor_drv_lock);
		imgsensor.dummy_line =
			(frame_length > imgsensor_info.hs_video.framelength) ?
				(frame_length -
				 imgsensor_info.hs_video.framelength) :
				0;
		imgsensor.frame_length = imgsensor_info.hs_video.framelength +
					 imgsensor.dummy_line;
		imgsensor.min_frame_length = imgsensor.frame_length;
		spin_unlock(&imgsensor_drv_lock);
		if (imgsensor.frame_length > imgsensor.shutter)
			set_dummy();
		break;
	case MSDK_SCENARIO_ID_SLIM_VIDEO:
		frame_length = imgsensor_info.slim_video.pclk / framerate * 10 /
			       imgsensor_info.slim_video.linelength;
		spin_lock(&imgsensor_drv_lock);
		imgsensor.dummy_line =
			(frame_length > imgsensor_info.slim_video.framelength) ?
				(frame_length -
				 imgsensor_info.slim_video.framelength) :
				0;
		imgsensor.frame_length = imgsensor_info.slim_video.framelength +
					 imgsensor.dummy_line;
		imgsensor.min_frame_length = imgsensor.frame_length;
		spin_unlock(&imgsensor_drv_lock);
		if (imgsensor.frame_length > imgsensor.shutter)
			set_dummy();
		break;
	case MSDK_SCENARIO_ID_CUSTOM1:
		frame_length = imgsensor_info.custom1.pclk / framerate * 10 /
			       imgsensor_info.custom1.linelength;
		spin_lock(&imgsensor_drv_lock);
		imgsensor.dummy_line =
			(frame_length > imgsensor_info.custom1.framelength) ?
				(frame_length -
				 imgsensor_info.custom1.framelength) :
				0;
		imgsensor.frame_length = imgsensor_info.custom1.framelength +
					 imgsensor.dummy_line;
		imgsensor.min_frame_length = imgsensor.frame_length;
		spin_unlock(&imgsensor_drv_lock);
		if (imgsensor.frame_length > imgsensor.shutter)
			set_dummy();
		break;
	case MSDK_SCENARIO_ID_CUSTOM2:
		frame_length = imgsensor_info.custom2.pclk / framerate * 10 /
			       imgsensor_info.custom2.linelength;
		spin_lock(&imgsensor_drv_lock);
		imgsensor.dummy_line =
			(frame_length > imgsensor_info.custom2.framelength) ?
				(frame_length -
				 imgsensor_info.custom2.framelength) :
				0;
		imgsensor.frame_length = imgsensor_info.custom2.framelength +
					 imgsensor.dummy_line;
		imgsensor.min_frame_length = imgsensor.frame_length;
		spin_unlock(&imgsensor_drv_lock);
		if (imgsensor.frame_length > imgsensor.shutter)
			set_dummy();
		break;
	case MSDK_SCENARIO_ID_CUSTOM3:
		frame_length = imgsensor_info.custom3.pclk / framerate * 10 /
			       imgsensor_info.custom3.linelength;
		spin_lock(&imgsensor_drv_lock);
		imgsensor.dummy_line =
			(frame_length > imgsensor_info.custom3.framelength) ?
				(frame_length -
				 imgsensor_info.custom3.framelength) :
				0;
		imgsensor.frame_length = imgsensor_info.custom3.framelength +
					 imgsensor.dummy_line;
		imgsensor.min_frame_length = imgsensor.frame_length;
		spin_unlock(&imgsensor_drv_lock);
		if (imgsensor.frame_length > imgsensor.shutter)
			set_dummy();
		break;
	case MSDK_SCENARIO_ID_CUSTOM4:
		frame_length = imgsensor_info.custom4.pclk / framerate * 10 /
			       imgsensor_info.custom4.linelength;
		spin_lock(&imgsensor_drv_lock);
		imgsensor.dummy_line =
			(frame_length > imgsensor_info.custom4.framelength) ?
				(frame_length -
				 imgsensor_info.custom4.framelength) :
				0;
		imgsensor.frame_length = imgsensor_info.custom4.framelength +
					 imgsensor.dummy_line;
		imgsensor.min_frame_length = imgsensor.frame_length;
		spin_unlock(&imgsensor_drv_lock);
		if (imgsensor.frame_length > imgsensor.shutter)
			set_dummy();
		break;
	case MSDK_SCENARIO_ID_CUSTOM5:
		frame_length = imgsensor_info.custom5.pclk / framerate * 10 /
			       imgsensor_info.custom5.linelength;
		spin_lock(&imgsensor_drv_lock);
		imgsensor.dummy_line =
			(frame_length > imgsensor_info.custom5.framelength) ?
				(frame_length -
				 imgsensor_info.custom5.framelength) :
				0;
		imgsensor.frame_length = imgsensor_info.custom5.framelength +
					 imgsensor.dummy_line;
		imgsensor.min_frame_length = imgsensor.frame_length;
		spin_unlock(&imgsensor_drv_lock);
		if (imgsensor.frame_length > imgsensor.shutter)
			set_dummy();
		break;
	default: //coding with  preview scenario by default
		frame_length = imgsensor_info.pre.pclk / framerate * 10 /
			       imgsensor_info.pre.linelength;
		spin_lock(&imgsensor_drv_lock);
		imgsensor.dummy_line =
			(frame_length > imgsensor_info.pre.framelength) ?
				(frame_length -
				 imgsensor_info.pre.framelength) :
				0;
		imgsensor.frame_length =
			imgsensor_info.pre.framelength + imgsensor.dummy_line;
		imgsensor.min_frame_length = imgsensor.frame_length;
		spin_unlock(&imgsensor_drv_lock);
		if (imgsensor.frame_length > imgsensor.shutter)
			set_dummy();
		LOG_INF("error scenario_id = %d, we use preview scenario \n",
			scenario_id);
		break;
	}
	return ERROR_NONE;
}

static kal_uint32
get_default_framerate_by_scenario(enum MSDK_SCENARIO_ID_ENUM scenario_id,
				  MUINT32 *framerate)
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
	default:
		break;
	}

	return ERROR_NONE;
}

static kal_uint32 set_test_pattern_mode(kal_bool enable)
{
	LOG_INF("enable: %d\n", enable);

	if (enable) {
		// 0x5E00[8]: 1 enable,  0 disable
		// 0x5E00[1:0]; 00 Color bar, 01 Random Data, 10 Square, 11 BLACK
		write_cmos_sensor(0x0600, 0x0002);
	} else {
		// 0x5E00[8]: 1 enable,  0 disable
		// 0x5E00[1:0]; 00 Color bar, 01 Random Data, 10 Square, 11 BLACK
		write_cmos_sensor(0x0600, 0x0000);
	}
	spin_lock(&imgsensor_drv_lock);
	imgsensor.test_pattern = enable;
	spin_unlock(&imgsensor_drv_lock);
	return ERROR_NONE;
}
/*hs14 code for SR-AL6528A-01-60 by jianghongyan at 2022-11-16 start*/
static kal_uint32 feature_control(MSDK_SENSOR_FEATURE_ENUM feature_id,
				  UINT8 *feature_para, UINT32 *feature_para_len)
{
	UINT16 *feature_return_para_16 = (UINT16 *)feature_para;
	UINT16 *feature_data_16 = (UINT16 *)feature_para;
	UINT32 *feature_return_para_32 = (UINT32 *)feature_para;
	UINT32 *feature_data_32 = (UINT32 *)feature_para;
	unsigned long long *feature_data = (unsigned long long *)feature_para;
	//unsigned long long *feature_return_para=(unsigned long long *) feature_para;

	struct SENSOR_WINSIZE_INFO_STRUCT *wininfo;
	MSDK_SENSOR_REG_INFO_STRUCT *sensor_reg_data =
		(MSDK_SENSOR_REG_INFO_STRUCT *)feature_para;

	LOG_INF("feature_id = %d\n", feature_id);
	switch (feature_id) {
	case SENSOR_FEATURE_GET_PERIOD:
		*feature_return_para_16++ = imgsensor.line_length;
		*feature_return_para_16 = imgsensor.frame_length;
		*feature_para_len = 4;
		break;
	case SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ:
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
		write_cmos_sensor(sensor_reg_data->RegAddr,
				  sensor_reg_data->RegData);
		break;
	case SENSOR_FEATURE_GET_REGISTER:
		sensor_reg_data->RegData =
			read_cmos_sensor(sensor_reg_data->RegAddr);
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
		set_auto_flicker_mode((BOOL)*feature_data_16,
				      *(feature_data_16 + 1));
		break;
	case SENSOR_FEATURE_SET_MAX_FRAME_RATE_BY_SCENARIO:
		set_max_framerate_by_scenario((enum MSDK_SCENARIO_ID_ENUM) *
						      feature_data,
					      *(feature_data + 1));
		break;
	case SENSOR_FEATURE_GET_DEFAULT_FRAME_RATE_BY_SCENARIO:
		get_default_framerate_by_scenario(
			(enum MSDK_SCENARIO_ID_ENUM) * (feature_data),
			(MUINT32 *)(uintptr_t)(*(feature_data + 1)));
		break;
	case SENSOR_FEATURE_SET_TEST_PATTERN:
		set_test_pattern_mode((BOOL)*feature_data);
		break;
	case SENSOR_FEATURE_GET_TEST_PATTERN_CHECKSUM_VALUE: //for factory mode auto testing
		*feature_return_para_32 = imgsensor_info.checksum_value;
		*feature_para_len = 4;
		break;
	case SENSOR_FEATURE_SET_FRAMERATE:
		LOG_INF("current fps :%d\n", *feature_data_32);
		spin_lock(&imgsensor_drv_lock);
		imgsensor.current_fps = (UINT16)*feature_data_32;
		spin_unlock(&imgsensor_drv_lock);
		break;
	case SENSOR_FEATURE_SET_HDR:
		LOG_INF("hdr enable :%d\n", *feature_data_32);
		spin_lock(&imgsensor_drv_lock);
		imgsensor.ihdr_en = (BOOL)*feature_data_32;
		spin_unlock(&imgsensor_drv_lock);
		break;
	case SENSOR_FEATURE_GET_CROP_INFO:
		LOG_INF("SENSOR_FEATURE_GET_CROP_INFO scenarioId:%lld\n", (UINT32)*feature_data);

		wininfo = (struct SENSOR_WINSIZE_INFO_STRUCT *)(uintptr_t)(
			*(feature_data + 1));

		switch (*feature_data_32) {
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			memcpy((void *)wininfo,
			       (void *)&imgsensor_winsize_info[1],
			       sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
			break;
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			memcpy((void *)wininfo,
			       (void *)&imgsensor_winsize_info[2],
			       sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
			break;
		case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
			memcpy((void *)wininfo,
			       (void *)&imgsensor_winsize_info[3],
			       sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
			break;
		case MSDK_SCENARIO_ID_SLIM_VIDEO:
			memcpy((void *)wininfo,
			       (void *)&imgsensor_winsize_info[4],
			       sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
			break;
		case MSDK_SCENARIO_ID_CUSTOM1:
			memcpy((void *)wininfo,
			       (void *)&imgsensor_winsize_info[5],
			       sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
			break;
		case MSDK_SCENARIO_ID_CUSTOM2:
			memcpy((void *)wininfo,
			       (void *)&imgsensor_winsize_info[6],
			       sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
			break;
		case MSDK_SCENARIO_ID_CUSTOM3:
			memcpy((void *)wininfo,
			       (void *)&imgsensor_winsize_info[7],
			       sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
			break;
		case MSDK_SCENARIO_ID_CUSTOM4:
			memcpy((void *)wininfo,
			       (void *)&imgsensor_winsize_info[8],
			       sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
			break;
		case MSDK_SCENARIO_ID_CUSTOM5:
			memcpy((void *)wininfo,
			       (void *)&imgsensor_winsize_info[9],
			       sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
			break;
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		default:
			memcpy((void *)wininfo,
			       (void *)&imgsensor_winsize_info[0],
			       sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
			break;
		}
		break;
	case SENSOR_FEATURE_SET_IHDR_SHUTTER_GAIN:
		LOG_INF("SENSOR_SET_SENSOR_IHDR LE=%d, SE=%d, Gain=%d\n",
			(UINT16)*feature_data, (UINT16) * (feature_data + 1),
			(UINT16) * (feature_data + 2));
		ihdr_write_shutter_gain((UINT16)*feature_data,
					(UINT16) * (feature_data + 1),
					(UINT16) * (feature_data + 2));
		break;
	default:
		break;
	}

	return ERROR_NONE;
} /*    feature_control()  */
/*hs14 code for SR-AL6528A-01-60 by jianghongyan at 2022-11-16 end*/
static struct SENSOR_FUNCTION_STRUCT sensor_func = {
	open, get_info, get_resolution, feature_control, control, close
};

UINT32
A1401FRONTS5K3L6LY_MIPI_RAW_SensorInit(struct SENSOR_FUNCTION_STRUCT **pfFunc)
{
	/* To Do : Check Sensor status here */
	if (pfFunc != NULL)
		*pfFunc = &sensor_func;
	return ERROR_NONE;
} /*	A1401FRONTS5K3L6LY_MIPI_RAW_SensorInit	*/
/*hs14 code for SR-AL6528A-01-88 by pengxutao at 2022-11-1 end*/
