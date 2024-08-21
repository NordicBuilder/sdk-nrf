/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>

#include <nrf_rpc.h>

#include "entropy_ser.h"

#include "net_core_monitor.h"
#include <helpers/nrfx_reset_reason.h>
#include <zephyr/sys/reboot.h>

#define BUFFER_LENGTH 10

static uint8_t buffer[BUFFER_LENGTH];

static void result_callback(int result, uint8_t *buffer, size_t length)
{
	size_t i;

	if (result) {
		printk("Entropy remote get failed: %d\n", result);
		return;
	}

	for (i = 0; i < length; i++) {
		printk("  0x%02x", buffer[i]);
	}

	printk("\n");
}

/* This is the override for the __weak handler. */
void ncm_net_core_event_handler(enum ncm_event_type event, uint32_t reset_reas)
{
	switch (event) {
	case NCM_EVT_NET_CORE_RESET:
		printk("The network core reset\n");
		if (reset_reas & NRF_RESET_RESETREAS_RESETPIN_MASK) {
			printk("Reset by pin-reset\n");
		} else if (reset_reas & NRF_RESET_RESETREAS_DOG0_MASK) {
			printk("Reset by application watchdog timer 0\n");
		} else if (reset_reas & NRF_RESET_RESETREAS_SREQ_MASK) {
			printk("Reset by soft-reset\n");
		} else if (reset_reas) {
			printk("Reset by a different source (0x%08X)\n", reset_reas);
			printk("SoC reboot cold in 2 seconds\n");
			k_sleep(K_SECONDS(2));
			sys_reboot(SYS_REBOOT_COLD);
		}
		break;
	case NCM_EVT_NET_CORE_FREEZE:
		printk("The network core is not responding.\n");
		break;
	}
}

int main(void)
{
	int err;

	printk("Entropy sample started[APP Core].\n");

	err = entropy_remote_init();
	if (err) {
		printk("Remote entropy driver initialization failed\n");
		return 0;
	}

	printk("Remote init send\n");

	while (true) {
		k_sleep(K_MSEC(2000));

		err = entropy_remote_get(buffer, sizeof(buffer));
		if (err) {
			printk("Entropy remote get failed: %d\n", err);
			continue;
		}

		for (int i = 0; i < BUFFER_LENGTH; i++) {
			printk("  0x%02x", buffer[i]);
		}

		printk("\n");

		k_sleep(K_MSEC(2000));

		err = entropy_remote_get_inline(buffer, sizeof(buffer));
		if (err) {
			printk("Entropy remote get failed: %d\n", err);
			continue;
		}

		for (int i = 0; i < BUFFER_LENGTH; i++) {
			printk("  0x%02x", buffer[i]);
		}

		printk("\n");

		k_sleep(K_MSEC(2000));

		err = entropy_remote_get_async(sizeof(buffer), result_callback);
		if (err) {
			printk("Entropy remote get async failed: %d\n", err);
			continue;
		}

		k_sleep(K_MSEC(2000));

		err = entropy_remote_get_cbk(sizeof(buffer), result_callback);
		if (err) {
			printk("Entropy remote get callback failed: %d\n", err);
			continue;
		}
	}
}
