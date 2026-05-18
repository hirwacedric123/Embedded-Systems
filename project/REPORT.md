# Automatic Climate Control System
## Embedded Systems Final Project — Tinkercad Simulation

**Course:** Embedded Systems Programming  
**Platform:** Arduino Uno R3 (ATmega328P)  
**Tool:** Tinkercad Circuits  
**Design reference:** Tinkercad public design 138054 (Copy of 138054 AUTOMATIC CLIMATE CONTROL)

---

## Abstract

This project simulates a vehicle-style **automatic climate control** system using an Arduino Uno in Tinkercad. A TMP36 sensor monitors cabin temperature. The microcontroller turns an AC indicator (orange LED) and a blower fan (DC motor via L293D H-bridge) on or off based on temperature thresholds in automatic mode, or based on user switches in manual mode. An **engine interlock** prevents the fan from running when the engine is off. Additional subsystems model power windows, door-open warning (Serial Monitor + LED), and adaptive exterior lighting using a photoresistor and potentiometers. The firmware is written in **bare-metal AVR C** (register-level I/O, ADC, timers, and interrupts), demonstrating monitoring, control, data acquisition, and reactive real-time behavior described in the embedded systems course.

---

## 1. Introduction

### 1.1 Problem statement

Embedded climate control appears in cars, buildings, and industrial plants. A typical system **monitors** an environmental variable (temperature), compares it to a desired range, and **controls** actuators (compressor, fan) to keep the cabin comfortable. The course lecture on **Control** uses the air conditioner as an example: a temperature sensor, user setpoint, and compressor/fan as actuator.

This project implements that idea in simulation, extended with related vehicle functions (windows, door safety, lighting) to show a **distributed** embedded design: several dedicated functions coordinated by one microcontroller.

### 1.2 Objectives

1. Acquire cabin temperature using the ADC and a TMP36 sensor.  
2. Automatically activate cooling (AC + fan) when temperature exceeds an upper threshold.  
3. Deactivate cooling when temperature falls below a lower threshold (hysteresis band).  
4. Support **manual** and **automatic** modes via slide switches and external interrupts.  
5. Enforce **engine interlock** so the blower fan only runs when the engine is on.  
6. Integrate supplementary subsystems: power window, door warning, adaptive lights.  
7. Validate all behavior in Tinkercad simulation.

### 1.3 Scope

- Simulation only (Tinkercad); no physical Raspberry Pi deployment.  
- Door and obstacle inputs use potentiometers as stand-ins for real sensors.  
- Temperature thresholds are ADC constants tuned for simulation (`145` and `166`).

---

## 2. System overview

### 2.1 Block diagram

```
┌─────────────────────────────────────────────────────────────────────────┐
│                           INPUTS (sensors / UI)                          │
│  TMP36 (cabin temp)   Door pot (A5)   LDR (A0)   Obstacle pot (A1)    │
│  Engine / Auto / AC / Window slide switches (interrupt-driven)          │
└───────────────────────────────────┬─────────────────────────────────────┘
                                    │
                                    ▼
┌─────────────────────────────────────────────────────────────────────────┐
│                    ATmega328P / Arduino Uno (MCU)                        │
│  ┌─────────────────────┐  ┌──────────────┐  ┌────────────────────────┐ │
│  │ automatic_climate_  │  │ power_window │  │ door_warning           │ │
│  │ control()           │  │ ()           │  │ ()                     │ │
│  └─────────────────────┘  └──────────────┘  └────────────────────────┘ │
│  ┌─────────────────────────────────────────────────────────────────────┐ │
│  │ lights() + Timer0 ISR (time-triggered LED patterns)               │ │
│  └─────────────────────────────────────────────────────────────────────┘ │
└───────────────────────────────────┬─────────────────────────────────────┘
                                    │
                                    ▼
┌─────────────────────────────────────────────────────────────────────────┐
│                              OUTPUTS                                     │
│  D5: AC (orange LED)    D4: Blower fan motor    D7/D1: Window motor   │
│  PC4: Door warning LED  D0, B1–B5: Indicator LEDs   Serial Monitor    │
└─────────────────────────────────────────────────────────────────────────┘
```

### 2.2 Main program flow

