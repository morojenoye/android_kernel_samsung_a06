/*****************************************************************************
 *
 * Filename:
 * ---------
 *	 S5KJN1mipiraw_sensor.c
 *
 * Project:
 * --------
 *	 ALPS MT6873
 *
 * Description:
 * ------------
 *---------------------------------------------------------------------------
 * Upper this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/
/*hs14 code for AL6528ADEU-1011 by majunfeng at 2022-11-2 start*/
/*hs14 code for AL6528A-348 by rongyi at 2022-10-26 start*/
/*hs14 code for SR-AL6528A-01-60 by jianghongyan at 2022-11-08 start*/
#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/atomic.h>
#include <linux/types.h>

#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_imgsensor_errcode.h"
#include "kd_camera_typedef.h"
#include "a1401backs5kjn1hlt_mipi_raw.h"

/*hs14 code for SR-AL6528A-01-54 by chengzhi at 2022-10-03 start*/
#define FPTPDAFSUPPORT 1
/*hs14 code for SR-AL6528A-01-54 by chengzhi at 2022-10-03 end*/

#define MULTI_WRITE 1
#define OTP_DATA_NUMBER 9

#define PFX "A1401BACKS5KJN1HLT_MIPI_RAW_camera_sensor"

#define LOG_INF(format, args...) pr_err(PFX "[%s] " format, __func__, ##args)
/*hs14 code for AL6528ADEU-1011 by xutengtao at 2022-10-25 start*/
static DEFINE_SPINLOCK(imgsensor_drv_lock);
static struct imgsensor_info_struct imgsensor_info = {
	.sensor_id = A1401BACKS5KJN1HLT_SENSOR_ID ,
	.checksum_value = 0xdb9c643,
	.pre = {
		.pclk = 544000000,
		.linelength = 5200,
		.framelength = 3486,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 4080,
		.grabwindow_height = 3072,
		.mipi_data_lp2hs_settle_dc = 85,
		.mipi_pixel_rate = 547200000,
		.max_framerate = 300,
	},
	.cap = {
		.pclk = 544000000,
		.linelength = 5200,
		.framelength = 3486,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 4080,
		.grabwindow_height = 3072,
		.mipi_data_lp2hs_settle_dc = 85,
		.mipi_pixel_rate = 547200000,
		.max_framerate = 300,
	},
	.normal_video = {
		.pclk = 544000000,
		.linelength = 5200,
		.framelength = 3486,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 4080,
		.grabwindow_height = 3072,
		.mipi_data_lp2hs_settle_dc = 85,
		.mipi_pixel_rate = 547200000,
		.max_framerate = 300,
	},
	.hs_video = {
        .pclk =  	600000000,
        .linelength = 	2096,
        .framelength =	2380,
        .startx = 	0,
        .starty = 	0,
        .grabwindow_width =	1280,
        .grabwindow_height = 	720,
        .mipi_data_lp2hs_settle_dc = 85,//Check with MTK
        .max_framerate =	1200,
        .mipi_pixel_rate =	640000000,
	},
	.slim_video = {
		.pclk = 600000000,
		.linelength = 4768,
		.framelength = 4192,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 1920,
		.grabwindow_height = 1080,
		.mipi_data_lp2hs_settle_dc = 40,
		.mipi_pixel_rate = 320000000,
		.max_framerate = 300,
	},
	.custom1 = {
		.pclk = 560000000,
		.linelength = 4832,
		.framelength = 4816,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 3264,
		.grabwindow_height = 1836,
		.mipi_data_lp2hs_settle_dc = 40,
		.mipi_pixel_rate = 556800000,
		.max_framerate = 240,
	},
/*hs14 code for SR-AL6528A-01-60 by caoshiyong at 2022-10-09 start*/
	.custom2 = {////can't work single
		.pclk = 560000000,
		.linelength = 5910,
		.framelength = 3948,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 4080,
		.grabwindow_height = 3072,
		.mipi_data_lp2hs_settle_dc = 40,
		.mipi_pixel_rate = 480000000,
		.max_framerate = 240,
	},
/*hs14 code for SR-AL6528A-01-60 by caoshiyong at 2022-10-09 end*/
	.custom3 = {
		.pclk = 560000000,
		.linelength = 5910,
		.framelength = 3152,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 4080,
		.grabwindow_height = 2296,
		.mipi_data_lp2hs_settle_dc = 85,
		.mipi_pixel_rate = 480000000,
		.max_framerate = 300,
	},
	.custom4 = {
		.pclk = 560000000,
		.linelength = 8688,
		.framelength = 6400,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 8160,
		.grabwindow_height = 6144,
		.mipi_data_lp2hs_settle_dc = 85,
		.mipi_pixel_rate = 556800000,
		.max_framerate = 100,
	},
	.margin = 5,
	.min_shutter = 5,
	.min_gain = BASEGAIN, /*1x gain*/
	.max_gain = 64*BASEGAIN, /*64x gain*/
	.min_gain_iso = 100,
	.exp_step = 2,
	.gain_step = 1,
	.gain_type = 2,
	.max_frame_length = 0xFFFF,
	.ae_shut_delay_frame = 0,
	.ae_sensor_gain_delay_frame = 0,
	.ae_ispGain_delay_frame = 2,
	.frame_time_delay_frame = 1,
	.ihdr_support = 0,
	.ihdr_le_firstline = 0,
	.sensor_mode_num = 9,////here must modify when add setting
/*hs14 code for SR-AL6528A-01-20 by renxinglin at 2022/11/17 start*/
	.cap_delay_frame = 2,
	.pre_delay_frame = 2,
	.video_delay_frame = 2,
	.hs_video_delay_frame = 2,
	.slim_video_delay_frame = 2,
	.custom1_delay_frame = 2,
	.custom2_delay_frame = 2,
	.custom3_delay_frame = 2,
	.custom4_delay_frame = 2,
/*hs14 code for SR-AL6528A-01-20 by renxinglin at 2022/11/17 end*/
	.isp_driving_current = ISP_DRIVING_6MA,//ISP_DRIVING_8MA
	.sensor_interface_type = SENSOR_INTERFACE_TYPE_MIPI,
	.mipi_sensor_type = MIPI_OPHY_NCSI2,
	.mipi_settle_delay_mode = 1,
/*hs14 code for SR-AL6528A-01-54 by chengzhi at 2022-10-01 start*/
/* C3T code for HQ-225408 by fudongguo at 2022/07/25 start */
	.sensor_output_dataformat = SENSOR_OUTPUT_FORMAT_RAW_4CELL_BAYER_Gr,
	//.sensor_output_dataformat = SENSOR_OUTPUT_FORMAT_RAW_Gr,
/* C3T code for HQ-225408 by fudongguo at 2022/07/25 end */
/*hs14 code for SR-AL6528A-01-54 by chengzhi at 2022-10-01 end*/
	.mclk = 24,
	.mipi_lane_num = SENSOR_MIPI_4_LANE,
/*hs14 code for SR-AL6528A-01-522 by majunfeng at 2022/09/12 start*/
	.i2c_addr_table = {0xAC, 0x20,0xff},
/*hs14 code for SR-AL6528A-01-522 by majunfeng at 2022/09/12 end*/
	.i2c_speed = 1000,
};
/*hs14 code for AL6528ADEU-1011 by xutengtao at 2022-10-25 end*/
static struct imgsensor_struct imgsensor = {
	.mirror = IMAGE_NORMAL,
	.sensor_mode = IMGSENSOR_MODE_INIT,
	.shutter = 0x0200,
	.gain = 0x0100,
	.dummy_pixel = 0,
	.dummy_line = 0,
	.current_fps = 0,
	.autoflicker_en = KAL_FALSE,
	.test_pattern = KAL_FALSE,
	.current_scenario_id = MSDK_SCENARIO_ID_CAMERA_PREVIEW,
	.ihdr_en = KAL_FALSE,
	.i2c_write_id = 0xAC,
	//cxc long exposure >
	.current_ae_effective_frame = 2,
	//cxc long exposure <
};

static struct SENSOR_VC_INFO_STRUCT SENSOR_VC_INFO[9] = {
	/* preview mode setting */
	{ 0x02,	  0x0A,	  0x00, 0x08, 0x40,   0x00,   0x00, 0x2B,
	  0x0ff0, 0x0c00, 0x01, 0x00, 0x0000, 0x0000, 0x01, 0x30,
	  0x027C, 0x0BF0, 0x03, 0x00, 0x0000, 0x0000 },
	/* capture mode setting */
	{ 0x02,	  0x0A,	  0x00, 0x08, 0x40,   0x00,   0x00, 0x2B,
	  0x0ff0, 0x0c00, 0x01, 0x00, 0x0000, 0x0000, 0x01, 0x30,
	  0x027C, 0x0BF0, 0x03, 0x00, 0x0000, 0x0000 },
	/* normal_video mode setting */
	{ 0x02,	  0x0A,	  0x00, 0x08, 0x40,   0x00,   0x00, 0x2B,
	  0x0ff0, 0x0c00, 0x01, 0x00, 0x0000, 0x0000, 0x01, 0x30,
	  0x027C, 0x0BF0, 0x03, 0x00, 0x0000, 0x0000 },
	/* high_speed_video mode setting */
	{ 0x02,	  0x0A,	  0x00, 0x08, 0x40,   0x00,   0x00, 0x2B,
	  0x0500, 0x02D0, 0x01, 0x00, 0x0000, 0x0000, 0x00, 0x30,
	  0x026C, 0x02E0, 0x03, 0x00, 0x0000, 0x0000 },
	/* slim_video mode setting */
	{ 0x02,	  0x0A,	  0x00, 0x08, 0x40,   0x00,   0x00, 0x2B,
	  0x0780, 0x0438, 0x01, 0x00, 0x0000, 0x0000, 0x00, 0x30,
	  0x026C, 0x02E0, 0x03, 0x00, 0x0000, 0x0000 },
	/* custom1 mode setting */
	{ 0x02,	  0x0A,	  0x00, 0x08, 0x40,   0x00,   0x00, 0x2B,
	  0x0ff0, 0x0c00, 0x01, 0x00, 0x0000, 0x0000, 0x01, 0x30,
	  0x027C, 0x0BF0, 0x03, 0x00, 0x0000, 0x0000 },
	/* custom2 mode setting */
	{ 0x02,	  0x0A,	  0x00, 0x08, 0x40,   0x00,   0x00, 0x2B,
	  0x0ff0, 0x0c00, 0x01, 0x00, 0x0000, 0x0000, 0x01, 0x30,
	  0x027C, 0x0BF0, 0x03, 0x00, 0x0000, 0x0000 },
/*hs14 code for AL6528ADEU-1829 by liluling at 2022/11/2 begin*/
	/* custom3 mode setting */
	{ 0x02,//VC_Num
	  0x0A,//VC_PixelNum
	  0x00,//ModeSelect    /* 0:auto 1:direct */
	  0x08,//EXPO_Ratio    /* 1/1, 1/2, 1/4, 1/8 */
	  0x40,//0DValue		/* 0D Value */
	  0x00,
	  0x00, 0x2B,0x0ff0, 0x08F8,// VC0 Maybe image data
	  0x01, 0x00, 0x0000, 0x0000,// VC1 MVHDR
	  0x01, 0x30,0x027C, 0x08F0, // VC2 PDAF
	  0x03, 0x00, 0x0000, 0x0000 },// VC3
/*hs14 code for AL6528ADEU-1829 by liluling at 2022/11/2 end*/
	/* custom4 mode setting */
	{ 0x02,	  0x0A,	  0x00, 0x08, 0x40,   0x00,   0x00, 0x2B,
	  0x0ff0, 0x0c00, 0x01, 0x00, 0x0000, 0x0000, 0x01, 0x30,
	  0x027C, 0x0BF0, 0x03, 0x00, 0x0000, 0x0000 },
};

static struct SENSOR_WINSIZE_INFO_STRUCT imgsensor_winsize_info[9] = {
	//preview
	{ 8160, 6144, 0, 0, 8160, 6144, 4080, 3072, 0, 0, 4080, 3072, 0, 0,
	  4080, 3072 },
	//capture
	{ 8160, 6144, 0, 0, 8160, 6144, 4080, 3072, 0, 0, 4080, 3072, 0, 0,
	  4080, 3072 },
	//normal_video
	{ 8160, 6144, 0, 0, 8160, 6144, 4080, 3072, 0, 0, 4080, 3072, 0, 0,
	  4080, 3072 },
	//hs_video
	{ 8160, 6144, 0, 0, 8160, 6144, 2040, 1536,380, 408, 1280, 720, 0, 0,
	 1280,  720},
	//slim_video
	{ 8160, 6144, 240, 912, 7680, 4320, 1920, 1080, 0, 0, 1920, 1080, 0, 0,
	  1920, 1080 },
	//custom1
	{ 8160, 6144, 816, 1236, 6528, 3672, 3264, 1836, 0, 0, 3264, 1836, 0, 0,
	  3264, 1836 },
	//custom2
	{ 8160, 6144, 0, 0, 8160, 6144, 4080, 3072, 0, 0, 4080, 3072, 0, 0,
	  4080, 3072 },
	//custom3
	{ 8160, 6144, 0, 776, 8160, 4592, 4080, 2296, 0, 0, 4080, 2296, 0, 0,
	  4080, 2296 },
	//custom4
	{ 8160, 6144, 0, 0, 8160, 6144, 8160, 6144, 0, 0, 8160, 6144, 0, 0,
	  8160, 6144 },
};

