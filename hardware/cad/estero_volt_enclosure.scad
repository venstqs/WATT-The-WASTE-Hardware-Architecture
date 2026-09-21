/**
 * ============================================================================
 * Estero-Volt: Parametric 3D Enclosure Model (OpenSCAD)
 * Project: Benthic Microbial Fuel Cell (BMFC) IoT Telemetry Node
 * Hardware Revision: v2.0.0-PROD
 * 
 * Instructions:
 * 1. Download OpenSCAD (Free & Open Source: https://openscad.org).
 * 2. Open this file in OpenSCAD.
 * 3. Choose your view mode below: "assembled", "exploded", or individual parts.
 * 4. Press F5 for Preview, F6 for Full Render, and F7 to Export STL for 3D Printing!
 * ============================================================================
 */

// ============================================================================
// CONFIGURATION & RENDERING PARAMETERS
// ============================================================================
$fn = 60; // Curve resolution (Set to 120 for production STL export)

// View Mode: "assembled", "exploded", "dome_only", "collar_only", "housing_only", "reactor_only"
VIEW_MODE = "assembled";

// Exploded view vertical spacing
EXPLODE_OFFSET = 40;

// Dimensions (All in millimeters)
DOME_OD         = 110.0;
DOME_WALL       = 3.0;
DOME_HEIGHT     = 65.0;

COLLAR_OD       = 110.0;
COLLAR_ID       = 98.0;
COLLAR_HEIGHT   = 24.0;
ORING_W         = 3.2;
ORING_D         = 1.9;

HOUSING_OD      = 110.4;
HOUSING_WALL    = 3.2;
HOUSING_DEPTH   = 55.0;
HORN_MOUTH_DIA  = 65.0;
HORN_NECK_DIA   = 22.2;
HORN_LENGTH     = 35.0;

SPINE_OD        = 26.0;
SPINE_ID        = 14.0;
SPINE_HEIGHT    = 45.0;

REACTOR_OD      = 90.0;
REACTOR_WALL    = 3.0;
REACTOR_HEIGHT  = 140.0;

// ============================================================================
// COMPONENT MODULES
// ============================================================================

// 1. TOP DOME (Polycarbonate Clear Dome Cap)
module top_dome() {
    color([0.8, 0.9, 1.0, 0.4]) {
        difference() {
            // Outer dome shell
            union() {
                cylinder(h=15, d=DOME_OD, center=false);
                translate([0, 0, 15])
                    resize([DOME_OD, DOME_OD, (DOME_HEIGHT - 15) * 2])
                        sphere(d=DOME_OD);
            }
            // Inner hollow core
            union() {
                translate([0, 0, -1])
                    cylinder(h=16, d=DOME_OD - (2 * DOME_WALL), center=false);
                translate([0, 0, 15])
                    resize([DOME_OD - (2 * DOME_WALL), DOME_OD - (2 * DOME_WALL), (DOME_HEIGHT - 15 - DOME_WALL) * 2])
                        sphere(d=DOME_OD - (2 * DOME_WALL));
            }
            // Cut off bottom hemisphere
            translate([0, 0, -DOME_HEIGHT])
                cylinder(h=DOME_HEIGHT, d=DOME_OD * 2, center=false);
        }
        // Inward mounting shoulder ring
        translate([0, 0, 0])
            difference() {
                cylinder(h=6, d=DOME_OD - 0.5, center=false);
                translate([0, 0, -1])
                    cylinder(h=8, d=DOME_OD - (2 * DOME_WALL) - 4, center=false);
            }
    }
}

// 2. BAYONET COLLAR (Delrin / Anodized Aluminum Quarter-Turn Ring)
module bayonet_collar() {
    color([0.15, 0.15, 0.15, 1.0]) {
        difference() {
            // Main ring body
            cylinder(h=COLLAR_HEIGHT, d=COLLAR_OD, center=false);
            
            // Central bore
            translate([0, 0, -1])
                cylinder(h=COLLAR_HEIGHT + 2, d=COLLAR_ID, center=false);
            
            // Dual radial O-ring grooves
            translate([0, 0, 6.0])
                difference() {
                    cylinder(h=ORING_W, d=COLLAR_OD + 1, center=false);
                    cylinder(h=ORING_W, d=COLLAR_OD - (2 * ORING_D), center=false);
                }
            translate([0, 0, 13.0])
                difference() {
                    cylinder(h=ORING_W, d=COLLAR_OD + 1, center=false);
                    cylinder(h=ORING_W, d=COLLAR_OD - (2 * ORING_D), center=false);
                }
        }
        // 3x Bayonet Locking Lugs
        for (i = [0:2]) {
            rotate([0, 0, i * 120])
                translate([COLLAR_OD / 2 - 5, -8, COLLAR_HEIGHT])
                    cube([6, 16, 5]);
        }
    }
}

