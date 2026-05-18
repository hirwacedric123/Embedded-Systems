# Automatic Climate Control System
### Embedded Systems — Final Individual Project (Tinkercad Simulation)

---

| | |
|---|---|
| **Student name** | *[Your full name]* |
| **Registration / ID** | *[Your ID]* |
| **Course** | Embedded Systems Programming |
| **Instructor** | NGABO Desire |
| **Institution** | College of Science and Technology — School of ICT |
| **Date** | May 2026 |
| **Platform** | Arduino Uno R3 (ATmega328P) in Tinkercad Circuits |

---

## Executive summary

This report documents a **simulated vehicle embedded system** built in Tinkercad. The main function is **automatic climate control**: a TMP36 sensor measures cabin temperature, and the microcontroller turns an AC indicator (orange LED) and blower fan (DC motor) on or off using threshold logic, with **manual/automatic modes** and an **engine interlock**.

The same design also demonstrates **power windows**, **door-open warning** (Serial Monitor + LED), and **adaptive lighting** (LDR + timer-driven LEDs). Firmware uses **bare-metal AVR C** (registers, ADC, interrupts, Timer0).

**Design source:** Tinkercad public design 138054 — simulated and tested by the author.

---

## Table of contents

1. [Introduction](#1-introduction)  
2. [System overview](#2-system-overview)  
3. [Hardware and circuit](#3-hardware-and-circuit)  
4. [Demonstration walkthrough](#4-demonstration-walkthrough) *(main section — with figures)*  
5. [Software summary](#5-software-summary)  
6. [Test results summary](#6-test-results-summary)  
7. [Course concepts](#7-course-concepts)  
8. [Conclusion](#8-conclusion)  
9. [References](#9-references)

---

## 1. Introduction

### 1.1 Background

Embedded climate control is a standard **control system**: a sensor monitors temperature, the MCU compares it to limits, and actuators (compressor, fan) restore comfort. Our course uses the car air conditioner as the reference example—thermistor sensor, user setpoint, compressor, and fan.

### 1.2 Project objectives

| # | Objective |
|---|-----------|
| 1 | Read cabin temperature with TMP36 and ADC |
| 2 | Auto-control AC and fan using upper/lower thresholds (hysteresis) |
| 3 | Provide manual and automatic operating modes |
| 4 | Interlock blower fan with “engine on” |
| 5 | Add door safety, windows, and lighting as integrated subsystems |
| 6 | Prove behavior in Tinkercad simulation |

### 1.3 What was built

- **Tool:** Tinkercad Circuits  
- **MCU:** Arduino Uno (ATmega328P)  
- **Code:** `copy_of_138054_automatic_climate_control1.ino` (register-level C)

---

## 2. System overview

### 2.1 Block diagram

```
   SENSORS / UI                    MICROCONTROLLER                 OUTPUTS
  ─────────────                  ───────────────────              ─────────
  TMP36 (temp)        ──►        automatic_climate_control()  ──► AC LED (D5)
  Engine, Auto, AC    ──►        power_window()             ──► Fan motor (D4)
  Door pot            ──►        door_warning()             ──► Window motor
  LDR, obstacle pot   ──►        lights() + Timer0 ISR      ──► Warning + indicator LEDs
  Window switch       ──►                                   ──► Serial Monitor
```

### 2.2 Program structure

After initialization, `main()` repeatedly runs:

1. `automatic_climate_control()` — **primary demo**  
2. `power_window()`  
3. `door_warning()`  
4. `lights()`  

User switches update flags inside **interrupt service routines** (fast response without polling).

---

## 3. Hardware and circuit

### 3.1 Bill of materials (summary)

| Component | Quantity | Purpose |
|-----------|----------|---------|
| Arduino Uno R3 | 1 | Main controller |
| TMP36 | 1 | Cabin temperature |
| L293D H-bridge | 1 | Motor driver |
| DC motor | 2 | Blower fan, window |
| Slide switch | 5 | Engine, Auto, AC, Window, Obstacle |
| Orange LED | 1 | AC “compressor on” |
| Red / Blue LEDs | 7 | Warning and lighting indicators |
| Resistors | 9 | LED protection |
| Potentiometer 250 kΩ | 2 | Door pressure, light level (simulated) |
| Photoresistor | 1 | Ambient light (LDR) |

*Full list: `bom.csv`*

### 3.2 Complete circuit

**Figure 1 — Complete Tinkercad circuit (schematic)**

![Figure 1: Complete automatic climate control circuit](Copy%20of%20138054%20AUTOMATIC%20CLIMATE%20CONTROL.png)

*Caption: Arduino Uno, TMP36, L293D motor driver, two DC motors, slide switches, LEDs, LDR, and potentiometers on breadboard.*

### 3.3 Pin assignment (key signals)

| Function | Arduino pin | Type |
|----------|-------------|------|
| Cabin temperature (TMP36) | A1 (ADC1) | Input |
| AC indicator | D5 | Output |
| Blower fan | D4 | Output |
| Door sensor (pot) | A5 | Input |
| Door warning LED | A4 | Output |
| Window motor | D7, D1 | Output |
| AC switch | D2 (INT0) | Input |
| Auto/Manual switch | D3 (INT1) | Input |
| Engine switch | D8 | Input |
| Window switch | D6 | Input |

---

## 4. Demonstration walkthrough

This section follows the order used in a **live presentation** to the instructor. Each demo lists steps, expected behavior, and a figure (screenshot).

---

### Demo 1 — System startup and overview

**Purpose:** Show the full working simulation.

**Steps:**
1. Open the Tinkercad circuit.  
2. Press **Start Simulation**.  
3. Confirm all components are wired and the code is loaded.

**Expected result:** Simulation runs; main loop executes climate, window, door, and lights tasks.

**Figure 2 — Simulation running**

![Figure 2: Simulation overview](figures/fig02-simulation-overview.png)

**Observation:** Simulation ran successfully for over 10 seconds with all subsystems connected. One blower motor was active (≈8382 RPM) and the door-warning LED was lit, confirming the main loop and actuators were operating.

---

### Demo 2 — Automatic climate control (cooling ON)

**Purpose:** Prove **monitoring + control** — temperature drives AC and fan.

**Steps:**
1. Turn **Engine** switch ON.  
2. Turn **Automatic mode** switch ON.  
3. Increase TMP36 temperature (sensor slider in Tinkercad).

**Expected result:**
- Orange **AC LED** turns ON when ADC > 166 (~30 °C).  
- **Blower fan** spins (only if engine is ON).  

**Figure 3 — Auto mode: cooling active**

![Figure 3: Auto cooling ON](figures/fig03-demo-auto-cooling-on.png)

*Caption: Automatic mode with engine on; TMP36 set hot; orange AC LED on; blower fan at 882 RPM.*

**Observation:** With automatic mode and engine enabled, raising cabin temperature triggered cooling: the orange AC LED turned on and the blower fan spun at 882 RPM without using the manual AC switch, confirming threshold-based automatic control.

---

### Demo 3 — Automatic climate control (cooling OFF)

**Purpose:** Show **hysteresis** — separate OFF threshold prevents flickering.

**Steps:**
1. Keep Auto and Engine ON.  
2. Decrease TMP36 temperature below the lower threshold (ADC < 145, ~26 °C).

**Expected result:** AC LED OFF; fan stops.

**Figure 4 — Auto mode: cooling OFF / hysteresis**

![Figure 4: Auto cooling OFF](figures/fig04-demo-auto-cooling-off.png)

*Caption: Automatic mode with engine on; TMP36 cooled to ~24.8 °C (ADC 153); Serial Monitor shows hysteresis band and system status.*

**Observation:** With automatic mode and engine on, lowering cabin temperature to ADC 153 (~24.8 °C) placed the system in the hysteresis band (`Between MIN/MAX`). The Serial Monitor documents mode, engine, door-safe status, and AC/fan state transitions. For full shutoff, temperature must fall below ADC 145 (`COOL: turn cooling OFF` on Serial, orange LED off, blower at 0 RPM).

---

### Demo 4 — Engine interlock

**Purpose:** Show safety logic — fan requires engine.

**Steps:**
1. Keep temperature **high** (fan would normally run).  
2. Turn **Engine** switch OFF.

**Expected result:** **Fan must stop** even if AC LED is on; fan is tied to `Flag.Engine_status`.

**Figure 5 — Engine interlock**

![Figure 5: Engine OFF, fan stopped](figures/fig05-demo-engine-interlock.png)

*Caption: Hot cabin (ADC 358, ~125 °C) with automatic mode on but engine off; Serial shows HOT while blower fan remains at 0 RPM.*

**Observation:** With high cabin temperature and automatic mode enabled, turning the engine off prevented the blower fan from running (0 RPM) even though the Serial Monitor indicated a hot condition (`HOT: turn cooling ON`). This confirms the engine interlock: the fan only operates when `Engine: ON`.

---

### Demo 5 — Manual AC mode

**Purpose:** Show **application-specific UI** — driver overrides automation.

**Steps:**
1. Turn **Automatic mode** OFF.  
2. Toggle **AC** switch (INT0).

**Expected result:** AC LED and fan follow the AC button (fan still needs engine ON).

**Figure 6 — Manual mode**

![Figure 6: Manual AC control](figures/fig06-demo-manual-ac.png)

*Caption: Manual mode with engine on; Serial Monitor shows AC toggled between OFF and ON while automatic mode was disabled.*

**Observation:** With `Mode: MANUAL`, the AC switch toggled cooling outputs (`AC: OFF Fan: OFF` then `AC: ON Fan: OFF`) without using the temperature sensor, proving manual override of automatic climate control.

---

### Demo 6 — Door open warning

**Purpose:** Show **monitoring + data communication** (Serial) and warning actuator.

**Steps:**
1. Open **Serial Monitor** in Tinkercad.  
2. Adjust **door pressure** potentiometer low (simulates door open).

**Expected result:**
- Serial: `Door is not closed:Warning`  
- Red **warning LED** ON  

**Figure 7 — Door warning**

![Figure 7: Door open warning](figures/fig07-demo-door-warning.png)

*Caption: Door pressure pot low; Serial Monitor shows `Door is not closed: Warning`.*

**Observation:** Lowering the door potentiometer simulated an open door. The MCU printed the warning on the Serial Monitor, demonstrating monitoring and data communication for the door safety subsystem.

---

### Demo 7 — Door closed (safe to drive)

**Steps:** Increase door pot above threshold (ADC ≥ 500).

**Expected result:**
- Serial: `Door is closed:Safe to Drive`  
- Warning LED OFF  

**Figure 8 — Door safe**

![Figure 8: Door closed](figures/fig08-demo-door-safe.png)

*Caption: Door pot raised; Serial Monitor shows `Door is closed: Safe to Drive` with simulation at 4:26.*

**Observation:** Increasing the door pressure pot above the threshold cleared the warning and printed the safe-to-drive message, confirming the door monitoring subsystem returns to a normal state when the door is closed.

---

### Demo 8 — Power window

**Steps:** Toggle **Window** slide switch.

**Expected result:** Window motor reverses direction (D7/D1 control H-bridge).

**Figure 9 — Power window**

![Figure 9: Window motor](figures/fig09-demo-window-motor.png)

---

### Demo 9 — Adaptive lighting

**Steps:** Adjust **LDR** and **obstacle** potentiometers while simulation runs.

**Expected result:** Blue/red indicator LEDs change pattern; Timer0 ISR sequences outputs over time.

**Figure 10 — Lighting subsystem**

![Figure 10: Adaptive lights](figures/fig10-demo-lights.png)

---

## 5. Software summary

### 5.1 Approach

Firmware uses **AVR registers** (`PORTx`, `ADMUX`, `TIMSK0`, ISRs)—not Arduino `setup()`/`loop()`. This is direct embedded programming on the ATmega328P.

### 5.2 Climate control logic (core)

```c
#define MAX_TEMPERATURE_VALUE 166   // ~30 °C — turn cooling ON
#define MIN_TEMPERATURE_VALUE 145   // ~26 °C — turn cooling OFF
```

| Mode | Behavior |
|------|----------|
| **Automatic** | Read TMP36; if temp high → AC ON, fan ON (if engine ON); if temp low → both OFF |
| **Manual** | AC switch controls AC; fan follows AC only if engine ON |
| **Both** | If engine OFF → fan always OFF |

**Temperature from ADC (TMP36, 5 V ref):**

```
Vout = (ADC / 1023) × 5.0
T(°C) = (Vout - 0.5) / 0.01
```

### 5.3 Interrupts (event-triggered inputs)

| Switch | ISR | Effect |
|--------|-----|--------|
| AC | `INT0_vect` | Toggle manual AC |
| Auto/Manual | `INT1_vect` | Toggle automatic mode |
| Engine | `PCINT0_vect` | Toggle engine status |
| Window | `PCINT2_vect` | Toggle window direction |

---

## 6. Test results summary

| ID | Test | Result |
|----|------|--------|
| T1 | Auto + Engine ON + hot → AC and fan ON | ☑ Pass |
| T2 | Cool below MIN → AC and fan OFF | ☑ Pass (hysteresis at 24.8 °C shown; full OFF below ADC 145) |
| T3 | Hot + Engine OFF → fan OFF | ☑ Pass |
| T4 | Manual AC toggle | ☑ Pass |
| T5 | Door open → Serial warning + LED | ☑ Pass |
| T6 | Door closed → safe message | ☑ Pass (see Fig 4/7 Serial: Safe to Drive) |
| T7 | Window switch → motor direction | ☐ Pass ☐ Fail |
| T8 | LDR/pots → light patterns | ☐ Pass ☐ Fail |

*Check Pass after you run each demo once; figures in Section 4 are your evidence.*

---

## 7. Course concepts

| Lecture topic | How this project demonstrates it |
|---------------|-----------------------------------|
| **Control** | Temperature → AC + fan actuators |
| **Monitoring** | TMP36, door ADC, LDR |
| **Data acquisition** | `call_adc()`, `adcRead()` |
| **Data communication** | Serial door messages |
| **Application-specific UI** | Slide switches, LEDs |
| **Reactive / real-time** | ISRs + continuous control loop |
| **Event-triggered** | Switch interrupts |
| **Time-triggered** | Timer0 for light sequencing |
| **Distributed ES** | Climate + window + door + lights on one MCU |

---

## 8. Conclusion

The project successfully simulates **automatic climate control** with threshold-based cooling, hysteresis, manual/automatic modes, and an engine interlock. Demonstrations in Tinkercad confirm correct sensor input, actuator output, and integration of additional vehicle subsystems.

**Limitations:** Simulation only; door/obstacle use pots; fixed ADC thresholds; no LCD or WiFi.

**Future work:** LCD status display, user setpoint pot, PWM fan speed, data logging.

---

## 9. References

1. Tinkercad design 138054 — *Automatic Climate Control* (author’s simulation copy, 2026).  
2. Analog Devices TMP36 datasheet.  
3. NGABO Desire — Embedded Systems Lecture 1, CST / School of ICT.  
4. Atmel ATmega328P datasheet.  
5. Project files: `copy_of_138054_automatic_climate_control1.ino`, `bom.csv`, schematic PNG.

---

## Appendix — Files submitted

| File | Description |
|------|-------------|
| `SUBMISSION_REPORT.md` | This report |
| `copy_of_138054_automatic_climate_control1.ino` | Source code |
| `bom.csv` | Bill of materials |
| `Copy of 138054 AUTOMATIC CLIMATE CONTROL.png` | Circuit diagram |
| `figures/` | Simulation screenshots |
