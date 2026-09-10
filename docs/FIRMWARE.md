# Embedded Firmware Architecture & State Machine

This document outlines the ultra-low-power embedded firmware architecture for the **Estero-Volt** off-grid edge node, implemented on the Espressif **ESP32-WROOM-32E**.

---

## 1. Firmware Architectural Philosophy

The Estero-Volt firmware is designed under an **Event-Driven Transient Execution Paradigm**. The microcontroller never idles in an active polling loop; it operates as an ultra-low-leakage state machine that sleeps for $>99.9\%$ of its operational life, waking only when physical storage conditions allow or when hydrological safety limits are reached.

```
       +--------------------------------------------------------+
       |                   DEEP SLEEP STATE                     |
       |  - CPU Core: Powered OFF                               |
       |  - High-Speed Clocks: OFF                              |
       |  - Radios (Wi-Fi/BT/LoRa): Powered OFF / Sleep         |
       |  - Standby Current: 10 - 15 uA                         |
       +---------------------------+----------------------------+
                                   |
           Wakeup Trigger Event:   |
           1. BQ25504 VBAT_OK High | (Supercap hits 3.6V)
           2. RTC Timer Expired    | (Dynamic Duty Cycle)
                                   v
       +--------------------------------------------------------+
       | STATE 1: BOOT & GPIO HYGIENE                           |
       | - Set unused pins to INPUT_PULLDOWN (eliminate leakage)|
       | - Increment persistent boot counter in RTC SRAM        |
       +---------------------------+----------------------------+
                                   |
                                   v
       +--------------------------------------------------------+
       | STATE 2: STORAGE ENERGY AUDIT                          |
       | - Sample 3:1 divider on ADC1_CH6 (GPIO 34)             |
       | - Calculate electrostatic State-of-Charge (SOC %)      |
       +---------------------------+----------------------------+
                                   |
                                   v
       +--------------------------------------------------------+
       | STATE 3: SENSOR POWER GATING & ACOUSTIC RANGING        |
       | - Assert GPIO 25 (Turn ON 5V Boost & JSN-SR04T)        |
       | - 12 us trigger pulse on GPIO 32                       |
       | - Measure pulse duration on GPIO 35 (ECHO)             |
       | - Calculate canal water distance (mm)                  |
       | - De-assert GPIO 25 (Shut down 5V rail immediately)    |
       +---------------------------+----------------------------+
                                   |
                                   v
       +--------------------------------------------------------+
       | STATE 4: EDGE-AI HYDROLOGICAL INFERENCE                |
       | - Feed recent time-series distance to Quantized LSTM   |
       | - Predict flash-flood trajectory (2 - 4 hour horizon)  |
       +---------------------------+----------------------------+
                                   |
                                   v
       +--------------------------------------------------------+
       | STATE 5: RF TELEMETRY TRANSMISSION (LoRaWAN)           |
       | - Initialize SPI bus & SX1276 (RFM95W)                 |
       | - Pack 10-byte binary frame (Node ID, Dist, VBAT, SOC) |
       | - Transmit packet at SF10 / 125 kHz / +17 dBm          |
       | - Transition SX1276 to Sleep mode                      |
       +---------------------------+----------------------------+
                                   |
                                   v
       +--------------------------------------------------------+
       | STATE 6: ADAPTIVE DUTY CYCLING & SLEEP ARMING          |
       | - Compute sleep duration (2 min / 7 min / 15 min / 60m)|
       | - Arm RTC Timer Wakeup + RTC GPIO 33 (VBAT_OK)         |
       | - Isolate RTC IOs (`rtc_gpio_isolate`)                 |
       | - Execute `esp_deep_sleep_start()`                     |
       +--------------------------------------------------------+
```

---

## 2. Low-Power GPIO Configuration & Parasitic Current Elimination

When an ESP32 enters deep sleep, standard GPIO pins enter a high-impedance (floating) state unless explicitly latched or configured. A floating digital CMOS gate can oscillate between logical high and low thresholds due to ambient electromagnetic noise, drawing hundreds of microamperes in shoot-through current across the internal CMOS inverter stage.

### 2.1 Mitigation Implementation
1. **Unused GPIO Clamping**:
   All unbonded or unrouted pins (`GPIO 0, 4, 12, 13, 15, 16, 17, 21, 22`) are pulled down internally to digital ground:
   ```cpp
   gpio_reset_pin(pin);
   pinMode(pin, INPUT_PULLDOWN);
   ```
2. **RTC IO Isolation**:
   RTC domains remain powered during deep sleep. Pins tied to the RTC subsystem (`GPIO 25, 33`) can leak current through internal pull resistors into low-impedance external circuitry. During the pre-sleep arming sequence, `rtc_gpio_isolate()` disconnects internal pads:
   ```cpp
   rtc_gpio_isolate(PIN_VBAT_OK);
   rtc_gpio_isolate(PIN_SENS_PWR_EN);
   ```
3. **CPU Downclocking**:
   The default ESP32 CPU frequency ($240\,\text{MHz}$) draws $\approx 65\,\text{mA}$. The system configuration (`platformio.ini`) downclocks the XTAL PLL to **$80\,\text{MHz}$**, reducing active run current to $\approx 25\,\text{mA}$ with zero impact on acoustic measurement timing precision or SPI communication.

---

## 3. Sensor Power Gating & Acoustic Measurement Engine

The JSN-SR04T waterproof ultrasonic module draws $30\,\text{mA}$ quiescent current if permanently powered. Over a 15-minute interval, continuous power would consume $135\,\text{Joules}$—over 1000 times the available energy budget.