static struct SET_PD_BLOCK_INFO_T imgsensor_pd_info = {
	.i4OffsetX = 8,
	.i4OffsetY = 8,
	.i4PitchX = 8,
	.i4PitchY = 8,
	.i4PairNum = 4,
	.i4SubBlkW = 8,
	.i4SubBlkH = 2,
	.i4PosL = {
		{ 9, 8 }, 
		{11, 11}, 
		{15, 12}, 
		{13, 15}, 
	},
	.i4PosR = {
		{ 8, 8 }, 
		{10, 11}, 
		{14, 12}, 
		{12, 15}, 
	},
	.iMirrorFlip = 0,
	.i4BlockNumX = 508,
	.i4BlockNumY = 382,
	.i4Crop = { {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
};
/*hs14 code for AL6528ADEU-1829 by liluling at 2022/11/2 begin*/
static struct SET_PD_BLOCK_INFO_T imgsensor_pd_info_16_9 = {
	.i4OffsetX = 8,
	.i4OffsetY = 8,
	.i4PitchX = 8,
	.i4PitchY = 8,
	.i4PairNum = 4,
	.i4SubBlkW = 8,
	.i4SubBlkH = 2,
	.i4PosL = {
		{ 9, 8 },
		{11, 11},
		{15, 12},
		{13, 15},
	},
	.i4PosR = {
		{ 8, 8 },
		{10, 11},
		{14, 12},
		{12, 15},
	},
	.iMirrorFlip = 0,
	.i4BlockNumX = 508,
	.i4BlockNumY = 288,
	.i4Crop = { {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 388}, {0, 0}, {0, 0} },
};
/*hs14 code for AL6528ADEU-1829 by liluling at 2022/11/2 end*/
#if MULTI_WRITE
#define I2C_BUFFER_LEN 1020 /*trans max is 255, each 3 bytes*/
#else
#define I2C_BUFFER_LEN 4
#endif

static kal_uint16 s5kjn1_multi_write_cmos_sensor(kal_uint16 *para,
						 kal_uint32 len)
{
	char puSendCmd[I2C_BUFFER_LEN];
	kal_uint32 tosend, IDX;
	kal_uint16 addr = 0, addr_last = 0, data;

	tosend = 0;
	IDX = 0;
	while (len > IDX) {
		addr = para[IDX];
		{
			puSendCmd[tosend++] = (char)(addr >> 8);
			puSendCmd[tosend++] = (char)(addr & 0xFF);
			data = para[IDX + 1];
			puSendCmd[tosend++] = (char)(data >> 8);
			puSendCmd[tosend++] = (char)(data & 0xFF);
			IDX += 2;
			addr_last = addr;
		}
#if MULTI_WRITE
		if ((I2C_BUFFER_LEN - tosend) < 4 || len == IDX ||
		    addr != addr_last) {
			iBurstWriteReg_multi(puSendCmd, tosend,
					     imgsensor.i2c_write_id, 4,
					     imgsensor_info.i2c_speed);
			tosend = 0;
		}
#else
		iWriteRegI2CTiming(puSendCmd, 4, imgsensor.i2c_writMULTe_id,
				   imgsensor_info.i2c_speed);
		tosend = 0;

#endif
	}
	return 0;
}

static kal_uint16 read_cmos_sensor_8(kal_uint16 addr)
{
	kal_uint16 get_byte = 0;
	char pu_send_cmd[2] = { (char)(addr >> 8), (char)(addr & 0xFF) };

	iReadRegI2CTiming(pu_send_cmd, 2, (u8 *)&get_byte, 1,
			  imgsensor.i2c_write_id, imgsensor_info.i2c_speed);
	return get_byte;
}

static kal_uint16 read_cmos_sensor(kal_uint32 addr)
{
	kal_uint16 get_byte = 0;
	char pu_send_cmd[2] = { (char)(addr >> 8), (char)(addr & 0xFF) };

	iReadRegI2CTiming(pu_send_cmd, 2, (u8 *)&get_byte, 1,
			  imgsensor.i2c_write_id, imgsensor_info.i2c_speed);
	return get_byte;
}

static void write_cmos_sensor_byte(kal_uint32 addr, kal_uint32 para)
{
	char pu_send_cmd[3] = { (char)(addr >> 8), (char)(addr & 0xFF),
				(char)(para & 0xFF) };

	iWriteRegI2CTiming(pu_send_cmd, 3, imgsensor.i2c_write_id,
			   imgsensor_info.i2c_speed);
}

static void write_cmos_sensor(kal_uint16 addr, kal_uint16 para)
{
	char pusendcmd[4] = { (char)(addr >> 8), (char)(addr & 0xFF),
			      (char)(para >> 8), (char)(para & 0xFF) };

	iWriteRegI2CTiming(pusendcmd, 4, imgsensor.i2c_write_id,
			   imgsensor_info.i2c_speed);
}

static void set_dummy(void)
{
	LOG_INF("dummyline = %d, dummypixels = %d\n", imgsensor.dummy_line,
		imgsensor.dummy_pixel);

	write_cmos_sensor(0x0340, imgsensor.frame_length & 0xFFFF);
	write_cmos_sensor(0x0342, imgsensor.line_length & 0xFFFF);
}

static void set_max_framerate(UINT16 framerate, kal_bool min_framelength_en)
{
	kal_uint32 frame_length = imgsensor.frame_length;

	LOG_INF("framerate = %d, min framelength should enable(%d)\n",
		framerate, min_framelength_en);
	frame_length = imgsensor.pclk / framerate * 10 / imgsensor.line_length;
	spin_lock(&imgsensor_drv_lock);
	imgsensor.frame_length = (frame_length > imgsensor.min_frame_length) ?
					 frame_length :
					 imgsensor.min_frame_length;
	imgsensor.dummy_line =
		imgsensor.frame_length - imgsensor.min_frame_length;

	if (imgsensor.frame_length > imgsensor_info.max_frame_length) {
		imgsensor.frame_length = imgsensor_info.max_frame_length;
		imgsensor.dummy_line =
			imgsensor.frame_length - imgsensor.min_frame_length;
	}
	if (min_framelength_en)
		imgsensor.min_frame_length = imgsensor.frame_length;
	spin_unlock(&imgsensor_drv_lock);
	set_dummy();
}

#if 1
static void check_streamoff(void)
{
	unsigned int i = 0;
	int timeout = (10000 / imgsensor.current_fps) + 1;

	mdelay(3);
	for (i = 0; i < timeout; i++) {
		if (read_cmos_sensor_8(0x0005) != 0xFF)
			mdelay(1);
		else
			break;
	}
	LOG_INF(" check_streamoff exit!\n");
}

static kal_uint32 streaming_control(kal_bool enable)
{
	unsigned int i = 0;

	LOG_INF("streaming_enable(0=Sw Standby,1=streaming): %d\n", enable);
	if (enable) {
		//		write_cmos_sensor(0x6028, 0x4000);
		//		mdelay(5);
		write_cmos_sensor_byte(0x0100, 0X01);
		for (i = 0; i < 5; i++) {
			pr_err("%s streaming check is %d", __func__,
			       read_cmos_sensor_8(0x0005));
			pr_err("%s streaming check 0x0100 is %d", __func__,
			       read_cmos_sensor_8(0x0100));
			pr_err("%s streaming check 0x6028 is %d", __func__,
			       read_cmos_sensor(0x6028));
		}
	} else {
		//		write_cmos_sensor(0x6028, 0x4000);
		//		mdelay(5);
		write_cmos_sensor_byte(0x0100, 0x00);
		check_streamoff();
	}
	//	mdelay(10);
	return ERROR_NONE;
}
#endif
#if 0
static void write_shutter(kal_uint16 shutter)
{
	kal_uint16 realtime_fps = 0;

	spin_lock(&imgsensor_drv_lock);
	if (shutter > imgsensor.min_frame_length - imgsensor_info.margin)
		imgsensor.frame_length = shutter + imgsensor_info.margin;
	else
		imgsensor.frame_length = imgsensor.min_frame_length;
	if (imgsensor.frame_length > imgsensor_info.max_frame_length)
		imgsensor.frame_length = imgsensor_info.max_frame_length;
	spin_unlock(&imgsensor_drv_lock);
	shutter =
		(shutter < imgsensor_info.min_shutter)
		  ? imgsensor_info.min_shutter : shutter;
	shutter =
		(shutter >
		 (imgsensor_info.max_frame_length -
		 imgsensor_info.margin)) ? (imgsensor_info.max_frame_length
				 - imgsensor_info.margin) : shutter;
	if (imgsensor.autoflicker_en) {
		realtime_fps =
			imgsensor.pclk / imgsensor.line_length * 10 /
			imgsensor.frame_length;
		if (realtime_fps >= 297 && realtime_fps <= 305)
			set_max_framerate(296, 0);
		else if (realtime_fps >= 147 && realtime_fps <= 150)
			set_max_framerate(146, 0);
	} else {

		write_cmos_sensor(0x0340, imgsensor.frame_length & 0xFFFF);
	}

	write_cmos_sensor(0x0202, (shutter) & 0xFFFF);
	LOG_INF("Exit! shutter =%d, framelength =%d\n", shutter,
			imgsensor.frame_length);

}
#endif

//cxc long exposure >
static bool bNeedSetNormalMode = KAL_FALSE;
#define SHUTTER_1 94750 //30396
#define SHUTTER_2 189501 //60792
#define SHUTTER_4 379003 //56049
#define SHUTTER_8 758006 //46563
#define SHUTTER_16 1516012 //27591
#define SHUTTER_32 3032024

// static void check_output_stream_off(void)
// {
// 	kal_uint16 read_count = 0, read_register0005_value = 0;

// 	for (read_count = 0; read_count <= 4; read_count++) {
// 		read_register0005_value = read_cmos_sensor_8(0x0005);

// 		if (read_register0005_value == 0xff)
// 			break;
// 		mdelay(50);

// 		if (read_count == 4)
// 			LOG_INF("cxc stream off error\n");
// 	}
// }
//cxc long exposure <

/*************************************************************************
*FUNCTION
*  set_shutter
*
*DESCRIPTION
*  This function set e-shutter of sensor to change exposure time.
*
*PARAMETERS
*  iShutter : exposured lines
*
*RETURNS
*  None
*
*GLOBALS AFFECTED
*
*************************************************************************/
static void set_shutter(kal_uint32 shutter)
{
	unsigned long flags;
	kal_uint16 realtime_fps = 0;
	kal_uint32 CintR = 0;
	kal_uint32 Time_Frame = 0;

	//LOG_INF("---------------------------------------set_shutter---------------------------------------\n");

	spin_lock_irqsave(&imgsensor_drv_lock, flags);
	imgsensor.shutter = shutter;
	spin_unlock_irqrestore(&imgsensor_drv_lock, flags);

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
	if (shutter < imgsensor_info.min_shutter)
		shutter = imgsensor_info.min_shutter;

	if (imgsensor.autoflicker_en) {
		realtime_fps = imgsensor.pclk / imgsensor.line_length * 10 / imgsensor.frame_length;
		if (realtime_fps >= 297 && realtime_fps <= 305) {
			set_max_framerate(296, 0);
		} else if (realtime_fps >= 147 && realtime_fps <= 150) {
			set_max_framerate(146, 0);
		} else {
			// Extend frame length
			write_cmos_sensor(0x0340, imgsensor.frame_length);
		}
	} else {
		// Extend frame length
		write_cmos_sensor(0x0340, imgsensor.frame_length);
	}

	if (shutter > 0xFFF0) {

		bNeedSetNormalMode = KAL_TRUE;
		if(shutter >= 3448275){
			shutter = 3448275;
		}
		CintR = ((unsigned long long)shutter) / 128;
		Time_Frame = CintR + 0x0002;
		LOG_INF("[cameradebug-longexp]CintR = %d\n", CintR);
		write_cmos_sensor(0x0340, Time_Frame & 0xFFFF);
		write_cmos_sensor(0x0202, CintR & 0xFFFF);
		write_cmos_sensor(0x0702, 0x0700);
		write_cmos_sensor(0x0704, 0x0700);

		LOG_INF("[cameradebug-longexp]download long shutter setting shutter = %d\n", shutter);
	} else {
		if (bNeedSetNormalMode == KAL_TRUE) {
			bNeedSetNormalMode = KAL_FALSE;
			write_cmos_sensor(0x0702, 0x0000);
			write_cmos_sensor(0x0704, 0x0000);

			LOG_INF("[cameradebug-longexp]return to normal shutter =%d, framelength =%d\n", shutter, imgsensor.frame_length);

		}
		write_cmos_sensor(0x0340, imgsensor.frame_length);
		write_cmos_sensor(0x0202, imgsensor.shutter);
	}
}

/*************************************************************************
*FUNCTION
*  set_shutter_frame_length
*
*DESCRIPTION
*  for frame &3A sync
*
*************************************************************************/
static void set_shutter_frame_length(kal_uint16 shutter, kal_uint16 frame_length)
{
	unsigned long flags;
	kal_uint16 realtime_fps = 0;
	kal_int32 dummy_line = 0;
	spin_lock_irqsave(&imgsensor_drv_lock, flags);
	imgsensor.shutter = shutter;
	spin_unlock_irqrestore(&imgsensor_drv_lock, flags);

	spin_lock(&imgsensor_drv_lock);
	if (frame_length > 1)
        dummy_line = frame_length - imgsensor.frame_length;
	imgsensor.frame_length = imgsensor.frame_length + dummy_line;

	if (shutter > imgsensor.frame_length - imgsensor_info.margin)
		imgsensor.frame_length = shutter + imgsensor_info.margin;

	if (imgsensor.frame_length > imgsensor_info.max_frame_length)
		imgsensor.frame_length = imgsensor_info.max_frame_length;
	spin_unlock(&imgsensor_drv_lock);

	shutter = (shutter < imgsensor_info.min_shutter) ? imgsensor_info.min_shutter : shutter;
	shutter = (shutter > (imgsensor_info.max_frame_length - imgsensor_info.margin))
	? (imgsensor_info.max_frame_length - imgsensor_info.margin) : shutter;

	if (imgsensor.autoflicker_en) {
		realtime_fps = imgsensor.pclk
			/ imgsensor.line_length * 10 / imgsensor.frame_length;
		if (realtime_fps >= 297 && realtime_fps <= 305)
			set_max_framerate(296, 0);
		else if (realtime_fps >= 147 && realtime_fps <= 150)
			set_max_framerate(146, 0);
		else
			write_cmos_sensor(0x0340, imgsensor.frame_length);

	} else {
			write_cmos_sensor(0x0340, imgsensor.frame_length);
	}

	write_cmos_sensor(0x0202, imgsensor.shutter);
	LOG_INF("Exit! shutter =%d, framelength =%d/%d, dummy_line=%d\n",
		shutter, imgsensor.frame_length,
		frame_length, dummy_line);
}
static kal_uint16 gain2reg(const kal_uint16 gain)
{
	kal_uint16 reg_gain = 0x0000;

	reg_gain = gain / 2;

	return (kal_uint16)reg_gain;
}

/*************************************************************************
*FUNCTION
*  set_gain
*
*DESCRIPTION
*  This function is to set global gain to sensor.
*
*PARAMETERS
*  iGain : sensor global gain(base: 0x40)
*
*RETURNS
*  the actually gain set to sensor.
*
*GLOBALS AFFECTED
*
*************************************************************************/
static kal_uint16 set_gain(kal_uint16 gain)
{
	kal_uint16 reg_gain;

	LOG_INF("set_gain %d\n", gain);
	if (gain < BASEGAIN || gain > 64 * BASEGAIN) {
		LOG_INF("Error gain setting");
		if (gain < BASEGAIN)
			gain = BASEGAIN;
		else if (gain > 64 * BASEGAIN)
			gain = 64 * BASEGAIN;
	}
	reg_gain = gain2reg(gain);
	spin_lock(&imgsensor_drv_lock);
	imgsensor.gain = reg_gain;
	spin_unlock(&imgsensor_drv_lock);
	LOG_INF("gain = %d , reg_gain = 0x%x\n ", gain, reg_gain);
	write_cmos_sensor(0x0204, (reg_gain & 0xFFFF));
	return gain;
}

#if 0
static void
ihdr_write_shutter_gain(kal_uint16 le, kal_uint16 se, kal_uint16 gain)
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

		write_cmos_sensor(0x380e, imgsensor.frame_length >> 8);
		write_cmos_sensor(0x380f, imgsensor.frame_length & 0xFF);
		write_cmos_sensor(0x3502, (le << 4) & 0xFF);
		write_cmos_sensor(0x3501, (le >> 4) & 0xFF);
		write_cmos_sensor(0x3500, (le >> 12) & 0x0F);
		write_cmos_sensor(0x3512, (se << 4) & 0xFF);
		write_cmos_sensor(0x3511, (se >> 4) & 0xFF);
		write_cmos_sensor(0x3510, (se >> 12) & 0x0F);
		set_gain(gain);
	}
}
#endif
static void set_mirror_flip(kal_uint8 image_mirror)
{
	LOG_INF("image_mirror = %d\n", image_mirror);
	spin_lock(&imgsensor_drv_lock);
	imgsensor.mirror = image_mirror;
	spin_unlock(&imgsensor_drv_lock);
	/*hs14 code for SR-AL6528A-01-67 by hudongdong at 2022-9-7 start*/
	/*hs14 code for SR-AL6528A-01-67 by jianghongyan at 2022-9-13 start*/
	switch (image_mirror) {
	case IMAGE_NORMAL:
		write_cmos_sensor_byte(0x0101, 0x00);
		break;
	case IMAGE_H_MIRROR:
		write_cmos_sensor_byte(0x0101, 0x01);
		break;
	case IMAGE_V_MIRROR:
		write_cmos_sensor_byte(0x0101, 0x02);
		break;
	case IMAGE_HV_MIRROR:
		write_cmos_sensor_byte(0x0101, 0x03);
		break;
	default:
		LOG_INF("Error image_mirror setting\n");
		break;
	}
	/*hs14 code for SR-AL6528A-01-67 by jianghongyan at 2022-9-13 end*/
	/*hs14 code for SR-AL6528A-01-67 by hudongdong at 2022-9-7 end*/
}

/*************************************************************************
*FUNCTION
*  night_mode
*
*DESCRIPTION
*  This function night mode of sensor.
*
*PARAMETERS
*  bEnable: KAL_TRUE -> enable night mode, otherwise, disable night mode
*
*RETURNS
*  None
*
*GLOBALS AFFECTED
*
*************************************************************************/
#if 0
static void night_mode(kal_bool enable)
{

}
#endif

#if MULTI_WRITE
kal_uint16 addr_data_pair_init_s5kjn1acc[] = {
	0x6028, 0x2400, 0x602A, 0x1354, 0x6F12, 0x0100, 0x6F12, 0x7017, 0x602A,
	0x13B2, 0x6F12, 0x0000, 0x602A, 0x1236, 0x6F12, 0x0000, 0x602A, 0x1A0A,
	0x6F12, 0x4C0A, 0x602A, 0x2210, 0x6F12, 0x3401, 0x602A, 0x2176, 0x6F12,
	0x6400, 0x602A, 0x222E, 0x6F12, 0x0001, 0x602A, 0x06B6, 0x6F12, 0x0A00,
	0x602A, 0x06BC, 0x6F12, 0x1001, 0x602A, 0x2140, 0x6F12, 0x0101, 0x602A,
	0x1A0E, 0x6F12, 0x9600, 0x6028, 0x4000, 0xF44E, 0x0011, 0xF44C, 0x0B0B,
	0xF44A, 0x0006, 0x0118, 0x0002, 0x011A, 0x0001,
};
#endif

#define EEPROM_READY 1
#if EEPROM_READY //stan

#define S5KJN1_EEPROM_SLAVE_ADDR 0xA0
#define S5KJN1_EEPROM_SIZE  0x3192
static uint8_t S5KJN1_eeprom[S5KJN1_EEPROM_SIZE] = {0};

//(16bitnum=364BYE/2) + 1 switch page + 1 switch addr
#define HWGCC_PAIT_INITCOUNT (364/2+1+1)*2
static kal_uint16 hw_gcc_pair_init[HWGCC_PAIT_INITCOUNT];

static void S5KJN1_read_data_from_eeprom(kal_uint8 slave, kal_uint32 start_add, uint32_t size)
{
	int i = 0;
	int backupWriteId;
	int hwgccWriteCount = 0;
	spin_lock(&imgsensor_drv_lock);
	backupWriteId = imgsensor.i2c_write_id;
	imgsensor.i2c_write_id = slave;
	spin_unlock(&imgsensor_drv_lock);
    pr_err("[cameradebug-eeprom]s5kjn1_enter_read_data_from_eeprom,backupWriteId=0x%x\n",backupWriteId);
	//read eeprom data
	for (i = 0; i < size; i ++) {
		S5KJN1_eeprom[i] = read_cmos_sensor_8(start_add);
		start_add ++;
	}

	spin_lock(&imgsensor_drv_lock);
	imgsensor.i2c_write_id = backupWriteId;
	spin_unlock(&imgsensor_drv_lock);

	//preload hwgcc
	hw_gcc_pair_init[hwgccWriteCount] = 0x6028;
	hwgccWriteCount++;
	hw_gcc_pair_init[hwgccWriteCount] = 0x2400;
	hwgccWriteCount++;
	hw_gcc_pair_init[hwgccWriteCount] = 0x602A;
	hwgccWriteCount++;
	hw_gcc_pair_init[hwgccWriteCount] = 0x0CFC;
	hwgccWriteCount++;
	for(; hwgccWriteCount<HWGCC_PAIT_INITCOUNT/2 ; )
	{
		hw_gcc_pair_init[hwgccWriteCount] = 0x6F12;
		hwgccWriteCount++;
		hw_gcc_pair_init[hwgccWriteCount] = S5KJN1_eeprom[(hwgccWriteCount+1)/2-2 + 12344];
		hwgccWriteCount++;
	}
	pr_err("[cameradebug-eeprom]s5kjn1 hwgccWriteCount=%d\n",hwgccWriteCount);
}

#define FOUR_CELL_SIZE 9490
#define Four_Cell_Default_Data_Size 594

#define XTC_SIZE 3502
#define SENSOR_XTC_SIZE 768
#define PDXTC_SIZE 4000
#define SWGGC_SIZE 626
#define XTC_OFFSET 3440
#define SENSOR_XTC_OFFSET 6944
#define PDXTC_OFFSET 7714
#define SWGGC_OFFSET 11716

static char Four_Cell_Size_Array[FOUR_CELL_SIZE + 2];
static char Four_Cell_Default_Data[Four_Cell_Default_Data_Size] = {0x00};

static void read_4cell_from_eeprom(char *data)
{
	int i;
	Four_Cell_Size_Array[0] = (FOUR_CELL_SIZE & 0xff);/*Low*/
	Four_Cell_Size_Array[1] = ((FOUR_CELL_SIZE >> 8) & 0xff);/*High*/

	if (data != NULL) {
		pr_err("[cameradebug-remosaic]jn1 memcpy  xtc to data\n");
		memcpy(data, Four_Cell_Size_Array, 2);	//adress
		memcpy(data+2, &S5KJN1_eeprom[XTC_OFFSET], XTC_SIZE);	//XTC  here from 3502->4096(+594) only for libso
		memcpy(data+2+XTC_SIZE, &Four_Cell_Default_Data[0], Four_Cell_Default_Data_Size);
		memcpy(data+2+XTC_SIZE+Four_Cell_Default_Data_Size, &S5KJN1_eeprom[SENSOR_XTC_OFFSET], SENSOR_XTC_SIZE);	//sensor XTC
		memcpy(data+2+XTC_SIZE+Four_Cell_Default_Data_Size+SENSOR_XTC_SIZE, &S5KJN1_eeprom[PDXTC_OFFSET], PDXTC_SIZE);//PDXTC
		memcpy(data+2+XTC_SIZE+Four_Cell_Default_Data_Size+SENSOR_XTC_SIZE+PDXTC_SIZE, &S5KJN1_eeprom[SWGGC_OFFSET], SWGGC_SIZE);//SW GGC
	}
	////this 8 only test data right or not
	for(i=0;i< 8;i++)
	{
		pr_err("[cameradebug-remosaic-data]data[%d]:0x%x\n",i,data[i]);
	}
}
#endif


static void sensor_init(void)
{
	LOG_INF("sensor_init start\n");
	write_cmos_sensor(0x6028, 0x4000);
	write_cmos_sensor(0x0000, 0x0003);
	write_cmos_sensor(0x0000, 0x38e1);
	write_cmos_sensor(0x001e, 0x0007);
	write_cmos_sensor(0x6028, 0x4000);
	write_cmos_sensor(0x6010, 0x0001);
	mdelay(5);
	write_cmos_sensor(0x6226, 0x0001);
	mdelay(10);
	s5kjn1_multi_write_cmos_sensor(addr_data_pair_init_s5kjn1acc,
				       sizeof(addr_data_pair_init_s5kjn1acc) /
					       sizeof(kal_uint16));
#if EEPROM_READY //stan
	s5kjn1_multi_write_cmos_sensor(hw_gcc_pair_init,
		sizeof(hw_gcc_pair_init) / sizeof(kal_uint16));
#endif
}

