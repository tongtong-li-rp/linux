/* Copyright (c) 2019,2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2023 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef _CAM_IR_LED_DEV_H_
#define _CAM_IR_LED_DEV_H_

#include <linux/leds.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <media/v4l2-subdev.h>
#include <media/v4l2-ioctl.h>
#include <media/v4l2-event.h>
#include <media/cam_sensor.h>
#include <media/cam_req_mgr.h>
#include "cam_req_mgr_util.h"
#include "cam_req_mgr_interface.h"
#include "cam_subdev.h"
#include "cam_mem_mgr.h"
#include "cam_sensor_cmn_header.h"
#include "cam_soc_util.h"
#include "cam_debug_util.h"
#include "cam_sensor_io.h"

/* msleep value as per specs from older SPs
 will be updated as per new HW specs, as and when support is provided for current SP */
#define CAM_IR_MSLEEP_VALUE 100

#define CAMX_IR_LED_DEV_NAME "cam-ir-led-dev"
#define CAM_IR_LED_PIPELINE_DELAY 1
#define CAM_IR_LED_PACKET_OPCODE_OFF 0
#define CAM_IR_LED_PACKET_OPCODE_ON  1
#define CAM_IR_CUT_PACKET_OPCODE_ON  3
#define CAM_IR_CUT_PACKET_OPCODE_OFF 4

enum cam_ir_led_switch_trigger_ops {
	IR_LED_SWITCH_OFF = 0,
	IR_LED_SWITCH_ON,
};

enum cam_ir_led_driver_type {
	IR_LED_DRIVER_GPIO,
	IR_LED_DRIVER_PMIC,
	IR_LED_DRIVER_I2C,
	IR_LED_DRIVER_DEFAULT,
};

enum cam_ir_led_state {
	CAM_IR_LED_STATE_INIT = 0,
	CAM_IR_LED_STATE_ACQUIRE,
	CAM_IR_LED_STATE_START,
	CAM_IR_LED_STATE_ON,
	CAM_IR_LED_STATE_OFF,
};

/**
 * struct cam_ir_led_intf_params
 * @device_hdl   : Device Handle
 * @session_hdl  : Session Handle
 * @link_hdl     : Link Handle
 * @ops          : KMD operations
 * @crm_cb       : Callback API pointers
 */
struct cam_ir_led_intf_params {
	int32_t                     device_hdl;
	int32_t                     session_hdl;
	int32_t                     link_hdl;
	struct cam_req_mgr_kmd_ops  ops;
	struct cam_req_mgr_crm_cb  *crm_cb;
};

/**
 * struct cam_ir_led_common_attr
 * @is_settings_valid  : Notify the valid settings
 * @request_id         : Request id provided by umd
 * @count              : Number of led count
 * @cmd_type           : Command buffer type
 */
struct cam_ir_led_common_attr {
	bool      is_settings_valid;
	uint64_t  request_id;
	uint16_t  count;
	uint8_t   cmd_type;
};

/**
 * struct ir_led_init_packet
 * @cmn_attr   : Provides common attributes
 * @ir_led_type : Ir_led type(PMIC/I2C/GPIO)
 */
struct cam_ir_led_init_packet {
	struct cam_ir_led_common_attr cmn_attr;
	uint8_t                       ir_led_type;
};

/**
 *  struct cam_ir_led_private_soc
 * @switch_trigger_name : Switch trigger name
 * @ir_led_trigger_name  : Ir_led trigger name array
 * @ir_led_op_current    : Ir_led operational current
 * @ir_led_max_current   : Max supported current for LED in ir_led mode
 * @ir_led_max_duration  : Max turn on duration for LED in Ir_led mode
 * @torch_trigger_name  : Torch trigger name array
 * @torch_op_current    : Torch operational current
 * @torch_max_current   : Max supported current for LED in torch mode
 */

struct cam_ir_led_private_soc {
	const char   *switch_trigger_name;
	const char   *ir_led_trigger_name;
	uint32_t     ir_led_op_current;
	uint32_t     ir_led_max_current;
	uint32_t     ir_led_max_duration;
	const char   *torch_trigger_name;
	uint32_t     torch_op_current;
	uint32_t     torch_max_current;
};

/**
 *  struct cam_ir_led_ctrl
 * @soc_info            : Soc related information
 * @pdev                : Platform device
 * @pwm_dev             : PWM device handle
 * @func_tbl            : structure of h/w specific function pointers
 * @of_node             : Of Node ptr
 * @v4l2_dev_str        : V4L2 device structure
 * @ir_led_mutex        : Mutex for ir_led operations
 * @ir_led_state        : Current ir_led state (INIT/ACQUIRE/START/ON/OFF)
 * @device_hdl          : Device Handle
 * @ir_led_driver_type  : ir_led driver type (GPIO/PWM)
 * @io_master_info      : Information about the communication master
 */
struct cam_ir_led_ctrl {
	struct cam_hw_soc_info      soc_info;
	struct platform_device      *pdev;
	struct pwm_device           *pwm_dev;
	struct cam_ir_led_func      *func_tbl;
	struct device_node          *of_node;
	struct cam_subdev           v4l2_dev_str;
	struct mutex                ir_led_mutex;
	enum   cam_ir_led_state     ir_led_state;
	int32_t                     device_hdl;
	enum cam_ir_led_driver_type ir_led_driver_type;
	struct camera_io_master     io_master_info;
};

struct cam_ir_led_func {
	int32_t (*camera_ir_led_init)(struct cam_ir_led_ctrl *);
	int32_t (*camera_ir_led_release)(struct cam_ir_led_ctrl *);
	int32_t (*camera_ir_led_off)(struct cam_ir_led_ctrl *);
	int32_t (*camera_ir_led_on)(struct cam_ir_led_ctrl *,
		struct cam_ir_led_set_on_off *);
	int32_t (*camera_ir_cut_off)(struct cam_ir_led_ctrl *);
	int32_t (*camera_ir_cut_on)(struct cam_ir_led_ctrl *,
		struct cam_ir_led_set_on_off *);
};

struct cam_ir_led_table {
	enum cam_ir_led_driver_type ir_led_driver_type;
	struct cam_ir_led_func func_tbl;
};

/**
 * @brief : API to register IR LED hw to platform framework.
 * @return struct platform_device pointer on on success, or ERR_PTR() on error.
 */
int cam_ir_led_init_module(void);

/**
 * @brief : API to remove IR LED Hw from platform framework.
 */
void cam_ir_led_exit_module(void);

#endif /*_CAM_IR_LED_DEV_H_*/
