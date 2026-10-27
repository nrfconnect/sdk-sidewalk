/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/*
 * Shell backend for Registration over Wire (ROW).
 *
 * One UART carries the shell (commands, prompt, echo, logs) and the
 * 0xAA | LEN | payload frames of the Sidewalk serial registration, in both
 * directions. Received bytes are demultiplexed in the UART interrupt.
 * All transmissions share one lock, so a frame is always written as one unit.
 */

#include <sid_row_demux.h>
#include <sid_row_uart.h>

#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>
#include <zephyr/sys/ring_buffer.h>

#define ROW_UART_NODE DT_CHOSEN(zephyr_shell_uart)
#define ROW_SHELL_PROMPT "uart:~$ "
#define ROW_RX_CHUNK_SIZE 16
#define ROW_MFG_DATA_WRITE_DELAY_MS 10

BUILD_ASSERT(!IS_ENABLED(CONFIG_UART_CONSOLE),
	     "The UART console writes bypass the ROW transmit lock, disable CONFIG_UART_CONSOLE");

struct row_uart {
	const struct device *dev;
	shell_transport_handler_t handler;
	void *context;
	struct k_spinlock rx_lock;
	struct sid_row_demux demux;
	struct ring_buf console_rb;
	uint8_t console_buf[CONFIG_SIDEWALK_ROW_CONSOLE_RING_SIZE];
	sid_row_uart_frame_ready_cb_t frame_ready_cb;
	bool panic_mode;
};

static struct row_uart row_uart = {
	.dev = DEVICE_DT_GET(ROW_UART_NODE),
};

K_MUTEX_DEFINE(row_tx_mutex);

static uint8_t row_tx_frame[SID_ROW_FRAME_PAYLOAD_MAX + SID_ROW_FRAME_HEADER_SIZE];

static void row_uart_isr(const struct device *dev, void *user_data)
{
	struct row_uart *ru = user_data;
	uint8_t chunk[ROW_RX_CHUNK_SIZE];
	bool console_rx = false;
	bool frame_ready = false;
	int len = 0;

	if (!uart_irq_update(dev) || !uart_irq_rx_ready(dev)) {
		return;
	}

	while ((len = uart_fifo_read(dev, chunk, sizeof(chunk))) > 0) {
		k_spinlock_key_t key = k_spin_lock(&ru->rx_lock);

		for (int i = 0; i < len; i++) {
			switch (sid_row_demux_feed(&ru->demux, chunk[i])) {
			case SID_ROW_DEMUX_CONSOLE:
				/* A full buffer drops the byte. The host paces its command lines. */
				if (ring_buf_put(&ru->console_rb, &chunk[i], 1)) {
					console_rx = true;
				}
				break;
			case SID_ROW_DEMUX_FRAME_READY:
				frame_ready = true;
				break;
			default:
				break;
			}
		}
		k_spin_unlock(&ru->rx_lock, key);
	}

	if (console_rx && ru->handler) {
		ru->handler(SHELL_TRANSPORT_EVT_RX_RDY, ru->context);
	}
	if (frame_ready && ru->frame_ready_cb) {
		ru->frame_ready_cb();
	}
}

static void row_uart_tx_raw(const struct device *dev, const uint8_t *data, size_t len)
{
	for (size_t i = 0; i < len; i++) {
		uart_poll_out(dev, data[i]);
	}
}

static bool row_uart_tx_lock(void)
{
	if (row_uart.panic_mode || k_is_in_isr() || k_is_pre_kernel()) {
		return false;
	}
	(void)k_mutex_lock(&row_tx_mutex, K_FOREVER);
	return true;
}

static void row_uart_tx_unlock(bool locked)
{
	if (locked) {
		(void)k_mutex_unlock(&row_tx_mutex);
	}
}

/* Shell transport */

static int row_shell_init(const struct shell_transport *transport, const void *config,
			  shell_transport_handler_t evt_handler, void *context)
{
	ARG_UNUSED(config);
	struct row_uart *ru = transport->ctx;

	if (!device_is_ready(ru->dev)) {
		return -ENODEV;
	}

	ru->handler = evt_handler;
	ru->context = context;
	ring_buf_init(&ru->console_rb, sizeof(ru->console_buf), ru->console_buf);
	sid_row_demux_reset(&ru->demux);

	int err = uart_irq_callback_user_data_set(ru->dev, row_uart_isr, ru);

	if (err) {
		return err;
	}
	uart_irq_rx_enable(ru->dev);

	return 0;
}

