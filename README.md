# Microcontrollers: Interface and Programming

![STM32](https://img.shields.io/badge/MCU-STM32%20Nucleo--F401RE-blue)
![Arduino](https://img.shields.io/badge/Board-Arduino%20Uno-teal)
![nRF52840](https://img.shields.io/badge/BLE-nRF52840%20DK-purple)
![Zephyr](https://img.shields.io/badge/RTOS-Zephyr-darkred)
![License](https://img.shields.io/badge/License-MIT-green)
![Last Commit](https://img.shields.io/github/last-commit/SardarZiyatkhanov/Microcontrollers-Interface-and-Programming)
![Repo Size](https://img.shields.io/github/repo-size/SardarZiyatkhanov/Microcontrollers-Interface-and-Programming)

This repository contains my lab work for **Microcontrollers: Interface & Programming**, covering both STM32-based embedded programming and Bluetooth Low Energy work on the nRF52840 DK.

**Student:** Sardar Ziyatkhanov
**Student ID:** P000017499

<p align="center">
  <img src="Lab1_Flashing_LED/images/Pasted image 20260320183154.png" alt="STM32 Nucleo board with external LED on breadboard" width="350">
</p>

Throughout the course, the labs moved from basic GPIO control and low-power STM32 operation to UART communication and BLE advertising/connection testing. The main platforms used were the **STM32 Nucleo-F401RE**, **Arduino Uno**, and **Nordic nRF52840 DK**.

---

## Table of Contents

- [Overview](#overview)
- [Hardware / Tools Used](#hardware--tools-used)
- [Labs Included](#labs-included)
- [Repository Structure](#repository-structure)
- [What This Repository Shows](#what-this-repository-shows)
- [Notes](#notes)
- [Contributing](#contributing)
- [License](#license)

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

| Lab | Title | Summary |
|-----|-------|---------|
| [Lab 1](Lab1_Flashing_LED/README.md) | Flashing LED | A basic GPIO lab where an external LED was connected to the STM32 Nucleo-F401RE and toggled with a visible delay, configured via CubeMX and `HAL_GPIO_TogglePin()`, verified with an oscilloscope. |
| [Lab 2](Lab2_Sleep_Mode_Battery_Life/README.md) | Battery Life Estimation with Sleep Mode | A low-power lab where the STM32 periodically activates a buzzer using a **TIM2 interrupt** and spends the rest of the cycle in **Sleep mode**, with current measurement used to estimate battery lifetime. |
| [Lab 3A](Lab3A_Stop_Mode/README.md) | Stop Mode | A low-power lab demonstrating **Stop mode**, where the MCU wakes from a button interrupt, restores the system clock, and retains SRAM content (proven via a wake counter). |
| [Lab 3B](Lab3B_Standby_Mode/README.md) | Standby Mode | A continuation of the low-power task, showing how **Standby mode** differs from Stop mode, including reset-after-wake behavior and loss of volatile SRAM state. |
| [Lab 4](Lab4_UART_With_Arduino/README.md) | UART Communication with Arduino | A communication lab where STM32 wakes periodically, requests button count data from Arduino over UART, logs it to PC in CSV format, and returns to **Stop mode**. |
| [BLE Tasks 6-8](Microcontrollers_BLE_Tasks_6_7_8/README.md) | nRF52840 BLE Advertising, Connection, and Modification | BLE tasks covering nRF52840 DK advertising setup, connection verification with the nRF Connect mobile app, GATT service inspection, and modifying the device name and advertising interval. |

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

---

## Contributing

This is a personal academic repository, so it isn't actively seeking code contributions. That said, if you spot an error or have a suggestion, feel free to open an issue. See [CONTRIBUTING.md](CONTRIBUTING.md) for details, including a note on academic integrity.

---

## License

This project is licensed under the [MIT License](LICENSE).
