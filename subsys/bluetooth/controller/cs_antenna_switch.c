/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <stdint.h>
#include "cs_antenna_switch.h"

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <soc.h>
#include <nrfx_gpiote.h>
#include <gpiote_nrfx.h>
#include <helpers/nrfx_gppi.h>

#if DT_NODE_EXISTS(DT_NODELABEL(cs_antenna_switch))
#define ANTENNA_SWITCH_NODE DT_NODELABEL(cs_antenna_switch)
#else
#define ANTENNA_SWITCH_NODE DT_INVALID_NODE
#error No channel sounding antenna switch nodes registered in DTS.
#endif

#if !DT_NODE_HAS_COMPAT(ANTENNA_SWITCH_NODE, nordic_bt_cs_antenna_switch)
#error Configured antenna switch node is not compatible.
#endif

#if DT_NODE_HAS_PROP(ANTENNA_SWITCH_NODE, multiplexing_mode)
#define MULTIPLEXED DT_PROP(ANTENNA_SWITCH_NODE, multiplexing_mode)
#else
#define MULTIPLEXED false
#endif

#define ANTENNA_NOT_SET 0xFF

BUILD_ASSERT(DT_NODE_HAS_PROP(ANTENNA_SWITCH_NODE, ant_gpios));

#define NUM_GPIOS DT_PROP_LEN(ANTENNA_SWITCH_NODE, ant_gpios)

// TBD how to set this
// #define NUM_GPIOTE_CHANNELS 2
uint8_t gpiote_channel_table[NUM_GPIOS];

#if MULTIPLEXED
#if CONFIG_BT_CTLR_SDC_CS_NUM_ANTENNAS == 2
BUILD_ASSERT(NUM_GPIOS >= 1);
#else
BUILD_ASSERT(NUM_GPIOS >= 2);
#endif
#else
BUILD_ASSERT(NUM_GPIOS >= CONFIG_BT_CTLR_SDC_CS_NUM_ANTENNAS);
#endif /* MULTIPLEXED */

static uint8_t currently_active_antenna = ANTENNA_NOT_SET;
static const struct gpio_dt_spec gpio_dt_spec_table[] = {
	GPIO_DT_SPEC_GET_BY_IDX_OR(ANTENNA_SWITCH_NODE, ant_gpios, 0, {0}),
	GPIO_DT_SPEC_GET_BY_IDX_OR(ANTENNA_SWITCH_NODE, ant_gpios, 1, {0}),
#if !MULTIPLEXED
	GPIO_DT_SPEC_GET_BY_IDX_OR(ANTENNA_SWITCH_NODE, ant_gpios, 2, {0}),
	GPIO_DT_SPEC_GET_BY_IDX_OR(ANTENNA_SWITCH_NODE, ant_gpios, 3, {0}),
#endif
};

static const uint32_t gpiote_psel_table[] = {
	NRF_DT_GPIOS_TO_PSEL_BY_IDX(ANTENNA_SWITCH_NODE, ant_gpios, 0),
	NRF_DT_GPIOS_TO_PSEL_BY_IDX(ANTENNA_SWITCH_NODE, ant_gpios, 1),
#if !MULTIPLEXED
	NRF_DT_GPIOS_TO_PSEL_BY_IDX(ANTENNA_SWITCH_NODE, ant_gpios, 2),
	NRF_DT_GPIOS_TO_PSEL_BY_IDX(ANTENNA_SWITCH_NODE, ant_gpios, 3),
#endif
};

/* Antenna control below is implemented as described in the CS documentation.
 *
 * Example valid device tree configuration for the nRF54L15:
 *
 * / {
 *   cs_antenna_switch: cs-antenna-config {
 *     status = "okay";
 *     compatible = "nordic,bt-cs-antenna-switch";
 *     ant-gpios = <&gpio1 11 (GPIO_ACTIVE_HIGH)>,
 *                 <&gpio1 12 (GPIO_ACTIVE_HIGH)>,
 *                 <&gpio1 13 (GPIO_ACTIVE_HIGH)>,
 *                 <&gpio1 14 (GPIO_ACTIVE_HIGH)>;
 *     multiplexing-mode = <0>;
 *   };
 * };
 *
 */
