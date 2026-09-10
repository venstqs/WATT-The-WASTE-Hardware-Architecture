# Engineering Bill of Materials (BOM)

**Project**: Estero-Volt Off-Grid Telemetry Node  
**Hardware Revision**: v1.0.0  
**Target Environment**: Tropical Urban Canal / Waste-to-Energy Bioreactor  
**Operating Conditions**: $-10^\circ\text{C}$ to $+65^\circ\text{C}$, $100\%$ RH Condensing, $H_2S$ Corrosive Atmosphere  

---

## 1. Active Semiconductors & Integrated Circuits

| Designator | Description | Manufacturer | Manufacturer Part Number (MPN) | Package / Footprint | Qty | Critical Engineering Specifications & Notes |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **U1** | Ultra-Low Power Boost Harvester with MPPT | Texas Instruments | **BQ25504RGTR** | VQFN-20 (3.5x3.5mm) | 1 | Cold-start $V_{IN} \ge 330\,\text{mV}$; continuous extraction down to $80\,\text{mV}$. MPPT sampled at 50% $V_{OC}$. |
| **U2** | Nano-Power Synchronous Buck Converter | Texas Instruments | **TPS62840DLYR** | WSON-6 (1.5x1.5mm) | 1 | Operating $I_Q = 60\,\text{nA}$; $750\,\text{mA}$ peak output. Regulates $3.0-4.8\,\text{V}$ supercap rail to stable $3.3\,\text{V}$. |
| **U3** | Nano-Power Synchronous Boost Converter | Texas Instruments | **TPS61099YFFR** | DSBGA-6 (0.8x1.2mm) | 1 | $1\,\mu\text{A}$ shutdown current; steps $3.3\,\text{V}$ up to $5.0\,\text{V}$ for the ultrasonic sensor during $35\,\text{ms}$ burst. |
| **U4** | High-Side Controlled Slew-Rate Load Switch | Texas Instruments | **TPS22919DCKR** | SC-70-6 (SOT-363) | 1 | $R_{ON} = 90\,\text{m}\Omega$; ultra-low leakage ($< 10\,\text{nA}$). Gates power to 5V boost and sensor rail via ESP32 GPIO 25. |
| **U5** | Precision Supercapacitor MOSFET Balancer | Advanced Linear Devices | **ALD910022SAL** | SOIC-8 | 2 | Matched gate threshold $V_t = 2.20\,\text{V}$. Standby leakage $< 10\,\text{pA}$ per cell; active shunting prevents supercap overvoltage. |
| **U6** | 32-Bit Dual-Core Microcontroller Module | Espressif Systems | **ESP32-WROOM-32E (4MB)** | SMD Module (18x25.5mm)| 1 | PCB trace / external IPEX antenna connector. Operates in deep sleep mode ($10-15\,\mu\text{A}$) with RTC GPIO wakeup. |
| **U7** | LoRa Transceiver Module (+20dBm) | HopeRF / Semtech | **RFM95W-915S2** | SMD-16 (16x16mm) | 1 | SX1276 silicon. Configured for $915\,\text{MHz}$, SF10, 125 kHz BW. Communicates over VSPI bus with DIO0/DIO1 interrupts. |
| **Q1** | N-Channel MOSFET (Level Shifting) | Diodes Inc. | **BSS138-7-F** | SOT-23-3 | 1 | $V_{DS} = 50\,\text{V}$, $R_{DS(on)} = 3.5\,\Omega$. Bidirectional level shifter for JSN-SR04T $5\,\text{V}$ Echo to ESP32 $3.3\,\text{V}$ GPIO 35. |
| **D1** | Dual Surface Mount Schottky Barrier Diode | Nexperia | **BAT54S,215** | SOT-23-3 | 1 | Ultra-fast switching ($t_{rr} < 5\,\text{ns}$). Clamps voltage spikes on ESP32 ADC and ultrasonic Echo lines to rail limits. |

---

## 2. Energy Storage & Passives