// 3. MIDDLE HOUSING (Main Sensor Body, Horn & Wall Mount)
module middle_housing() {
    color([0.25, 0.28, 0.30, 1.0]) {
        difference() {
            union() {
                // Main cylindrical cavity
                cylinder(h=HOUSING_DEPTH, d=HOUSING_OD, center=false);
                
                // Forward 45-degree Acoustic Horn
                translate([HOUSING_OD / 2 - 5, 0, HOUSING_DEPTH / 2])
                    rotate([0, 45, 0])
                        cylinder(h=HORN_LENGTH, d1=HORN_NECK_DIA + 6, d2=HORN_MOUTH_DIA, center=false);
                
                // Rear Wall-Mounting Bracket Flange
                translate([-HOUSING_OD / 2 - 12, -45, 5])
                    cube([12, 90, 45]);
            }
            
            // Hollow out main cavity
            translate([0, 0, HOUSING_WALL])
                cylinder(h=HOUSING_DEPTH + 1, d=HOUSING_OD - (2 * HOUSING_WALL), center=false);
            
            // Hollow out acoustic horn interior
            translate([HOUSING_OD / 2 - 5, 0, HOUSING_DEPTH / 2])
                rotate([0, 45, 0])
                    translate([0, 0, -2])
                        cylinder(h=HORN_LENGTH + 4, d1=HORN_NECK_DIA, d2=HORN_MOUTH_DIA - 4, center=false);
            
            // Mounting bolt holes in rear flange (4x M6)
            translate([-HOUSING_OD / 2 - 13, -35, 12]) rotate([0, 90, 0]) cylinder(h=15, d=6.5);
            translate([-HOUSING_OD / 2 - 13, 35, 12])  rotate([0, 90, 0]) cylinder(h=15, d=6.5);
            translate([-HOUSING_OD / 2 - 13, -35, 42]) rotate([0, 90, 0]) cylinder(h=15, d=6.5);
            translate([-HOUSING_OD / 2 - 13, 35, 42])  rotate([0, 90, 0]) cylinder(h=15, d=6.5);
            
            // Cable gland threaded hole on side
            translate([0, HOUSING_OD / 2 - 2, HOUSING_DEPTH / 2])
                rotate([90, 0, 0])
                    cylinder(h=10, d=16);
        }
        
        // Central structural spine (internal cable conduit)
        translate([0, 0, -SPINE_HEIGHT])
            difference() {
                cylinder(h=SPINE_HEIGHT, d=SPINE_OD, center=false);
                translate([0, 0, -1])
                    cylinder(h=SPINE_HEIGHT + 2, d=SPINE_ID, center=false);
            }
    }
}

// 4. BOTTOM BIO-REACTOR (Submerged BMFC Sludge Basket & Electrodes)
module bottom_bio_reactor() {
    color([0.2, 0.2, 0.2, 1.0]) {
        difference() {
            // Main cylindrical basket
            cylinder(h=REACTOR_HEIGHT, d=REACTOR_OD, center=false);
            
            // Hollow inner chamber
            translate([0, 0, REACTOR_WALL])
                cylinder(h=REACTOR_HEIGHT + 1, d=REACTOR_OD - (2 * REACTOR_WALL), center=false);
            
            // 72x Sludge/Water Flow Perforations
            for (z = [15 : 20 : 120]) {
                for (angle = [0 : 30 : 330]) {
                    rotate([0, 0, angle + (z % 40 == 15 ? 15 : 0)])
                        translate([REACTOR_OD / 2 - 5, 0, z])
                            rotate([0, 90, 0])
                                cylinder(h=10, d=6.0);
                }
            }
        }
        
        // 4x Bottom Triangular Anchor Fins
        for (a = [0, 90, 180, 270]) {
            rotate([0, 0, a])
                translate([REACTOR_OD / 2 - 2, -2, 0])
                    linear_extrude(height=25)
                        polygon(points=[[0,0], [30,0], [0,25]]);
        }
        
        // Internal Bioanode Core (PANI Carbon Felt)
        color([0.05, 0.05, 0.05, 1.0])
            translate([0, 0, 10])
                cylinder(h=80, d=45);
        
        // Internal Cathode Mesh (MnO2 Catalytic Layer)
        color([0.6, 0.5, 0.4, 0.7])
            translate([0, 0, 95])
                difference() {
                    cylinder(h=35, d=55);
                    cylinder(h=37, d=52);
                }
    }
}

// ============================================================================
// MAIN ASSEMBLY CONTROLLER
// ============================================================================

if (VIEW_MODE == "assembled") {
    // 1. Bottom Bio-Reactor
    translate([0, 0, 0])
        bottom_bio_reactor();
    
    // 2. Middle Housing & Sensor
    translate([0, 0, REACTOR_HEIGHT + SPINE_HEIGHT])
        middle_housing();
    
    // 3. Bayonet Collar
    translate([0, 0, REACTOR_HEIGHT + SPINE_HEIGHT + HOUSING_DEPTH])
        bayonet_collar();
    
    // 4. Top Dome Cap
    translate([0, 0, REACTOR_HEIGHT + SPINE_HEIGHT + HOUSING_DEPTH + COLLAR_HEIGHT])
        top_dome();
}
else if (VIEW_MODE == "exploded") {
    // Exploded along vertical axis
    translate([0, 0, 0])
        bottom_bio_reactor();
    
    translate([0, 0, REACTOR_HEIGHT + SPINE_HEIGHT + EXPLODE_OFFSET])
        middle_housing();
    
    translate([0, 0, REACTOR_HEIGHT + SPINE_HEIGHT + HOUSING_DEPTH + (EXPLODE_OFFSET * 2)])
        bayonet_collar();
    
    translate([0, 0, REACTOR_HEIGHT + SPINE_HEIGHT + HOUSING_DEPTH + COLLAR_HEIGHT + (EXPLODE_OFFSET * 3)])
        top_dome();
}
else if (VIEW_MODE == "dome_only") {
    top_dome();
}
else if (VIEW_MODE == "collar_only") {
    bayonet_collar();
}
else if (VIEW_MODE == "housing_only") {
    middle_housing();
}
else if (VIEW_MODE == "reactor_only") {
    bottom_bio_reactor();
}
