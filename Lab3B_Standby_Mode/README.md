**Course:** Microcontrollers: Interface & Programming (ENCE-4709 - 20908)
**Instructor:** Dr. Alexz Farrall  
**Lab Number & Title:**  Lab 3B - Standby Mode
**Student Name:** Sardar Ziyatkhanov  
**Student ID:** P000017499  
**Program / Section:** BSCE26  
**Date:** March, 2026

---

## Lab Requirements 

In the second phase, reconfigure the system to enter Standby mode instead, using the dedicated wake-up mechanism to bring the MCU out of low power; on wake, the program should start from reset and indicate via a distinct LED pattern that it has restarted, allowing you to observe that memory and execution state are lost.
You must provide evidence for both modes by demonstrating the different wake behaviors, LED outputs, and variable retention or loss, and by briefly explaining how these observations confirm the characteristics of Stop versus Standby operation.

## Power Preservation Modes

After working with Sleep and Stop modes, the only mode left is Standby mode. It is the mode, giving maximum power preservation, at the cost of volatile memory. It basically shuts down 1.2 V internal voltage which powers CPU and some peripherals. Only some circuitry remains active, which includes PWR (**Power Control Block**). Since PWR is the one, monitoring specific wake-up sources, we are still able to exit the mode by interrupt
## CubeMX Configuration 

PA5 is an output for LED. PA0 is enabled as input (`GPIO_Input`) which is external button, sending triggering interrupt - internal button is not usable in standby mode since the button as a peripheral does not work as well.

![image](images/Pasted%20image%2020260320202533.png)

## Code Explanation

```cpp
if (__HAL_PWR_GET_FLAG(PWR_FLAG_SB) != RESET) {
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB);

    for (int i = 0; i < 10; i++) {
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
        HAL_Delay(100);
    }
} else {
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
    HAL_Delay(2000);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
}
```

Here in this code, the standby behavior is shown.

`if (__HAL_PWR_GET_FLAG(PWR_FLAG_SB) != RESET)` - check is standby flag is set, if yes - wake-up from Standby, not from power on.

`__HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB);` - clear standby flag.

```cpp
    for (int i = 0; i < 10; i++) {
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
        HAL_Delay(100);
    }
```
blinking LED for 10 times indicating wake up from standby.

```cpp
else {
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
    HAL_Delay(2000);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
```

if standby flag is not set, this means normal startup and turn the LED for 2 seconds, indicating that

```cpp title:"main loop"
while (1)
{
    HAL_Delay(1000);

    standbyCounter++;

    Blink_Count(standbyCounter);

    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);

    HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN1);
    HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1);

    HAL_PWR_EnterSTANDBYMode();
}
```

`HAL_Delay(1000);` gives some time to  observe the startup pattern with the counter display separately.
`standbyCounter++;` increments the counter variable stored in SRAM (it is zero, since doesn't save).
`Blink_Count(standbyCounter);` blink counter times
`__HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);` clears the wake-up flag before entering Standby again   
`HAL_PWR_DisableWakeUpPin(...)` followed by `HAL_PWR_EnableWakeUpPin(...)` helps avoid false wake detection on PA0, since edge-sensitive  
`HAL_PWR_EnterSTANDBYMode();` places MCU in Standby. After wakeup code doe not continue from the next line; instead, MCU restarts from beginning

```cpp title:"LED blink"
static void Blink_Count(uint32_t count)
{
    for (uint32_t i = 0; i < count; i++)
    {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
        HAL_Delay(150);
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
        HAL_Delay(150);
    }
    HAL_Delay(400);
}
```

LED blink function with count value that will never increment more than 1.

Overall:
- when normal reset LED will stay on for 2 seconds
- after wake up from standby, quickly strobe 10 times