static int row_shell_uninit(const struct shell_transport *transport)
{
	struct row_uart *ru = transport->ctx;

	uart_irq_rx_disable(ru->dev);

	return 0;
}

static int row_shell_enable(const struct shell_transport *transport, bool blocking_tx)
{
	struct row_uart *ru = transport->ctx;

	if (blocking_tx) {
		ru->panic_mode = true;
	}

	return 0;
}

static int row_shell_write(const struct shell_transport *transport, const void *data,
			   size_t length, size_t *cnt)
{
	struct row_uart *ru = transport->ctx;
	const bool locked = row_uart_tx_lock();

	row_uart_tx_raw(ru->dev, data, length);
	row_uart_tx_unlock(locked);

	*cnt = length;
	if (ru->handler) {
		ru->handler(SHELL_TRANSPORT_EVT_TX_RDY, ru->context);
	}

	return 0;
}

static int row_shell_read(const struct shell_transport *transport, void *data, size_t length,
			  size_t *cnt)
{
	struct row_uart *ru = transport->ctx;
	k_spinlock_key_t key = k_spin_lock(&ru->rx_lock);

	*cnt = ring_buf_get(&ru->console_rb, data, length);
	k_spin_unlock(&ru->rx_lock, key);

	return 0;
}

static const struct shell_transport_api row_shell_transport_api = {
	.init = row_shell_init,
	.uninit = row_shell_uninit,
	.enable = row_shell_enable,
	.write = row_shell_write,
	.read = row_shell_read,
};

static struct shell_transport row_shell_transport = {
	.api = &row_shell_transport_api,
	.ctx = &row_uart,
};

SHELL_DEFINE(shell_row, ROW_SHELL_PROMPT, &row_shell_transport,
	     CONFIG_SIDEWALK_ROW_SHELL_LOG_QUEUE_SIZE, CONFIG_SIDEWALK_ROW_SHELL_LOG_QUEUE_TIMEOUT,
	     SHELL_FLAG_OLF_CRLF);

/* Serial interface for the Sidewalk serial registration */

static void row_serial_init(void)
{
	/* The UART is brought up together with the shell backend. */
}

static void row_serial_read(uint8_t *data, size_t len, size_t *bytes_read)
{
	k_spinlock_key_t key = k_spin_lock(&row_uart.rx_lock);

	*bytes_read = sid_row_demux_take_frame(&row_uart.demux, data, len);
	k_spin_unlock(&row_uart.rx_lock, key);
}

static void row_serial_write(const uint8_t *data, size_t len, size_t *bytes_written)
{
	const bool locked = row_uart_tx_lock();
	const size_t frame_len =
		sid_row_frame_encode(data, len, row_tx_frame, sizeof(row_tx_frame));

	if (frame_len) {
		row_uart_tx_raw(row_uart.dev, row_tx_frame, frame_len);
	}
	row_uart_tx_unlock(locked);

	*bytes_written = frame_len ? len : 0;
}

static void row_serial_mfg_data_write(const uint8_t *data, size_t len, size_t *bytes_written)
{
	k_msleep(ROW_MFG_DATA_WRITE_DELAY_MS);
	row_serial_write(data, len, bytes_written);
}

static struct serial_ifc_context row_serial_ifc = {
	.serial_read = row_serial_read,
	.serial_write = row_serial_write,
	.serial_init = row_serial_init,
	.serial_mfg_data_write = row_serial_mfg_data_write,
	.serial_ctx = NULL,
};

struct serial_ifc_context *sid_row_uart_get_ifc(void)
{
	return &row_serial_ifc;
}

void sid_row_uart_set_frame_ready_cb(sid_row_uart_frame_ready_cb_t cb)
{
	row_uart.frame_ready_cb = cb;
}

static int row_shell_backend_init(void)
{
	/* No colors and no VT100 sequences: the host tool parses the replies verbatim. */
	static const struct shell_backend_config_flags cfg_flags = {
		.insert_mode = 0,
		.echo = 1,
		.obscure = 0,
		.mode_delete = 1,
		.use_colors = 0,
		.use_vt100 = 0,
	};

	return shell_init(&shell_row, NULL, cfg_flags, IS_ENABLED(CONFIG_SHELL_LOG_BACKEND),
			  CONFIG_LOG_MAX_LEVEL);
}

SYS_INIT(row_shell_backend_init, POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEVICE);