void cs_antenna_switch_func(uint8_t antenna_number)
{
	(void) antenna_number;
	nrfx_gpiote_t *ant_gpiote =
		&GPIOTE_NRFX_INST_BY_NODE(NRF_DT_GPIOTE_NODE(ANTENNA_SWITCH_NODE, ant_gpios));

	nrfx_gppi_ep_clear(nrfx_gpiote_set_task_address_get(ant_gpiote, gpiote_psel_table[2]));
	nrfx_gppi_ep_clear(nrfx_gpiote_clr_task_address_get(ant_gpiote, gpiote_psel_table[3]));

	const uint32_t ppi_channel = 6;
	nrfx_gppi_ep_to_ch_attach(nrfx_gpiote_set_task_address_get(ant_gpiote, gpiote_psel_table[0]),
                            ppi_channel);

	nrfx_gppi_ep_to_ch_attach(nrfx_gpiote_clr_task_address_get(ant_gpiote, gpiote_psel_table[1]),
                            ppi_channel);

// 	int err;
// #if DT_NODE_HAS_PROP(DT_NODELABEL(cs_antenna_switch), ant_gpios)
// 	nrfx_gpiote_t *ant_gpiote =
// 		&GPIOTE_NRFX_INST_BY_NODE(NRF_DT_GPIOTE_NODE(ANTENNA_SWITCH_NODE, ant_gpios));
// 	for (uint8_t i = 0; i < NUM_GPIOTE_CHANNELS; i++) {
// 		const nrfx_gpiote_output_config_t gpiote_output_cfg = NRFX_GPIOTE_DEFAULT_OUTPUT_CONFIG;
// 		const nrfx_gpiote_task_config_t task_cfg_ant_switch = {
// 			.task_ch = gpiote_channel_table[i],
// 			.polarity = NRF_GPIOTE_POLARITY_TOGGLE,
// 			.init_val = NRF_GPIOTE_INITIAL_VALUE_LOW,
// 		};
// 		err = nrfx_gpiote_output_configure(ant_gpiote, gpio_dt_spec_table[0].pin, &gpiote_output_cfg, &task_cfg_ant_switch);
// 		__ASSERT(err == 0, "Failed to configure GPIOTE channel for CS (%d)", err);
// 	}
// #endif

// 	int err;
// #if MULTIPLEXED
// 	err = gpio_pin_set_dt(&gpio_dt_spec_table[0], antenna_number & (1 << 0));
// 	__ASSERT_NO_MSG(err == 0);

// #if NUM_GPIOS > 1
// 	err = gpio_pin_set_dt(&gpio_dt_spec_table[1], antenna_number & (1 << 1));
// 	__ASSERT_NO_MSG(err == 0);
// #endif
// #else
// 	if (currently_active_antenna != antenna_number) {
// 		if (currently_active_antenna != ANTENNA_NOT_SET) {
// 			err = gpio_pin_set_dt(&gpio_dt_spec_table[currently_active_antenna], false);
// 			__ASSERT_NO_MSG(err == 0);
// 		}

// 		err = gpio_pin_set_dt(&gpio_dt_spec_table[antenna_number], true);
// 		__ASSERT_NO_MSG(err == 0);
// 	}

// 	currently_active_antenna = antenna_number;
// #endif
}

void cs_antenna_switch_init(void)
{
	int err;
#if DT_NODE_HAS_PROP(DT_NODELABEL(cs_antenna_switch), ant_gpios)
	nrfx_gpiote_t *ant_gpiote =
		&GPIOTE_NRFX_INST_BY_NODE(NRF_DT_GPIOTE_NODE(ANTENNA_SWITCH_NODE, ant_gpios));
	for (uint8_t i = 0; i < NUM_GPIOS; i++) {
		err = nrfx_gpiote_channel_alloc(ant_gpiote, &gpiote_channel_table[i]);
			__ASSERT(err == 0, "Failed to allocate GPIOTE channel for CS (%d)", err);
		}
#endif

	for (uint8_t i = 0; i < NUM_GPIOS; i++) {
		err = gpio_pin_configure_dt(&gpio_dt_spec_table[i], GPIO_OUTPUT_INACTIVE);
		__ASSERT(err == 0, "Failed to initialize GPIOs for CS (%d)", err);
	}

#if DT_NODE_HAS_PROP(DT_NODELABEL(cs_antenna_switch), ant_gpios)
	for (uint8_t i = 0; i < NUM_GPIOS; i++) {
		const nrfx_gpiote_output_config_t gpiote_output_cfg = NRFX_GPIOTE_DEFAULT_OUTPUT_CONFIG;
		const nrfx_gpiote_task_config_t task_cfg_ant_switch = {
			.task_ch = gpiote_channel_table[i],
			.polarity = NRF_GPIOTE_POLARITY_TOGGLE,
			.init_val = NRF_GPIOTE_INITIAL_VALUE_LOW,
		};
		err = nrfx_gpiote_output_configure(ant_gpiote, gpiote_psel_table[i], &gpiote_output_cfg, &task_cfg_ant_switch);
		__ASSERT(err == 0, "Failed to configure GPIOTE channel for CS (%d)", err);
	}
#endif
}

void cs_antenna_switch_clear(void)
{
	int err;

	for (uint8_t i = 0; i < NUM_GPIOS; i++) {
		err = gpio_pin_set_dt(&gpio_dt_spec_table[i], false);
		__ASSERT_NO_MSG(err == 0);
	}

	currently_active_antenna = ANTENNA_NOT_SET;
}
