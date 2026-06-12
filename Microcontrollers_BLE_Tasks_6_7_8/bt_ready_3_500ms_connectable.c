/*
 * We need 500 ms advertising interval for Task 8.
 * BLE uses 0.625 ms units:
 * 500 / 0.625 = 800 = 0x0320.
 */
#define ADV_INTERVAL_500MS 0x0320

static void bt_ready(void)
{
	int err;

	printk("Bluetooth initialized\n");

	/* Load saved settings if enabled in prj.conf. */
	if (IS_ENABLED(CONFIG_SETTINGS)) {
		settings_load();
	}

	/*
	 * Custom connectable advertising.
	 * Main difference from fast mode: here we manually define interval.
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
