/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
void k_sem_give(struct k_sem *sem)
{
	ARG_UNUSED(sem);
}

int k_sem_take(struct k_sem *sem, k_timeout_t timeout)
{
	ARG_UNUSED(sem);
	ARG_UNUSED(timeout);
	return 0;
}
