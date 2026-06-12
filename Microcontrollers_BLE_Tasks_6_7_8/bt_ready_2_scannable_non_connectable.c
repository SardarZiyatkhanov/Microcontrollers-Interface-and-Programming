static void bt_ready(void)
{
	int err;

	printk("Bluetooth initialized\n");

	/* Same settings load step, not the important change here. */
	if (IS_ENABLED(CONFIG_SETTINGS)) {
		settings_load();
	}

	/*
	 * Scannable but non-connectable advertising.
	 * Device still appears in scanner and can provide scan response data,
	 * but phone should not be able to establish a connection.
	 * This is exactly what we checked in nRF Connect.
	 */
	err = bt_le_adv_start(BT_LE_ADV_SCAN,
			      ad, ARRAY_SIZE(ad),
			      sd, ARRAY_SIZE(sd));

	if (err) {
		printk("Advertising failed to start (err %d)\n", err);
		return;
	}

	printk("Scannable non-connectable advertising successfully started\n");
}