#if MULTI_WRITE
kal_uint16 addr_data_pair_preview_s5kjn1acc[] = {
0x6028,	0x2400,
0x602A,	0x1A28,
0x6F12,	0x4C00,
0x602A,	0x065A,
0x6F12,	0x0000,
0x602A,	0x139E,
0x6F12,	0x0100,
0x602A,	0x139C,
0x6F12,	0x0000,
0x602A,	0x13A0,
0x6F12,	0x0A00,
0x6F12,	0x0120,
0x602A,	0x2072,
0x6F12,	0x0000,
0x602A,	0x1A64,
0x6F12,	0x0301,
0x6F12,	0xFF00,
0x602A,	0x19E6,
0x6F12,	0x0200,
0x602A,	0x1A30,
0x6F12,	0x3401,
0x602A,	0x19FC,
0x6F12,	0x0B00,
0x602A,	0x19F4,
0x6F12,	0x0606,
0x602A,	0x19F8,
0x6F12,	0x1010,
0x602A,	0x1B26,
0x6F12,	0x6F80,
0x6F12,	0xA060,
0x602A,	0x1A3C,
0x6F12,	0x6207,
0x602A,	0x1A48,
0x6F12,	0x6207,
0x602A,	0x1444,
0x6F12,	0x2000,
0x6F12,	0x2000,
0x602A,	0x144C,
0x6F12,	0x3F00,
0x6F12,	0x3F00,
0x602A,	0x7F6C,
0x6F12,	0x0100,
0x6F12,	0x2F00,
0x6F12,	0xFA00,
0x6F12,	0x2400,
0x6F12,	0xE500,
0x602A,	0x0650,
0x6F12,	0x0600,
0x602A,	0x0654,
0x6F12,	0x0000,
0x602A,	0x1A46,
0x6F12,	0x8A00,
0x602A,	0x1A52,
0x6F12,	0xBF00,
0x602A,	0x0674,
0x6F12,	0x0500,
0x6F12,	0x0500,
0x6F12,	0x0500,
0x6F12,	0x0500,
0x602A,	0x0668,
0x6F12,	0x0800,
0x6F12,	0x0800,
0x6F12,	0x0800,
0x6F12,	0x0800,
0x602A,	0x0684,
0x6F12,	0x4001,
0x602A,	0x0688,
0x6F12,	0x4001,
0x602A,	0x147C,
0x6F12,	0x1000,
0x602A,	0x1480,
0x6F12,	0x1000,
0x602A,	0x19F6,
0x6F12,	0x0904,
0x602A,	0x0812,
0x6F12,	0x0000,
0x602A,	0x1A02,
0x6F12,	0x1800,
0x602A,	0x2148,
0x6F12,	0x0100,
0x602A,	0x2042,
0x6F12,	0x1A00,
0x602A,	0x0874,
0x6F12,	0x0100,
0x602A,	0x09C0,
0x6F12,	0x2008,
0x602A,	0x09C4,
0x6F12,	0x2000,
0x602A,	0x19FE,
0x6F12,	0x0E1C,
0x602A,	0x4D92,
0x6F12,	0x0100,
0x602A,	0x84C8,
0x6F12,	0x0100,
0x602A,	0x4D94,
0x6F12,	0x0005,
0x6F12,	0x000A,
0x6F12,	0x0010,
0x6F12,	0x0810,
0x6F12,	0x000A,
0x6F12,	0x0040,
0x6F12,	0x0810,
0x6F12,	0x0810,
0x6F12,	0x8002,
0x6F12,	0xFD03,
0x6F12,	0x0010,
0x6F12,	0x1510,
0x602A,	0x3570,
0x6F12,	0x0000,
0x602A,	0x3574,
0x6F12,	0x1201,
0x602A,	0x21E4,
0x6F12,	0x0400,
0x602A,	0x21EC,
0x6F12,	0x1F04,
0x602A,	0x2080,
0x6F12,	0x0101,
0x6F12,	0xFF00,
0x6F12,	0x7F01,
0x6F12,	0x0001,
0x6F12,	0x8001,
0x6F12,	0xD244,
0x6F12,	0xD244,
0x6F12,	0x14F4,
0x6F12,	0x0000,
0x6F12,	0x0000,
0x6F12,	0x0000,
0x602A,	0x20BA,
0x6F12,	0x121C,
0x6F12,	0x111C,
0x6F12,	0x54F4,
0x602A,	0x120E,
0x6F12,	0x1000,
0x602A,	0x212E,
0x6F12,	0x0200,
0x602A,	0x13AE,
0x6F12,	0x0101,
0x602A,	0x0718,
0x6F12,	0x0001,
0x602A,	0x0710,
0x6F12,	0x0002,
0x6F12,	0x0804,
0x6F12,	0x0100,
0x602A,	0x1B5C,
0x6F12,	0x0000,
0x602A,	0x0786,
0x6F12,	0x7701,
0x602A,	0x2022,
0x6F12,	0x0500,
0x6F12,	0x0500,
0x602A,	0x1360,
0x6F12,	0x0100,
0x602A,	0x1376,
0x6F12,	0x0100,
0x6F12,	0x6038,
0x6F12,	0x7038,
0x6F12,	0x8038,
0x602A,	0x1386,
0x6F12,	0x0B00,
0x602A,	0x06FA,
0x6F12,	0x1000,
0x602A,	0x4A94,
0x6F12,	0x0900,
0x6F12,	0x0000,
0x6F12,	0x0300,
0x6F12,	0x0000,
0x6F12,	0x0000,
0x6F12,	0x0000,
0x6F12,	0x0000,
0x6F12,	0x0000,
0x6F12,	0x0300,
0x6F12,	0x0000,
0x6F12,	0x0900,
0x6F12,	0x0000,
0x6F12,	0x0000,
0x6F12,	0x0000,
0x6F12,	0x0000,
0x6F12,	0x0000,
0x602A,	0x0A76,
0x6F12,	0x1000,
0x602A,	0x0AEE,
0x6F12,	0x1000,
0x602A,	0x0B66,
0x6F12,	0x1000,
0x602A,	0x0BDE,
0x6F12,	0x1000,
0x602A,	0x0BE8,
0x6F12,	0x3000,
0x6F12,	0x3000,
0x602A,	0x0C56,
0x6F12,	0x1000,
0x602A,	0x0C60,
0x6F12,	0x3000,
0x6F12,	0x3000,
0x602A,	0x0CB6,
0x6F12,	0x0100,
0x602A,	0x0CF2,
0x6F12,	0x0001,
0x602A,	0x0CF0,
0x6F12,	0x0101,
0x602A,	0x11B8,
0x6F12,	0x0100,
0x602A,	0x11F6,
0x6F12,	0x0020,
0x602A,	0x4A74,
0x6F12,	0x0000,
0x6F12,	0x0000,
0x6F12,	0xD8FF,
0x6F12,	0x0000,
0x6F12,	0x0000,
0x6F12,	0x0000,
0x6F12,	0x0000,
0x6F12,	0x0000,
0x6F12,	0xD8FF,
0x6F12,	0x0000,
0x6F12,	0x0000,
0x6F12,	0x0000,
0x6F12,	0x0000,
0x6F12,	0x0000,
0x6F12,	0x0000,
0x6F12,	0x0000,
0x602A,	0x218E,
0x6F12,	0x0000,
0x602A,	0x2268,
0x6F12,	0xF279,
0x602A,	0x5006,
0x6F12,	0x0000,
0x602A,	0x500E,
0x6F12,	0x0100,
0x602A,	0x4E70,
0x6F12,	0x2062,
0x6F12,	0x5501,
0x602A,	0x06DC,
0x6F12,	0x0000,
0x6F12,	0x0000,
0x6F12,	0x0000,
0x6F12,	0x0000,
0x6028,	0x4000,
0xF46A,	0xAE80,
0x0344,	0x0000,
0x0346,	0x0000,
0x0348,	0x1FFF,
0x034A,	0x181F,
0x034C,	0x0FF0,
0x034E,	0x0C00,
0x0350,	0x0008,
0x0352,	0x0008,
0x0900,	0x0122,
0x0380,	0x0002,
0x0382,	0x0002,
0x0384,	0x0002,
0x0386,	0x0002,
0x0110,	0x1002,
0x0114,	0x0301,
0x0116,	0x3000,
0x0136,	0x1800,
0x013E,	0x0000,
0x0300,	0x0006,
0x0302,	0x0001,
0x0304,	0x0004,
0x0306,	0x0088,
0x0308,	0x0008,
0x030A,	0x0001,
0x030C,	0x0000,
0x030E,	0x0004,
0x0310,	0x0072,
0x0312,	0x0000,
0x080E,	0x0000,
0x0340,	0x0D9E,
0x0342,	0x1450,
0x0702,	0x0000,
0x0202,	0x0100,
0x0200,	0x0100,
0x0D00,	0x0101,
0x0D02,	0x0101,
0x0D04,	0x0102,
0x6226,	0x0000,
};
#endif

static void preview_setting(void)
{
	LOG_INF("preview_setting start\n");
	s5kjn1_multi_write_cmos_sensor(
		addr_data_pair_preview_s5kjn1acc,
		sizeof(addr_data_pair_preview_s5kjn1acc) / sizeof(kal_uint16));
}
/*hs14 code for SR-AL6528A-01-54 by majunfeng at 2022-9-28 start*/
#if MULTI_WRITE
kal_uint16 addr_data_pair_hs_video_s5kjn1acc[] = {
    0x6028, 0x2400,
    0x602A, 0x1A28,
    0x6F12, 0x4C00,
    0x602A, 0x065A,
    0x6F12, 0x0000,
    0x602A, 0x139E,
    0x6F12, 0x0300,
    0x602A, 0x139C,
    0x6F12, 0x0000,
    0x602A, 0x13A0,
    0x6F12, 0x0A00,
    0x6F12, 0x0020,
    0x602A, 0x2072,
    0x6F12, 0x0000,
    0x602A, 0x1A64,
    0x6F12, 0x0301,
    0x6F12, 0x3F00,
    0x602A, 0x19E6,
    0x6F12, 0x0201,
    0x602A, 0x1A30,
    0x6F12, 0x3401,
    0x602A, 0x19FC,
    0x6F12, 0x0B00,
    0x602A, 0x19F4,
    0x6F12, 0x0606,
    0x602A, 0x19F8,
    0x6F12, 0x1010,
    0x602A, 0x1B26,
    0x6F12, 0x6F80,
    0x6F12, 0xA020,
    0x602A, 0x1A3C,
    0x6F12, 0x5207,
    0x602A, 0x1A48,
    0x6F12, 0x5207,
    0x602A, 0x1444,
    0x6F12, 0x2100,
    0x6F12, 0x2100,
    0x602A, 0x144C,
    0x6F12, 0x4200,
    0x6F12, 0x4200,
    0x602A, 0x7F6C,
    0x6F12, 0x0100,
    0x6F12, 0x3100,
    0x6F12, 0xF700,
    0x6F12, 0x2600,
    0x6F12, 0xE100,
    0x602A, 0x0650,
    0x6F12, 0x0600,
    0x602A, 0x0654,
    0x6F12, 0x0000,
    0x602A, 0x1A46,
    0x6F12, 0x8600,
    0x602A, 0x1A52,
    0x6F12, 0xBF00,
    0x602A, 0x0674,
    0x6F12, 0x0500,
    0x6F12, 0x0500,
    0x6F12, 0x0500,
    0x6F12, 0x0500,
    0x602A, 0x0668,
    0x6F12, 0x0800,
    0x6F12, 0x0800,
    0x6F12, 0x0800,
    0x6F12, 0x0800,
    0x602A, 0x0684,
    0x6F12, 0x4001,
    0x602A, 0x0688,
    0x6F12, 0x4001,
    0x602A, 0x147C,
    0x6F12, 0x1000,
    0x602A, 0x1480,
    0x6F12, 0x1000,
    0x602A, 0x19F6,
    0x6F12, 0x0904,
    0x602A, 0x0812,
    0x6F12, 0x0000,
    0x602A, 0x1A02,
    0x6F12, 0x0800,
    0x602A, 0x2148,
    0x6F12, 0x0100,
    0x602A, 0x2042,
    0x6F12, 0x1A00,
    0x602A, 0x0874,
    0x6F12, 0x1100,
    0x602A, 0x09C0,
    0x6F12, 0x1803,
    0x602A, 0x09C4,
    0x6F12, 0x1803,
    0x602A, 0x19FE,
    0x6F12, 0x0E1C,
    0x602A, 0x4D92,
    0x6F12, 0x0100,
    0x602A, 0x84C8,
    0x6F12, 0x0100,
    0x602A, 0x4D94,
    0x6F12, 0x4001,
    0x6F12, 0x0004,
    0x6F12, 0x0010,
    0x6F12, 0x0810,
    0x6F12, 0x0004,
    0x6F12, 0x0010,
    0x6F12, 0x0810,
    0x6F12, 0x0810,
    0x6F12, 0x0000,
    0x6F12, 0x0000,
    0x6F12, 0x0010,
    0x6F12, 0x0010,
    0x602A, 0x3570,
    0x6F12, 0x0000,
    0x602A, 0x3574,
    0x6F12, 0x3801,
    0x602A, 0x21E4,
    0x6F12, 0x0400,
    0x602A, 0x21EC,
    0x6F12, 0x6801,
    0x602A, 0x2080,
    0x6F12, 0x0100,
    0x6F12, 0x7F00,
    0x6F12, 0x0002,
    0x6F12, 0x8000,
    0x6F12, 0x0002,
    0x6F12, 0xC244,
    0x6F12, 0xD244,
    0x6F12, 0x14F4,
    0x6F12, 0x141C,
    0x6F12, 0x111C,
    0x6F12, 0x54F4,
    0x602A, 0x20BA,
    0x6F12, 0x0000,
    0x6F12, 0x0000,
    0x6F12, 0x0000,
    0x602A, 0x120E,
    0x6F12, 0x1000,
    0x602A, 0x212E,
    0x6F12, 0x0A00,
    0x602A, 0x13AE,
    0x6F12, 0x0102,
    0x602A, 0x0718,
    0x6F12, 0x0005,
    0x602A, 0x0710,
    0x6F12, 0x0004,
    0x6F12, 0x0401,
    0x6F12, 0x0100,
    0x602A, 0x1B5C,
    0x6F12, 0x0300,
    0x602A, 0x0786,
    0x6F12, 0x7701,
    0x602A, 0x2022,
    0x6F12, 0x0101,
    0x6F12, 0x0101,
    0x602A, 0x1360,
    0x6F12, 0x0000,
    0x602A, 0x1376,
    0x6F12, 0x0200,
    0x6F12, 0x6038,
    0x6F12, 0x7038,
    0x6F12, 0x8038,
    0x602A, 0x1386,
    0x6F12, 0x0B00,
    0x602A, 0x06FA,
    0x6F12, 0x1000,
    0x602A, 0x4A94,
    0x6F12, 0x1600,
    0x6F12, 0x0000,
    0x6F12, 0x1000,
    0x6F12, 0x0000,
    0x6F12, 0x0000,
    0x6F12, 0x0000,
    0x6F12, 0x0000,
    0x6F12, 0x0000,
    0x6F12, 0x1000,
    0x6F12, 0x0000,
    0x6F12, 0x1600,
    0x6F12, 0x0000,
    0x6F12, 0x0000,
    0x6F12, 0x0000,
    0x6F12, 0x0000,
    0x6F12, 0x0000,
    0x602A, 0x0A76,
    0x6F12, 0x1000,
    0x602A, 0x0AEE,
    0x6F12, 0x1000,
    0x602A, 0x0B66,
    0x6F12, 0x1000,
    0x602A, 0x0BDE,
    0x6F12, 0x1000,
    0x602A, 0x0BE8,
    0x6F12, 0x3000,
    0x6F12, 0x3000,
    0x602A, 0x0C56,
    0x6F12, 0x1000,
    0x602A, 0x0C60,
    0x6F12, 0x3000,
    0x6F12, 0x3000,
    0x602A, 0x0CB6,
    0x6F12, 0x0000,
    0x602A, 0x0CF2,
    0x6F12, 0x0001,
    0x602A, 0x0CF0,
    0x6F12, 0x0101,
    0x602A, 0x11B8,
    0x6F12, 0x0000,
    0x602A, 0x11F6,
    0x6F12, 0x0010,
    0x602A, 0x4A74,
    0x6F12, 0x0000,
    0x6F12, 0x0000,
    0x6F12, 0xD8FF,
    0x6F12, 0x0000,
    0x6F12, 0x0000,
    0x6F12, 0x0000,
    0x6F12, 0x0000,
    0x6F12, 0x0000,
    0x6F12, 0xD8FF,
    0x6F12, 0x0000,
    0x6F12, 0x0000,
    0x6F12, 0x0000,
    0x6F12, 0x0000,
    0x6F12, 0x0000,
    0x6F12, 0x0000,
    0x6F12, 0x0000,
    0x602A, 0x218E,
    0x6F12, 0x0000,
    0x602A, 0x2268,
    0x6F12, 0xF279,
    0x602A, 0x5006,
    0x6F12, 0x0000,
    0x602A, 0x500E,
    0x6F12, 0x0100,
    0x602A, 0x4E70,
    0x6F12, 0x2062,
    0x6F12, 0x5501,
    0x602A, 0x06DC,
    0x6F12, 0x0000,
    0x6F12, 0x0000,
    0x6F12, 0x0000,
    0x6F12, 0x0000,
    0x6028, 0x4000,
    0xF46A, 0xAE80,
    0x0344, 0x05F0,
    0x0346, 0x0660,
    0x0348, 0x1A0F,
    0x034A, 0x11BF,
    0x034C, 0x0500,
    0x034E, 0x02D0,
    0x0350, 0x0004,
    0x0352, 0x0004,
    0x0900, 0x0144,
    0x0380, 0x0002,
    0x0382, 0x0006,
    0x0384, 0x0002,
    0x0386, 0x0006,
    0x0110, 0x1002,
    0x0114, 0x0300,
    0x0116, 0x3000,
    0x0136, 0x1800,
    0x013E, 0x0000,
    0x0300, 0x0006,
    0x0302, 0x0001,
    0x0304, 0x0004,
    0x0306, 0x0096,
    0x0308, 0x0008,
    0x030A, 0x0001,
    0x030C, 0x0000,
    0x030E, 0x0003,
    0x0310, 0x0064,
    0x0312, 0x0000,
    0x080E, 0x0000,
    0x0340, 0x094C,
    0x0342, 0x0830,
    0x0702, 0x0000,
    0x0202, 0x0100,
    0x0200, 0x0100,
    0x0D00, 0x0101,
    0x0D02, 0x0001,
    0x0D04, 0x0002,
    0x6226, 0x0000,
};

#endif
/*hs14 code for SR-AL6528A-01-54 by majunfeng at 2022-9-28 start*/
static void hs_video_setting(void) /// 120fps
{
	LOG_INF("hs_video_setting RES_1280x720_120fps\n");
	s5kjn1_multi_write_cmos_sensor(
		addr_data_pair_hs_video_s5kjn1acc,
		sizeof(addr_data_pair_hs_video_s5kjn1acc) / sizeof(kal_uint16));
}

