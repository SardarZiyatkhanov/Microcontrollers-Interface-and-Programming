**Course:** Microcontrollers: Interface & Programming (ENCE-4709 - 20908)
**Instructor:** Dr. Alexz Farrall  
**Lab Number & Title:**  Lab 2 - Battery Life Estimation with Sleep mode
**Student Name:** Sardar Ziyatkhanov  
**Student ID:** P000017499  
**Program / Section:** BSCE26  
**Date:** March, 2026

---

## Objective

The objective of this lab is to construct the system with an active buzzer on GPIO pin so that it turns ON periodically for a short duration and remains OFF otherwise. While the buzzer is OFF, the MCU is expected to be placed into **Sleep mode** using a CPU sleep instruction, not a busy-wait delay.
By choosing suitable timing values for the ON duration and the repetition period, the average current, energy consumed per cycle, and the expected battery lifetime for a 1000 mAh battery, should be estimated, clearly stating any assumptions made.
Evidence of the accuracy of estimation by measuring and annotating the current waveform for one full cycle, showing both the active buzzer current and the sleep current, and comparing the measured behavior to the calculated values should be done.

## Useful Formulas

The battery lifetime is calculated by
$$Lifetime(hours) = \frac{C_{battery}}{I_{avg}}$$
$$Lifetime(days) = \frac{C_{battery}}{24 \cdot I_{avg}}$$
The average current is calculated using
$$I_{avg}=\frac{I_{active}t_{active}\;+\;I_{idle}t_{idle}}{T}$$
For this lab, buzzer stayed active for **200ms** and passive for **1800ms**, meaning the total cycle is **2000ms** or **2s**
Assumptions made were that the active current is **40mA** and passive current is **1mA**. Plugging those into our formulas, we get:
$$I_{avg}=\frac{40 \cdot 0.2\;+\;1 \cdot 1.8}{2} = 4.9\;mA$$
$$Lifetime = \frac{1000\,mA}{4.9\,mA} = 204.1\;hours \approx 8.5\,days$$

Later, we will test the accuracy of our assumption.

## CubeMX Configuration

**PA5** was configured as an **GPIO_OUTPUT**. From practical point of view, along with providing signal to the external buzzer, on-board led as will indicate the existence of the signal. However from the point of energy-consumption, LED will light up for no strong purpose, consuming extra energy.

In this lap, instead of employing `HAL_delay()`, we use **TIM2** timer which is much more beneficial from energy-preservation point. 

![image](images/Pasted%20image%2020260320191126.png)

To configure it, in CubeMX we set prescaler to **15999** and auto-reload value to **99**, along with enabling **TIM2 interrupt** in NVIC (to both those values 1 is added, since counting starts from zero). NVIC is basically responsible for the prioritization of interrupts. So, the selected low-power mode is **sleep mode**, because the buzzer still requires clock-related operation.

![image](images/Pasted%20image%2020260320190644.png)
$$f \approx \frac{16,000,000}{16,000 \cdot 100} = 10\,Hz$$
$$T = \frac{1}{10} = 0.1s = 100\,ms$$

As mentioned before, timing is done through interrupts, so TIM2 should be enabled as the interrupt source in NVIC settings.
## Code Explanation

```cpp
MX_TIM2_Init();
```
TIM2 initialization

```cpp
#define TICK_MS         100u   // match with TIM2 setup
#define PERIOD_MS       2000u  // total cycle length
#define BUZZ_ON_MS      200u   // buzzer ON time inside the cycle

#define PERIOD_TICKS    (PERIOD_MS / TICK_MS)   // 20
#define BUZZ_ON_TICKS   (BUZZ_ON_MS / TICK_MS)  // 2
```

- Those values correspond to the 200ms-active phase and 1800ms passive phase.

```cpp
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2)
    {
        tick_in_cycle++;
        if (tick_in_cycle >= PERIOD_TICKS)
            tick_in_cycle = 0;

        if (tick_in_cycle < BUZZ_ON_TICKS)
	        // buzzer ON
            HAL_GPIO_WritePin(BUZZ_GPIO_PORT, BUZZ_PIN, GPIO_PIN_SET);   
        else
	        // buzzer OFF
            HAL_GPIO_WritePin(BUZZ_GPIO_PORT, BUZZ_PIN, GPIO_PIN_RESET); 
    }
}
```

- this function will be called automatically, every time TIM2 reaches update event.
- `htim` tells which timer caused the interrupt.
- when interrupt is caused by TIM2, the buzzer logic executes.

```cpp
  while (1)
  {
	  __WFI();
  }
```

in the main loop we specify `__WFI():` which:
	- is a low power loop (Wait For Interrupt).
	- suspends CPU execution, turns off core clocks
	- puts MCU into sleep mode. 
	- wakes up only when interrupt occurs (TIM2 timer), where execution happens
	- then goes back to sleep

![image](images/Pasted%20image%2020260320192734.png)

To avoid heavy resistor "eating" our current, we select smaller one (10-Ohm) and measure the current going through the system. 

![image](images/Pasted%20image%2020260320192909.png)
![image](images/Pasted%20image%2020260320192919.png)

Here we see that the time when signal goes out the PA5 and period match to those configured in the code.

![image](images/Pasted%20image%2020260320193032.png)

During active state the current going through the system equals to **32mA** (320 mv/10 ohms)

![image](images/Pasted%20image%2020260320193240.png)

During passive state - **0.8mA** (8/10)

So,
$$I_{avg}=\frac{32 \cdot 0.2\;+\;0.8 \cdot 1.8}{2} = 3.92\;mA$$
$$Lifetime = \frac{1000\,mA}{3.98\,mA} = 255.1\;hours \approx 10.63\,days$$