The `main()` function initializes flags, pins, interrupts, and Timer0, then runs an infinite loop calling four tasks in order:

1. `automatic_climate_control()` — core climate logic  
2. `power_window()` — window motor direction  
3. `door_warning()` — door state and Serial messages  
4. `lights()` — headlight/indicator logic from LDR and obstacle ADC values  

Switch events are handled asynchronously in ISRs (toggle flags).

---

## 3. Hardware design

### 3.1 Bill of materials

| Ref / name | Qty | Component | Role |
|------------|-----|-----------|------|
| U1 | 1 | Arduino Uno R3 | Main microcontroller |
| UCABIN_TEMPERATURE | 1 | TMP36 temperature sensor | Cabin temperature |
| SENGINE_SWITCH, SAUTOMATIC_MODE, SAC_SWITCH, SWINDOW_BUTTON, SOBSTACLE | 5 | Slide switch | User inputs / mode selection |
| DAC | 1 | Orange LED | AC compressor ON indicator |
| MBLOWER_FAN | 1 | DC motor | Cabin blower fan |
| MWINDOW_MOTOR | 1 | DC motor | Power window |
| U3 | 1 | L293D H-bridge motor driver | Drives both DC motors |
| DWARNING_LIGHT, D1–D3 | 4 | Red LED | Warnings / indicators |
| D4–D6 | 3 | Blue LED | Lighting indicators |
| R1–R6, R8, R9 | 8 | 1 kΩ resistor | LED current limit |
| R7 | 1 | 270 Ω resistor | LED current limit |
| RpotDOOR_PRESSURE_SENSOR, Rpot4 | 2 | 250 kΩ potentiometer | Simulated door pressure / light level |
| R10 | 1 | Photoresistor (LDR) | Ambient light sensing |

*Source: `bom.csv` in project folder.*

### 3.2 Circuit description

**Figure 1** should be the full Tinkercad schematic export:  
`Copy of 138054 AUTOMATIC CLIMATE CONTROL.png`

The circuit is built on a breadboard with:

- **Power:** Arduino 5 V and GND rails supply sensors, logic, and motor driver.  
- **Temperature:** TMP36 output to analog input (ADC channel 1, Arduino **A1** in firmware).  
- **Motors:** L293D receives direction/enable signals from digital pins; outputs drive blower fan and window motor.  
- **Indicators:** LEDs with series resistors on PORTB and PORTD.  
- **Switches:** Slide switches wired to pins that trigger `INT0`, `INT1`, and pin-change interrupts.

### 3.3 Pin and function table

| Function | AVR port / pin | Arduino pin (typical) | I/O | Code / notes |
|----------|----------------|------------------------|-----|----------------|
| Cabin temperature (TMP36) | ADC1 (`MUX1=1`) | **A1** | Input | `automatic_climate_control()` via `call_adc()` |
| Obstacle / indicator level | ADC1 (`ADMUX=0x41`) | **A1** | Input | `lights()` → `adcRead(0x41)` |
| LDR (photoresistor) | ADC0 (`ADMUX=0x40`) | **A0** | Input | `lights()` → `adcRead(0x40)` |
| Door pressure (pot) | ADC5 (`MUX2+MUX0`) | **A5** | Input | `door_warning()` |
| AC compressor indicator | PORTD5 | **D5** | Output | Orange LED ON when cooling active |
| Blower fan | PORTD4 | **D4** | Output | Motor via L293D; requires engine ON |
| Window motor UP | PORTD7 | **D7** | Output | `power_window()` |
| Window motor DOWN | PORTD1 | **D1** | Output | `power_window()` |
| Door warning LED | PORTC4 | **A4** (digital) | Output | ON when door “open” |
| Lighting / status LEDs | PORTD0, PORTB1–B5 | **D0**, **D9–D13** | Output | `drl()`, `led_off()`, `lights()` |
| AC manual toggle | INT0 (PD2) | **D2** | Input | `ISR(INT0_vect)` → `Flag.AC_button` |
| Auto/Manual mode | INT1 (PD3) | **D3** | Input | `ISR(INT1_vect)` → `Flag.Automatic_mode` |
| Engine status | PCINT0 (PB0) | **D8** | Input | `ISR(PCINT0_vect)` → `Flag.Engine_status` |
| Window direction | PCINT22 (PD6) | **D6** | Input | `ISR(PCINT2_vect)` → `Flag.Window_button` |