#if MULTI_WRITE
kal_uint16 addr_data_pair_capture_s5kjn1acc[] = {
0x6028,	0x2400,
0x602A,	0x1A28,
0x6F12,	0x4C00,
0x602A,	0x065A,
0x6F12,	0x0000,
0x602A,	0x139E,
0x6F12,	0x0100,
0x602A,	0x139C,
0x6F12,	0x0000,
0x602A,	0x13A0,
0x6F12,	0x0A00,
0x6F12,	0x0120,
0x602A,	0x2072,
0x6F12,	0x0000,
0x602A,	0x1A64,
0x6F12,	0x0301,
0x6F12,	0xFF00,
0x602A,	0x19E6,
0x6F12,	0x0200,
0x602A,	0x1A30,
0x6F12,	0x3401,
0x602A,	0x19FC,
0x6F12,	0x0B00,
0x602A,	0x19F4,
0x6F12,	0x0606,
0x602A,	0x19F8,
0x6F12,	0x1010,
0x602A,	0x1B26,
0x6F12,	0x6F80,
0x6F12,	0xA060,
0x602A,	0x1A3C,
0x6F12,	0x6207,
0x602A,	0x1A48,
0x6F12,	0x6207,
0x602A,	0x1444,
0x6F12,	0x2000,
0x6F12,	0x2000,
0x602A,	0x144C,
0x6F12,	0x3F00,
0x6F12,	0x3F00,
0x602A,	0x7F6C,
0x6F12,	0x0100,
0x6F12,	0x2F00,
0x6F12,	0xFA00,
0x6F12,	0x2400,
0x6F12,	0xE500,
0x602A,	0x0650,
0x6F12,	0x0600,
0x602A,	0x0654,
0x6F12,	0x0000,
0x602A,	0x1A46,
0x6F12,	0x8A00,
0x602A,	0x1A52,
0x6F12,	0xBF00,
0x602A,	0x0674,
0x6F12,	0x0500,
0x6F12,	0x0500,
0x6F12,	0x0500,
0x6F12,	0x0500,
0x602A,	0x0668,
0x6F12,	0x0800,
0x6F12,	0x0800,
0x6F12,	0x0800,
0x6F12,	0x0800,
0x602A,	0x0684,
0x6F12,	0x4001,
0x602A,	0x0688,
0x6F12,	0x4001,
0x602A,	0x147C,
0x6F12,	0x1000,
0x602A,	0x1480,
0x6F12,	0x1000,
0x602A,	0x19F6,
0x6F12,	0x0904,
0x602A,	0x0812,
0x6F12,	0x0000,
0x602A,	0x1A02,
0x6F12,	0x1800,
0x602A,	0x2148,
0x6F12,	0x0100,
0x602A,	0x2042,
0x6F12,	0x1A00,
0x602A,	0x0874,
0x6F12,	0x0100,
0x602A,	0x09C0,
0x6F12,	0x2008,
0x602A,	0x09C4,
0x6F12,	0x2000,
0x602A,	0x19FE,
0x6F12,	0x0E1C,
0x602A,	0x4D92,
0x6F12,	0x0100,
0x602A,	0x84C8,
0x6F12,	0x0100,
0x602A,	0x4D94,
0x6F12,	0x0005,
0x6F12,	0x000A,
0x6F12,	0x0010,
0x6F12,	0x0810,
0x6F12,	0x000A,
0x6F12,	0x0040,
0x6F12,	0x0810,
0x6F12,	0x0810,
0x6F12,	0x8002,
0x6F12,	0xFD03,
0x6F12,	0x0010,
0x6F12,	0x1510,
0x602A,	0x3570,
0x6F12,	0x0000,
0x602A,	0x3574,
0x6F12,	0x1201,
0x602A,	0x21E4,
0x6F12,	0x0400,
0x602A,	0x21EC,
0x6F12,	0x1F04,
0x602A,	0x2080,
0x6F12,	0x0101,
0x6F12,	0xFF00,
0x6F12,	0x7F01,
0x6F12,	0x0001,
0x6F12,	0x8001,
0x6F12,	0xD244,
0x6F12,	0xD244,
0x6F12,	0x14F4,
0x6F12,	0x0000,
0x6F12,	0x0000,
0x6F12,	0x0000,
0x602A,	0x20BA,
0x6F12,	0x121C,
0x6F12,	0x111C,
0x6F12,	0x54F4,
0x602A,	0x120E,
0x6F12,	0x1000,
0x602A,	0x212E,
0x6F12,	0x0200,
0x602A,	0x13AE,
0x6F12,	0x0101,
0x602A,	0x0718,
0x6F12,	0x0001,
0x602A,	0x0710,
0x6F12,	0x0002,
0x6F12,	0x0804,
0x6F12,	0x0100,
0x602A,	0x1B5C,
0x6F12,	0x0000,
0x602A,	0x0786,
0x6F12,	0x7701,
0x602A,	0x2022,
0x6F12,	0x0500,
0x6F12,	0x0500,
0x602A,	0x1360,
0x6F12,	0x0100,
0x602A,	0x1376,
0x6F12,	0x0100,
0x6F12,	0x6038,
0x6F12,	0x7038,
0x6F12,	0x8038,
0x602A,	0x1386,
0x6F12,	0x0B00,
0x602A,	0x06FA,
0x6F12,	0x1000,
0x602A,	0x4A94,
0x6F12,	0x0900,
0x6F12,	0x0000,
0x6F12,	0x0300,
0x6F12,	0x0000,
0x6F12,	0x0000,
0x6F12,	0x0000,
0x6F12,	0x0000,
0x6F12,	0x0000,
0x6F12,	0x0300,
0x6F12,	0x0000,
0x6F12,	0x0900,
0x6F12,	0x0000,
0x6F12,	0x0000,
0x6F12,	0x0000,
0x6F12,	0x0000,
0x6F12,	0x0000,
0x602A,	0x0A76,
0x6F12,	0x1000,
0x602A,	0x0AEE,
0x6F12,	0x1000,
0x602A,	0x0B66,
0x6F12,	0x1000,
0x602A,	0x0BDE,
0x6F12,	0x1000,
0x602A,	0x0BE8,
0x6F12,	0x3000,
0x6F12,	0x3000,
0x602A,	0x0C56,
0x6F12,	0x1000,
0x602A,	0x0C60,
0x6F12,	0x3000,
0x6F12,	0x3000,
0x602A,	0x0CB6,
0x6F12,	0x0100,
0x602A,	0x0CF2,
0x6F12,	0x0001,
0x602A,	0x0CF0,
0x6F12,	0x0101,
0x602A,	0x11B8,
0x6F12,	0x0100,
0x602A,	0x11F6,
0x6F12,	0x0020,
0x602A,	0x4A74,
0x6F12,	0x0000,
0x6F12,	0x0000,
0x6F12,	0xD8FF,
0x6F12,	0x0000,
0x6F12,	0x0000,
0x6F12,	0x0000,
0x6F12,	0x0000,
0x6F12,	0x0000,
0x6F12,	0xD8FF,
0x6F12,	0x0000,
0x6F12,	0x0000,
0x6F12,	0x0000,
0x6F12,	0x0000,
0x6F12,	0x0000,
0x6F12,	0x0000,
0x6F12,	0x0000,
0x602A,	0x218E,
0x6F12,	0x0000,
0x602A,	0x2268,
0x6F12,	0xF279,
0x602A,	0x5006,
0x6F12,	0x0000,
0x602A,	0x500E,
0x6F12,	0x0100,
0x602A,	0x4E70,
0x6F12,	0x2062,
0x6F12,	0x5501,
0x602A,	0x06DC,
0x6F12,	0x0000,
0x6F12,	0x0000,
0x6F12,	0x0000,
0x6F12,	0x0000,
0x6028,	0x4000,
0xF46A,	0xAE80,
0x0344,	0x0000,
0x0346,	0x0000,
0x0348,	0x1FFF,
0x034A,	0x181F,
0x034C,	0x0FF0,
0x034E,	0x0C00,
0x0350,	0x0008,
0x0352,	0x0008,
0x0900,	0x0122,
0x0380,	0x0002,
0x0382,	0x0002,
0x0384,	0x0002,
0x0386,	0x0002,
0x0110,	0x1002,
0x0114,	0x0301,
0x0116,	0x3000,
0x0136,	0x1800,
0x013E,	0x0000,
0x0300,	0x0006,
0x0302,	0x0001,
0x0304,	0x0004,
0x0306,	0x0088,
0x0308,	0x0008,
0x030A,	0x0001,
0x030C,	0x0000,
0x030E,	0x0004,
0x0310,	0x0072,
0x0312,	0x0000,
0x080E,	0x0000,
0x0340,	0x0D9E,
0x0342,	0x1450,
0x0702,	0x0000,
0x0202,	0x0100,
0x0200,	0x0100,
0x0D00,	0x0101,
0x0D02,	0x0101,
0x0D04,	0x0102,
0x6226,	0x0000,
};
#endif

static void capture_setting(kal_uint16 currefps)
{
	LOG_INF("start\n");
	s5kjn1_multi_write_cmos_sensor(
		addr_data_pair_capture_s5kjn1acc,
		sizeof(addr_data_pair_capture_s5kjn1acc) / sizeof(kal_uint16));
}

#if MULTI_WRITE
kal_uint16 addr_data_pair_normal_video_s5kjn1acc[] = {
0x6028,	0x2400,
0x602A,	0x1A28,
0x6F12,	0x4C00,
0x602A,	0x065A,
0x6F12,	0x0000,
0x602A,	0x139E,
0x6F12,	0x0100,
0x602A,	0x139C,
0x6F12,	0x0000,
0x602A,	0x13A0,
0x6F12,	0x0A00,
0x6F12,	0x0120,
0x602A,	0x2072,
0x6F12,	0x0000,
0x602A,	0x1A64,
0x6F12,	0x0301,
0x6F12,	0xFF00,
0x602A,	0x19E6,
0x6F12,	0x0200,
0x602A,	0x1A30,
0x6F12,	0x3401,
0x602A,	0x19FC,
0x6F12,	0x0B00,
0x602A,	0x19F4,
0x6F12,	0x0606,
0x602A,	0x19F8,
0x6F12,	0x1010,
0x602A,	0x1B26,
0x6F12,	0x6F80,
0x6F12,	0xA060,
0x602A,	0x1A3C,
0x6F12,	0x6207,
0x602A,	0x1A48,
0x6F12,	0x6207,
0x602A,	0x1444,
0x6F12,	0x2000,
0x6F12,	0x2000,
0x602A,	0x144C,
0x6F12,	0x3F00,
0x6F12,	0x3F00,
0x602A,	0x7F6C,
0x6F12,	0x0100,
0x6F12,	0x2F00,
0x6F12,	0xFA00,
0x6F12,	0x2400,
0x6F12,	0xE500,
0x602A,	0x0650,
0x6F12,	0x0600,
0x602A,	0x0654,
0x6F12,	0x0000,
0x602A,	0x1A46,
0x6F12,	0x8A00,
0x602A,	0x1A52,
0x6F12,	0xBF00,
0x602A,	0x0674,
0x6F12,	0x0500,
0x6F12,	0x0500,
0x6F12,	0x0500,
0x6F12,	0x0500,
0x602A,	0x0668,
0x6F12,	0x0800,
0x6F12,	0x0800,
0x6F12,	0x0800,
0x6F12,	0x0800,
0x602A,	0x0684,
0x6F12,	0x4001,
0x602A,	0x0688,
0x6F12,	0x4001,
0x602A,	0x147C,
0x6F12,	0x1000,
0x602A,	0x1480,
0x6F12,	0x1000,
0x602A,	0x19F6,
0x6F12,	0x0904,
0x602A,	0x0812,
0x6F12,	0x0000,
0x602A,	0x1A02,
0x6F12,	0x1800,
0x602A,	0x2148,
0x6F12,	0x0100,
0x602A,	0x2042,
0x6F12,	0x1A00,
0x602A,	0x0874,
0x6F12,	0x0100,
0x602A,	0x09C0,
0x6F12,	0x2008,
0x602A,	0x09C4,
0x6F12,	0x2000,
0x602A,	0x19FE,
0x6F12,	0x0E1C,
0x602A,	0x4D92,
0x6F12,	0x0100,
0x602A,	0x84C8,
0x6F12,	0x0100,
0x602A,	0x4D94,
0x6F12,	0x0005,
0x6F12,	0x000A,
0x6F12,	0x0010,
0x6F12,	0x0810,
0x6F12,	0x000A,
0x6F12,	0x0040,
0x6F12,	0x0810,
0x6F12,	0x0810,
0x6F12,	0x8002,
0x6F12,	0xFD03,
0x6F12,	0x0010,
0x6F12,	0x1510,
0x602A,	0x3570,
0x6F12,	0x0000,
0x602A,	0x3574,
0x6F12,	0x1201,
0x602A,	0x21E4,
0x6F12,	0x0400,
0x602A,	0x21EC,
0x6F12,	0x1F04,
0x602A,	0x2080,
0x6F12,	0x0101,
0x6F12,	0xFF00,
0x6F12,	0x7F01,
0x6F12,	0x0001,
0x6F12,	0x8001,
0x6F12,	0xD244,
0x6F12,	0xD244,
0x6F12,	0x14F4,
0x6F12,	0x0000,
0x6F12,	0x0000,
0x6F12,	0x0000,
0x602A,	0x20BA,
0x6F12,	0x121C,
0x6F12,	0x111C,
0x6F12,	0x54F4,
0x602A,	0x120E,
0x6F12,	0x1000,
0x602A,	0x212E,
0x6F12,	0x0200,
0x602A,	0x13AE,
0x6F12,	0x0101,
0x602A,	0x0718,
0x6F12,	0x0001,
0x602A,	0x0710,
0x6F12,	0x0002,
0x6F12,	0x0804,
0x6F12,	0x0100,
0x602A,	0x1B5C,
0x6F12,	0x0000,
0x602A,	0x0786,
0x6F12,	0x7701,
0x602A,	0x2022,
0x6F12,	0x0500,
0x6F12,	0x0500,
0x602A,	0x1360,
0x6F12,	0x0100,
0x602A,	0x1376,
0x6F12,	0x0100,
0x6F12,	0x6038,
0x6F12,	0x7038,
0x6F12,	0x8038,
0x602A,	0x1386,
0x6F12,	0x0B00,
0x602A,	0x06FA,
0x6F12,	0x1000,
0x602A,	0x4A94,
0x6F12,	0x0900,
0x6F12,	0x0000,
0x6F12,	0x0300,
0x6F12,	0x0000,
0x6F12,	0x0000,
0x6F12,	0x0000,
0x6F12,	0x0000,
0x6F12,	0x0000,
0x6F12,	0x0300,
0x6F12,	0x0000,
0x6F12,	0x0900,
0x6F12,	0x0000,
0x6F12,	0x0000,
0x6F12,	0x0000,
0x6F12,	0x0000,
0x6F12,	0x0000,
0x602A,	0x0A76,
0x6F12,	0x1000,
0x602A,	0x0AEE,
0x6F12,	0x1000,
0x602A,	0x0B66,
0x6F12,	0x1000,
0x602A,	0x0BDE,
0x6F12,	0x1000,
0x602A,	0x0BE8,
0x6F12,	0x3000,
0x6F12,	0x3000,
0x602A,	0x0C56,
0x6F12,	0x1000,
0x602A,	0x0C60,
0x6F12,	0x3000,
0x6F12,	0x3000,
0x602A,	0x0CB6,
0x6F12,	0x0100,
0x602A,	0x0CF2,
0x6F12,	0x0001,
0x602A,	0x0CF0,
0x6F12,	0x0101,
0x602A,	0x11B8,
0x6F12,	0x0100,
0x602A,	0x11F6,
0x6F12,	0x0020,
0x602A,	0x4A74,
0x6F12,	0x0000,
0x6F12,	0x0000,
0x6F12,	0xD8FF,
0x6F12,	0x0000,
0x6F12,	0x0000,
0x6F12,	0x0000,
0x6F12,	0x0000,
0x6F12,	0x0000,
0x6F12,	0xD8FF,
0x6F12,	0x0000,
0x6F12,	0x0000,
0x6F12,	0x0000,
0x6F12,	0x0000,
0x6F12,	0x0000,
0x6F12,	0x0000,
0x6F12,	0x0000,
0x602A,	0x218E,
0x6F12,	0x0000,
0x602A,	0x2268,
0x6F12,	0xF279,
0x602A,	0x5006,
0x6F12,	0x0000,
0x602A,	0x500E,
0x6F12,	0x0100,
0x602A,	0x4E70,
0x6F12,	0x2062,
0x6F12,	0x5501,
0x602A,	0x06DC,
0x6F12,	0x0000,
0x6F12,	0x0000,
0x6F12,	0x0000,
0x6F12,	0x0000,
0x6028,	0x4000,
0xF46A,	0xAE80,
0x0344,	0x0000,
0x0346,	0x0000,
0x0348,	0x1FFF,
0x034A,	0x181F,
0x034C,	0x0FF0,
0x034E,	0x0C00,
0x0350,	0x0008,
0x0352,	0x0008,
0x0900,	0x0122,
0x0380,	0x0002,
0x0382,	0x0002,
0x0384,	0x0002,
0x0386,	0x0002,
0x0110,	0x1002,
0x0114,	0x0301,
0x0116,	0x3000,
0x0136,	0x1800,
0x013E,	0x0000,
0x0300,	0x0006,
0x0302,	0x0001,
0x0304,	0x0004,
0x0306,	0x0088,
0x0308,	0x0008,
0x030A,	0x0001,
0x030C,	0x0000,
0x030E,	0x0004,
0x0310,	0x0072,
0x0312,	0x0000,
0x080E,	0x0000,
0x0340,	0x0D9E,
0x0342,	0x1450,
0x0702,	0x0000,
0x0202,	0x0100,
0x0200,	0x0100,
0x0D00,	0x0101,
0x0D02,	0x0101,
0x0D04,	0x0102,
0x6226,	0x0000,
};
#endif

static void normal_video_setting(kal_uint16 currefps) // 30fps
{
	LOG_INF("normal_video_setting start\n");
	s5kjn1_multi_write_cmos_sensor(
		addr_data_pair_normal_video_s5kjn1acc,
		sizeof(addr_data_pair_normal_video_s5kjn1acc) /
			sizeof(kal_uint16));
}

