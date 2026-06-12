/* main.c - BLE sample modified for our lab task */

/*
 * Copyright (c) 2015-2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/kernel.h>

#include <zephyr/settings/settings.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/services/bas.h>
#include <zephyr/bluetooth/services/cts.h>
#include <zephyr/bluetooth/services/hrs.h>
#include <zephyr/bluetooth/services/ias.h>

/*
 * Custom 128-bit vendor service.
 * We include it in advertising data, so the scanner can already see
 * that this device has one non-standard service before connection.
 */
#define BT_UUID_CUSTOM_SERVICE_VAL \
	BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abcdef0)

static const struct bt_uuid_128 vnd_uuid = BT_UUID_INIT_128(
	BT_UUID_CUSTOM_SERVICE_VAL);

static const struct bt_uuid_128 vnd_enc_uuid = BT_UUID_INIT_128(
	BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abcdef1));

static const struct bt_uuid_128 vnd_auth_uuid = BT_UUID_INIT_128(
	BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abcdef2));

#define VND_MAX_LEN 20
#define BT_HR_HEARTRATE_DEFAULT_MIN 90U
#define BT_HR_HEARTRATE_DEFAULT_MAX 160U

/*
 * Advertising interval is written in BLE units, not directly in ms.
 * 1 unit = 0.625 ms.
 * 500 / 0.625 = 800 = 0x0320.
 * This directly answers the interval calculation part of Task 8.
 */
#define ADV_INTERVAL_500MS 0x0320

/* Small default values used by the custom vendor characteristics. */
static uint8_t vnd_value[VND_MAX_LEN + 1] = { 'V', 'e', 'n', 'd', 'o', 'r'};
static uint8_t vnd_auth_value[VND_MAX_LEN + 1] = { 'V', 'e', 'n', 'd', 'o', 'r'};
static uint8_t vnd_wwr_value[VND_MAX_LEN + 1] = { 'V', 'e', 'n', 'd', 'o', 'r' };

/* Read callback for vendor characteristic. Runs when central reads this value. */
static ssize_t read_vnd(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			void *buf, uint16_t len, uint16_t offset)
{
	const char *value = attr->user_data;

	return bt_gatt_attr_read(conn, attr, buf, len, offset, value,
				 strlen(value));
}

/* Write callback for vendor characteristic. Runs when central writes data to it. */
static ssize_t write_vnd(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			 const void *buf, uint16_t len, uint16_t offset,
			 uint8_t flags)
{
	uint8_t *value = attr->user_data;

	/* Avoid writing outside the prepared vendor buffer. */
	if (offset + len > VND_MAX_LEN) {
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
	}

	memcpy(value + offset, buf, len);
	value[offset + len] = 0;

	return len;
}

/*
 * Used for vendor indication.
 * Notification just sends data, but indication expects confirmation from central.
 */
static uint8_t simulate_vnd;
static uint8_t indicating;
static struct bt_gatt_indicate_params ind_params;

/* CCC changes when central enables/disables indication for this characteristic. */
static void vnd_ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
	simulate_vnd = (value == BT_GATT_CCC_INDICATE) ? 1 : 0;
}

static void indicate_cb(struct bt_conn *conn,
			struct bt_gatt_indicate_params *params, uint8_t err)
{
	printk("Indication %s\n", err != 0U ? "fail" : "success");
}

static void indicate_destroy(struct bt_gatt_indicate_params *params)
{
	printk("Indication complete\n");
	indicating = 0U;
}

/*
 * Longer vendor value, used to demonstrate long GATT data.
 * This is not the main advertising task, but appears after connection in GATT.
 */
#define VND_LONG_MAX_LEN 74
static uint8_t vnd_long_value[VND_LONG_MAX_LEN + 1] = {
		  'V', 'e', 'n', 'd', 'o', 'r', ' ', 'd', 'a', 't', 'a', '1',
		  'V', 'e', 'n', 'd', 'o', 'r', ' ', 'd', 'a', 't', 'a', '2',
		  'V', 'e', 'n', 'd', 'o', 'r', ' ', 'd', 'a', 't', 'a', '3',
		  'V', 'e', 'n', 'd', 'o', 'r', ' ', 'd', 'a', 't', 'a', '4',
		  'V', 'e', 'n', 'd', 'o', 'r', ' ', 'd', 'a', 't', 'a', '5',
		  'V', 'e', 'n', 'd', 'o', 'r', ' ', 'd', 'a', 't', 'a', '6',
		  '.', ' ' };

