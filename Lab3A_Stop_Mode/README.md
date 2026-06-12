**Course:** Microcontrollers: Interface & Programming (ENCE-4709 - 20908)
**Instructor:** Dr. Alexz Farrall  
**Lab Number & Title:**  Lab 3A - Stop Mode
**Student Name:** Sardar Ziyatkhanov  
**Student ID:** P000017499  
**Program / Section:** BSCE26  
**Date:** March, 2026

## Lab Requirements

Design and implement a battery-powered door sensor on the STM32 Nucleo-F401RE that demonstrates the practical difference between Stop mode and Standby mode.
In the first phase, configure the MCU to enter Stop mode after initialization and use the on-board USER button as a wake source; when the button is pressed, the system should wake, perform a visible action such as blinking the LED, retain a simple piece of state in memory such as a counter, and then return to Stop mode, allowing you to observe that execution resumes and data is preserved.

## Power Preservation Modes

In the previous lab, the utilized the Sleep mode which turned the CPU off, left some clocks on, kept memory and preserved some energy. In this lab, another type of mode is used - **Stop mode**. Compared with the sleep mode, it preserves even more energy, turns most of the clocks off, *keeps the memory*; however, wakes up from an interrupt a bit slower than sleep mode. This mode is considered as the most practical, since offers great energy consumption and most-importantly, keeps the memory (if compare to the Standby mode, which wipes it all out, will be discussed in lab report 3b).

## CubeMX Configuration

In this lab, no external circuitry is used, only onboard LED indicates what is happening. Since onboard LED is connected to the PA5, configure it as `GPIO_Output`. Additionally, set PC13 as an input (`GPIO_EXTI13`), which will be tracked by EXTI - this is for on-board button. 
![image](images/Pasted%20image%2020260320195106.png)

**EXTI line `[15:10]`** is the group of external interrupt lines for pins 10 to 15, handled by one shared interrupt vector. It tracks the activity on the pins assigned.
![image](images/Pasted%20image%2020260320195239.png)

## Code Explanation

```cpp title:"main loop"
Blink_LD2(2);

while (1)
{
    button_woke_flag = 0;

    Enter_Stop_Mode();

    if (button_woke_flag)
    {
        wake_counter++;
        Blink_LD2(wake_counter);
    }
}
```

Overall flow and logic of the system is as follows:
1. LED blinks twice, indicating boot
2. wake-ip flag is being cleared
3. **entering Stop mode**
4. waking up from button press
5. increment the specified counter
6. blink LED once again

By each button press, the number of times the LED blinks increases, indicating the memory preservation feature of the Stop mode.


```cpp title:"Enter_Stop_Mode()"
static void Enter_Stop_Mode(void)
{
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
    __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_13);

    HAL_SuspendTick();

    HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);

    HAL_ResumeTick();
    SystemClock_Config();
}
```

`__HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);` - clearing power wake-up flag.

`__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_13);` - clearing any processing EXTI interrupt to avoid false wake-up.

`HAL_SuspendTick();` - stopping **SysTick interrupt**, which occurs every 1 ms by default, is used to maintain the system time base and support functions such as delays, timeout tracking, and periodic timing. If not stopped, it can wake MCU repeatedly and mess the system's logic.

`HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);` - this is line responsible for entering into the Stop mode, everything before in the current function was just preparation for that.
	- `PWR_LOWPOWERREGULATOR_ON` - to use the low-power regulator for power preservation.
	- `PWR_STOPENTRY_WFI` → entering stop mode using Wait For Interrupt

So at that point the system sleeps and waits for interrupt.

`HAL_ResumeTick();` - after wake-up, go back to the normal operation with SysTick enabled.

`SystemClock_Config();` - after waking-up from Stop mode, the clock needs to be reconfigured, since most of the clock were shut down. Without doing this - timing, delays, peripherals may work incorrectly.

```cpp title:"Blinking LED"
static void Blink_LD2(uint32_t times)
{
    for (uint32_t i = 0; i < times; i++)
    {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
        HAL_Delay(150);
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
        HAL_Delay(150);
    }
    HAL_Delay(400);
}
```

This function is straightforward and blink LED ***counter***-times.