### 3.1 Gated Actuation Protocol
1. **Power-On**: ESP32 drives `PIN_SENS_PWR_EN (GPIO 25)` HIGH, biasing the NPN driver to pull the gate of the high-side P-FET to ground. This activates the TI TPS61099 boost regulator, establishing a $5.0\,\text{V}$ rail.
2. **Settling Delay**: A mandatory $15\,\text{ms}$ settling window allows the boost output filter capacitors ($22\,\mu\text{F}$) to reach steady state and the sensor's internal oscillator to lock.
3. **Trigger Pulse**: A $12\,\mu\text{s}$ active-high pulse is emitted on `PIN_TRIG (GPIO 32)`.
4. **Echo Acquisition**: The sensor transmits an 8-cycle $40\,\text{kHz}$ ultrasonic burst and asserts its `ECHO` output HIGH. The pulse width represents the acoustic travel time:
   $$\text{Distance (mm)} = \frac{T_{high}\,(\mu\text{s}) \times 0.348\,\text{mm}/\mu\text{s}}{2}$$
   *(Calibrated for the speed of sound at $28^\circ\text{C}$ ambient tropical estero conditions).*
5. **Immediate Power-Off**: `PIN_SENS_PWR_EN` is pulled LOW immediately after pulse acquisition. Total powered window is $< 35\,\text{ms}$.

---

## 4. Edge-AI Predictive Hydrological Modeling

As validated in the Estero-Volt engineering evaluation, the node integrates a lightweight, on-device quantized neural network to forecast flash floods without relying on cloud computation:

* **Architecture**: 2-layer Long Short-Term Memory (LSTM) network with 50 hidden units per layer.
* **Quantization**: 8-bit Integer (INT8) quantization post-training via TensorFlow Lite for Microcontrollers (TFLM).
* **Model Footprint**: $85\,\text{KB}$ in Flash memory; executes within $48\,\text{KB}$ of internal SRAM.
* **Inference Latency**: $< 150\,\text{ms}$ at $80\,\text{MHz}$ CPU clock.
* **Accuracy Benchmark**: Mean Absolute Error (MAE) of $4.2\,\text{cm}$, Root Mean Square Error (RMSE) of $5.8\,\text{cm}$, $R^2 = 0.91$ against real flood stage benchmarks.

---

## 5. Adaptive Duty Cycling (ADC) Engine

Telemetry intervals are dynamically adapted based on real-time flood risk and supercapacitor State-of-Charge (SOC). This ensures high-cadence data during disaster events while preserving power during calm periods:

| State | Environmental Condition | Supercapacitor SOC | Sleep Duration ($T_{sleep}$) | Operational Rationale |
| :--- | :--- | :--- | :--- | :--- |
| **Critical Flash Flood** | Distance $< 120\,\text{cm}$ | $\text{SOC} \ge 30\%$ | **2 Minutes** | Delivers real-time water rising velocity to municipal disaster response (CDRRMO). |
| **Stressed Flood State** | Distance $< 120\,\text{cm}$ | $\text{SOC} < 30\%$ | **5 Minutes** | Flood warning maintained while preventing brownout under high RF activity. |
| **Early Warning** | $120\,\text{cm} \le \text{Dist} < 180\,\text{cm}$ | $\text{SOC} \ge 25\%$ | **7 Minutes** | Monitored transition period as canal enters high stage. |
| **Nominal Baseline** | Distance $\ge 180\,\text{cm}$ | $\text{SOC} \ge 25\%$ | **15 Minutes** | Standard monitoring; leaves $>96\%$ of harvested energy in reserve. |
| **Energy Hibernation** | Any Distance | $\text{SOC} < 25\%$ | **60 Minutes** | Emergency recovery; stops active transmissions to allow BMFC biofilm to recharge the supercapacitor bank. |

---

## 6. Binary Telemetry Frame Specification

To minimize RF airtime (ToA - Time on Air) and preserve power, transmission data is serialized into a packed 10-byte binary payload rather than human-readable JSON or ASCII strings:

```
+---------------+---------------------+-------------------+------------------+---------------+----------------+
| Node ID (1B)  | Sequence Count (4B) | Distance mm (2B)  | Voltage mV (2B)  | SOC % (1B)    | Status (1B)    |
| Byte 0        | Bytes 1 - 4         | Bytes 5 - 6       | Bytes 7 - 8      | Byte 9        | Byte 10        |
+---------------+---------------------+-------------------+------------------+---------------+----------------+
```

### 6.1 Status Bitmask Definition
* **Bit 0**: `FLAG_CRITICAL_FLOOD` (1 = Water level breached $120\,\text{cm}$ critical mark).
* **Bit 1**: `FLAG_LOW_ENERGY` (1 = Supercapacitor bank below $20\%$ SOC).
* **Bit 2**: `FLAG_SENSOR_FAULT` (1 = Ultrasonic acoustic echo timeout or hardware fault).
* **Bit 3**: `FLAG_TX_SUCCESS` (1 = Previous packet transmission acknowledged).
* **Bits 4-7**: Reserved for future environmental sensor payloads (e.g., $pH$, dissolved oxygen).

### 6.2 LoRa Modulation & Time-on-Air (ToA)
* **Preamble Length**: 8 symbols.
* **Payload Length**: 10 bytes.
* **Spreading Factor**: SF10.
* **Bandwidth**: 125 kHz.
* **Calculated Time on Air (ToA)**: $\approx 51.4\,\text{ms}$.
* At $120\,\text{mA}$ peak PA current at $3.3\,\text{V}$, total energy expended per transmission is:
  $$E_{TX} = 3.3\,\text{V} \times 0.120\,\text{A} \times 0.0514\,\text{s} \approx \mathbf{20.35\,\text{mJ}}$$