| Designator | Description | Manufacturer | Manufacturer Part Number (MPN) | Package / Footprint | Qty | Critical Engineering Specifications & Notes |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **SC1, SC2, SC3** | Electric Double Layer Supercapacitor (EDLC) | Eaton / Bussmann | **XV3550-5R5506-R** | Radial Can (Cylindrical)| 3 | $50\,\text{F}$, $5.5\,\text{V}$ rating; 3 in series ($C_{eff} = 16.67\,\text{F}$, rated $16.5\,\text{V}$). Low ESR ($< 25\,\text{m}\Omega$). Operating temp $-40^\circ\text{C}$ to $+65^\circ\text{C}$. |
| **L1** | Power Inductor for BQ25504 Boost | Coilcraft | **LPS4018-223MRB** | SMD (4.0x4.0x1.8mm) | 1 | $22\,\mu\text{H} \pm 20\%$, $I_{sat} = 0.90\,\text{A}$, low DC resistance ($DCR = 0.36\,\Omega$). Critical for micro-watt harvesting efficiency. |
| **L2** | Power Inductor for TPS62840 Buck | Murata | **DFE201610E-2R2M=P2**| 0806 (2.0x1.6mm) | 1 | $2.2\,\mu\text{H} \pm 20\%$, $I_{sat} = 1.6\,\text{A}$, $DCR = 0.11\,\Omega$. Low radiation profile. |
| **L3** | Power Inductor for TPS61099 Boost | TDK | **VLS201610HBX-1R0M-1**| 0806 (2.0x1.6mm) | 1 | $1.0\,\mu\text{H} \pm 20\%$, $I_{sat} = 2.1\,\text{A}$, $DCR = 0.07\,\Omega$. |
| **C_REF** | VREF_SAMP Storage Capacitor | Murata | **GRM1885C1H103JA01D** | 0603 | 1 | $10\,\text{nF} \pm 5\%$, $50\,\text{V}$, C0G/NP0 dielectric. Ultra-low dielectric absorption and zero leakage for MPPT hold. |
| **C_BYP1** | BQ25504 VREF Decoupling Capacitor | TDK | **C1608X7R1C104K080AA** | 0603 | 1 | $0.1\,\mu\text{F} \pm 10\%$, $16\,\text{V}$, X7R ceramic. |
| **C_IN1** | BMFC Input Filter Capacitor | Murata | **GRM21BR61A106KE19L** | 0805 | 1 | $10\,\mu\text{F} \pm 10\%$, $10\,\text{V}$, X5R ceramic. Low ESR at harvester input. |
| **C_OUT1** | VSTOR Output Filter Capacitor | Murata | **GRM21BR61C226ME44L** | 0805 | 2 | $22\,\mu\text{F} \pm 20\%$, $16\,\text{V}$, X5R ceramic. Paralleled with supercapacitor bank. |
| **C_ADC** | ADC Input Filter Capacitor | Murata | **GRM188R71C104KA01D** | 0603 | 1 | $100\,\text{nF} \pm 10\%$, $16\,\text{V}$, X7R ceramic. Lowers source impedance for ESP32 SAR ADC sampling. |
| **R_OV1** | Overvoltage Setting Resistor (High) | Vishay Dale | **RCA06036M19FKEA** | 0603 | 1 | $6.19\,\text{M}\Omega \pm 1\%$, ASTM B809-95 Anti-Sulfur rated thick film. |
| **R_OV2** | Overvoltage Setting Resistor (Low) | Vishay Dale | **RCA06033M74FKEA** | 0603 | 1 | $3.74\,\text{M}\Omega \pm 1\%$, ASTM B809-95 Anti-Sulfur rated thick film. |
| **R_UV1, R_UV2** | Undervoltage Setting Resistors | Vishay Dale | **RCA06034M99FKEA** | 0603 | 2 | $4.99\,\text{M}\Omega \pm 1\%$, ASTM B809-95 Anti-Sulfur rated thick film. |
| **R_OK1** | VBAT_OK High Setting Resistor | Vishay Dale | **RCA06034M70FKEA** | 0603 | 1 | $4.70\,\text{M}\Omega \pm 1\%$, ASTM B809-95 Anti-Sulfur rated thick film. |
| **R_OK2** | VBAT_OK Hysteresis Mid Resistor | Vishay Dale | **RCA06031M10FKEA** | 0603 | 1 | $1.10\,\text{M}\Omega \pm 1\%$, ASTM B809-95 Anti-Sulfur rated thick film. |
| **R_OK3** | VBAT_OK Low Setting Resistor | Vishay Dale | **RCA06034M22FKEA** | 0603 | 1 | $4.22\,\text{M}\Omega \pm 1\%$, ASTM B809-95 Anti-Sulfur rated thick film. |
| **R_ADC1** | Supercapacitor ADC Divider (Top) | Susumu | **RR0816P-205-D** | 0603 | 1 | $2.00\,\text{M}\Omega \pm 0.5\%$, Thin Film, 25ppm/$^\circ\text{C}$ temperature stability. |
| **R_ADC2** | Supercapacitor ADC Divider (Bottom) | Susumu | **RR0816P-105-D** | 0603 | 1 | $1.00\,\text{M}\Omega \pm 0.5\%$, Thin Film, 25ppm/$^\circ\text{C}$ temperature stability. |
| **R_PU1-4** | SPI & Level Shifter Pull-up Resistors | Panasonic | **ERJ-U03F1002V** | 0603 | 4 | $10.0\,\text{k}\Omega \pm 1\%$, Anti-Sulfuration treated resistor array. |

