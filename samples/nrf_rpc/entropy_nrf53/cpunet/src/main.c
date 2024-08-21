/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>

static void reset_work_handler(struct k_work *work);
K_WORK_DELAYABLE_DEFINE(reset_work, reset_work_handler);

static void reset_work_handler(struct k_work *work)
{
	k_oops();
}

int main(void)
{
	/* The only activity of this application is interaction with the APP
	 * core using serialized communication through the nRF RPC library.
	 * The necessary handlers are registered through nRF RPC interface
	 * and start at system boot.
	 */
	printk("Entropy sample started[NET Core].\n");
	k_work_schedule(&reset_work, K_SECONDS(10));
	return 0;
}
