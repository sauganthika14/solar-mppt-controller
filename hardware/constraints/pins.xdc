# Xilinx Design Constraints (XDC) Map for Solar MPPT PWM Driver
# Targeted Package Placement Mapping to ZedBoard Expansion Headers

# Pin Allocation for PMOD JA1 - Port 1 (Top Left Header Configuration Location)
set_property PACKAGE_PIN Y11 [get_ports pwm_out_0]

# Voltage Mode Specification for Single-Ended CMOS 3.3V Signaling Standard
set_property IOSTANDARD LVCMOS33 [get_ports pwm_out_0]