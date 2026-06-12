static void bt_ready(void)
{
	int err;

	printk("Bluetooth initialized\n");

	/* Load previous BLE settings if this feature is enabled. */
	if (IS_ENABLED(CONFIG_SETTINGS)) {
		settings_load();
	}

	/*
	 * Default fast connectable advertising.
	 * We used this first because it is simple and phone detects it quickly.
	 * Since it is connectable, nRF Connect shows the Connect button.
	 */
	err = bt_le_adv_start(BT_LE_ADV_CONN_FAST_1,
			      ad, ARRAY_SIZE(ad),
			      sd, ARRAY_SIZE(sd));

	if (err) {
		printk("Advertising failed to start (err %d)\n", err);
		return;
	}

	printk("Fast connectable advertising successfully started\n");
}