#if MULTI_WRITE
kal_uint16 addr_data_pair_slim_video_s5kjn1acc[] = {
    0x6028, 0x2400,
    0x602A, 0x1A28,
    0x6F12, 0x4C00,
    0x602A, 0x065A,
    0x6F12, 0x0000,
    0x602A, 0x139E,
    0x6F12, 0x0300,
    0x602A, 0x139C,
    0x6F12, 0x0000,
    0x602A, 0x13A0,
    0x6F12, 0x0A00,
    0x6F12, 0x0020,
    0x602A, 0x2072,
    0x6F12, 0x0000,
    0x602A, 0x1A64,
    0x6F12, 0x0301,
    0x6F12, 0x3F00,
    0x602A, 0x19E6,
    0x6F12, 0x0201,
    0x602A, 0x1A30,
    0x6F12, 0x3401,
    0x602A, 0x19FC,
    0x6F12, 0x0B00,
    0x602A, 0x19F4,
    0x6F12, 0x0606,
    0x602A, 0x19F8,
    0x6F12, 0x1010,
    0x602A, 0x1B26,
    0x6F12, 0x6F80,
    0x6F12, 0xA020,
    0x602A, 0x1A3C,
    0x6F12, 0x5207,
    0x602A, 0x1A48,
    0x6F12, 0x5207,
    0x602A, 0x1444,
    0x6F12, 0x2100,
    0x6F12, 0x2100,
    0x602A, 0x144C,
    0x6F12, 0x4200,
    0x6F12, 0x4200,
    0x602A, 0x7F6C,
    0x6F12, 0x0100,
    0x6F12, 0x3100,
    0x6F12, 0xF700,
    0x6F12, 0x2600,
    0x6F12, 0xE100,
    0x602A, 0x0650,
    0x6F12, 0x0600,
    0x602A, 0x0654,
    0x6F12, 0x0000,
    0x602A, 0x1A46,
    0x6F12, 0x8600,
    0x602A, 0x1A52,
    0x6F12, 0xBF00,
    0x602A, 0x0674,
    0x6F12, 0x0500,
    0x6F12, 0x0500,
    0x6F12, 0x0500,
    0x6F12, 0x0500,
    0x602A, 0x0668,
    0x6F12, 0x0800,
    0x6F12, 0x0800,
    0x6F12, 0x0800,
    0x6F12, 0x0800,
    0x602A, 0x0684,
    0x6F12, 0x4001,
    0x602A, 0x0688,
    0x6F12, 0x4001,
    0x602A, 0x147C,
    0x6F12, 0x1000,
    0x602A, 0x1480,
    0x6F12, 0x1000,
    0x602A, 0x19F6,
    0x6F12, 0x0904,
    0x602A, 0x0812,
    0x6F12, 0x0000,
    0x602A, 0x1A02,
    0x6F12, 0x0800,
    0x602A, 0x2148,
    0x6F12, 0x0100,
    0x602A, 0x2042,
    0x6F12, 0x1A00,
    0x602A, 0x0874,
    0x6F12, 0x1100,
    0x602A, 0x09C0,
    0x6F12, 0x9800,
    0x602A, 0x09C4,
    0x6F12, 0x9800,
    0x602A, 0x19FE,
    0x6F12, 0x0E1C,
    0x602A, 0x4D92,
    0x6F12, 0x0100,
    0x602A, 0x84C8,
    0x6F12, 0x0100,
    0x602A, 0x4D94,
    0x6F12, 0x4001,
    0x6F12, 0x0004,
    0x6F12, 0x0010,
    0x6F12, 0x0810,
    0x6F12, 0x0004,
    0x6F12, 0x0010,
    0x6F12, 0x0810,
    0x6F12, 0x0810,
    0x6F12, 0x0000,
    0x6F12, 0x0000,
    0x6F12, 0x0010,
    0x6F12, 0x0010,
    0x602A, 0x3570,
    0x6F12, 0x0000,
    0x602A, 0x3574,
    0x6F12, 0x9400,
    0x602A, 0x21E4,
    0x6F12, 0x0400,
    0x602A, 0x21EC,
    0x6F12, 0x4F01,
    0x602A, 0x2080,
    0x6F12, 0x0100,
    0x6F12, 0x7F00,
    0x602A, 0x2086,
    0x6F12, 0x8000,
    0x6F12, 0x0002,
    0x6F12, 0xC244,
    0x6F12, 0xD244,
    0x6F12, 0x14F4,
    0x6F12, 0x141C,
    0x602A, 0x208E,
    0x6F12, 0x14F4,
    0x602A, 0x208A,
    0x6F12, 0xC244,
    0x6F12, 0xD244,
    0x6F12, 0x0000,
    0x602A, 0x120E,
    0x6F12, 0x1000,
    0x602A, 0x212E,
    0x6F12, 0x0A00,
    0x602A, 0x13AE,
    0x6F12, 0x0102,
    0x602A, 0x0718,
    0x6F12, 0x0005,
    0x602A, 0x0710,
    0x6F12, 0x0004,
    0x6F12, 0x0401,
    0x6F12, 0x0100,
    0x602A, 0x1B5C,
    0x6F12, 0x0300,
    0x602A, 0x0786,
    0x6F12, 0x7701,
    0x602A, 0x2022,
    0x6F12, 0x0101,
    0x6F12, 0x0101,
    0x602A, 0x1360,
    0x6F12, 0x0000,
    0x602A, 0x1376,
    0x6F12, 0x0200,
    0x6F12, 0x6038,
    0x6F12, 0x7038,
    0x6F12, 0x8038,
    0x602A, 0x1386,
    0x6F12, 0x0B00,
    0x602A, 0x06FA,
    0x6F12, 0x1000,
    0x602A, 0x4A94,
    0x6F12, 0x0C00,
    0x6F12, 0x0000,
    0x6F12, 0x0600,
    0x6F12, 0x0000,
    0x6F12, 0x0000,
    0x6F12, 0x0000,
    0x6F12, 0x0000,
    0x6F12, 0x0000,
    0x6F12, 0x0600,
    0x6F12, 0x0000,
    0x6F12, 0x0C00,
    0x6F12, 0x0000,
    0x6F12, 0x0000,
    0x6F12, 0x0000,
    0x6F12, 0x0000,
    0x6F12, 0x0000,
    0x602A, 0x0A76,
    0x6F12, 0x1000,
    0x602A, 0x0AEE,
    0x6F12, 0x1000,
    0x602A, 0x0B66,
    0x6F12, 0x1000,
    0x602A, 0x0BDE,
    0x6F12, 0x1000,
    0x602A, 0x0BE8,
    0x6F12, 0x3000,
    0x6F12, 0x3000,
    0x602A, 0x0C56,
    0x6F12, 0x1000,
    0x602A, 0x0C60,
    0x6F12, 0x3000,
    0x6F12, 0x3000,
    0x602A, 0x0CB6,
    0x6F12, 0x0000,
    0x602A, 0x0CF2,
    0x6F12, 0x0001,
    0x602A, 0x0CF0,
    0x6F12, 0x0101,
    0x602A, 0x11B8,
    0x6F12, 0x0000,
    0x602A, 0x11F6,
    0x6F12, 0x0010,
    0x602A, 0x4A74,
    0x6F12, 0x0000,
    0x6F12, 0x0000,
    0x6F12, 0xD8FF,
    0x6F12, 0x0000,
    0x6F12, 0x0000,
    0x6F12, 0x0000,
    0x6F12, 0x0000,
    0x6F12, 0x0000,
    0x6F12, 0xD8FF,
    0x6F12, 0x0000,
    0x6F12, 0x0000,
    0x6F12, 0x0000,
    0x6F12, 0x0000,
    0x6F12, 0x0000,
    0x6F12, 0x0000,
    0x6F12, 0x0000,
    0x602A, 0x218E,
    0x6F12, 0x0000,
    0x602A, 0x2268,
    0x6F12, 0xF279,
    0x602A, 0x5006,
    0x6F12, 0x0000,
    0x602A, 0x500E,
    0x6F12, 0x0100,
    0x602A, 0x4E70,
    0x6F12, 0x2062,
    0x6F12, 0x5501,
    0x602A, 0x06DC,
    0x6F12, 0x0000,
    0x6F12, 0x0000,
    0x6F12, 0x0000,
    0x6F12, 0x0000,
    0x6028, 0x4000,
    0xF46A, 0xAE80,
    0x0344, 0x00F0,
    0x0346, 0x0390,
    0x0348, 0x1F0F,
    0x034A, 0x148F,
    0x034C, 0x0780,
    0x034E, 0x0438,
    0x0350, 0x0004,
    0x0352, 0x0004,
    0x0900, 0x0144,
    0x0380, 0x0002,
    0x0382, 0x0006,
    0x0384, 0x0002,
    0x0386, 0x0006,
    0x0110, 0x1002,
    0x0114, 0x0301,
    0x0116, 0x3000,
    0x0136, 0x1800,
    0x013E, 0x0000,
    0x0300, 0x0006,
    0x0302, 0x0001,
    0x0304, 0x0004,
    0x0306, 0x0096,
    0x0308, 0x0008,
    0x030A, 0x0001,
    0x030C, 0x0000,
    0x030E, 0x0003,
    0x0310, 0x0064,
    0x0312, 0x0001,
    0x080E, 0x0000,
    0x0340, 0x1060,
    0x0342, 0x12A0,
    0x0702, 0x0000,
    0x0202, 0x0100,
    0x0200, 0x0100,
    0x0D00, 0x0101,
    0x0D02, 0x0001,
    0x0D04, 0x0102,
    0x6226, 0x0000,
};
#endif

static void slim_video_setting(void) //60fps
{
	LOG_INF("JN1_8160*6144_setting\n");
	s5kjn1_multi_write_cmos_sensor(
		addr_data_pair_slim_video_s5kjn1acc,
		sizeof(addr_data_pair_slim_video_s5kjn1acc) /
			sizeof(kal_uint16));
}

#if MULTI_WRITE
kal_uint16 addr_data_pair_custom1_s5kjn1acc[] = {
	0x6028, 0x2400, 0x602A, 0x1A28, 0x6F12, 0x4C00, 0x602A, 0x065A, 0x6F12,
	0x0000, 0x602A, 0x139E, 0x6F12, 0x0100, 0x602A, 0x139C, 0x6F12, 0x0000,
	0x602A, 0x13A0, 0x6F12, 0x0A00, 0x6F12, 0x0120, 0x602A, 0x2072, 0x6F12,
	0x0000, 0x602A, 0x1A64, 0x6F12, 0x0301, 0x6F12, 0xFF00, 0x602A, 0x19E6,
	0x6F12, 0x0200, 0x602A, 0x1A30, 0x6F12, 0x3401, 0x602A, 0x19FC, 0x6F12,
	0x0B00, 0x602A, 0x19F4, 0x6F12, 0x0606, 0x602A, 0x19F8, 0x6F12, 0x1010,
	0x602A, 0x1B26, 0x6F12, 0x6F80, 0x6F12, 0xA060, 0x602A, 0x1A3C, 0x6F12,
	0x6207, 0x602A, 0x1A48, 0x6F12, 0x6207, 0x602A, 0x1444, 0x6F12, 0x2000,
	0x6F12, 0x2000, 0x602A, 0x144C, 0x6F12, 0x3F00, 0x6F12, 0x3F00, 0x602A,
	0x7F6C, 0x6F12, 0x0100, 0x6F12, 0x2F00, 0x6F12, 0xFA00, 0x6F12, 0x2400,
	0x6F12, 0xE500, 0x602A, 0x0650, 0x6F12, 0x0600, 0x602A, 0x0654, 0x6F12,
	0x0000, 0x602A, 0x1A46, 0x6F12, 0x8A00, 0x602A, 0x1A52, 0x6F12, 0xBF00,
	0x602A, 0x0674, 0x6F12, 0x0500, 0x6F12, 0x0500, 0x6F12, 0x0500, 0x6F12,
	0x0500, 0x602A, 0x0668, 0x6F12, 0x0800, 0x6F12, 0x0800, 0x6F12, 0x0800,
	0x6F12, 0x0800, 0x602A, 0x0684, 0x6F12, 0x4001, 0x602A, 0x0688, 0x6F12,
	0x4001, 0x602A, 0x147C, 0x6F12, 0x1000, 0x602A, 0x1480, 0x6F12, 0x1000,
	0x602A, 0x19F6, 0x6F12, 0x0904, 0x602A, 0x0812, 0x6F12, 0x0000, 0x602A,
	0x1A02, 0x6F12, 0x1800, 0x602A, 0x2148, 0x6F12, 0x0100, 0x602A, 0x2042,
	0x6F12, 0x1A00, 0x602A, 0x0874, 0x6F12, 0x0100, 0x602A, 0x09C0, 0x6F12,
	0x2008, 0x602A, 0x09C4, 0x6F12, 0x2000, 0x602A, 0x19FE, 0x6F12, 0x0E1C,
	0x602A, 0x4D92, 0x6F12, 0x0100, 0x602A, 0x84C8, 0x6F12, 0x0100, 0x602A,
	0x4D94, 0x6F12, 0x0005, 0x6F12, 0x000A, 0x6F12, 0x0010, 0x6F12, 0x0810,
	0x6F12, 0x000A, 0x6F12, 0x0040, 0x6F12, 0x0810, 0x6F12, 0x0810, 0x6F12,
	0x8002, 0x6F12, 0xFD03, 0x6F12, 0x0010, 0x6F12, 0x1510, 0x602A, 0x3570,
	0x6F12, 0x0000, 0x602A, 0x3574, 0x6F12, 0x1201, 0x602A, 0x21E4, 0x6F12,
	0x0400, 0x602A, 0x21EC, 0x6F12, 0x1F04, 0x602A, 0x2080, 0x6F12, 0x0101,
	0x6F12, 0xFF00, 0x6F12, 0x7F01, 0x6F12, 0x0001, 0x6F12, 0x8001, 0x6F12,
	0xD244, 0x6F12, 0xD244, 0x6F12, 0x14F4, 0x6F12, 0x0000, 0x6F12, 0x0000,
	0x6F12, 0x0000, 0x602A, 0x20BA, 0x6F12, 0x121C, 0x6F12, 0x111C, 0x6F12,
	0x54F4, 0x602A, 0x120E, 0x6F12, 0x1000, 0x602A, 0x212E, 0x6F12, 0x0200,
	0x602A, 0x13AE, 0x6F12, 0x0101, 0x602A, 0x0718, 0x6F12, 0x0001, 0x602A,
	0x0710, 0x6F12, 0x0002, 0x6F12, 0x0804, 0x6F12, 0x0100, 0x602A, 0x1B5C,
	0x6F12, 0x0000, 0x602A, 0x0786, 0x6F12, 0x7701, 0x602A, 0x2022, 0x6F12,
	0x0500, 0x6F12, 0x0500, 0x602A, 0x1360, 0x6F12, 0x0100, 0x602A, 0x1376,
	0x6F12, 0x0100, 0x6F12, 0x6038, 0x6F12, 0x7038, 0x6F12, 0x8038, 0x602A,
	0x1386, 0x6F12, 0x0B00, 0x602A, 0x06FA, 0x6F12, 0x0000, 0x602A, 0x4A94,
	0x6F12, 0x0900, 0x6F12, 0x0000, 0x6F12, 0x0300, 0x6F12, 0x0000, 0x6F12,
	0x0000, 0x6F12, 0x0000, 0x6F12, 0x0000, 0x6F12, 0x0000, 0x6F12, 0x0300,
	0x6F12, 0x0000, 0x6F12, 0x0900, 0x6F12, 0x0000, 0x6F12, 0x0000, 0x6F12,
	0x0000, 0x6F12, 0x0000, 0x6F12, 0x0000, 0x602A, 0x0A76, 0x6F12, 0x1000,
	0x602A, 0x0AEE, 0x6F12, 0x1000, 0x602A, 0x0B66, 0x6F12, 0x1000, 0x602A,
	0x0BDE, 0x6F12, 0x1000, 0x602A, 0x0BE8, 0x6F12, 0x3000, 0x6F12, 0x3000,
	0x602A, 0x0C56, 0x6F12, 0x1000, 0x602A, 0x0C60, 0x6F12, 0x3000, 0x6F12,
	0x3000, 0x602A, 0x0CB6, 0x6F12, 0x0100, 0x602A, 0x0CF2, 0x6F12, 0x0001,
	0x602A, 0x0CF0, 0x6F12, 0x0101, 0x602A, 0x11B8, 0x6F12, 0x0100, 0x602A,
	0x11F6, 0x6F12, 0x0020, 0x602A, 0x4A74, 0x6F12, 0x0000, 0x6F12, 0x0000,
	0x6F12, 0xD8FF, 0x6F12, 0x0000, 0x6F12, 0x0000, 0x6F12, 0x0000, 0x6F12,
	0x0000, 0x6F12, 0x0000, 0x6F12, 0xD8FF, 0x6F12, 0x0000, 0x6F12, 0x0000,
	0x6F12, 0x0000, 0x6F12, 0x0000, 0x6F12, 0x0000, 0x6F12, 0x0000, 0x6F12,
	0x0000, 0x602A, 0x218E, 0x6F12, 0x0000, 0x602A, 0x2268, 0x6F12, 0xF279,
	0x602A, 0x5006, 0x6F12, 0x0000, 0x602A, 0x500E, 0x6F12, 0x0100, 0x602A,
	0x4E70, 0x6F12, 0x2062, 0x6F12, 0x5501, 0x602A, 0x06DC, 0x6F12, 0x0000,
	0x6F12, 0x0000, 0x6F12, 0x0000, 0x6F12, 0x0000, 0x6028, 0x4000, 0xF46A,
	0xAE80, 0x0344, 0x0330, 0x0346, 0x04D4, 0x0348, 0x1CCF, 0x034A, 0x134B,
	0x034C, 0x0CC0, 0x034E, 0x072C, 0x0350, 0x0008, 0x0352, 0x0008, 0x0900,
	0x0122, 0x0380, 0x0002, 0x0382, 0x0002, 0x0384, 0x0002, 0x0386, 0x0002,
	0x0110, 0x1002, 0x0114, 0x0301, 0x0116, 0x3000, 0x0136, 0x1800, 0x013E,
	0x0000, 0x0300, 0x0006, 0x0302, 0x0001, 0x0304, 0x0004, 0x0306, 0x008C,
	0x0308, 0x0008, 0x030A, 0x0001, 0x030C, 0x0000, 0x030E, 0x0004, 0x0310,
	0x00C8, 0x0312, 0x0001, 0x080E, 0x0000, 0x0340, 0x12D0, 0x0342, 0x12E0,
	0x0702, 0x0000, 0x0202, 0x0100, 0x0200, 0x0100, 0x0D00, 0x0101, 0x0D02,
	0x0101, 0x0D04, 0x0102, 0x6226, 0x0000,
};
#endif

static void custom1_setting(void)
{
	LOG_INF("E\n");
	s5kjn1_multi_write_cmos_sensor(
		addr_data_pair_custom1_s5kjn1acc,
		sizeof(addr_data_pair_custom1_s5kjn1acc) / sizeof(kal_uint16));
}

#if MULTI_WRITE
kal_uint16 addr_data_pair_custom2_s5kjn1acc[] = {
0x6028, 0x2400,
0x602A, 0x1A28,
0x6F12, 0x4C00,
0x602A, 0x065A,
0x6F12, 0x0000,
0x602A, 0x139E,
0x6F12, 0x0100,
0x602A, 0x139C,
0x6F12, 0x0000,
0x602A, 0x13A0,
0x6F12, 0x0A00,
0x6F12, 0x0120,
0x602A, 0x2072,
0x6F12, 0x0000,
0x602A, 0x1A64,
0x6F12, 0x0301,
0x6F12, 0xFF00,
0x602A, 0x19E6,
0x6F12, 0x0200,
0x602A, 0x1A30,
0x6F12, 0x3401,
0x602A, 0x19FC,
0x6F12, 0x0B00,
0x602A, 0x19F4,
0x6F12, 0x0606,
0x602A, 0x19F8,
0x6F12, 0x1010,
0x602A, 0x1B26,
0x6F12, 0x6F80,
0x6F12, 0xA060,
0x602A, 0x1A3C,
0x6F12, 0x6207,
0x602A, 0x1A48,
0x6F12, 0x6207,
0x602A, 0x1444,
0x6F12, 0x2000,
0x6F12, 0x2000,
0x602A, 0x144C,
0x6F12, 0x3F00,
0x6F12, 0x3F00,
0x602A, 0x7F6C,
0x6F12, 0x0100,
0x6F12, 0x2F00,
0x6F12, 0xFA00,
0x6F12, 0x2400,
0x6F12, 0xE500,
0x602A, 0x0650,
0x6F12, 0x0600,
0x602A, 0x0654,
0x6F12, 0x0000,
0x602A, 0x1A46,
0x6F12, 0x8A00,
0x602A, 0x1A52,
0x6F12, 0xBF00,
0x602A, 0x0674,
0x6F12, 0x0500,
0x6F12, 0x0500,
0x6F12, 0x0500,
0x6F12, 0x0500,
0x602A, 0x0668,
0x6F12, 0x0800,
0x6F12, 0x0800,
0x6F12, 0x0800,
0x6F12, 0x0800,
0x602A, 0x0684,
0x6F12, 0x4001,
0x602A, 0x0688,
0x6F12, 0x4001,
0x602A, 0x147C,
0x6F12, 0x1000,
0x602A, 0x1480,
0x6F12, 0x1000,
0x602A, 0x19F6,
0x6F12, 0x0904,
0x602A, 0x0812,
0x6F12, 0x0000,
0x602A, 0x1A02,
0x6F12, 0x1800,
0x602A, 0x2148,
0x6F12, 0x0100,
0x602A, 0x2042,
0x6F12, 0x1A00,
0x602A, 0x0874,
0x6F12, 0x0100,
0x602A, 0x09C0,
0x6F12, 0x2008,
0x602A, 0x09C4,
0x6F12, 0x2000,
0x602A, 0x19FE,
0x6F12, 0x0E1C,
0x602A, 0x4D92,
0x6F12, 0x0100,
0x602A, 0x84C8,
0x6F12, 0x0100,
0x602A, 0x4D94,
0x6F12, 0x0005,
0x6F12, 0x000A,
0x6F12, 0x0010,
0x6F12, 0x0810,
0x6F12, 0x000A,
0x6F12, 0x0040,
0x6F12, 0x0810,
0x6F12, 0x0810,
0x6F12, 0x8002,
0x6F12, 0xFD03,
0x6F12, 0x0010,
0x6F12, 0x1510,
0x602A, 0x3570,
0x6F12, 0x0000,
0x602A, 0x3574,
0x6F12, 0x1201,
0x602A, 0x21E4,
0x6F12, 0x0400,
0x602A, 0x21EC,
0x6F12, 0x1F04,
0x602A, 0x2080,
0x6F12, 0x0101,
0x6F12, 0xFF00,
0x6F12, 0x7F01,
0x6F12, 0x0001,
0x6F12, 0x8001,
0x6F12, 0xD244,
0x6F12, 0xD244,
0x6F12, 0x14F4,
0x6F12, 0x0000,
0x6F12, 0x0000,
0x6F12, 0x0000,
0x602A, 0x20BA,
0x6F12, 0x121C,
0x6F12, 0x111C,
0x6F12, 0x54F4,
0x602A, 0x120E,
0x6F12, 0x1000,
0x602A, 0x212E,
0x6F12, 0x0200,
0x602A, 0x13AE,
0x6F12, 0x0101,
0x602A, 0x0718,
0x6F12, 0x0001,
0x602A, 0x0710,
0x6F12, 0x0002,
0x6F12, 0x0804,
0x6F12, 0x0100,
0x602A, 0x1B5C,
0x6F12, 0x0000,
0x602A, 0x0786,
0x6F12, 0x7701,
0x602A, 0x2022,
0x6F12, 0x0500,
0x6F12, 0x0500,
0x602A, 0x1360,
0x6F12, 0x0100,
0x602A, 0x1376,
0x6F12, 0x0100,
0x6F12, 0x6038,
0x6F12, 0x7038,
0x6F12, 0x8038,
0x602A, 0x1386,
0x6F12, 0x0B00,
0x602A, 0x06FA,
0x6F12, 0x0000,
0x602A, 0x4A94,
0x6F12, 0x0900,
0x6F12, 0x0000,
0x6F12, 0x0300,
0x6F12, 0x0000,
0x6F12, 0x0000,
0x6F12, 0x0000,
0x6F12, 0x0000,
0x6F12, 0x0000,
0x6F12, 0x0300,
0x6F12, 0x0000,
0x6F12, 0x0900,
0x6F12, 0x0000,
0x6F12, 0x0000,
0x6F12, 0x0000,
0x6F12, 0x0000,
0x6F12, 0x0000,
0x602A, 0x0A76,
0x6F12, 0x1000,
0x602A, 0x0AEE,
0x6F12, 0x1000,
0x602A, 0x0B66,
0x6F12, 0x1000,
0x602A, 0x0BDE,
0x6F12, 0x1000,
0x602A, 0x0BE8,
0x6F12, 0x3000,
0x6F12, 0x3000,
0x602A, 0x0C56,
0x6F12, 0x1000,
0x602A, 0x0C60,
0x6F12, 0x3000,
0x6F12, 0x3000,
0x602A, 0x0CB6,
0x6F12, 0x0100,
0x602A, 0x0CF2,
0x6F12, 0x0001,
0x602A, 0x0CF0,
0x6F12, 0x0101,
0x602A, 0x11B8,
0x6F12, 0x0100,
0x602A, 0x11F6,
0x6F12, 0x0020,
0x602A, 0x4A74,
0x6F12, 0x0000,
0x6F12, 0x0000,
0x6F12, 0xD8FF,
0x6F12, 0x0000,
0x6F12, 0x0000,
0x6F12, 0x0000,
0x6F12, 0x0000,
0x6F12, 0x0000,
0x6F12, 0xD8FF,
0x6F12, 0x0000,
0x6F12, 0x0000,
0x6F12, 0x0000,
0x6F12, 0x0000,
0x6F12, 0x0000,
0x6F12, 0x0000,
0x6F12, 0x0000,
0x602A, 0x218E,
0x6F12, 0x0000,
0x602A, 0x2268,
0x6F12, 0xF279,
0x602A, 0x5006,
0x6F12, 0x0000,
0x602A, 0x500E,
0x6F12, 0x0100,
0x602A, 0x4E70,
0x6F12, 0x2062,
0x6F12, 0x5501,
0x602A, 0x06DC,
0x6F12, 0x0000,
0x6F12, 0x0000,
0x6F12, 0x0000,
0x6F12, 0x0000,
0x6028, 0x4000,
0xF46A, 0xAE80,
0x0344, 0x0000,
0x0346, 0x0000,
0x0348, 0x1FFF,
0x034A, 0x181F,
0x034C, 0x0FF0,
0x034E, 0x0C00,
0x0350, 0x0008,
0x0352, 0x0008,
0x0900, 0x0122,
0x0380, 0x0002,
0x0382, 0x0002,
0x0384, 0x0002,
0x0386, 0x0002,
0x0110, 0x1002,
0x0114, 0x0301,
0x0116, 0x3000,
0x0136, 0x1800,
0x013E, 0x0000,
0x0300, 0x0006,
0x0302, 0x0001,
0x0304, 0x0004,
0x0306, 0x008C,
0x0308, 0x0008,
0x030A, 0x0001,
0x030C, 0x0000,
0x030E, 0x0004,
0x0310, 0x00C8,
0x0312, 0x0001,
0x080E, 0x0000,
0x0340, 0x0F6C,
0x0342, 0x1716,
0x0702, 0x0000,
0x0202, 0x0100,
0x0200, 0x0100,
0x0D00, 0x0101,
0x0D02, 0x0101,
0x0D04, 0x0102,
0x6226, 0x0000,
};
#endif

