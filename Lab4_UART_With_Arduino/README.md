**Course:** Microcontrollers: Interface & Programming (ENCE-4709 - 20908)
**Instructor:** Dr. Alexz Farrall  
**Lab Number & Title:**  Lab 4 - UART
**Student Name:** Sardar Ziyatkhanov  
**Student ID:** P000017499  
**Program / Section:** BSCE26  
**Date:** March, 2026

---

## Lab Requirements

Using STM32CubeIDE on the Nucleo-F401RE, configure a UART link to an Arduino (Arduino code written in the Arduino IDE) where the Arduino counts pushbutton presses.
The STM32 must remain asleep most of the time and, every 10 seconds, wake via a periodic interrupt, request the latest count from the Arduino over UART, log the received count together with a timestamp (RTC time or a seconds counter) to a storage method you can demonstrate (serial log to the PC is acceptable), then return to sleep until the next wake event.
For example: STM32 prints timestamp, count lines to the same UART that is connected to your PC terminal. You copy or capture the terminal output to a .csv file.

## System Overview 

In this lab STM works in junction with Arduino, both communication using UART serial communication protocol. The system looks like this:

PC <- STM <- Arduino Uno <- Button
![image](images/Pasted%20image%2020260320210020.png)
UART communication is a 2-wire asynchronous communication protocol using two lines: Tx (transmit) and Rx (receive). When two devices are connected to each other using UART, they are connected in a cross pattern where $Tx_1 -> Rx_2$  and $Rx_1 -> Tx_2$ 

Overall system works as follows:
- Arduino waits for button press, increments counter on each press
- STM sleeps in Stop
- Every 10 seconds STM wakes up from RTC wake-up timer
- STM clears the wake flag, updates its timestamp
- STM sends char `?` to Arduino through **USART1**
- Arduino gets this requests, copies the couter value (button press) and sends to STM
- STM receives that message
- STM formats the data as CSV line: timestamp, count
- STM sends CSV line to PC through **USART2**
- STM returns to Stop mode

Baudrate must match everywhere: PC, Arduino, STM - key for clear UART communication 

## CubeMX Configuration

Configured PA9 and PA10 for USART1 transmit/receive
![image](images/Pasted%20image%2020260320213719.png)

PA2 and PA3 for USART2 transmit/receive
![image](images/Pasted%20image%2020260320213728.png)


### Why RTC not Stop mode

RTC wakeup since we need to wake up only after each 10 seconds - fast timers are redundant.
Stop mode is a good power preserver, along with the feature of keeping the memory which will keep the timestamp in SRAM.
![image](images/Pasted%20image%2020260320213949.png)
## Codes Explanation

### STM main loop

```cpp
RTC_SetWakeup_10s();
rtc_woke_flag = 1;

while (1)
{
    if (rtc_woke_flag)
    {
        rtc_woke_flag = 0;
        timestamp_s += 10;

        memset(arduino_line, 0, sizeof(arduino_line));
        Arduino_RequestCount(arduino_line, sizeof(arduino_line));
        PC_LogCSV(timestamp_s, arduino_line);
    }

    EnterStopMode();
    ReconfigClockAfterStop();
}
```

wakes up, sets the RTC wakeup flag, since it is set, increments the timestamp by 10, resets the RTC wakeup flag, clears the buffer before receiving data from arduino, performs UART exchange with arduino, and formats final message for the PC. Enters Stop mode and stays there.

### RTC Wakeup Configuration

```cpp
static void RTC_SetWakeup_10s(void)
{
    if (HAL_RTCEx_DeactivateWakeUpTimer(&hrtc) != HAL_OK)
    {
        Error_Handler();
    }

    if (HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, 10 - 1, RTC_WAKEUPCLOCK_CK_SPRE_16BITS) != HAL_OK)
    {
        Error_Handler();
    }
}
```
- function, first, turns off the RTC wakeup timer, passes the address of the RTC and checks for error, if ok, continues.
- then sets RTC wakeup timer, enables interrupt mode. 10-1 since counts from 0.

### Request to Arduino

```cpp
static void Arduino_RequestCount(char *out, uint16_t outSize)
{
    uint8_t q = '?';

    if (HAL_UART_Transmit(&huart1, &q, 1, 100) != HAL_OK)
    {
        strncpy(out, "TX_ERR", outSize);
        out[outSize - 1] = '\0';
        return;
    }

    if (!UART_ReadLine(&huart1, out, outSize, 500))
    {
        strncpy(out, "RX_ERR", outSize);
        out[outSize - 1] = '\0';
    }
}
```
- using UARTS1 send 1-bit character q (which is ?) with 100ms-timeout if ok, otherwise  copies transmission error message to the output buffer, terminates string properly and exits.
- still using USARTS1 read one text line from Arduino with timeout of 500ms.

### Reading UART line of STM

```cpp
static uint8_t UART_ReadLine(UART_HandleTypeDef *huart, char *buf, uint16_t maxLen, uint32_t timeout_ms)
{
    uint16_t i = 0;
    uint8_t ch = 0;

    while (i < (maxLen - 1))
    {
        if (HAL_UART_Receive(huart, &ch, 1, timeout_ms) != HAL_OK)
        {
            buf[0] = '\0';
            return 0;
        }

        if (ch == '\n') break;
        if (ch == '\r') continue;

        buf[i++] = (char)ch;
    }

    buf[i] = '\0';
    return 1;
}
```

- `i` tracks the buffer position and `ch` stores one received byte.
- `while (i < (maxLen - 1))` keeps receiving until buffer is almost full.
- `HAL_UART_Receive(huart, &ch, 1, timeout_ms)` receive 1 byte (8 bits) from UART
- `if (ch == '\n') break;` to stop reading when new line is reached.
- ` if (ch == '\r') continue;` ignore carriage return
- ` if (ch == '\r') continue;` to store normal char into buffer and then move to the next position
- `return 1;` to report successful reception.

### Sending CSV line to PC

```cpp
static void PC_LogCSV(uint32_t t_s, const char *countStr)
{
    char line[64];
    int n = snprintf(line, sizeof(line), "%lu,%s\r\n",
                     (unsigned long)t_s,
                     (countStr && countStr[0]) ? countStr : "EMPTY");

    HAL_UART_Transmit(&huart2, (uint8_t*)line, (uint16_t)n, 200);
}
```

As soon as STM has the timestamp and received count string from arduino in formats them and sends to PC using USART2

![image](images/Pasted%20image%2020260320213208.png)
### Arduino setup

```cpp
void setup() {
    pinMode(BTN_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(BTN_PIN), isrButton, FALLING);

    Serial.begin(115200);
    delay(50);
}
```

Setting the button mode, declaring ISR on falling edge, setting baud rate.

```cpp
const unsigned long DEBOUNCE_MS = 50;
volatile unsigned long lastIsrTime = 0;

void isrButton() {
    unsigned long now = millis();
    if (now - lastIsrTime >= DEBOUNCE_MS) {
        pressCount++;
        lastIsrTime = now;
    }
}
```

setting debounce delay for button, interrupt time, and ISR which increments counter with each button press.

### Arduino Reply to STM

```cpp
void loop() {
    if (Serial.available() > 0) {
        char c = (char)Serial.read();

        if (c == '?') {
            noInterrupts();
            unsigned long countCopy = pressCount;
            interrupts();

            Serial.println(countCopy);
        }
    }
}
```
if serial is not empty, and if character `?` is detected, reset interrupt, record counter value, interrupt, send the value to serial. 