static ssize_t write_long_vnd(struct bt_conn *conn,
			      const struct bt_gatt_attr *attr, const void *buf,
			      uint16_t len, uint16_t offset, uint8_t flags)
{
	uint8_t *value = attr->user_data;

	/* Prepare write is accepted here, real copy happens on actual write. */
	if (flags & BT_GATT_WRITE_FLAG_PREPARE) {
		return 0;
	}

	if (offset + len > VND_LONG_MAX_LEN) {
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
	}

	memcpy(value + offset, buf, len);
	value[offset + len] = 0;

	return len;
}

static const struct bt_uuid_128 vnd_long_uuid = BT_UUID_INIT_128(
	BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abcdef3));

static struct bt_gatt_cep vnd_long_cep = {
	.properties = BT_GATT_CEP_RELIABLE_WRITE,
};

static const struct bt_uuid_128 vnd_write_cmd_uuid = BT_UUID_INIT_128(
	BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abcdef4));

/* This characteristic accepts write without response only. */
static ssize_t write_without_rsp_vnd(struct bt_conn *conn,
				     const struct bt_gatt_attr *attr,
				     const void *buf, uint16_t len, uint16_t offset,
				     uint8_t flags)
{
	uint8_t *value = attr->user_data;

	if (!(flags & BT_GATT_WRITE_FLAG_CMD)) {
		/* Regular write request is rejected, because this one is "write no response". */
		return BT_GATT_ERR(BT_ATT_ERR_WRITE_REQ_REJECTED);
	}

	if (offset + len > VND_MAX_LEN) {
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
	}

	memcpy(value + offset, buf, len);
	value[offset + len] = 0;

	return len;
}

/*
 * Vendor Primary Service Declaration.
 * This whole block describes a custom GATT service.
 * It becomes visible after connection, when phone opens GATT server.
 */
BT_GATT_SERVICE_DEFINE(vnd_svc,
	BT_GATT_PRIMARY_SERVICE(&vnd_uuid),

	/* Encrypted read/write characteristic with indication support. */
	BT_GATT_CHARACTERISTIC(&vnd_enc_uuid.uuid,
			       BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE |
			       BT_GATT_CHRC_INDICATE,
			       BT_GATT_PERM_READ_ENCRYPT |
			       BT_GATT_PERM_WRITE_ENCRYPT,
			       read_vnd, write_vnd, vnd_value),

	/* CCC is needed so central can enable indication. */
	BT_GATT_CCC(vnd_ccc_cfg_changed,
		    BT_GATT_PERM_READ | BT_GATT_PERM_WRITE_ENCRYPT),

	/* Authenticated characteristic, used by the sample security part. */
	BT_GATT_CHARACTERISTIC(&vnd_auth_uuid.uuid,
			       BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,
			       BT_GATT_PERM_READ_AUTHEN |
			       BT_GATT_PERM_WRITE_AUTHEN,
			       read_vnd, write_vnd, vnd_auth_value),

	/* Long characteristic. Useful for seeing larger GATT data handling. */
	BT_GATT_CHARACTERISTIC(&vnd_long_uuid.uuid, BT_GATT_CHRC_READ |
			       BT_GATT_CHRC_WRITE | BT_GATT_CHRC_EXT_PROP,
			       BT_GATT_PERM_READ | BT_GATT_PERM_WRITE |
			       BT_GATT_PERM_PREPARE_WRITE,
			       read_vnd, write_long_vnd, &vnd_long_value),

	BT_GATT_CEP(&vnd_long_cep),

	/* Write without response characteristic. */
	BT_GATT_CHARACTERISTIC(&vnd_write_cmd_uuid.uuid,
			       BT_GATT_CHRC_WRITE_WITHOUT_RESP,
			       BT_GATT_PERM_WRITE, NULL,
			       write_without_rsp_vnd, &vnd_wwr_value),
);

/*
 * Main advertising data.
 * This is visible before connection in scanner app.
 */
static const struct bt_data ad[] = {
	/* General discoverable + BLE only, not Classic Bluetooth. */
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),

	/* Standard 16-bit services shown in scanner: HRS, BAS, CTS. */
	BT_DATA_BYTES(BT_DATA_UUID16_ALL,
		      BT_UUID_16_ENCODE(BT_UUID_HRS_VAL),
		      BT_UUID_16_ENCODE(BT_UUID_BAS_VAL),
		      BT_UUID_16_ENCODE(BT_UUID_CTS_VAL)),

	/* Custom vendor service, uses 128-bit UUID. */
	BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_CUSTOM_SERVICE_VAL),
};