static void custom2_setting(void)
{
	LOG_INF("E\n");
	s5kjn1_multi_write_cmos_sensor(
		addr_data_pair_custom2_s5kjn1acc,
		sizeof(addr_data_pair_custom2_s5kjn1acc) / sizeof(kal_uint16));
}

#if MULTI_WRITE
kal_uint16 addr_data_pair_custom3_s5kjn1acc[] = {
	0x6028, 0x2400, 0x602A, 0x1A28, 0x6F12, 0x4C00, 0x602A, 0x065A, 0x6F12,
	0x0000, 0x602A, 0x139E, 0x6F12, 0x0100, 0x602A, 0x139C, 0x6F12, 0x0000,
	0x602A, 0x13A0, 0x6F12, 0x0A00, 0x6F12, 0x0120, 0x602A, 0x2072, 0x6F12,
	0x0000, 0x602A, 0x1A64, 0x6F12, 0x0301, 0x6F12, 0xFF00, 0x602A, 0x19E6,
	0x6F12, 0x0200, 0x602A, 0x1A30, 0x6F12, 0x3401, 0x602A, 0x19FC, 0x6F12,
	0x0B00, 0x602A, 0x19F4, 0x6F12, 0x0606, 0x602A, 0x19F8, 0x6F12, 0x1010,
	0x602A, 0x1B26, 0x6F12, 0x6F80, 0x6F12, 0xA060, 0x602A, 0x1A3C, 0x6F12,
	0x6207, 0x602A, 0x1A48, 0x6F12, 0x6207, 0x602A, 0x1444, 0x6F12, 0x2000,
	0x6F12, 0x2000, 0x602A, 0x144C, 0x6F12, 0x3F00, 0x6F12, 0x3F00, 0x602A,
	0x7F6C, 0x6F12, 0x0100, 0x6F12, 0x2F00, 0x6F12, 0xFA00, 0x6F12, 0x2400,
	0x6F12, 0xE500, 0x602A, 0x0650, 0x6F12, 0x0600, 0x602A, 0x0654, 0x6F12,
	0x0000, 0x602A, 0x1A46, 0x6F12, 0x8A00, 0x602A, 0x1A52, 0x6F12, 0xBF00,
	0x602A, 0x0674, 0x6F12, 0x0500, 0x6F12, 0x0500, 0x6F12, 0x0500, 0x6F12,
	0x0500, 0x602A, 0x0668, 0x6F12, 0x0800, 0x6F12, 0x0800, 0x6F12, 0x0800,
	0x6F12, 0x0800, 0x602A, 0x0684, 0x6F12, 0x4001, 0x602A, 0x0688, 0x6F12,
	0x4001, 0x602A, 0x147C, 0x6F12, 0x1000, 0x602A, 0x1480, 0x6F12, 0x1000,
	0x602A, 0x19F6, 0x6F12, 0x0904, 0x602A, 0x0812, 0x6F12, 0x0000, 0x602A,
	0x1A02, 0x6F12, 0x1800, 0x602A, 0x2148, 0x6F12, 0x0100, 0x602A, 0x2042,
	0x6F12, 0x1A00, 0x602A, 0x0874, 0x6F12, 0x0100, 0x602A, 0x09C0, 0x6F12,
	0x2008, 0x602A, 0x09C4, 0x6F12, 0x2000, 0x602A, 0x19FE, 0x6F12, 0x0E1C,
	0x602A, 0x4D92, 0x6F12, 0x0100, 0x602A, 0x84C8, 0x6F12, 0x0100, 0x602A,
	0x4D94, 0x6F12, 0x0005, 0x6F12, 0x000A, 0x6F12, 0x0010, 0x6F12, 0x0810,
	0x6F12, 0x000A, 0x6F12, 0x0040, 0x6F12, 0x0810, 0x6F12, 0x0810, 0x6F12,
	0x8002, 0x6F12, 0xFD03, 0x6F12, 0x0010, 0x6F12, 0x1510, 0x602A, 0x3570,
	0x6F12, 0x0000, 0x602A, 0x3574, 0x6F12, 0x0000, 0x602A, 0x21E4, 0x6F12,
	0x0400, 0x602A, 0x21EC, 0x6F12, 0xC000, 0x602A, 0x2080, 0x6F12, 0x0101,
	0x6F12, 0xFF00, 0x6F12, 0x7F01, 0x6F12, 0x0001, 0x6F12, 0x8001, 0x6F12,
	0xD244, 0x6F12, 0xD244, 0x6F12, 0x14F4, 0x6F12, 0x0000, 0x6F12, 0x0000,
	0x6F12, 0x0000, 0x602A, 0x20BA, 0x6F12, 0x121C, 0x6F12, 0x111C, 0x6F12,
	0x54F4, 0x602A, 0x120E, 0x6F12, 0x1000, 0x602A, 0x212E, 0x6F12, 0x0200,
	0x602A, 0x13AE, 0x6F12, 0x0101, 0x602A, 0x0718, 0x6F12, 0x0001, 0x602A,
	0x0710, 0x6F12, 0x0002, 0x6F12, 0x0804, 0x6F12, 0x0100, 0x602A, 0x1B5C,
	0x6F12, 0x0000, 0x602A, 0x0786, 0x6F12, 0x7701, 0x602A, 0x2022, 0x6F12,
	0x0500, 0x6F12, 0x0500, 0x602A, 0x1360, 0x6F12, 0x0100, 0x602A, 0x1376,
	0x6F12, 0x0100, 0x6F12, 0x6038, 0x6F12, 0x7038, 0x6F12, 0x8038, 0x602A,
	0x1386, 0x6F12, 0x0B00, 0x602A, 0x06FA, 0x6F12, 0x0000, 0x602A, 0x4A94,
	0x6F12, 0x0900, 0x6F12, 0x0000, 0x6F12, 0x0300, 0x6F12, 0x0000, 0x6F12,
	0x0000, 0x6F12, 0x0000, 0x6F12, 0x0000, 0x6F12, 0x0000, 0x6F12, 0x0300,
	0x6F12, 0x0000, 0x6F12, 0x0900, 0x6F12, 0x0000, 0x6F12, 0x0000, 0x6F12,
	0x0000, 0x6F12, 0x0000, 0x6F12, 0x0000, 0x602A, 0x0A76, 0x6F12, 0x1000,
	0x602A, 0x0AEE, 0x6F12, 0x1000, 0x602A, 0x0B66, 0x6F12, 0x1000, 0x602A,
	0x0BDE, 0x6F12, 0x1000, 0x602A, 0x0BE8, 0x6F12, 0x3000, 0x6F12, 0x3000,
	0x602A, 0x0C56, 0x6F12, 0x1000, 0x602A, 0x0C60, 0x6F12, 0x3000, 0x6F12,
	0x3000, 0x602A, 0x0CB6, 0x6F12, 0x0100, 0x602A, 0x0CF2, 0x6F12, 0x0001,
	0x602A, 0x0CF0, 0x6F12, 0x0101, 0x602A, 0x11B8, 0x6F12, 0x0100, 0x602A,
	0x11F6, 0x6F12, 0x0020, 0x602A, 0x4A74, 0x6F12, 0x0000, 0x6F12, 0x0000,
	0x6F12, 0xD8FF, 0x6F12, 0x0000, 0x6F12, 0x0000, 0x6F12, 0x0000, 0x6F12,
	0x0000, 0x6F12, 0x0000, 0x6F12, 0xD8FF, 0x6F12, 0x0000, 0x6F12, 0x0000,
	0x6F12, 0x0000, 0x6F12, 0x0000, 0x6F12, 0x0000, 0x6F12, 0x0000, 0x6F12,
	0x0000, 0x602A, 0x218E, 0x6F12, 0x0000, 0x602A, 0x2268, 0x6F12, 0xF279,
	0x602A, 0x5006, 0x6F12, 0x0000, 0x602A, 0x500E, 0x6F12, 0x0100, 0x602A,
	0x4E70, 0x6F12, 0x2062, 0x6F12, 0x5501, 0x602A, 0x06DC, 0x6F12, 0x0000,
	0x6F12, 0x0000, 0x6F12, 0x0000, 0x6F12, 0x0000, 0x6028, 0x4000, 0xF46A,
	0xAE80, 0x0344, 0x0000, 0x0346, 0x0308, 0x0348, 0x1FFF, 0x034A, 0x1517,
	0x034C, 0x0FF0, 0x034E, 0x08F8, 0x0350, 0x0008, 0x0352, 0x0008, 0x0900,
	0x0122, 0x0380, 0x0002, 0x0382, 0x0002, 0x0384, 0x0002, 0x0386, 0x0002,
	0x0110, 0x1002, 0x0114, 0x0301, 0x0116, 0x3000, 0x0136, 0x1800, 0x013E,
	0x0000, 0x0300, 0x0006, 0x0302, 0x0001, 0x0304, 0x0004, 0x0306, 0x008C,
	0x0308, 0x0008, 0x030A, 0x0001, 0x030C, 0x0000, 0x030E, 0x0004, 0x0310,
	0x00C8, 0x0312, 0x0001, 0x080E, 0x0000, 0x0340, 0x0C50, 0x0342, 0x1716,
	0x0702, 0x0000, 0x0202, 0x0100, 0x0200, 0x0100, 0x0D00, 0x0101, 0x0D02,
	0x0101, 0x0D04, 0x0102, 0x6226, 0x0000,

};
#endif

static void custom3_setting(void)
{
	LOG_INF("E\n");
	s5kjn1_multi_write_cmos_sensor(
		addr_data_pair_custom3_s5kjn1acc,
		sizeof(addr_data_pair_custom3_s5kjn1acc) / sizeof(kal_uint16));
}

#if MULTI_WRITE
kal_uint16 addr_data_pair_custom4_s5kjn1acc[] = {
0x6028, 0x2400,
0x602A, 0x1A28,
0x6F12, 0x4C00,
0x602A, 0x065A,
0x6F12, 0x0000,
0x602A, 0x139E,
0x6F12, 0x0400,
0x602A, 0x139C,
0x6F12, 0x0100,
0x602A, 0x13A0,
0x6F12, 0x0500,
0x6F12, 0x0120,
0x602A, 0x2072,
0x6F12, 0x0101,
0x602A, 0x1A64,
0x6F12, 0x0001,
0x6F12, 0x0000,
0x602A, 0x19E6,
0x6F12, 0x0200,
0x602A, 0x1A30,
0x6F12, 0x3403,
0x602A, 0x19FC,
0x6F12, 0x0700,
0x602A, 0x19F4,
0x6F12, 0x0707,
0x602A, 0x19F8,
0x6F12, 0x0B0B,
0x602A, 0x1B26,
0x6F12, 0x6F80,
0x6F12, 0xA060,
0x602A, 0x1A3C,
0x6F12, 0x8207,
0x602A, 0x1A48,
0x6F12, 0x8207,
0x602A, 0x1444,
0x6F12, 0x2000,
0x6F12, 0x2000,
0x602A, 0x144C,
0x6F12, 0x3F00,
0x6F12, 0x3F00,
0x602A, 0x7F6C,
0x6F12, 0x0100,
0x6F12, 0x2F00,
0x6F12, 0xFA00,
0x6F12, 0x2400,
0x6F12, 0xE500,
0x602A, 0x0650,
0x6F12, 0x0600,
0x602A, 0x0654,
0x6F12, 0x0000,
0x602A, 0x1A46,
0x6F12, 0x8500,
0x602A, 0x1A52,
0x6F12, 0x9800,
0x602A, 0x0674,
0x6F12, 0x0500,
0x6F12, 0x0500,
0x6F12, 0x0500,
0x6F12, 0x0500,
0x602A, 0x0668,
0x6F12, 0x0800,
0x6F12, 0x0800,
0x6F12, 0x0800,
0x6F12, 0x0800,
0x602A, 0x0684,
0x6F12, 0x4001,
0x602A, 0x0688,
0x6F12, 0x4001,
0x602A, 0x147C,
0x6F12, 0x0400,
0x602A, 0x1480,
0x6F12, 0x0400,
0x602A, 0x19F6,
0x6F12, 0x0404,
0x602A, 0x0812,
0x6F12, 0x0000,
0x602A, 0x1A02,
0x6F12, 0x1800,
0x602A, 0x2148,
0x6F12, 0x0100,
0x602A, 0x2042,
0x6F12, 0x1A00,
0x602A, 0x0874,
0x6F12, 0x0106,
0x602A, 0x09C0,
0x6F12, 0x4000,
0x602A, 0x09C4,
0x6F12, 0x4000,
0x602A, 0x19FE,
0x6F12, 0x0C1C,
0x602A, 0x4D92,
0x6F12, 0x0000,
0x602A, 0x84C8,
0x6F12, 0x0000,
0x602A, 0x4D94,
0x6F12, 0x0000,
0x6F12, 0x0000,
0x6F12, 0x0000,
0x6F12, 0x0000,
0x6F12, 0x0000,
0x6F12, 0x0000,
0x6F12, 0x0000,
0x6F12, 0x0000,
0x6F12, 0x0000,
0x6F12, 0x0000,
0x6F12, 0x0000,
0x6F12, 0x0000,
0x602A, 0x3570,
0x6F12, 0x0000,
0x602A, 0x3574,
0x6F12, 0x7306,
0x602A, 0x21E4,
0x6F12, 0x0400,
0x602A, 0x21EC,
0x6F12, 0x6902,
0x602A, 0x2080,
0x6F12, 0x0100,
0x6F12, 0xFF00,
0x6F12, 0x0002,
0x6F12, 0x0001,
0x6F12, 0x0002,
0x6F12, 0xD244,
0x6F12, 0xD244,
0x6F12, 0x14F4,
0x6F12, 0x101C,
0x6F12, 0x0D1C,
0x6F12, 0x54F4,
0x602A, 0x20BA,
0x6F12, 0x0000,
0x6F12, 0x0000,
0x6F12, 0x0000,
0x602A, 0x120E,
0x6F12, 0x1000,
0x602A, 0x212E,
0x6F12, 0x0200,
0x602A, 0x13AE,
0x6F12, 0x0100,
0x602A, 0x0718,
0x6F12, 0x0000,
0x602A, 0x0710,
0x6F12, 0x0010,
0x6F12, 0x0201,
0x6F12, 0x0800,
0x602A, 0x1B5C,
0x6F12, 0x0000,
0x602A, 0x0786,
0x6F12, 0x1401,
0x602A, 0x2022,
0x6F12, 0x0500,
0x6F12, 0x0500,
0x602A, 0x1360,
0x6F12, 0x0000,
0x602A, 0x1376,
0x6F12, 0x0000,
0x6F12, 0x6038,
0x6F12, 0x7038,
0x6F12, 0x8038,
0x602A, 0x1386,
0x6F12, 0x0B00,
0x602A, 0x06FA,
0x6F12, 0x1000,
0x602A, 0x4A94,
0x6F12, 0x0400,
0x6F12, 0x0400,
0x6F12, 0x0400,
0x6F12, 0x0400,
0x6F12, 0x0800,
0x6F12, 0x0800,
0x6F12, 0x0800,
0x6F12, 0x0800,
0x6F12, 0x0400,
0x6F12, 0x0400,
0x6F12, 0x0400,
0x6F12, 0x0400,
0x6F12, 0x0800,
0x6F12, 0x0800,
0x6F12, 0x0800,
0x6F12, 0x0800,
0x602A, 0x0A76,
0x6F12, 0x1000,
0x602A, 0x0AEE,
0x6F12, 0x1000,
0x602A, 0x0B66,
0x6F12, 0x1000,
0x602A, 0x0BDE,
0x6F12, 0x1000,
0x602A, 0x0BE8,
0x6F12, 0x5000,
0x6F12, 0x5000,
0x602A, 0x0C56,
0x6F12, 0x1000,
0x602A, 0x0C60,
0x6F12, 0x5000,
0x6F12, 0x5000,
0x602A, 0x0CB6,
0x6F12, 0x0000,
0x602A, 0x0CF2,
0x6F12, 0x0001,
0x602A, 0x0CF0,
0x6F12, 0x0101,
0x602A, 0x11B8,
0x6F12, 0x0000,
0x602A, 0x11F6,
0x6F12, 0x0010,
0x602A, 0x4A74,
0x6F12, 0x0000,
0x6F12, 0x0000,
0x6F12, 0x0000,
0x6F12, 0x0000,
0x6F12, 0x0000,
0x6F12, 0x0000,
0x6F12, 0x0000,
0x6F12, 0x0000,
0x6F12, 0x0000,
0x6F12, 0x0000,
0x6F12, 0x0000,
0x6F12, 0x0000,
0x6F12, 0x0000,
0x6F12, 0x0000,
0x6F12, 0x0000,
0x6F12, 0x0000,
0x602A, 0x218E,
0x6F12, 0x0000,
0x602A, 0x2268,
0x6F12, 0xF279,
0x602A, 0x5006,
0x6F12, 0x0000,
0x602A, 0x500E,
0x6F12, 0x0100,
0x602A, 0x4E70,
0x6F12, 0x2062,
0x6F12, 0x5501,
0x602A, 0x06DC,
0x6F12, 0x0000,
0x6F12, 0x0000,
0x6F12, 0x0000,
0x6F12, 0x0000,
0x6028, 0x4000,
0xF46A, 0xAE80,
0x0344, 0x0000,
0x0346, 0x0000,
0x0348, 0x1FFF,
0x034A, 0x181F,
0x034C, 0x1FE0,
0x034E, 0x1800,
0x0350, 0x0010,
0x0352, 0x0010,
0x0900, 0x0111,
0x0380, 0x0001,
0x0382, 0x0001,
0x0384, 0x0001,
0x0386, 0x0001,
0x0110, 0x1002,
0x0114, 0x0300,
0x0116, 0x3000,
0x0136, 0x1800,
0x013E, 0x0000,
0x0300, 0x0006,
0x0302, 0x0001,
0x0304, 0x0004,
0x0306, 0x008C,
0x0308, 0x0008,
0x030A, 0x0001,
0x030C, 0x0000,
0x030E, 0x0004,
0x0310, 0x0074,
0x0312, 0x0000,
0x080E, 0x0000,
0x0340, 0x1900,
0x0342, 0x21F0,
0x0702, 0x0000,
0x0202, 0x0100,
0x0200, 0x0100,
0x0D00, 0x0100,
0x0D02, 0x0001,
0x0D04, 0x0002,
0x6226, 0x0000,
};
#endif