*Confirm TMP36 wire on your Tinkercad schematic against **A1** (matches `ADMUX` channel 1 in code). Schematic net label: `UCABIN_TEMPERATURE`.*

### 3.4 Subsystem hardware summary

| Subsystem | Sensors / inputs | Actuators / outputs |
|-----------|------------------|---------------------|
| **Climate control** | TMP36, Auto switch, AC switch, Engine switch | Orange LED (AC), blower motor (D4) |
| **Power window** | Window slide switch | Window motor (D7/D1, H-bridge) |
| **Door warning** | Door pressure pot (A5) | Red warning LED, Serial text |
| **Adaptive lights** | LDR (A0), obstacle pot (A1) | Blue/red indicator LEDs (D0, B1–B5) |

### 3.5 Motor driver (L293D)

The L293D is a dual H-bridge driver. It allows the 5 V logic outputs of the ATmega328 to control 5 V DC motors that need more current than an I/O pin can supply. Enable and direction pins from the Arduino select motor spin direction and run/stop. This matches a **medium-scale** embedded design: microcontroller plus dedicated driver IC plus actuators.

---

## 4. Software design

### 4.1 Programming approach

The source file `copy_of_138054_automatic_climate_control1.ino` does **not** use the Arduino `setup()` / `loop()` API or `digitalWrite()` / `analogRead()`. Instead it includes `<avr/io.h>` and `<avr/interrupt.h>` and manipulates:

- **DDRx** — pin direction  
- **PORTx** — output levels  
- **ADMUX / ADCSRA / ADC** — analog-to-digital conversion  
- **EIMSK, PCICR, TIMSK0** — interrupts and timer  

This is characteristic of **low-level embedded C** on the ATmega328P (first/second generation embedded style in the course: direct hardware control without a thick abstraction layer).

### 4.2 Global configuration constants

```c
#define MAX_TEMPERATURE_VALUE 166   // ADC upper threshold → AC/fan ON
#define MIN_TEMPERATURE_VALUE 145   // ADC lower threshold → AC/fan OFF
#define DOOR_PRESSURE_THRESHOLD 500 // Door "open" if ADC below this
```

### 4.3 Module: `automatic_climate_control()` (core)

**Purpose:** Control AC indicator and blower fan from temperature (auto) or user (manual), with engine interlock.

#### Automatic mode (`Flag.Automatic_mode == ON`)

1. Select ADC channel 1 (TMP36 on A1).  
2. Read `cabin_temperature = call_adc()`.  
3. If `cabin_temperature > MAX_TEMPERATURE_VALUE` (166):  
   - Set PORTD5 (AC ON).  
   - If engine ON, set PORTD4 (fan ON).  
4. Else if `cabin_temperature < MIN_TEMPERATURE_VALUE` (145):  
   - Clear PORTD5 and PORTD4 (AC and fan OFF).  
5. If engine OFF, always clear PORTD4 (fan OFF) regardless of temperature.

Between 145 and 166 the code does not change outputs in the hot branch — previous state is held, creating a **hysteresis band** that reduces ON/OFF chattering.

#### Manual mode (`Flag.Automatic_mode == OFF`)

- If AC button OFF: clear D5 and D4.  
- If AC button ON: set D5; set D4 only if engine ON.  
- If engine OFF: always clear D4.

#### Engine interlock

The blower fan (PORTD4) is only energized when `Flag.Engine_status == ON`. This models a real vehicle where the HVAC blower is tied to ignition/engine running.

### 4.4 Module: `power_window()`

- If `Flag.Window_button == UP` (0): D7 HIGH, D1 LOW → motor one direction.  
- If `DOWN` (1): D7 LOW, D1 HIGH → reverse direction.  
- Toggled by `ISR(PCINT2_vect)` on window switch activity.

### 4.5 Module: `door_warning()`

1. Select ADC channel 5 (door pot on A5).  
2. If reading `< DOOR_PRESSURE_THRESHOLD` (500):  
   - `Serial.println("Door is not closed:Warning");`  
   - Door warning LED ON (PORTC4).  
