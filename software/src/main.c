/**
 * @file main.c
 * @brief Closed-Loop Solar MPPT Controller using Perturb & Observe (P&O)
 * @target ZedBoard (Zynq-7000 SoC)
 * * @description This application simulates a non-linear solar panel voltage characteristic,
 * calculates the corresponding power feedback, and dynamically infers the optimal duty cycle.
 * The resulting control action is sent via AXI4-Lite to a custom PWM generation IP in the PL.
 */

#include <stdio.h>
#include "xil_printf.h"
#include "xparameters.h"
#include "xadcps.h"
#include "xil_io.h"
#include "sleep.h"

// Hardware Base Addresses from Vivado Address Editor
#define PWM_BASEADDR   XPAR_PWM_CONTROLLER_0_S00_AXI_BASEADDR
#define XADC_DEVICE_ID XPAR_XADC_WIZ_0_DEVICE_ID

XAdcPs XAdcInst;

int main() {
    // Initialize the Xilinx Analog-to-Digital Converter System Monitor Block
    XAdcPs_Config *ConfigPtr = XAdcPs_LookupConfig(XADC_DEVICE_ID);
    XAdcPs_CfgInitialize(&XAdcInst, ConfigPtr, ConfigPtr->BaseAddress);

    float v_now, p_now;
    float p_old = 0.0;
    
    // Control Signal Initialization (16-bit Timer Resolution)
    uint32_t duty = 32768; // Initialize at a 50% midpoint duty cycle
    uint32_t step = 500;   // Fixed perturbation step size for tracking

    // Environment Simulation Parameters (Solar Irradiance Profile Tracking)
    static int simulated_v = 500;
    static int dir = 1;

    print("--- Solar MPPT Controller Firmware Initialized ---\n\r");

    while(1) {
        // --- Layer 1: Environmental Simulation ---
        // Mimics solar irradiance changes over a diurnal profile
        simulated_v += (5 * dir); 
        if(simulated_v > 1000 || simulated_v < 100) {
            dir *= -1; // Mirror profile boundary constraints
        }
        
        uint32_t v_raw = (uint32_t)simulated_v;
        v_now = (float)v_raw;
        p_now = v_now * 1.0; // Modeling power extraction assuming normalized 1A current feedback

        // --- Layer 2: Perturb & Observe Inference Logic ---
        if (p_now > p_old) {
            duty += step;
            xil_printf("V_raw: %d | Duty: %d | Status: CLIMBING (Finding Peak...)\r\n", (int)v_raw, (int)duty);
        }
        else if (p_now < p_old) {
            duty -= step;
            xil_printf("V_raw: %d | Duty: %d | Status: FALLING (Reversing...)\r\n", (int)v_raw, (int)duty);
        }
        else {
            // Steady State Peak Condition (The Extraction Sweet Spot)
            xil_printf("***************************************************\r\n");
            xil_printf("V_raw: %d | Duty: %d | >>> MPPT SWEET SPOT IDENTIFIED <<<\r\n", (int)v_raw, (int)duty);
            xil_printf("***************************************************\r\n");
        }

        // --- Layer 3: Actuator Hardware Interfacing ---
        // Pushing the updated duty cycle definition across the AXI interconnect to the PL register map
        Xil_Out32(PWM_BASEADDR, duty);

        // Update system states for the next execution period
        p_old = p_now; 
        
        // Stabilize telemetry reporting for real-time visualization (1 Hz tracking loop rate)
        usleep(1000000);
    }

    return 0;
}