/*
 * Scan response data.
 * We put device name here because advertising packet has limited space.
 */
static const struct bt_data sd[] = {
	BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME, sizeof(CONFIG_BT_DEVICE_NAME) - 1),
};

/* Called when MTU changes after connection. Useful debug for GATT data size. */
void mtu_updated(struct bt_conn *conn, uint16_t tx, uint16_t rx)
{
	printk("Updated MTU: TX: %d RX: %d bytes\n", tx, rx);
}

static struct bt_gatt_cb gatt_callbacks = {
	.att_mtu_updated = mtu_updated
};

/* Connection callback - proves in terminal that phone connected or connection failed. */
static void connected(struct bt_conn *conn, uint8_t err)
{
	if (err) {
		printk("Connection failed, err 0x%02x %s\n", err, bt_hci_err_to_str(err));
	} else {
		printk("Connected\n");
	}
}

/* Disconnection callback. Used when phone disconnects from peripheral. */
static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	printk("Disconnected, reason 0x%02x %s\n", reason, bt_hci_err_to_str(reason));
}

/* Immediate Alert Service callbacks. These are part of the sample services. */
static void alert_stop(void)
{
	printk("Alert stopped\n");
}

static void alert_start(void)
{
	printk("Mild alert started\n");
}

static void alert_high_start(void)
{
	printk("High alert started\n");
}

BT_CONN_CB_DEFINE(conn_callbacks) = {
	.connected = connected,
	.disconnected = disconnected,
};

BT_IAS_CB_DEFINE(ias_callbacks) = {
	.no_alert = alert_stop,
	.mild_alert = alert_start,
	.high_alert = alert_high_start,
};

static void bt_ready(void)
{
	int err;

	printk("Bluetooth initialized\n");

	/*
	 * Load saved BLE settings if enabled in prj.conf.
	 * For example bonding, security, or CCC values can be restored.
	 */
	if (IS_ENABLED(CONFIG_SETTINGS)) {
		settings_load();
	}

	/*
	 * Start connectable advertising with custom interval.
	 * This is the main Task 8 modification.
	 *
	 * BT_LE_ADV_OPT_CONNECTABLE - phone can connect.
	 * ADV_INTERVAL_500MS twice - min and max advertising interval are both 500 ms.
	 * NULL - not directed to a specific central.
	 */
	err = bt_le_adv_start(
		BT_LE_ADV_PARAM(BT_LE_ADV_OPT_CONNECTABLE,
				ADV_INTERVAL_500MS,
				ADV_INTERVAL_500MS,
				NULL),
		ad, ARRAY_SIZE(ad),
		sd, ARRAY_SIZE(sd));

	if (err) {
		printk("Advertising failed to start (err %d)\n", err);
		return;
	}

	printk("Advertising successfully started with 500 ms interval\n");
}

/* Display passkey if pairing/security asks for it. */
static void auth_passkey_display(struct bt_conn *conn, unsigned int passkey)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	printk("Passkey for %s: %06u\n", addr, passkey);
}

/* Called if pairing is cancelled. */
static void auth_cancel(struct bt_conn *conn)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	printk("Pairing cancelled: %s\n", addr);
}

static struct bt_conn_auth_cb auth_cb_display = {
	.passkey_display = auth_passkey_display,
	.passkey_entry = NULL,
	.cancel = auth_cancel,
};

/* Battery simulation: decreases value and returns to 100 when it reaches 0. */
static void bas_notify(void)
{
	uint8_t battery_level = bt_bas_get_battery_level();

	battery_level--;

	if (!battery_level) {
		battery_level = 100U;
	}

	bt_bas_set_battery_level(battery_level);
}

static uint8_t bt_heartrate = BT_HR_HEARTRATE_DEFAULT_MIN;

/*
 * Heart rate simulation.
 * This is why we see changing Heart Rate data in nRF Connect after connection.
 */
static void hrs_notify(void)
{
	bt_heartrate++;
	if (bt_heartrate == BT_HR_HEARTRATE_DEFAULT_MAX) {
		bt_heartrate = BT_HR_HEARTRATE_DEFAULT_MIN;
	}

	bt_hrs_notify(bt_heartrate);
}