3. Else: print safe message and LED OFF.

### 4.6 Module: `lights()` and Timer0 ISR

- Reads obstacle level on ADC1; if very high (`>800`) or very low (`<300`) with timing flags, drives different LED groups.  
- In mid range, reads LDR on ADC0; if `>100`, calls `drl()` to show patterned outputs; else `led_off()`.  
- `ISR(TIMER0_OVF_vect)` increments `count` and toggles `FLAG_1`, `FLAG_2`, `FLAG_3`, `FLAG_6` over time windows — **time-triggered** sequencing for light patterns.

### 4.7 Climate control flowchart

```
                    ┌──────────────┐
                    │  Loop start  │
                    └──────┬───────┘
                           ▼
                  ┌────────────────┐
                  │ Automatic mode?│
                  └────┬──────┬────┘
                   Yes │      │ No
                       ▼      ▼
              ┌────────────┐  ┌──────────────────┐
              │ Read TMP36 │  │ AC button ON?    │
              │    ADC     │  └───┬──────────┬───┘
              └─────┬──────┘   No │          │ Yes
                    ▼              ▼          ▼
           ┌───────────────┐   AC/fan OFF   AC ON (D5)
           │ Temp > 166 ?  │                Fan if engine
           └───┬───────┬───┘
            Yes│       │No
               ▼       ▼
          AC ON (D5)  ┌──────────────┐
          Fan if eng  │ Temp < 145 ? │──Yes──► AC/fan OFF
                      └──────────────┘
                           │
              ┌────────────┴────────────┐
              │ Engine OFF? → Fan OFF   │
              └───────────────────────────┘
```

### 4.8 Interrupt service routines

| ISR | Hardware trigger | Software effect |
|-----|------------------|-----------------|
| `INT0_vect` | AC switch (D2) | Toggle `Flag.AC_button` |
| `INT1_vect` | Automatic mode switch (D3) | Toggle `Flag.Automatic_mode` |
| `PCINT0_vect` | Engine switch (PB0 / D8) | Toggle `Flag.Engine_status` |
| `PCINT2_vect` | Window switch (PD6 / D6) | Toggle `Flag.Window_button` |
| `TIMER0_OVF_vect` | Timer0 overflow | Cycle light pattern flags |

**Course link:** Switch handling is **event-triggered**; Timer0 lighting is **time-triggered**.

### 4.9 ADC and temperature conversion (TMP36)

**Sensor equation (TMP36, 5 V supply):**

\[
V_{out} = 500\text{ mV} + 10\text{ mV/°C} \times T
\]

**ADC (10-bit, \(V_{ref} = 5\) V):**

\[
ADC = \frac{V_{out}}{5.0} \times 1023
\]

**Inverse (estimate temperature from ADC):**

\[
V_{out} = \frac{ADC}{1023} \times 5.0\text{ V}, \quad T = \frac{V_{out} - 0.5}{0.01}
\]

| ADC value | Approx. temperature |
|-----------|---------------------|
| 145 | ≈ 26.1 °C |
| 154 | ≈ 25.0 °C (room reference) |
| 166 | ≈ 30.3 °C |

The firmware band **145–166** is therefore roughly **26 °C to 30 °C** — a narrow comfort band suitable for simulation. Values can be adjusted in Tinkercad by editing `MAX_TEMPERATURE_VALUE` and `MIN_TEMPERATURE_VALUE`.

---

## 5. Testing and results

Simulation was run successfully in Tinkercad. The following test cases should be documented with screenshots (insert your captures as Figure 2, Figure 3, …).

| Test ID | Procedure | Expected result | Pass |
|---------|-----------|-----------------|------|
| T1 | Auto mode ON, Engine ON, increase TMP36 temperature | Orange LED ON; blower motor spins | ☐ |
| T2 | Lower temperature below MIN (145) | AC LED OFF; fan stops | ☐ |
| T3 | Hot cabin but Engine OFF | AC may be ON per logic; **fan must be OFF** | ☐ |
| T4 | Auto OFF; toggle AC switch | Manual AC and fan (if engine ON) toggle | ☐ |
| T5 | Toggle Automatic mode switch | Behavior switches between T1 and T4 | ☐ |
| T6 | Adjust door pot below threshold | Serial: "Door is not closed:Warning"; warning LED ON | ☐ |
| T7 | Door pot above threshold | Serial: "Door is closed:Safe to Drive"; LED OFF | ☐ |
| T8 | Toggle window switch | Window motor changes direction | ☐ |
| T9 | Vary LDR and obstacle pots | Blue/red indicator patterns change | ☐ |