static void custom4_setting(void)
{
	LOG_INF("E\n");
	s5kjn1_multi_write_cmos_sensor(
		addr_data_pair_custom4_s5kjn1acc,
		sizeof(addr_data_pair_custom4_s5kjn1acc) / sizeof(kal_uint16));
}

static kal_uint32 return_sensor_id(void)
{
	return ((read_cmos_sensor_8(0x0000) << 8) | read_cmos_sensor_8(0x0001));
}

/*static kal_uint16 get_vendor_id(void)
{
	kal_uint16 get_byte = 0;
	char pusendcmd[2] = { (char)(0x01 >> 8), (char)(0x01 & 0xFF) };

	iReadRegI2C(pusendcmd, 2, (u8 *) &get_byte, 1, 0xA2);

	return get_byte;
}
*/

/*************************************************************************
*FUNCTION
*  get_imgsensor_id
*
*DESCRIPTION
*  This function get the sensor ID
*
*PARAMETERS
*  *sensorID : return the sensor ID
*
*RETURNS
*  None
*
*GLOBALS AFFECTED
*
*************************************************************************/
/*hs14 code for AL6528ADEU-2675 by jianghongyan at 2022-11-20 start*/
extern int hbb_flag;

#define S5KJN1_EEPROM_VENDORID 0x01
static kal_uint32 get_imgsensor_id(UINT32 *sensor_id)
{
	kal_uint8 i = 0;
	kal_uint8 retry = 2;
	kal_uint32 s5kjn1VendorId = 0;
    spin_lock(&imgsensor_drv_lock);
    imgsensor.i2c_write_id = S5KJN1_EEPROM_SLAVE_ADDR;
    spin_unlock(&imgsensor_drv_lock);
    s5kjn1VendorId = read_cmos_sensor_8(0x0002);
	while (imgsensor_info.i2c_addr_table[i] != 0xff) {
		spin_lock(&imgsensor_drv_lock);
		imgsensor.i2c_write_id = imgsensor_info.i2c_addr_table[i];
		spin_unlock(&imgsensor_drv_lock);
		do {
			*sensor_id = return_sensor_id();
			LOG_INF("get_imgsensor_id  sensor_id: 0x%x\n",
				*sensor_id);
			if (*sensor_id == imgsensor_info.sensor_id) {
#if EEPROM_READY //stan
                if(s5kjn1VendorId == S5KJN1_EEPROM_VENDORID)
                {
                    S5KJN1_read_data_from_eeprom(S5KJN1_EEPROM_SLAVE_ADDR, 0x0000, S5KJN1_EEPROM_SIZE);
			        LOG_INF("[cameradebug-compatible] return vendorId=0x%x",s5kjn1VendorId);
                }
                else
                {
                    LOG_INF("[cameradebug-compatible] return vendorId error , vendorId=0x%x",s5kjn1VendorId);
			        *sensor_id = 0xFFFFFFFF;
                    return ERROR_SENSOR_CONNECT_FAIL;
                }
#endif
				pr_info("s5kjnsacc_ofilm i2c 0x%x, sid 0x%x\n",
					imgsensor.i2c_write_id, *sensor_id);
				return ERROR_NONE;
			} else {
				LOG_INF("check id fail i2c 0x%x, sid: 0x%x\n",
					imgsensor.i2c_write_id, *sensor_id);
				*sensor_id = 0xFFFFFFFF;
			}
			LOG_INF("Read fail, i2cid: 0x%x, Rsid: 0x%x, info.sid:0x%x.\n",
				imgsensor.i2c_write_id, *sensor_id,
				imgsensor_info.sensor_id);
			retry--;
		} while (retry > 0);
		i++;
		retry = 1;
	}
	if (*sensor_id != imgsensor_info.sensor_id) {
		*sensor_id = 0xFFFFFFFF;
		return ERROR_SENSOR_CONNECT_FAIL;
	}
	return ERROR_NONE;
}
/*hs14 code for AL6528ADEU-2675 by jianghongyan at 2022-11-20 end*/
/*************************************************************************
*FUNCTION
*  open
*
*DESCRIPTION
*  This function initialize the registers of CMOS sensor
*
*PARAMETERS
*  None
*
*RETURNS
*  None
*
*GLOBALS AFFECTED
*
*************************************************************************/
static kal_uint32 open(void)
{
	kal_uint8 i = 0;
	kal_uint8 retry = 2;
	kal_uint32 sensor_id = 0;

	while (imgsensor_info.i2c_addr_table[i] != 0xff) {
		spin_lock(&imgsensor_drv_lock);
		imgsensor.i2c_write_id = imgsensor_info.i2c_addr_table[i];
		spin_unlock(&imgsensor_drv_lock);
		do {
			sensor_id = return_sensor_id();
			LOG_INF("fdg open sensor_id: 0x%x, imgsensor_info.sensor_id \n",
				sensor_id);
			if (sensor_id == imgsensor_info.sensor_id) {
				LOG_INF("i2c write id : 0x%x, sensor id: 0x%x\n",
					imgsensor.i2c_write_id, sensor_id);
				break;
			}
			LOG_INF("Read sensor id fail , id: 0x%x, sensor id: 0x%x\n",
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
}

/*************************************************************************
*FUNCTION
*  close
*
*DESCRIPTION
*
*
*PARAMETERS
*  None
*
*RETURNS
*  None
*
*GLOBALS AFFECTED
*
*************************************************************************/
static kal_uint32 close(void)
{
	LOG_INF("E\n");

	return ERROR_NONE;
}

/*************************************************************************
*FUNCTION
*preview
*
*DESCRIPTION
*  This function start the sensor preview.
*
*PARAMETERS
*  *image_window : address pointer of pixel numbers in one period of HSYNC
**sensor_config_data : address pointer of line numbers in one period of VSYNC
*
*RETURNS
*  None
*
*GLOBALS AFFECTED
*
*************************************************************************/
static kal_uint32 preview(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
			  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("preview start\n");
	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_CAPTURE;
	imgsensor.pclk = imgsensor_info.pre.pclk;
	imgsensor.line_length = imgsensor_info.pre.linelength;
	imgsensor.frame_length = imgsensor_info.pre.framelength;
	imgsensor.min_frame_length = imgsensor_info.pre.framelength;

	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	preview_setting();
	set_mirror_flip(IMAGE_NORMAL);
	return ERROR_NONE;
}

/*************************************************************************
*FUNCTION
*  capture
*
*DESCRIPTION
*  This function setup the CMOS sensor in capture MY_OUTPUT mode
*
*PARAMETERS
*
*RETURNS
*  None
*
*GLOBALS AFFECTED
*
*************************************************************************/
static kal_uint32 capture(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
			  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	int i;

	LOG_INF("capture start\n");
	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_CAPTURE;
	if (imgsensor.current_fps == imgsensor_info.cap.max_framerate) {
		LOG_INF("capture30fps: use cap30FPS's setting: %d fps!\n",
			imgsensor.current_fps / 10);
		imgsensor.pclk = imgsensor_info.cap.pclk;
		imgsensor.line_length = imgsensor_info.cap.linelength;
		imgsensor.frame_length = imgsensor_info.cap.framelength;
		imgsensor.min_frame_length = imgsensor_info.cap.framelength;
		imgsensor.autoflicker_en = KAL_FALSE;
	} else if (imgsensor.current_fps == imgsensor_info.cap1.max_framerate) {
		LOG_INF("cap115fps: use cap1's setting: %d fps!\n",
			imgsensor.current_fps / 10);
		imgsensor.pclk = imgsensor_info.cap1.pclk;
		imgsensor.line_length = imgsensor_info.cap1.linelength;
		imgsensor.frame_length = imgsensor_info.cap1.framelength;
		imgsensor.min_frame_length = imgsensor_info.cap1.framelength;
		imgsensor.autoflicker_en = KAL_FALSE;
	} else {
		LOG_INF("Warning:current_fps %d is not support, use cap1\n",
			imgsensor.current_fps / 10);
		imgsensor.pclk = imgsensor_info.cap1.pclk;
		imgsensor.line_length = imgsensor_info.cap1.linelength;
		imgsensor.frame_length = imgsensor_info.cap1.framelength;
		imgsensor.min_frame_length = imgsensor_info.cap1.framelength;
		imgsensor.autoflicker_en = KAL_FALSE;
	}
	spin_unlock(&imgsensor_drv_lock);
	capture_setting(imgsensor.current_fps);
	set_mirror_flip(IMAGE_NORMAL);
	mdelay(10);

	for (i = 0; i < 10; i++) {
		LOG_INF("delay time = %d, the frame no = %d\n", i * 10,
			read_cmos_sensor(0x0005));
		mdelay(10);
	}

	return ERROR_NONE;
}

static kal_uint32 normal_video(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
			       MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("normal_video start\n");
	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_CAPTURE;
	imgsensor.pclk = imgsensor_info.normal_video.pclk;
	imgsensor.line_length = imgsensor_info.normal_video.linelength;
	imgsensor.frame_length = imgsensor_info.normal_video.framelength;
	imgsensor.min_frame_length = imgsensor_info.normal_video.framelength;

	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	normal_video_setting(imgsensor.current_fps);
	set_mirror_flip(IMAGE_NORMAL);
	return ERROR_NONE;
}

static kal_uint32 hs_video(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
			   MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("hs_video start\n");
	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_HIGH_SPEED_VIDEO;
	imgsensor.pclk = imgsensor_info.hs_video.pclk;

	imgsensor.line_length = imgsensor_info.hs_video.linelength;
	imgsensor.frame_length = imgsensor_info.hs_video.framelength;
	imgsensor.min_frame_length = imgsensor_info.hs_video.framelength;
	imgsensor.dummy_line = 0;
	imgsensor.dummy_pixel = 0;

	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	hs_video_setting();
	set_mirror_flip(IMAGE_NORMAL);
	return ERROR_NONE;
}

static kal_uint32 slim_video(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
			     MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("hs_video start\n");
	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_SLIM_VIDEO;
	imgsensor.pclk = imgsensor_info.slim_video.pclk;

	imgsensor.line_length = imgsensor_info.slim_video.linelength;
	imgsensor.frame_length = imgsensor_info.slim_video.framelength;
	imgsensor.min_frame_length = imgsensor_info.slim_video.framelength;
	imgsensor.dummy_line = 0;
	imgsensor.dummy_pixel = 0;

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

	imgsensor.line_length = imgsensor_info.custom1.linelength;
	imgsensor.frame_length = imgsensor_info.custom1.framelength;
	imgsensor.min_frame_length = imgsensor_info.custom1.framelength;
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

	imgsensor.line_length = imgsensor_info.custom2.linelength;
	imgsensor.frame_length = imgsensor_info.custom2.framelength;
	imgsensor.min_frame_length = imgsensor_info.custom2.framelength;
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

	imgsensor.line_length = imgsensor_info.custom3.linelength;
	imgsensor.frame_length = imgsensor_info.custom3.framelength;
	imgsensor.min_frame_length = imgsensor_info.custom3.framelength;
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

	imgsensor.line_length = imgsensor_info.custom4.linelength;
	imgsensor.frame_length = imgsensor_info.custom4.framelength;
	imgsensor.min_frame_length = imgsensor_info.custom4.framelength;
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	custom4_setting();
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
	return ERROR_NONE;
}

static kal_uint32 get_info(enum MSDK_SCENARIO_ID_ENUM scenario_id,
			   MSDK_SENSOR_INFO_STRUCT *sensor_info,
			   MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("scenario_id = %d\n", scenario_id);

	sensor_info->SensorClockPolarity = SENSOR_CLOCK_POLARITY_LOW;
	sensor_info->SensorClockFallingPolarity = SENSOR_CLOCK_POLARITY_LOW;
	sensor_info->SensorHsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
	sensor_info->SensorVsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
	sensor_info->SensorInterruptDelayLines = 4;
	sensor_info->SensorResetActiveHigh = FALSE;
	sensor_info->SensorResetDelayCount = 5;
	sensor_info->SensroInterfaceType = imgsensor_info.sensor_interface_type;
	sensor_info->MIPIsensorType = imgsensor_info.mipi_sensor_type;
	sensor_info->SettleDelayMode = imgsensor_info.mipi_settle_delay_mode;
	sensor_info->SensorOutputDataFormat =
		imgsensor_info.sensor_output_dataformat;
	sensor_info->CaptureDelayFrame = imgsensor_info.cap_delay_frame;
	sensor_info->PreviewDelayFrame = imgsensor_info.pre_delay_frame;
	sensor_info->VideoDelayFrame = imgsensor_info.video_delay_frame;
	sensor_info->HighSpeedVideoDelayFrame =
		imgsensor_info.hs_video_delay_frame;
	sensor_info->SlimVideoDelayFrame =
		imgsensor_info.slim_video_delay_frame;
	sensor_info->Custom1DelayFrame = imgsensor_info.custom1_delay_frame;
	sensor_info->Custom2DelayFrame = imgsensor_info.custom2_delay_frame;
	sensor_info->Custom3DelayFrame = imgsensor_info.custom3_delay_frame;
	sensor_info->Custom4DelayFrame = imgsensor_info.custom4_delay_frame;
	sensor_info->FrameTimeDelayFrame =
		imgsensor_info.frame_time_delay_frame;
	sensor_info->SensorMasterClockSwitch = 0;
	sensor_info->SensorDrivingCurrent = imgsensor_info.isp_driving_current;
	sensor_info->AEShutDelayFrame = imgsensor_info.ae_shut_delay_frame;
	sensor_info->AESensorGainDelayFrame =
		imgsensor_info.ae_sensor_gain_delay_frame;
	sensor_info->AEISPGainDelayFrame =
		imgsensor_info.ae_ispGain_delay_frame;
	sensor_info->FrameTimeDelayFrame =
		imgsensor_info.frame_time_delay_frame;
	sensor_info->IHDR_Support = imgsensor_info.ihdr_support;
	sensor_info->IHDR_LE_FirstLine = imgsensor_info.ihdr_le_firstline;
	sensor_info->SensorModeNum = imgsensor_info.sensor_mode_num;
	sensor_info->SensorMIPILaneNumber = imgsensor_info.mipi_lane_num;
	sensor_info->SensorClockFreq = imgsensor_info.mclk;
	sensor_info->SensorClockDividCount = 3;
	sensor_info->SensorClockRisingCount = 0;
	sensor_info->SensorClockFallingCount = 2;
	sensor_info->SensorPixelClockCount = 3;
	sensor_info->SensorDataLatchCount = 2;
	sensor_info->MIPIDataLowPwr2HighSpeedTermDelayCount = 0;
	sensor_info->MIPICLKLowPwr2HighSpeedTermDelayCount = 0;
	sensor_info->SensorWidthSampling = 0;
	sensor_info->SensorHightSampling = 0;
	sensor_info->SensorPacketECCOrder = 1;
#ifdef FPTPDAFSUPPORT
	sensor_info->PDAF_Support = 2;
#else
	sensor_info->PDAF_Support = 0;
#endif
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
	default:
		sensor_info->SensorGrabStartX = imgsensor_info.pre.startx;
		sensor_info->SensorGrabStartY = imgsensor_info.pre.starty;
		sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount =
			imgsensor_info.pre.mipi_data_lp2hs_settle_dc;
		break;
	}
	return ERROR_NONE;
}

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
	default:
		LOG_INF("Error ScenarioId setting");
		preview(image_window, sensor_config_data);
		return ERROR_INVALID_SCENARIO_ID;
	}
	return ERROR_NONE;
}

static kal_uint32 set_video_mode(UINT16 framerate)
{
	LOG_INF("framerate = %d\n ", framerate);

	if (framerate == 0)

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
	//LOG_INF("enable = %d, framerate = %d\n", enable, framerate);
	spin_lock(&imgsensor_drv_lock);
	if (enable)
		imgsensor.autoflicker_en = KAL_TRUE;
	else
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
		if (imgsensor.current_fps ==
		    imgsensor_info.cap1.max_framerate) {
			frame_length = imgsensor_info.cap1.pclk / framerate *
				       10 / imgsensor_info.cap1.linelength;
			spin_lock(&imgsensor_drv_lock);
			imgsensor.dummy_line =
				(frame_length >
				 imgsensor_info.cap1.framelength) ?
					(frame_length -
					 imgsensor_info.cap1.framelength) :
					0;
			imgsensor.frame_length =
				imgsensor_info.cap1.framelength +
				imgsensor.dummy_line;
			imgsensor.min_frame_length = imgsensor.frame_length;
			spin_unlock(&imgsensor_drv_lock);
		} else {
			if (imgsensor.current_fps !=
			    imgsensor_info.cap.max_framerate)
				LOG_INF("current_fps %d is not support, so use cap' %d fps!\n",
					framerate,
					imgsensor_info.cap.max_framerate / 10);
			frame_length = imgsensor_info.cap.pclk / framerate *
				       10 / imgsensor_info.cap.linelength;
			spin_lock(&imgsensor_drv_lock);
			imgsensor.dummy_line =
				(frame_length >
				 imgsensor_info.cap.framelength) ?
					(frame_length -
					 imgsensor_info.cap.framelength) :
					0;
			imgsensor.frame_length =
				imgsensor_info.cap.framelength +
				imgsensor.dummy_line;
			imgsensor.min_frame_length = imgsensor.frame_length;
			spin_unlock(&imgsensor_drv_lock);
		}
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
	default:
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
		LOG_INF("error scenario_id = %d, we use preview scenario\n",
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
	default:
		break;
	}
	return ERROR_NONE;
}

#if MULTI_WRITE
kal_uint16 addr_data_pair_set_test_pattern_s5kjn1acc[] = {
	0x0200, 0x0002, 0x0202, 0x0002, 0x0204, 0x0020, 0x020E, 0x0100,
	0x0210, 0x0100, 0x0212, 0x0100, 0x0214, 0x0100, 0x0600, 0x0002,
};
#endif

