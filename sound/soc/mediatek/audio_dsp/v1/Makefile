#
# Copyright (C) 2015 MediaTek Inc.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#

# MTK Platform Support
MTK_PLATFORM := $(subst ",,$(CONFIG_MTK_PLATFORM))

# adsp include path
ifeq ($(CONFIG_SND_SOC_MTK_AUDIO_DSP),y)
subdir-ccflags-y += -I$(srctree)/sound/soc/mediatek/audio_dsp/$(MTK_PLATFORM)
subdir-ccflags-y += -I$(srctree)/drivers/misc/mediatek/audio_ipi/common_v1/framework
subdir-ccflags-y += -I$(srctree)/sound/soc/mediatek/common
subdir-ccflags-y += -I$(srctree)/sound/soc/mediatek/$(MTK_PLATFORM)/
endif

# adsp driver path
ifeq ($(CONFIG_MTK_AUDIODSP_SUPPORT),y)
subdir-ccflags-y += -I$(srctree)/drivers/misc/mediatek/adsp/$(MTK_PLATFORM)
endif

ifeq ($(CONFIG_MTK_TINYSYS_SCP_SUPPORT),y)
subdir-ccflags-y += -I$(srctree)/drivers/misc/mediatek/scp/$(MTK_PLATFORM)
endif

# platform code
obj-y += ../$(MTK_PLATFORM)/

# spm
ccflags-y += -I$(srctree)/drivers/misc/mediatek/base/power/include

# MENU Governor Predict(low power)
ifeq ($(CONFIG_MTK_ACAO_SUPPORT),y)
ccflags-y += -I$(srctree)/drivers/misc/mediatek/base/power/mcdi/include/
endif

# common code
obj-$(CONFIG_SND_SOC_MTK_AUDIO_DSP) += audio_buf.o
obj-$(CONFIG_SND_SOC_MTK_AUDIO_DSP) += mtk-dsp-dai.o mtk-dsp-pcm.o mtk-dsp-common.o mtk-dsp-platform-driver.o mtk-dsp-mem-control.o
obj-$(CONFIG_MTK_AUDIODSP_SUPPORT) += mtk-dsp-core.o