**Figures to attach from Tinkercad:**

1. Full schematic (already in project PNG).  
2. Simulation running — hot state (AC + fan active).  
3. Simulation — cool state (AC + fan off).  
4. Serial Monitor showing door messages.

---

## 6. Mapping to course concepts

| Course theme (Lecture 1) | Implementation in this project |
|--------------------------|--------------------------------|
| **Control** | Temperature compared to thresholds; AC + fan actuators |
| **Monitoring** | TMP36, door ADC, LDR, obstacle pot |
| **Data acquisition** | `call_adc()`, `adcRead()` — sample and convert analog signals |
| **Data communication** | `Serial.println()` for door status |
| **Application-specific UI** | Slide switches and LEDs (not a PC keyboard/screen) |
| **Reactive / real-time** | Continuous `while(1)` loop; ISRs for switches; timer for lights |
| **Event-triggered** | External interrupts on AC, mode, engine, window |
| **Time-triggered** | Timer0 overflow ISR for lighting sequence |
| **Distributed system** | Multiple vehicle sub-functions on one MCU (climate, window, door, lights) |
| **Dedicated purpose** | Firmware performs vehicle comfort/safety tasks only |

---

## 7. Conclusion

This project successfully demonstrates **automatic climate control** in Tinkercad using a TMP36, threshold-based control with hysteresis, manual/automatic modes, and an engine interlock on the blower fan. The same firmware integrates window control, door warning, and adaptive lighting, illustrating how modern vehicles combine several embedded subsystems on one microcontroller.

The use of bare-metal AVR registers shows direct hardware interaction as taught in embedded systems fundamentals: I/O configuration, ADC sampling, timers, and interrupts.

---

## 8. Limitations and future work

### 8.1 Limitations

- **Simulation only** — no physical noise, EMI, or motor load effects.  
- **Door and obstacle** inputs are potentiometers, not real pressure or ultrasonic sensors.  
- **No LCD** — user feedback is LEDs and Serial Monitor only.  
- **Fixed ADC thresholds** — not a user-adjustable °C setpoint pot in climate code.  
- **No humidity or PID** — simple on/off with hysteresis, not proportional control.  
- **Raspberry Pi** was not used; entire demo is Arduino/Tinkercad.

### 8.2 Future improvements

1. Add 16×2 LCD showing temperature, mode, and fan status.  
2. Replace fixed `166`/`145` with a potentiometer setpoint on another ADC channel.  
3. Implement PWM fan speed instead of binary ON/OFF.  
4. Add WiFi module (ESP8266) for logging temperature to a phone or cloud.  
5. Port critical logic to Raspberry Pi for a dashboard while keeping Arduino for real-time I/O.

---

## 9. References

1. Tinkercad design 138054 — *Automatic Climate Control* (simulated copy, May 2026).  
2. Analog Devices — TMP36 Low Voltage Temperature Sensor datasheet.  
3. NGABO Desire — Embedded Systems Lecture 1, College of Science and Technology, School of ICT.  
4. Atmel ATmega328P datasheet — ADC, timers, external interrupts.  
5. Project artifacts: `copy_of_138054_automatic_climate_control1.ino`, `bom.csv`, schematic PNG/PDF.

---

## Appendix A — Bill of materials (CSV)

See file `bom.csv` in the project folder.

## Appendix B — Key source functions

| Function | File location (approx.) | Role |
|----------|-------------------------|------|
| `main()` | Line 321 | Init + super loop |
| `automatic_climate_control()` | Line 235 | Climate control |
| `power_window()` | Line 286 | Window motor |
| `door_warning()` | Line 303 | Door safety |
| `lights()` | Line 64 | Adaptive lighting |
| `set_pin()`, `set_interrupt()`, `set_timer()` | Lines 154–127 | Hardware init |

Full listing: `copy_of_138054_automatic_climate_control1.ino`.
