# PCB Design & Layout Guidelines

This document establishes the manufacturing, routing, impedance control, thermal management, and environmental protection specifications for the **Estero-Volt v1.0** controller board.

---

## 1. PCB Layer Stackup Architecture

<p align="center">
  <img src="images/pcb_3d_render.jpg" alt="Estero-Volt Custom Controller PCB 3D Render" width="950">
</p>

Due to the co-existence of micro-watt, high-impedance analog harvesting circuitry ($10\,\text{M}\Omega$ ladders), high-frequency digital switching ($240\,\text{MHz}$ MCU core), and a high-power $+20\,\text{dBm}$ ($100\,\text{mW}$) RF transceiver, a **4-layer stackup** is strictly mandatory. A 2-layer implementation is rejected due to unavoidable ground return discontinuities, high loop inductance, and poor immunity to ambient EMI in municipal canal conduits.

### 1.1 Layer Definition (Standard 1.6mm FR4-TG150)

```
========================================================================
Layer 1 (Top)      : Signals, Component Pads, RF 50-Ohm CPWG Traces (1 oz / 35 um)
------------------   Prepreg (Dielectric 2116, Er = 4.2, Thickness = 0.20 mm)
Layer 2 (Inner 1)  : Continuous Solid Ground Plane (1 oz / 35 um)
------------------   Core (FR4, Er = 4.5, Thickness = 1.06 mm)
Layer 3 (Inner 2)  : Power Rails (3.3V System, 5.0V Gated, VSTOR) (1 oz / 35 um)
------------------   Prepreg (Dielectric 2116, Er = 4.2, Thickness = 0.20 mm)
Layer 4 (Bottom)   : Low-speed Signals, Auxiliary Routing, Ground Flood (1 oz / 35 um)
========================================================================
```

* **Ground Plane Continuity (Layer 2)**: Maintained as an unbroken ground reference directly beneath Layer 1. No signal routing is permitted on Layer 2.
* **Tight Coupling**: The $0.20\,\text{mm}$ prepreg spacing between Layer 1 and Layer 2 creates low characteristic loop inductance for high-speed switching loops ($C_{bypass}$ to ESP32/LoRa VDD pins).

---

## 2. Trace Width Calculations & Current Capacity (IPC-2152)

Trace widths are engineered according to **IPC-2152** standards to prevent thermal dissipation and voltage sag across extreme dynamic load ranges.

### 2.1 Critical Conductor Sizing Specifications

| Trace / Net Group | Peak Current ($I_{peak}$) | Copper Weight | Target $\Delta T$ | Minimum Calculated Width | Implemented Layout Width | Design Rationale |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **BMFC Input (`VIN_DC`, `LBUS`)** | $15\,\text{mA}$ cont. | $1\,\text{oz}$ | $< 1.0^\circ\text{C}$ | $0.15\,\text{mm}$ | **$2.00\,\text{mm}$ (Polygon)** | Minimizes $I^2R$ conduction losses at $0.4\,\text{V}$ input; internal resistance must remain $< 0.05\,\Omega$. |
| **Supercap Bank (`VSTOR`, `VBAT`)** | $450\,\text{mA}$ peak | $1\,\text{oz}$ | $< 2.0^\circ\text{C}$ | $0.35\,\text{mm}$ | **$1.50\,\text{mm}$ (Pour)** | Prevents voltage droop during high-drain ESP32 boot and LoRa TX pulses. |
| **System Power (`+3V3`)** | $300\,\text{mA}$ peak | $1\,\text{oz}$ | $< 2.0^\circ\text{C}$ | $0.25\,\text{mm}$ | **$0.80\,\text{mm}$ (Net-Pour)**| Stable supply to ESP32 core and RFM95W power amplifier. |
| **Gated Ultrasonic (`+5V_SENS`)** | $40\,\text{mA}$ pulse | $1\,\text{oz}$ | $< 1.0^\circ\text{C}$ | $0.12\,\text{mm}$ | **$0.50\,\text{mm}$** | Supplies piezo transmitter without ringing or voltage dip. |
| **Digital High-Speed (SPI, UART)** | $< 5\,\text{mA}$ | $1\,\text{oz}$ | N/A | $0.127\,\text{mm}$ | **$0.20\,\text{mm}$** | Controlled capacitance; maintains rise-time integrity. |
| **Analog High-Z Nodes (`ROV`, `ROK`)**| $< 500\,\text{nA}$ | $1\,\text{oz}$ | N/A | $0.127\,\text{mm}$ | **$0.25\,\text{mm}$** | Isolated with guard rings to eliminate surface leakage currents. |