static kal_uint32 set_test_pattern_mode(kal_bool enable)
{
	LOG_INF("enable: %d\n", enable);
	if (enable) {
		s5kjn1_multi_write_cmos_sensor(
			addr_data_pair_set_test_pattern_s5kjn1acc,
			sizeof(addr_data_pair_set_test_pattern_s5kjn1acc) /
				sizeof(kal_uint16));
	} else {
		write_cmos_sensor(0x0600, 0x0000);
	}
	spin_lock(&imgsensor_drv_lock);
	imgsensor.test_pattern = enable;
	spin_unlock(&imgsensor_drv_lock);
	return ERROR_NONE;
}

static kal_uint32 feature_control(MSDK_SENSOR_FEATURE_ENUM feature_id,
				  UINT8 *feature_para, UINT32 *feature_para_len)
{
	UINT16 *feature_return_para_16 = (UINT16 *)feature_para;
	UINT16 *feature_data_16 = (UINT16 *)feature_para;
	UINT32 *feature_return_para_32 = (UINT32 *)feature_para;
	UINT32 *feature_data_32 = (UINT32 *)feature_para;
	unsigned long long *feature_data = (unsigned long long *)feature_para;
	UINT32 fps = 0;

	struct SET_PD_BLOCK_INFO_T *PDAFinfo;
	struct SENSOR_VC_INFO_STRUCT *pvcinfo;
	struct SENSOR_WINSIZE_INFO_STRUCT *wininfo;

	MSDK_SENSOR_REG_INFO_STRUCT *sensor_reg_data =
		(MSDK_SENSOR_REG_INFO_STRUCT *)feature_para;

	LOG_INF("feature_id = %d\n", feature_id);
	switch (feature_id) {
	case SENSOR_FEATURE_GET_GAIN_RANGE_BY_SCENARIO:
		*(feature_data + 1) = imgsensor_info.min_gain;
		*(feature_data + 2) = imgsensor_info.max_gain;
		break;
	case SENSOR_FEATURE_GET_BASE_GAIN_ISO_AND_STEP:
		*(feature_data + 0) = imgsensor_info.min_gain_iso;
		*(feature_data + 1) = imgsensor_info.gain_step;
		*(feature_data + 2) = imgsensor_info.gain_type;
		break;
	case SENSOR_FEATURE_GET_MIN_SHUTTER_BY_SCENARIO:
		*(feature_data + 1) = imgsensor_info.min_shutter;
		*(feature_data + 2) = imgsensor_info.exp_step;
		break;
	case SENSOR_FEATURE_GET_BINNING_TYPE:
		switch (*(feature_data + 1)) {
		case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
		case MSDK_SCENARIO_ID_SLIM_VIDEO:
			*feature_return_para_32 = 4; /*BINNING_AVERAGED*/
			break;
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG: 
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		case MSDK_SCENARIO_ID_CUSTOM1:
		case MSDK_SCENARIO_ID_CUSTOM2:
		case MSDK_SCENARIO_ID_CUSTOM3:
		case MSDK_SCENARIO_ID_CUSTOM4:
		default:
			*feature_return_para_32 = 1; /*BINNING_AVERAGED*/
			break;
		}
		LOG_INF(
			"SENSOR_FEATURE_GET_BINNING_TYPE AE_binning_type:%d,\n",
			*feature_return_para_32);
		*feature_para_len = 4;

		break;
	case SENSOR_FEATURE_GET_AWB_REQ_BY_SCENARIO:
		switch (*feature_data) {
		case MSDK_SCENARIO_ID_CUSTOM3:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) = 1;
			break;
		default:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) = 0;
			break;
		}
		break;
	case SENSOR_FEATURE_GET_PERIOD:
		*feature_return_para_16++ = imgsensor.line_length;
		*feature_return_para_16 = imgsensor.frame_length;
		*feature_para_len = 4;
		break;
	case SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ:
		LOG_INF("imgsensor.pclk = %d,current_fps = %d\n",
			 imgsensor.pclk, imgsensor.current_fps);
		*feature_return_para_32 = imgsensor.pclk;
		*feature_para_len = 4;
		break;
	case SENSOR_FEATURE_SET_ESHUTTER:
		set_shutter((kal_uint32)*feature_data);
		break;
	case SENSOR_FEATURE_SET_NIGHTMODE:

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
		/*get the lens driver ID from EEPROM or
		 *just return LENS_DRIVER_ID_DO_NOT_CARE
		 */

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

	case SENSOR_FEATURE_GET_TEST_PATTERN_CHECKSUM_VALUE:
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
		imgsensor.ihdr_mode = (UINT8)*feature_data_32;
		spin_unlock(&imgsensor_drv_lock);
		break;

	case SENSOR_FEATURE_GET_CROP_INFO:
		LOG_INF("SENSOR_FEATURE_GET_CROP_INFO scenarioId:%d\n",
			 (UINT32)*feature_data_32);

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
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		default:
			memcpy((void *)wininfo,
			       (void *)&imgsensor_winsize_info[0],
			       sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
			break;
		}
		break;
	case SENSOR_FEATURE_SET_IHDR_SHUTTER_GAIN:
		/*LOG_INF("SENSOR_SET_SENSOR_IHDR LE=%d, SE=%d, Gain=%d\n",
				 (UINT16) *feature_data,
				 (UINT16) *(feature_data + 1),
				 (UINT16) *(feature_data + 2));*/

		/*ihdr_write_shutter_gain((UINT16)*feature_data,
		 *(UINT16)*(feature_data+1),(UINT16)*(feature_data+2));
		 */
		break;
	case SENSOR_FEATURE_SET_AWB_GAIN:
		break;
	case SENSOR_FEATURE_SET_SHUTTER_FRAME_TIME:
		set_shutter_frame_length((UINT16) *feature_data, (UINT16) *(feature_data + 1));
		break;
	case SENSOR_FEATURE_GET_PERIOD_BY_SCENARIO:
		switch (*feature_data) {
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) =
				(imgsensor_info.cap.framelength << 16) +
				imgsensor_info.cap.linelength;
			break;
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) =
				(imgsensor_info.normal_video.framelength
				 << 16) +
				imgsensor_info.normal_video.linelength;
			break;
		case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) =
				(imgsensor_info.hs_video.framelength << 16) +
				imgsensor_info.hs_video.linelength;
			break;
		case MSDK_SCENARIO_ID_SLIM_VIDEO:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) =
				(imgsensor_info.slim_video.framelength << 16) +
				imgsensor_info.slim_video.linelength;
			break;
		case MSDK_SCENARIO_ID_CUSTOM1:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) =
				(imgsensor_info.custom1.framelength << 16) +
				imgsensor_info.custom1.linelength;
			break;
		case MSDK_SCENARIO_ID_CUSTOM2:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) =
				(imgsensor_info.custom2.framelength << 16) +
				imgsensor_info.custom2.linelength;
			break;
		case MSDK_SCENARIO_ID_CUSTOM3:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) =
				(imgsensor_info.custom3.framelength << 16) +
				imgsensor_info.custom3.linelength;
			break;
		case MSDK_SCENARIO_ID_CUSTOM4:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) =
				(imgsensor_info.custom4.framelength << 16) +
				imgsensor_info.custom4.linelength;
			break;
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		default:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) =
				(imgsensor_info.pre.framelength << 16) +
				imgsensor_info.pre.linelength;
			break;
		}
		break;
	case SENSOR_FEATURE_SET_HDR_SHUTTER:
		LOG_INF("SENSOR_FEATURE_SET_HDR_SHUTTER LE=%d, SE=%d\n",
			 (UINT16)*feature_data, (UINT16) * (feature_data + 1));
		/*ihdr_write_shutter(
		 *(UINT16)*feature_data,(UINT16)*(feature_data+1));
		 */
		break;

#if EEPROM_READY //stan
	case SENSOR_FEATURE_GET_4CELL_DATA:/*get 4 cell data from eeprom*/
	{
		int type = (kal_uint16)(*feature_data);
		char *data = (char *)(uintptr_t)(*(feature_data+1));
		//memset(data, 0, FOUR_CELL_SIZE + 2);
		pr_err("[cameradebug-remosaic]SENSOR_FEATURE_GET_4CELL_DATA type:%d",type);

		if (type == FOUR_CELL_CAL_TYPE_XTALK_CAL) {
			int buf_size = (kal_uint16)(*(feature_data +2));
			pr_err("[cameradebug-remosaic]buf_size:%d",buf_size);
			// if(buf_size < 8008){
			// 	break;
			// }
			read_4cell_from_eeprom(data);
			pr_err("[cameradebug-remosaic]Read Cross Talk = %02x %02x %02x %02x %02x %02x\n",
				(UINT16)data[0], (UINT16)data[1],
				(UINT16)data[2], (UINT16)data[3],
				(UINT16)data[4], (UINT16)data[5]);
		}
		break;
	}
#endif

	case SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ_BY_SCENARIO:
		switch (*feature_data) {
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) =
				imgsensor_info.cap.pclk;
			break;
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) =
				imgsensor_info.normal_video.pclk;
			break;
		case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) =
				imgsensor_info.hs_video.pclk;
			break;
		case MSDK_SCENARIO_ID_SLIM_VIDEO:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) =
				imgsensor_info.slim_video.pclk;
			break;
		case MSDK_SCENARIO_ID_CUSTOM1:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) =
				imgsensor_info.custom1.pclk;
			break;
		case MSDK_SCENARIO_ID_CUSTOM2:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) =
				imgsensor_info.custom2.pclk;
			break;
		case MSDK_SCENARIO_ID_CUSTOM3:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) =
				imgsensor_info.custom3.pclk;
			break;
		case MSDK_SCENARIO_ID_CUSTOM4:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) =
				imgsensor_info.custom4.pclk;
			break;
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		default:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) =
				imgsensor_info.pre.pclk;
			break;
		}
		break;

	case SENSOR_FEATURE_GET_PDAF_INFO:
		LOG_INF("SENSOR_FEATURE_GET_PDAF_INFO scenarioId:%d\n",
			 (UINT16)*feature_data);
		PDAFinfo = (struct SET_PD_BLOCK_INFO_T *)(uintptr_t)(
			*(feature_data + 1));
		switch (*feature_data) {
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		case MSDK_SCENARIO_ID_CUSTOM1:
			memcpy((void *)PDAFinfo, (void *)&imgsensor_pd_info,
			       sizeof(struct SET_PD_BLOCK_INFO_T));
			break;
		case MSDK_SCENARIO_ID_CUSTOM2:
			memcpy((void *)PDAFinfo, (void *)&imgsensor_pd_info,
			       sizeof(struct SET_PD_BLOCK_INFO_T));
			break;
		case MSDK_SCENARIO_ID_CUSTOM3:
			memcpy((void *)PDAFinfo, (void *)&imgsensor_pd_info_16_9,
			       sizeof(struct SET_PD_BLOCK_INFO_T));
			break;
		case MSDK_SCENARIO_ID_CUSTOM4:
			memcpy((void *)PDAFinfo, (void *)&imgsensor_pd_info,
			       sizeof(struct SET_PD_BLOCK_INFO_T));
			break;
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			memcpy((void *)PDAFinfo,
			       (void *)&imgsensor_pd_info,
			       sizeof(struct SET_PD_BLOCK_INFO_T));
			break;

		case MSDK_SCENARIO_ID_SLIM_VIDEO:
		case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
		default:
			break;
		}
		break;
	case SENSOR_FEATURE_GET_VC_INFO:
		LOG_INF("SENSOR_FEATURE_GET_VC_INFO %d\n",
			 (UINT16)*feature_data);
		pvcinfo = (struct SENSOR_VC_INFO_STRUCT *)(uintptr_t)(
			*(feature_data + 1));
		switch (*feature_data_32) {
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			memcpy((void *)pvcinfo, (void *)&SENSOR_VC_INFO[1],
			       sizeof(struct SENSOR_VC_INFO_STRUCT));
			break;
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			memcpy((void *)pvcinfo, (void *)&SENSOR_VC_INFO[2],
			       sizeof(struct SENSOR_VC_INFO_STRUCT));
			break;
		case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
			memcpy((void *)pvcinfo, (void *)&SENSOR_VC_INFO[3],
			       sizeof(struct SENSOR_VC_INFO_STRUCT));
			break;
		case MSDK_SCENARIO_ID_SLIM_VIDEO:
			memcpy((void *)pvcinfo, (void *)&SENSOR_VC_INFO[4],
			       sizeof(struct SENSOR_VC_INFO_STRUCT));
			break;
		case MSDK_SCENARIO_ID_CUSTOM1:
			memcpy((void *)pvcinfo, (void *)&SENSOR_VC_INFO[5],
			       sizeof(struct SENSOR_VC_INFO_STRUCT));
			break;
		case MSDK_SCENARIO_ID_CUSTOM2:
			memcpy((void *)pvcinfo, (void *)&SENSOR_VC_INFO[5],
			       sizeof(struct SENSOR_VC_INFO_STRUCT));
			break;
/*hs14 code for AL6528ADEU-1829 by liluling at 2022/11/2 begin*/
		case MSDK_SCENARIO_ID_CUSTOM3:
			memcpy((void *)pvcinfo, (void *)&SENSOR_VC_INFO[7],
			       sizeof(struct SENSOR_VC_INFO_STRUCT));
			break;
/*hs14 code for AL6528ADEU-1829 by liluling at 2022/11/2 end*/
		case MSDK_SCENARIO_ID_CUSTOM4:
			memcpy((void *)pvcinfo, (void *)&SENSOR_VC_INFO[5],
			       sizeof(struct SENSOR_VC_INFO_STRUCT));
			break;
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		default:
			memcpy((void *)pvcinfo, (void *)&SENSOR_VC_INFO[0],
			       sizeof(struct SENSOR_VC_INFO_STRUCT));
			break;
		}
		break;
	case SENSOR_FEATURE_GET_SENSOR_PDAF_CAPACITY:
		LOG_INF(
			"SENSOR_FEATURE_GET_SENSOR_PDAF_CAPACITY scenarioId:%d\n",
			(UINT16)*feature_data);

		switch (*feature_data) {
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		case MSDK_SCENARIO_ID_CUSTOM1:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) = 1;
			break;
		case MSDK_SCENARIO_ID_CUSTOM2:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) = 1;
			break;
		case MSDK_SCENARIO_ID_CUSTOM3:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) = 1;
			break;
		case MSDK_SCENARIO_ID_CUSTOM4:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) = 1;
			break;
		case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
		case MSDK_SCENARIO_ID_SLIM_VIDEO:
		default:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) = 0;
			break;
		}
		break;
	case SENSOR_FEATURE_SET_PDAF:
		LOG_INF("PDAF mode :%d\n", *feature_data_16);
		imgsensor.pdaf_mode = *feature_data_16;
		break;

	case SENSOR_FEATURE_SET_STREAMING_SUSPEND:
		LOG_INF("SENSOR_FEATURE_SET_STREAMING_SUSPEND feature_id=%d\n",feature_id);
		streaming_control(KAL_FALSE);
		break;
	case SENSOR_FEATURE_SET_STREAMING_RESUME:
		LOG_INF("SENSOR_FEATURE_SET_STREAMING_RESUME, shutter:%llu,feature_id=%d\n",
			 *feature_data,feature_id);
		if (*feature_data != 0)
			set_shutter(*feature_data);
		streaming_control(KAL_TRUE);
		break;
	case SENSOR_FEATURE_GET_PIXEL_RATE:
		switch (*feature_data) {
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) =
				(imgsensor_info.cap.pclk /
				 (imgsensor_info.cap.linelength - 80)) *
				imgsensor_info.cap.grabwindow_width;

			break;
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) =
				(imgsensor_info.normal_video.pclk /
				 (imgsensor_info.normal_video.linelength -
				  80)) *
				imgsensor_info.normal_video.grabwindow_width;

			break;
		case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) =
				(imgsensor_info.hs_video.pclk /
				 (imgsensor_info.hs_video.linelength - 80)) *
				imgsensor_info.hs_video.grabwindow_width;

			break;
		case MSDK_SCENARIO_ID_SLIM_VIDEO:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) =
				(imgsensor_info.slim_video.pclk /
				 (imgsensor_info.slim_video.linelength - 80)) *
				imgsensor_info.slim_video.grabwindow_width;

			break;
		case MSDK_SCENARIO_ID_CUSTOM1:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) =
				(imgsensor_info.custom1.pclk /
				 (imgsensor_info.custom1.linelength - 80)) *
				imgsensor_info.custom1.grabwindow_width;

			break;
		case MSDK_SCENARIO_ID_CUSTOM2:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) =
				(imgsensor_info.custom2.pclk /
				 (imgsensor_info.custom2.linelength - 80)) *
				imgsensor_info.custom2.grabwindow_width;

			break;
		case MSDK_SCENARIO_ID_CUSTOM3:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) =
				(imgsensor_info.custom3.pclk /
				 (imgsensor_info.custom3.linelength - 80)) *
				imgsensor_info.custom3.grabwindow_width;

			break;
		case MSDK_SCENARIO_ID_CUSTOM4:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) =
				(imgsensor_info.custom4.pclk /
				 (imgsensor_info.custom4.linelength - 80)) *
				imgsensor_info.custom4.grabwindow_width;

			break;
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		default:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) =
				(imgsensor_info.pre.pclk /
				 (imgsensor_info.pre.linelength - 80)) *
				imgsensor_info.pre.grabwindow_width;
			break;
		}
		break;

	case SENSOR_FEATURE_GET_MIPI_PIXEL_RATE:
		fps = (MUINT32)(*(feature_data + 2));

		switch (*feature_data) {
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) =
				imgsensor_info.cap.mipi_pixel_rate;
			break;
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) =
				imgsensor_info.normal_video.mipi_pixel_rate;
			break;
		case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) =
				imgsensor_info.hs_video.mipi_pixel_rate;
			break;
		case MSDK_SCENARIO_ID_SLIM_VIDEO:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) =
				imgsensor_info.slim_video.mipi_pixel_rate;
			break;
		case MSDK_SCENARIO_ID_CUSTOM1:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) =
				imgsensor_info.custom1.mipi_pixel_rate;
			break;
		case MSDK_SCENARIO_ID_CUSTOM2:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) =
				imgsensor_info.custom2.mipi_pixel_rate;
			break;
		case MSDK_SCENARIO_ID_CUSTOM3:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) =
				imgsensor_info.custom3.mipi_pixel_rate;
			break;
		case MSDK_SCENARIO_ID_CUSTOM4:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) =
				imgsensor_info.custom4.mipi_pixel_rate;
			break;
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		default:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) =
				imgsensor_info.pre.mipi_pixel_rate;
			break;
		}

		break;

		//cxc long exposure >
	case SENSOR_FEATURE_GET_AE_EFFECTIVE_FRAME_FOR_LE:
		*feature_return_para_32 = imgsensor.current_ae_effective_frame;
		break;
	case SENSOR_FEATURE_GET_AE_FRAME_MODE_FOR_LE:
		memcpy(feature_return_para_32, &imgsensor.ae_frm_mode,
		       sizeof(struct IMGSENSOR_AE_FRM_MODE));
		break;
		//cxc long exposure <

	default:
		break;
	}
	return ERROR_NONE;
}
/*hs14 code for SR-AL6528A-01-88 by pengxutao at 2022-11-1 end*/

static struct SENSOR_FUNCTION_STRUCT sensor_func = {
	open, get_info, get_resolution, feature_control, control, close
};

UINT32
A1401BACKS5KJN1HLT_MIPI_RAW_SensorInit(struct SENSOR_FUNCTION_STRUCT **pfFunc)
{
	LOG_INF("A1401BACKS5KJN1HLT_MIPI_RAW_SensorInit in  fufu\n");
	/* To Do : Check Sensor status here */
	if (pfFunc != NULL)
		*pfFunc = &sensor_func;
	return ERROR_NONE;
} /*	S5KJN1_MIPI_RAW_SensorInit	*/
/*hs14 code for SR-AL6528A-01-60 by jianghongyan at 2022-11-08 end*/
/*hs14 code for AL6528ADEU-1011 by majunfeng at 2022-11-2 end*/
