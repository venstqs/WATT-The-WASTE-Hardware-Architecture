# 3D Mechanical CAD & Physical Fabrication Directory

This directory contains the 3D computer-aided design (CAD) models, parametric scripts, and manufacturing specifications for fabricating the **Estero-Volt** off-grid IoT enclosure.

---

## 1. Ready-to-Print 3D Model: `estero_volt_enclosure.scad`

We have provided a fully parametric, open-source 3D CAD script in **OpenSCAD** format:
👉 [`hardware/cad/estero_volt_enclosure.scad`](estero_volt_enclosure.scad)

### How to Use It (Zero-CAD Experience Needed):
1. Download and install **OpenSCAD** (Free, open-source, 25 MB installer: [openscad.org](https://openscad.org)).
2. Open [`estero_volt_enclosure.scad`](estero_volt_enclosure.scad).
3. Change `VIEW_MODE` at the top of the file to render individual parts:
   * `"assembled"` — Full assembled probe view.
   * `"exploded"` — Exploded inspection view.
   * `"dome_only"` — Top clear polycarbonate cap.
   * `"collar_only"` — Bayonet locking collar with dual O-ring grooves.
   * `"housing_only"` — Middle housing with $45^\circ$ acoustic horn & wall bracket.
   * `"reactor_only"` — Submerged BMFC sludge basket & anchor fins.
4. Press **`F5`** to preview, **`F6`** to render, and **`F7`** to export directly as an **`.STL`** file for 3D printing!

---

## 2. Recommended CAD Software for Custom Modeling

If you want to modify, customize, or remodel the enclosure yourself:

| Software | Platform | Cost | Best For | Link |
| :--- | :--- | :--- | :--- | :--- |
| **Onshape** | **100% In-Browser** (Chrome, Edge) | **Free** (Student/Hobby) | Best overall; runs on any school laptop without installing software. Cloud autosave. | [onshape.com](https://www.onshape.com/en/products/free) |
| **Autodesk Fusion 360** | Desktop (Windows, Mac) | **Free** (Personal/Student) | Industry standard; native McMaster-Carr hardware import (O-rings, screws) & ray-traced rendering. | [autodesk.com/fusion-360](https://www.autodesk.com/products/fusion-360/personal) |
| **Tinkercad** | **In-Browser** | **Free** | Absolute beginners; drag-and-drop geometric shapes for rapid mockups. | [tinkercad.com](https://www.tinkercad.com) |
| **OpenSCAD** | Desktop (Win, Mac, Linux) | **Free & Open Source** | Code-based parametric modeling. Fast exports directly to `.STL`. | [openscad.org](https://openscad.org) |

*Step-by-step modeling guide with all sketch dimensions and constraints is available in [`docs/MECHANICAL_CAD.md`](../../docs/MECHANICAL_CAD.md).*

---

## 3. Where and How to 3D Print / Fabricate (Philippines & Global)

### A. University FabLabs & Regional Centers (Philippines)
* **DOST & DTI Shared Service Facilities (FabLabs)**:
  * Most state colleges and regional universities in the Philippines host public **FabLabs** (e.g., Bicol University FabLab, Ateneo de Naga makerspaces, DOST Regional Centers).
  * You can bring your `.STL` files on a USB drive and print on industrial-grade 3D printers; you usually only pay for the raw filament weight (**₱3 – ₱5 per gram**).
* **Local 3D Printing Hubs**:
  * Search Facebook Marketplace or Shopee for *"3D printing service Naga City / Camarines Sur / Bicol"*. Average turnaround is 2–3 days.

### B. Professional Online On-Demand Services
* **JLCPCB 3D Printing** ([jlcpcb.com/3d-printing](https://jlcpcb.com/3d-printing)):
  * Upload your `.STL` files directly to their web portal.
  * **Top Dome**: Choose **SLA Resin (Imagine Black or 8001 Clear Resin)** with UV protective coating for crystal optical transparency (~$4 – $6 per part).
  * **Middle Housing & Reactor**: Choose **Nylon SLS (PA12)** for extreme UV, water, and $H_2S$ sewer chemical resistance (~$5 – $8 per part).
  * Ships directly to the Philippines via DHL Express or FedEx in 4–6 business days.

---

## 4. 3D Print Slicer Settings (FDM Prusa / Ender / Bambu)

If printing on standard desktop FDM 3D printers:

* **Material**:
  * **Middle Housing & Reactor**: **PETG, ASA, or ABS** (Never use PLA; PLA softens under tropical $40^\circ\text{C}$ sun and degrades in wastewater).
  * **Top Dome**: High-transparency PETG with 100% concentric infill, or SLA Clear Resin.
* **Perimeters / Shells**: 4 to 5 perimeters ($1.6 - 2.0\,\text{mm}$ total solid wall) to guarantee hydrostatic watertightness under IP68 pressures.
* **Infill**: $35\% - 45\%$ Gyroid infill.
* **Layer Height**: $0.16\,\text{mm} - 0.20\,\text{mm}$ (Ensures clean threads and precision O-ring compression).
