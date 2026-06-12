# Microcontrollers: Interface and Programming

This repository contains my lab work for **Microcontrollers: Interface & Programming**, covering both STM32-based embedded programming and Bluetooth Low Energy work on the nRF52840 DK.

**Student:** Sardar Ziyatkhanov
**Student ID:** P000017499

Throughout the course, the labs moved from basic GPIO control and low-power STM32 operation to UART communication and BLE advertising/connection testing. The main platforms used were the **STM32 Nucleo-F401RE**, **Arduino Uno**, and **Nordic nRF52840 DK**.

---

## Overview

The goal of this repository is to keep all completed course labs in one place and document both the firmware implementation and the hardware/software testing process.

The repository includes work with:

- GPIO output control
- LED blinking and timing
- oscilloscope-based signal verification
- timer interrupt-based periodic tasks
- low-power modes: **Sleep**, **Stop**, and **Standby**
- current measurement and battery life estimation
- RTC wake-up configuration
- UART communication between **STM32** and **Arduino**
- serial CSV-style logging to PC
- BLE advertising using **nRF52840 DK**
- BLE scanning and connection verification using **nRF Connect**
- GATT services, characteristics, notify behavior, and advertising interval modification

---

## Hardware / Tools Used

### Main Boards

- **STM32 Nucleo-F401RE**
- **Arduino Uno**
- **Nordic nRF52840 DK**

### Software / IDEs

- **STM32CubeMX**
- **STM32CubeIDE**
- **STM32 HAL library**
- **Arduino IDE**
- **Visual Studio Code**
- **nRF Connect extension**
- **nRF Connect SDK**
- **Zephyr RTOS**
- **nRF Connect mobile app**

---

## Labs Included

### Lab 1 - Flashing LED

A basic GPIO lab where an external LED was connected to the STM32 Nucleo-F401RE and toggled with a visible delay.

**Main ideas:**

- configuring GPIO output in CubeMX
- using `HAL_GPIO_TogglePin()`
- understanding STM32 port/pin mapping
- checking LED timing with oscilloscope
- comparing code behavior with real waveform behavior

---

### Lab 2 - Battery Life Estimation with Sleep Mode

A low-power lab where the STM32 periodically activates a buzzer and spends the rest of the cycle in **Sleep mode**.

**Main ideas:**

- replacing busy-wait delay with **TIM2 interrupt**
- using `__WFI()` for low-power waiting
- configuring timer period using prescaler and auto-reload values
- measuring active and idle current
- calculating average current
- estimating battery lifetime from measured values

---

### Lab 3A - Stop Mode

A low-power lab demonstrating **Stop mode**, where the MCU wakes from an interrupt and keeps SRAM content.

**Main ideas:**

- using onboard button as wake-up source
- configuring `PC13` as external interrupt
- entering Stop mode with `HAL_PWR_EnterSTOPMode()`
- suspending and resuming SysTick
- restoring system clock after wake-up
- proving memory retention using a wake counter

---

### Lab 3B - Standby Mode

A continuation of the low-power task, showing how **Standby mode** differs from Stop mode.

**Main ideas:**

- using dedicated wake-up pin
- checking standby wake-up flag
- understanding reset-after-wake behavior
- showing that volatile SRAM state is lost
- comparing Stop mode and Standby mode practically
- using LED blink pattern to indicate reset/wake-up behavior

---

### Lab 4 - UART Communication with Arduino

A communication lab where STM32 wakes periodically, requests button count data from Arduino over UART, logs it to PC, and returns to low-power mode.

**Main ideas:**

- UART communication between STM32 and Arduino
- crossing TX and RX lines correctly
- using USART1 for STM32-Arduino communication
- using USART2 for STM32-PC serial logging
- RTC wake-up every 10 seconds
- requesting data with a `?` character
- logging timestamp and count in CSV-style format
- returning STM32 to Stop mode after each transaction

---

### Task 6 - nRF52840 DK Overview and BLE Advertising Setup

A BLE-focused task where the nRF52840 DK was studied and prepared as a BLE advertising device.

**Main ideas:**

- understanding nRF52840 DK hardware
- BLE advertising channels 37, 38, and 39
- advertising interval and power trade-off
- setting up nRF Connect SDK in VS Code
- selecting correct board target: `nrf52840dk_nrf52840`
- building and flashing a BLE sample
- verifying that the board can advertise without extra wiring

---

### Task 7 - BLE Project Build, Connection Verification, and Code Analysis

A practical BLE task where the board was detected and connected using the nRF Connect mobile app.

**Main ideas:**

- checking BLE advertising before connection
- observing device name, RSSI, MAC address, advertised services, and connectability
- connecting from phone to board
- opening the GATT server after connection
- observing standard services such as Heart Rate, Battery, Current Time, and Device Information
- understanding `main.c`, `prj.conf`, `bt_enable()`, `bt_ready()`, and `bt_le_adv_start()`
- testing scannable but non-connectable advertising mode

---

### Task 8 - Device Name and Advertising Interval Modification

A BLE modification task where the advertised device name and advertising interval were changed.

**Main ideas:**

- changing device name in `prj.conf`
- setting `CONFIG_BT_DEVICE_NAME="My_nRF52840"`
- changing advertising interval to 500 ms
- calculating BLE interval units:

```text
1 BLE interval unit = 0.625 ms
500 / 0.625 = 800
800 decimal = 0x0320
```

- using `#define ADV_INTERVAL_500MS 0x0320`
- passing custom advertising parameters into `BT_LE_ADV_PARAM(...)`
- verifying the result in nRF Connect

---

## Repository Structure

```text
Microcontrollers-Interface-and-Programming/
├── Lab1_Flashing_LED/
├── Lab2_Sleep_Mode_Battery_Life/
├── Lab3A_Stop_Mode/
├── Lab3B_Standby_Mode/
├── Lab4_UART_With_Arduino/
└── Microcontrollers_BLE_Tasks_6_7_8/
```

---

## What This Repository Shows

This repository reflects my practical work in microcontroller programming, especially:

- configuring peripherals in CubeMX
- writing embedded C firmware
- using STM32 HAL functions
- working with timers and interrupts
- applying low-power design principles
- communicating between microcontrollers
- testing with real hardware and oscilloscope evidence
- setting up Zephyr/nRF Connect SDK projects
- configuring BLE advertising and GATT behavior
- connecting theory, code, and observed results from actual lab testing

---

## Notes

Some folders include full project files, while others include the main source/configuration files and report evidence. For BLE tasks, the final uploaded code includes the important `main.c`, `prj.conf`, and tested `bt_ready()` variations used during the lab.