---

## 3. High-Frequency RF Layout (915 MHz LoRa Transmission)

The RF output from Pin 9 of the RFM95W to the external IPEX / SMA antenna connector must be routed as a **$50\,\Omega \pm 10\%$ Coplanar Waveguide with Ground (CPWG)** on Layer 1 over the solid ground plane of Layer 2.

```
       Top Layer Ground Pour             Top Layer Ground Pour
          +----------------+  Trace (W)  +----------------+
          |                |  <=======>  |                |
          |                |             |                |
          |       GND      |  [RF 50R]   |      GND       |
          +-------+--------+--+-------+--+--------+-------+
                  |    Gap (S)|       |Gap (S)    |
==================|===========|=======|===========|====================  Layer 1
       Dielectric | (H = 0.2mm, Er = 4.2)         |
==================|===============================|====================  Layer 2
                  +-------- Solid Ground Plane ---+
```

### 3.1 Calculated CPWG Geometric Parameters (Target $Z_0 = 50.2\,\Omega$)
* **Trace Width ($W$)**: $0.36\,\text{mm}$ ($14.2\,\text{mil}$).
* **Ground Spacing ($S$)**: $0.25\,\text{mm}$ ($10.0\,\text{mil}$).
* **Substrate Height ($H$)**: $0.20\,\text{mm}$ ($7.87\,\text{mil}$).
* **Copper Thickness ($T$)**: $35\,\mu\text{m}$ ($1\,\text{oz}$).
* **Substrate Relative Permittivity ($\epsilon_r$)**: $4.2$.

### 3.2 RF Layout Rules:
1. **Via Fencing**: Ground planes bordering the RF trace must feature continuous stitching vias with center-to-center pitch $\le 1.5\,\text{mm}$ ($\approx \lambda / 20$ at $915\,\text{MHz}$) directly connecting to Layer 2.
2. **Zero Bends**: The RF trace must route directly to the SMA/IPEX connector without $90^\circ$ bends. If routing requires turns, use $45^\circ$ mitered bends or circular arcs with radius $R \ge 3W$.
3. **No Splitting**: No copper clearances or trace crossovers are permitted on Layer 2 underneath the RF trace.

---

## 4. Grounding Architecture & Noise Isolation

A major failure mode of micro-watt energy harvesting systems is switching noise from the MCU or RF amplifier coupling into the high-impedance feedback dividers of the harvester IC, triggering premature undervoltage shutdowns.

```
+------------------------------------+      +------------------------------------+
|       ANALOG HARVESTING ZONE       |      |        DIGITAL / RF POWER ZONE     |
| - BQ25504 Harvester & Inductor     |      | - ESP32 Core & Clocks              |
| - 10 M-Ohm Resistor Ladders        |      | - RFM95W Power Amplifier           |
| - VREF_SAMP Storage Capacitor      |      | - TPS62840 Buck Converter          |
| - Quiet Analog Ground Plane (AGND) |      | - Digital Ground Plane (DGND)      |
+-----------------+------------------+      +------------------+-----------------+
                  |                                            |
                  +---------------------+----------------------+
                                        |
                                  +-----------+
                                  | Net-Tie / | Single 0-Ohm SMT Resistor
                                  | Star Point| Located at Supercap Return Pad
                                  +-----------+
```