---

## 3. Sensors, Connectors, & Electrochemistry Elements

| Designator | Description | Manufacturer | Manufacturer Part Number (MPN) | Specification / Footprint | Qty | Critical Engineering Specifications & Notes |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **SEN1** | Waterproof Non-Contact Ultrasonic Ranging Unit | Shenzhen Dianman | **JSN-SR04T-2.0** | Transceiver Module + Probe | 1 | IP67 sealed piezoelectric probe with $2.5\,\text{m}$ shielded coaxial cable. $20-600\,\text{cm}$ range. |
| **ANT1** | 915 MHz Helical Coil Quarter-Wave Antenna | Linx Technologies | **ANT-916-CW-HWR-SMA** | SMA Male / Center-pin | 1 | $+2.1\,\text{dBi}$ peak gain; omnidirectional radiation pattern. Sealed in UV-stabilized polyurethane. |
| **J1** | High-Current BMFC Input Screw Terminal | Phoenix Contact | **1725656 (MPT 0,5/ 2-2,54)** | Through-Hole (2.54mm pitch)| 1 | Beryllium copper tin-plated terminals for BMFC anode/cathode wire entry. |
| **J2** | High-Frequency RF Connector | Amphenol RF | **132134 (SMA End-Launch)** | Edge Mount (1.6mm PCB) | 1 | $50\,\Omega$ gold-plated brass end-launch SMA jack. |
| **BIO-A** | PANI-Modified Carbon Felt Bioanode | SGL Carbon / Custom | **SIGRACELL GFD 4.6 EA** | $100\,\text{mm} \times 100\,\text{mm} \times 4.6\,\text{mm}$| 1 | Chemically treated with $1\,\text{M}\ HNO_3$ activation and electrodeposited Polyaniline (PANI). $R_{int} = 5.5\,\Omega$. |
| **BIO-C** | Catalytic Air-Cathode with PTFE Layer | Fuel Cell Store | **W1S1011 Carbon Cloth** | $100\,\text{mm} \times 100\,\text{mm}$ | 1 | Coated with $MnO_2$ catalyst ($1.5\,\text{mg/cm}^2$) and 60% PTFE water-repellent gas diffusion membrane. |
| **ENC1** | Hermetic Tripartite Module Enclosure | Polycase / Custom | **WP-32F / CNC Polycarbonate**| Cylindrical (IP68 rated) | 1 | Integrated Viton O-ring seals, acrylic clear potting at submerged cable entry points. Resistant to $H_2S$ permeation. |
