# Viva / Demo Script — Automatic Climate Control

Use this 5–8 minute script when presenting or defending the project. Practice toggling switches in Tinkercad while speaking.

---

## Before you start

1. Open your Tinkercad circuit and press **Start Simulation**.  
2. Open **Serial Monitor** (door messages).  
3. Know switch names: **Engine**, **Automatic mode**, **AC**, **Window**, **Obstacle** (if used for lights).

---

## Opening (30 seconds)

> "My project is an automatic climate control system simulated on Arduino Uno in Tinkercad. It reads cabin temperature with a TMP36 sensor and controls an AC indicator and a blower fan. It also includes vehicle subsystems: power windows, door-open warning, and adaptive lighting. The code uses bare-metal AVR C with ADC, timers, and interrupts."

---

## Part 1 — Climate control in automatic mode (2 minutes)

**Setup:** Engine switch **ON**, Automatic mode **ON**, AC switch irrelevant.

**Action:** Increase TMP36 temperature (Tinkercad slider or heat the sensor).

**Say:**

> "In automatic mode, the MCU reads the TMP36 on ADC channel 1. When the ADC value exceeds 166 — about 30 degrees Celsius — the AC output on pin D5 turns on the orange LED, representing the compressor. If the engine is on, pin D4 also runs the blower fan through the L293D motor driver."

**Action:** Lower temperature until ADC would fall below 145 (~26 °C).

**Say:**

> "When temperature drops below 145, both AC and fan turn off. Between 145 and 166 we keep the previous state — that is hysteresis, which stops the system from flickering on and off at one threshold."

**If asked:** "Why engine interlock?"

> "The fan only runs when `Flag.Engine_status` is ON. In a real car the blower is not powered when the engine is off. AC LED can be on in auto mode, but PORTD4 is always cleared if the engine is off."

**Demo check:** Engine OFF while hot → fan must not spin.

---

## Part 2 — Manual mode (1 minute)

**Setup:** Toggle **Automatic mode** off (INT1 on D3).

**Action:** Toggle **AC switch** (INT0 on D2).

**Say:**

> "In manual mode, temperature is ignored. The AC button toggles the compressor LED and fan, but the fan still requires the engine to be on. This matches manual AC in a vehicle."

---

## Part 3 — Door warning (1 minute)

**Action:** Turn door pressure pot low (simulate open door).

**Say:**

> "`door_warning()` reads ADC channel 5 on pin A5. If the value is below 500, the firmware prints a warning on Serial and turns on the red warning LED on pin C4. Above 500 it prints that the door is closed and it is safe to drive."

**Point at Serial Monitor** while speaking.

---

## Part 4 — Window and lights (1 minute)

**Window:** Toggle window switch.

> "`power_window()` sets D7 high and D1 low for UP, or the opposite for DOWN, driving the window motor through the H-bridge."

**Lights:** Move LDR / obstacle pots.

> "The lighting subsystem reads ADC0 for the photoresistor and ADC1 for the obstacle pot. Timer0 overflow interrupt cycles flags so indicator LEDs on PORTB and PORTD show different patterns — that is time-triggered behavior combined with sensor input."

---

## Part 5 — Software architecture (1 minute)

**Say:**

> "The main loop calls four functions every iteration: climate, window, door, lights. Switch changes use interrupts — INT0 for AC, INT1 for auto mode, pin change for engine and window — so the CPU responds to events without polling every switch every loop. Climate control is the primary function; the others show a distributed embedded system on one board."

---

## Likely examiner questions — short answers

| Question | Answer |
|----------|--------|
| What sensor do you use? | TMP36 analog temperature sensor on ADC1 (A1). |
| What are 166 and 145? | ADC thresholds; ~30 °C ON, ~26 °C OFF with 5 V ref and TMP36 formula. |
| What is hysteresis? | Different ON and OFF thresholds to avoid chattering. |
| What drives the motors? | L293D H-bridge; logic from D4, D7, D1. |
| Why bare AVR code? | Direct register access to PORT, ADMUX, timers — typical embedded C. |
| What is `SET_BIT` / `CLR_BIT`? | Macros to set or clear one bit in a port register. |
| Event vs time triggered? | Switches = events (ISRs); Timer0 = time-triggered light sequence. |
| How does this relate to lecture AC example? | Sensor measures room/cabin temp; MCU controls compressor and fan actuators. |

---

## ADC → Celsius — explain in 20 seconds

> "TMP36 outputs 500 mV at 0 °C plus 10 mV per degree. With 5 V reference, ADC equals Vout divided by 5 times 1023. For example ADC 166 is about 0.81 V, which is roughly 31 °C. We used constants 166 and 145 instead of floating-point in the loop for speed and simplicity."

**Formulas (write on board if needed):**

```
Vout = (ADC / 1023) × 5.0
T(°C) = (Vout - 0.5) / 0.01
```

---

## Closing (20 seconds)

> "The project demonstrates monitoring and control from the course: temperature acquisition, threshold actuation, user interface with switches, real-time interrupts, and multiple subsystems on one microcontroller, all validated in Tinkercad simulation."

---

## Screenshot checklist for report

- [ ] Full circuit (PNG in project folder)  
- [ ] Simulation: hot + fan running  
- [ ] Simulation: cool + AC off  
- [ ] Serial Monitor door messages  
- [ ] Switch labels visible (optional)