### 4.1 Star-Point Grounding & Net-Tie
* **Separate Grounds**: Layout two distinct ground regions on Layer 1: **`AGND`** (Analog Ground for BQ25504) and **`DGND`** (Digital Ground for ESP32 and LoRa).
* **Single Connection**: Tie `AGND` to `DGND` at a single physical point using a copper Net-Tie or a $0\,\Omega$ ($0805$) precision jumper located directly adjacent to the negative terminal of the supercapacitor storage bank.
* **Return Currents**: High-current return paths from the LoRa power amplifier ($120\,\text{mA}$) flow exclusively through `DGND` back to the supercapacitor without crossing into `AGND`.

### 4.2 Guard Rings for Megaohm Resistors
* High-impedance nodes (`ROV`, `RUV`, `ROK`, `VREF_SAMP`) operate at nano-ampere current levels. Contaminants, moisture, or surface flux residues can create parasitic parallel resistances that corrupt voltage trip points.
* **Guard Ring**: Encircle all high-Z traces with a continuous `AGND` guard ring on Layer 1, stripped of solder mask, to intercept surface leakage currents.

---

## 5. Environmental Protection & $H_2S$ Resilience

Estero waterways present one of the most hostile operating environments for electronics: **$95\% - 100\%$ relative humidity**, condensing water vapor, and elevated concentrations of **biogenic hydrogen sulfide ($H_2S$)** and **ammonia ($NH_3$)** produced by anaerobic sewage digestion.

### 5.1 Atmospheric Corrosion Mitigation
1. **PCB Surface Finish**:
   * **STRICT PROHIBITION**: **HASL (Hot Air Solder Leveling)** is prohibited. Atmospheric sulfur attacks lead-free tin-lead solders, forming non-conductive tin-sulfide whiskering and dendritic creep corrosion.
   * **MANDATORY**: **ENIG (Electroless Nickel Immersion Gold)** per IPC-4552 ($3-5\,\mu\text{in}$ Gold over $120-240\,\mu\text{in}$ Electroless Nickel). Provides absolute barrier protection against sulfur permeation.
2. **Anti-Sulfuration Passive Components**:
   * Standard thick-film chip resistors use silver-palladium (Ag/Pd) inner terminations. $H_2S$ gas penetrates protective overcoats, reacting to form silver sulfide ($Ag_2S$), which expands, breaks the conductor, and causes open-circuit field failures.
   * **Specification**: All chip resistors must comply with **ASTM B809-95** (Flower of Sulfur Test) with gold-based or sulfur-resistant inner terminations (e.g., Vishay RCA series or Panasonic ERJ-U series).

### 5.2 Conformal Coating Protocol (IPC-CC-830B / MIL-I-46058C)
* **Material**: High-purity **Silicone Conformal Coating** (e.g., MG Chemicals 422B or Humiseal 1C49).
* **Properties**:
  * Operating temperature: $-65^\circ\text{C}$ to $+200^\circ\text{C}$.
  * Dielectric strength: $> 1000\,\text{V/mil}$.
  * Hydrophobic water repellency and high gas-diffusion resistance against $H_2S$.
* **Application Procedure**:
  1. Ultrasonic wash populated boards in pure isopropyl alcohol (IPA) to eliminate flux residue.
  2. Bake PCBs at $85^\circ\text{C}$ for 2 hours to drive out trapped board moisture.
  3. Mask LoRa SMA connectors, programming headers, and test pads.
  4. Apply two uniform coats ($50-75\,\mu\text{m}$ dry film thickness) using selective spray deposition.
  5. Inspect under UV illumination ($365\,\text{nm}$) for complete void-free coverage.

### 5.3 Creepage & Clearance (IPC-2221B)
Although the supercapacitor operating voltage is low ($< 5.5\,\text{V}$), condensing humidity in sewer canals causes ionic surface contamination:
* **Minimum Trace Clearance (Coated)**: $0.13\,\text{mm}$.
* **Minimum Conductor Spacing around Supercapacitor Terminals**: $1.00\,\text{mm}$.
* **BMFC Electrode Barrier**: Terminal block screws for BMFC anode/cathode inputs must be spaced $\ge 5.0\,\text{mm}$ apart to prevent galvanic bridge shorting across condensing electrolytic sewage droplets.