/*
 * Current Time Service demo variables.
 * This is only for sample behavior, not real clock synchronization.
 */
static int64_t unix_ms_ref;
static bool cts_notification_enabled;

/* Central enables/disables CTS notification through GATT. */
void bt_cts_notification_changed(bool enabled)
{
	cts_notification_enabled = enabled;
}

/* Phone can write current time, then device stores reference time. */
int bt_cts_cts_time_write(struct bt_cts_time_format *cts_time)
{
	int err;
	int64_t unix_ms;

	if (IS_ENABLED(CONFIG_BT_CTS_HELPER_API)) {
		err = bt_cts_time_to_unix_ms(cts_time, &unix_ms);
		if (err) {
			return err;
		}
	} else {
		return -ENOTSUP;
	}

	unix_ms_ref = unix_ms - k_uptime_get();
	return 0;
}

/* Fill current time value when central reads CTS. */
int bt_cts_fill_current_cts_time(struct bt_cts_time_format *cts_time)
{
	int64_t unix_ms = unix_ms_ref + k_uptime_get();

	if (IS_ENABLED(CONFIG_BT_CTS_HELPER_API)) {
		return bt_cts_time_from_unix_ms(cts_time, unix_ms);
	} else {
		return -ENOTSUP;
	}
}

const struct bt_cts_cb cts_cb = {
	.notification_changed = bt_cts_notification_changed,
	.cts_time_write = bt_cts_cts_time_write,
	.fill_current_cts_time = bt_cts_fill_current_cts_time,
};

/* Heart rate control point callback. Resets simulated value if request is valid. */
static int bt_hrs_ctrl_point_write(uint8_t request)
{
	printk("HRS Control point request: %d\n", request);
	if (request != BT_HRS_CONTROL_POINT_RESET_ENERGY_EXPANDED_REQ) {
		return -ENOTSUP;
	}

	bt_heartrate = BT_HR_HEARTRATE_DEFAULT_MIN;
	return 0;
}

static struct bt_hrs_cb hrs_cb = {
	.ctrl_point_write = bt_hrs_ctrl_point_write,
};

int main(void)
{
	struct bt_gatt_attr *vnd_ind_attr;
	char str[BT_UUID_STR_LEN];
	int err;

	/*
	 * First step: enable Bluetooth stack.
	 * Advertising before this would not make sense because BLE is not ready yet.
	 */
	err = bt_enable(NULL);
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return 0;
	}

	/*
	 * Start advertising after Bluetooth init.
	 * This makes board visible in nRF Connect scanner.
	 */
	bt_ready();

	/*
	 * Register services/callbacks used by the sample.
	 * These are what we check after connection in the GATT view.
	 */
	bt_cts_init(&cts_cb);
	bt_hrs_cb_register(&hrs_cb);

	bt_gatt_cb_register(&gatt_callbacks);
	bt_conn_auth_cb_register(&auth_cb_display);

	/*
	 * Find vendor characteristic for later indication.
	 * This is mostly for custom service demo.
	 */
	vnd_ind_attr = bt_gatt_find_by_uuid(vnd_svc.attrs, vnd_svc.attr_count,
					    &vnd_enc_uuid.uuid);
	bt_uuid_to_str(&vnd_enc_uuid.uuid, str, sizeof(str));
	printk("Indicate VND attr %p (UUID %s)\n", vnd_ind_attr, str);

	/*
	 * Main program loop.
	 * Every second we simulate values, so phone can observe changing data.
	 */
	while (1) {
		k_sleep(K_SECONDS(1));

		/* Send Current Time Service notification only if central enabled it. */
		if (cts_notification_enabled) {
			bt_cts_send_notification(BT_CTS_UPDATE_REASON_MANUAL);
		}

		/* Simulated Heart Rate Measurement notify. */
		hrs_notify();

		/* Simulated Battery Service update. */
		bas_notify();

		/*
		 * Vendor indication.
		 * We check "indicating" so we do not start another indication
		 * before previous one is completed.
		 */
		if (simulate_vnd && vnd_ind_attr) {
			if (indicating) {
				continue;
			}

			ind_params.attr = vnd_ind_attr;
			ind_params.func = indicate_cb;
			ind_params.destroy = indicate_destroy;
			ind_params.data = &indicating;
			ind_params.len = sizeof(indicating);

			if (bt_gatt_indicate(NULL, &ind_params) == 0) {
				indicating = 1U;
			}
		}
	}

	return 0;
}
