EESchema Schematic File Version 4
LIBS:pcb_topsulin-cache
EELAYER 29 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
Wire Wire Line
	1350 1100 1100 1100
Wire Wire Line
	1350 2100 1100 2100
Wire Wire Line
	1350 2200 1300 2200
$Comp
L power:GND #PWR0101
U 1 1 5BEFC3B3
P 1100 3150
F 0 "#PWR0101" H 1100 2900 50  0001 C CNN
F 1 "GND" H 1105 2977 50  0000 C CNN
F 2 "" H 1100 3150 50  0001 C CNN
F 3 "" H 1100 3150 50  0001 C CNN
	1    1100 3150
	1    0    0    -1  
$EndComp
Wire Wire Line
	1100 3150 1100 2100
Connection ~ 1100 2100
Wire Wire Line
	1100 2100 1100 1100
$Comp
L Connector_Generic:Conn_01x24 J3
U 1 1 5C0AD0DD
P 10500 1800
F 0 "J3" H 10580 1792 50  0000 L CNN
F 1 "Conn_01x24" H 10580 1701 50  0000 L CNN
F 2 "kicad_libs:e-paper-2in13-flex-2" H 10500 1800 50  0001 C CNN
F 3 "~" H 10500 1800 50  0001 C CNN
	1    10500 1800
	1    0    0    -1  
$EndComp
Text GLabel 10150 800  0    50   Input ~ 0
GDR
Text GLabel 10150 900  0    50   Input ~ 0
RESE
Text GLabel 10150 1500 0    50   Input ~ 0
EPD_BUSY
Text GLabel 10150 1600 0    50   Input ~ 0
EPD_RST
Text GLabel 10150 1700 0    50   Input ~ 0
EPD_DC
Text GLabel 10150 1800 0    50   Input ~ 0
EPD_CS
Text GLabel 10150 1900 0    50   Input ~ 0
EPD_CLK
Text GLabel 10150 2000 0    50   Input ~ 0
EPD_DIN
Wire Wire Line
	10300 800  10150 800 
Wire Wire Line
	10300 900  10150 900 
$Comp
L Device:C C8
U 1 1 5C0F7A80
P 9300 1100
F 0 "C8" V 9048 1100 50  0000 C CNN
F 1 "105" V 9139 1100 50  0000 C CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 9338 950 50  0001 C CNN
F 3 "~" H 9300 1100 50  0001 C CNN
	1    9300 1100
	0    1    1    0   
$EndComp
$Comp
L Device:C C5
U 1 1 5C0F7BC5
P 9000 1000
F 0 "C5" V 8748 1000 50  0000 C CNN
F 1 "105" V 8839 1000 50  0000 C CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 9038 850 50  0001 C CNN
F 3 "~" H 9000 1000 50  0001 C CNN
	1    9000 1000
	0    1    1    0   
$EndComp
Wire Wire Line
	10300 1100 9450 1100
Wire Wire Line
	10300 1000 9150 1000
Wire Wire Line
	8750 1000 8850 1000
Wire Wire Line
	8750 1100 9150 1100
Wire Wire Line
	10300 1500 10150 1500
Wire Wire Line
	10300 1600 10150 1600
Wire Wire Line
	10300 1700 10150 1700
Wire Wire Line
	10300 1800 10150 1800
Wire Wire Line
	10300 1900 10150 1900
Wire Wire Line
	10300 2000 10150 2000
$Comp
L power:+3V3 #PWR010
U 1 1 5C115179
P 9600 1900
F 0 "#PWR010" H 9600 1750 50  0001 C CNN
F 1 "+3V3" H 9615 2073 50  0000 C CNN
F 2 "" H 9600 1900 50  0001 C CNN
F 3 "" H 9600 1900 50  0001 C CNN
	1    9600 1900
	1    0    0    -1  
$EndComp
Wire Wire Line
	10300 2100 10150 2100
Wire Wire Line
	10300 2200 10150 2200
Wire Wire Line
	10150 2200 10150 2100
$Comp
L Device:C C9
U 1 1 5C119A89
P 9300 2100
F 0 "C9" V 9048 2100 50  0000 C CNN
F 1 "104" V 9139 2100 50  0000 C CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 9338 1950 50  0001 C CNN
F 3 "~" H 9300 2100 50  0001 C CNN
	1    9300 2100
	0    1    1    0   
$EndComp
Wire Wire Line
	9450 2100 9600 2100
Connection ~ 10150 2100
Wire Wire Line
	9600 1900 9600 2100
Connection ~ 9600 2100
Wire Wire Line
	9600 2100 10150 2100
Wire Wire Line
	8750 2100 9150 2100
Wire Wire Line
	10300 2300 8750 2300
Wire Wire Line
	8750 2300 8750 2100
$Comp
L Device:C C6
U 1 1 5C12AB65
P 9000 2400
F 0 "C6" V 8748 2400 50  0000 C CNN
F 1 "105" V 8839 2400 50  0000 C CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 9038 2250 50  0001 C CNN
F 3 "~" H 9000 2400 50  0001 C CNN
	1    9000 2400
	0    1    1    0   
$EndComp
$Comp
L Device:C C10
U 1 1 5C12AD98
P 9300 2500
F 0 "C10" V 9048 2500 50  0000 C CNN
F 1 "105" V 9139 2500 50  0000 C CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 9338 2350 50  0001 C CNN
F 3 "~" H 9300 2500 50  0001 C CNN
	1    9300 2500
	0    1    1    0   
$EndComp
Wire Wire Line
	10300 2400 9150 2400
Wire Wire Line
	10300 2500 9450 2500
Wire Wire Line
	8850 2400 8750 2400
Wire Wire Line
	8750 2400 8750 2300
Connection ~ 8750 2300
Wire Wire Line
	8750 2500 8750 2400
Connection ~ 8750 2400
Wire Wire Line
	8750 2500 9150 2500
$Comp
L Device:C C12
U 1 1 5C13BD05
P 9600 2600
F 0 "C12" V 9348 2600 50  0000 C CNN
F 1 "105" V 9439 2600 50  0000 C CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 9638 2450 50  0001 C CNN
F 3 "~" H 9600 2600 50  0001 C CNN
	1    9600 2600
	0    1    1    0   
$EndComp
Wire Wire Line
	10300 2600 9750 2600
Wire Wire Line
	9450 2600 8750 2600
Wire Wire Line
	8750 2600 8750 2500
Connection ~ 8750 2500
Text GLabel 10150 2700 0    50   Input ~ 0
PREVGH
Text GLabel 10150 2900 0    50   Input ~ 0
PREVGL
Wire Wire Line
	10300 2700 10150 2700
Wire Wire Line
	10300 2900 10150 2900
$Comp
L Device:C C7
U 1 1 5C14DD58
P 9000 2800
F 0 "C7" V 8748 2800 50  0000 C CNN
F 1 "105" V 8839 2800 50  0000 C CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 9038 2650 50  0001 C CNN
F 3 "~" H 9000 2800 50  0001 C CNN
	1    9000 2800
	0    1    1    0   
$EndComp
$Comp
L Device:C C11
U 1 1 5C14DF10
P 9300 3000
F 0 "C11" V 9048 3000 50  0000 C CNN
F 1 "105" V 9139 3000 50  0000 C CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 9338 2850 50  0001 C CNN
F 3 "~" H 9300 3000 50  0001 C CNN
	1    9300 3000
	0    1    1    0   
$EndComp
Wire Wire Line
	10300 2800 9150 2800
Wire Wire Line
	10300 3000 9450 3000
Wire Wire Line
	8750 2600 8750 2800
Wire Wire Line
	8750 2800 8850 2800
Connection ~ 8750 2600
Wire Wire Line
	9150 3000 8750 3000
Wire Wire Line
	8750 3000 8750 2800
Connection ~ 8750 2800
$Comp
L power:GND #PWR09
U 1 1 5C15B420
P 8750 3200
F 0 "#PWR09" H 8750 2950 50  0001 C CNN
F 1 "GND" H 8755 3027 50  0000 C CNN
F 2 "" H 8750 3200 50  0001 C CNN
F 3 "" H 8750 3200 50  0001 C CNN
	1    8750 3200
	1    0    0    -1  
$EndComp
Wire Wire Line
	8750 3000 8750 3200
Connection ~ 8750 3000
Wire Wire Line
	8750 1400 8750 2100
Wire Wire Line
	8750 1400 10300 1400
Connection ~ 8750 2100
Wire Wire Line
	8750 1000 8750 1100
Wire Wire Line
	8750 1100 8750 1400
Connection ~ 8750 1100
Connection ~ 8750 1400
Text GLabel 5800 2850 0    50   Input ~ 0
GDR
$Comp
L Device:R R1
U 1 1 5C17447E
P 6100 3300
F 0 "R1" H 6170 3346 50  0000 L CNN
F 1 "10K" H 6170 3255 50  0000 L CNN
F 2 "Resistor_SMD:R_0603_1608Metric" V 6030 3300 50  0001 C CNN
F 3 "~" H 6100 3300 50  0001 C CNN
	1    6100 3300
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR04
U 1 1 5C174599
P 6100 3600
F 0 "#PWR04" H 6100 3350 50  0001 C CNN
F 1 "GND" H 6105 3427 50  0000 C CNN
F 2 "" H 6100 3600 50  0001 C CNN
F 3 "" H 6100 3600 50  0001 C CNN
	1    6100 3600
	1    0    0    -1  
$EndComp
Wire Wire Line
	5800 2850 6100 2850
Wire Wire Line
	6100 3450 6100 3600
$Comp
L Device:Q_NMOS_GSD Q1
U 1 1 5C17BA28
P 6750 2850
F 0 "Q1" H 6955 2896 50  0000 L CNN
F 1 "Q_NMOS_GSD" H 6955 2805 50  0000 L CNN
F 2 "Package_TO_SOT_SMD:SOT-23" H 6950 2950 50  0001 C CNN
F 3 "~" H 6750 2850 50  0001 C CNN
	1    6750 2850
	1    0    0    -1  
$EndComp
$Comp
L Device:R R2
U 1 1 5C17BB68
P 6850 3300
F 0 "R2" H 6920 3346 50  0000 L CNN
F 1 "0.47R" H 6920 3255 50  0000 L CNN
F 2 "Resistor_SMD:R_0603_1608Metric" V 6780 3300 50  0001 C CNN
F 3 "~" H 6850 3300 50  0001 C CNN
	1    6850 3300
	1    0    0    -1  
$EndComp
Wire Wire Line
	6100 2850 6100 3150
Wire Wire Line
	6100 2850 6550 2850
Connection ~ 6100 2850
Wire Wire Line
	6850 3050 6850 3100
$Comp
L power:GND #PWR05
U 1 1 5C18AA4A
P 6850 3600
F 0 "#PWR05" H 6850 3350 50  0001 C CNN
F 1 "GND" H 6855 3427 50  0000 C CNN
F 2 "" H 6850 3600 50  0001 C CNN
F 3 "" H 6850 3600 50  0001 C CNN
	1    6850 3600
	1    0    0    -1  
$EndComp
Wire Wire Line
	6850 3450 6850 3600
Text GLabel 7350 3100 2    50   Input ~ 0
RESE
Wire Wire Line
	7350 3100 6850 3100
Connection ~ 6850 3100
Wire Wire Line
	6850 3100 6850 3150
$Comp
L power:GND #PWR03
U 1 1 5C1927C5
P 6100 2550
F 0 "#PWR03" H 6100 2300 50  0001 C CNN
F 1 "GND" H 6105 2377 50  0000 C CNN
F 2 "" H 6100 2550 50  0001 C CNN
F 3 "" H 6100 2550 50  0001 C CNN
	1    6100 2550
	1    0    0    -1  
$EndComp
Wire Wire Line
	6100 2400 6100 2550
$Comp
L Device:C C1
U 1 1 5C1A6F02
P 6100 2250
F 0 "C1" H 6215 2296 50  0000 L CNN
F 1 "4.7uF" H 6215 2205 50  0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 6138 2100 50  0001 C CNN
F 3 "~" H 6100 2250 50  0001 C CNN
	1    6100 2250
	1    0    0    -1  
$EndComp
$Comp
L Device:L L1
U 1 1 5C1A6F75
P 6500 2000
F 0 "L1" V 6322 2000 50  0000 C CNN
F 1 "L" V 6413 2000 50  0000 C CNN
F 2 "Inductor_SMD:L_0805_2012Metric" H 6500 2000 50  0001 C CNN
F 3 "~" H 6500 2000 50  0001 C CNN
	1    6500 2000
	0    1    1    0   
$EndComp
$Comp
L power:+3V3 #PWR02
U 1 1 5C1AB386
P 6100 1750
F 0 "#PWR02" H 6100 1600 50  0001 C CNN
F 1 "+3V3" H 6115 1923 50  0000 C CNN
F 2 "" H 6100 1750 50  0001 C CNN
F 3 "" H 6100 1750 50  0001 C CNN
	1    6100 1750
	1    0    0    -1  
$EndComp
$Comp
L power:+3V3 #PWR01
U 1 1 5C1AF68D
P 1300 850
F 0 "#PWR01" H 1300 700 50  0001 C CNN
F 1 "+3V3" H 1315 1023 50  0000 C CNN
F 2 "" H 1300 850 50  0001 C CNN
F 3 "" H 1300 850 50  0001 C CNN
	1    1300 850 
	1    0    0    -1  
$EndComp
Wire Wire Line
	1300 850  1300 2200
$Comp
L Device:C C2
U 1 1 5C1B8729
P 6850 1400
F 0 "C2" H 6965 1446 50  0000 L CNN
F 1 "104" H 6965 1355 50  0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 6888 1250 50  0001 C CNN
F 3 "~" H 6850 1400 50  0001 C CNN
	1    6850 1400
	1    0    0    -1  
$EndComp
$Comp
L Device:D D3
U 1 1 5C1BD168
P 7350 2000
F 0 "D3" H 7350 1784 50  0000 C CNN
F 1 "D" H 7350 1875 50  0000 C CNN
F 2 "Diode_SMD:D_SOD-123" H 7350 2000 50  0001 C CNN
F 3 "~" H 7350 2000 50  0001 C CNN
	1    7350 2000
	-1   0    0    1   
$EndComp
$Comp
L Device:D D2
U 1 1 5C1BD26D
P 7350 1100
F 0 "D2" H 7350 1316 50  0000 C CNN
F 1 "D" H 7350 1225 50  0000 C CNN
F 2 "Diode_SMD:D_SOD-123" H 7350 1100 50  0001 C CNN
F 3 "~" H 7350 1100 50  0001 C CNN
	1    7350 1100
	1    0    0    -1  
$EndComp
$Comp
L Device:D D1
U 1 1 5C1BD2FB
P 7350 750
F 0 "D1" H 7350 534 50  0000 C CNN
F 1 "D" H 7350 625 50  0000 C CNN
F 2 "Diode_SMD:D_SOD-123" H 7350 750 50  0001 C CNN
F 3 "~" H 7350 750 50  0001 C CNN
	1    7350 750 
	-1   0    0    1   
$EndComp
Wire Wire Line
	6850 2000 7200 2000
Wire Wire Line
	7200 1100 6850 1100
Wire Wire Line
	6850 1100 6850 1250
Wire Wire Line
	7200 750  6850 750 
Wire Wire Line
	6850 750  6850 1100
Connection ~ 6850 1100
$Comp
L power:GND #PWR08
U 1 1 5C1CBA49
P 7750 750
F 0 "#PWR08" H 7750 500 50  0001 C CNN
F 1 "GND" V 7755 622 50  0000 R CNN
F 2 "" H 7750 750 50  0001 C CNN
F 3 "" H 7750 750 50  0001 C CNN
	1    7750 750 
	0    -1   -1   0   
$EndComp
Wire Wire Line
	7500 750  7750 750 
Text GLabel 7750 1100 2    50   Input ~ 0
PREVGL
Text GLabel 7750 2000 2    50   Input ~ 0
PREVGH
Wire Wire Line
	7500 1100 7600 1100
Wire Wire Line
	7500 2000 7600 2000
$Comp
L Device:C C3
U 1 1 5C1DB7B9
P 7600 1400
F 0 "C3" H 7715 1446 50  0000 L CNN
F 1 "105" H 7715 1355 50  0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 7638 1250 50  0001 C CNN
F 3 "~" H 7600 1400 50  0001 C CNN
	1    7600 1400
	1    0    0    -1  
$EndComp
$Comp
L Device:C C4
U 1 1 5C1DB857
P 7600 2300
F 0 "C4" H 7715 2346 50  0000 L CNN
F 1 "105" H 7715 2255 50  0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 7638 2150 50  0001 C CNN
F 3 "~" H 7600 2300 50  0001 C CNN
	1    7600 2300
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR06
U 1 1 5C1DB8E6
P 7600 1700
F 0 "#PWR06" H 7600 1450 50  0001 C CNN
F 1 "GND" H 7605 1527 50  0000 C CNN
F 2 "" H 7600 1700 50  0001 C CNN
F 3 "" H 7600 1700 50  0001 C CNN
	1    7600 1700
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR07
U 1 1 5C1DB936
P 7600 2600
F 0 "#PWR07" H 7600 2350 50  0001 C CNN
F 1 "GND" H 7605 2427 50  0000 C CNN
F 2 "" H 7600 2600 50  0001 C CNN
F 3 "" H 7600 2600 50  0001 C CNN
	1    7600 2600
	1    0    0    -1  
$EndComp
Connection ~ 6850 2000
Wire Wire Line
	6850 2000 6850 2650
Wire Wire Line
	7600 1100 7600 1250
Connection ~ 7600 1100
Wire Wire Line
	7600 1100 7750 1100
Wire Wire Line
	7600 2000 7600 2150
Connection ~ 7600 2000
Wire Wire Line
	7600 2000 7750 2000
Wire Wire Line
	7600 1550 7600 1700
Wire Wire Line
	7600 2450 7600 2600
Wire Wire Line
	6850 1550 6850 2000
Wire Wire Line
	6350 2000 6100 2000
Wire Wire Line
	6100 2000 6100 2100
Wire Wire Line
	6650 2000 6850 2000
Wire Wire Line
	6100 2000 6100 1750
Connection ~ 6100 2000
Text GLabel 950  1900 0    50   Input ~ 0
EPD_BUSY
Text GLabel 950  1600 0    50   Input ~ 0
EPD_RST
Text GLabel 950  1500 0    50   Input ~ 0
EPD_DC
Text GLabel 950  1400 0    50   Input ~ 0
EPD_CS
Text GLabel 950  1300 0    50   Input ~ 0
EPD_CLK
Text GLabel 950  1200 0    50   Input ~ 0
EPD_DIN
$Comp
L Device:Battery_Cell BT1
U 1 1 5C21F127
P 9050 4500
F 0 "BT1" H 9168 4596 50  0000 L CNN
F 1 "Battery_Cell" H 9168 4505 50  0000 L CNN
F 2 "Battery:BatteryHolder_MPD_BC2003_1x2032" V 9050 4560 50  0001 C CNN
F 3 "~" V 9050 4560 50  0001 C CNN
	1    9050 4500
	1    0    0    -1  
$EndComp
$Comp
L power:+3V3 #PWR013
U 1 1 5C21F233
P 9050 4100
F 0 "#PWR013" H 9050 3950 50  0001 C CNN
F 1 "+3V3" H 9065 4273 50  0000 C CNN
F 2 "" H 9050 4100 50  0001 C CNN
F 3 "" H 9050 4100 50  0001 C CNN
	1    9050 4100
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR014
U 1 1 5C21F286
P 9050 4800
F 0 "#PWR014" H 9050 4550 50  0001 C CNN
F 1 "GND" H 9055 4627 50  0000 C CNN
F 2 "" H 9050 4800 50  0001 C CNN
F 3 "" H 9050 4800 50  0001 C CNN
	1    9050 4800
	1    0    0    -1  
$EndComp
Wire Wire Line
	9050 4600 9050 4800
Wire Wire Line
	9050 4100 9050 4300
$Comp
L Connector:Conn_01x01_Male J1
U 1 1 5C22BF7B
P 1150 4700
F 0 "J1" H 1256 4878 50  0000 C CNN
F 1 "Conn_01x01_Male" H 1256 4787 50  0000 C CNN
F 2 "TestPoint:TestPoint_Pad_2.0x2.0mm" H 1150 4700 50  0001 C CNN
F 3 "~" H 1150 4700 50  0001 C CNN
	1    1150 4700
	1    0    0    -1  
$EndComp
$Comp
L Connector:Conn_01x01_Male J2
U 1 1 5C22C105
P 1150 4950
F 0 "J2" H 1256 5128 50  0000 C CNN
F 1 "Conn_01x01_Male" H 1256 5037 50  0000 C CNN
F 2 "TestPoint:TestPoint_Pad_2.0x2.0mm" H 1150 4950 50  0001 C CNN
F 3 "~" H 1150 4950 50  0001 C CNN
	1    1150 4950
	1    0    0    -1  
$EndComp
$Comp
L Connector:Conn_01x01_Male J4
U 1 1 5C22C189
P 1150 5200
F 0 "J4" H 1256 5378 50  0000 C CNN
F 1 "Conn_01x01_Male" H 1256 5287 50  0000 C CNN
F 2 "TestPoint:TestPoint_Pad_2.0x2.0mm" H 1150 5200 50  0001 C CNN
F 3 "~" H 1150 5200 50  0001 C CNN
	1    1150 5200
	1    0    0    -1  
$EndComp
$Comp
L Connector:Conn_01x01_Male J5
U 1 1 5C22C1F7
P 1150 5450
F 0 "J5" H 1256 5628 50  0000 C CNN
F 1 "Conn_01x01_Male" H 1256 5537 50  0000 C CNN
F 2 "TestPoint:TestPoint_Pad_2.0x2.0mm" H 1150 5450 50  0001 C CNN
F 3 "~" H 1150 5450 50  0001 C CNN
	1    1150 5450
	1    0    0    -1  
$EndComp
Wire Wire Line
	1350 4700 1850 4700
Wire Wire Line
	1350 4950 1850 4950
Wire Wire Line
	1350 5200 1650 5200
Wire Wire Line
	1350 5450 1850 5450
Text GLabel 1850 4700 2    50   Input ~ 0
A
Text GLabel 1850 4950 2    50   Input ~ 0
B
Text GLabel 1850 5200 2    50   Input ~ 0
C
Text GLabel 1850 5450 2    50   Input ~ 0
SW
Wire Wire Line
	2750 1500 3300 1500
Wire Wire Line
	2750 1600 3300 1600
$Comp
L power:GND #PWR012
U 1 1 5C283978
P 3200 1850
F 0 "#PWR012" H 3200 1600 50  0001 C CNN
F 1 "GND" H 3205 1677 50  0000 C CNN
F 2 "" H 3200 1850 50  0001 C CNN
F 3 "" H 3200 1850 50  0001 C CNN
	1    3200 1850
	1    0    0    -1  
$EndComp
$Comp
L power:+3V3 #PWR011
U 1 1 5C283A39
P 3200 1300
F 0 "#PWR011" H 3200 1150 50  0001 C CNN
F 1 "+3V3" H 3215 1473 50  0000 C CNN
F 2 "" H 3200 1300 50  0001 C CNN
F 3 "" H 3200 1300 50  0001 C CNN
	1    3200 1300
	1    0    0    -1  
$EndComp
Wire Wire Line
	3300 1400 3200 1400
Wire Wire Line
	3200 1400 3200 1300
Wire Wire Line
	3300 1700 3200 1700
Wire Wire Line
	3200 1700 3200 1850
$Comp
L Connector:Conn_01x01_Male J6
U 1 1 5C293351
P 3500 1400
F 0 "J6" H 3473 1330 50  0000 R CNN
F 1 "Conn_01x01_Male" H 3473 1421 50  0000 R CNN
F 2 "TestPoint:TestPoint_Pad_1.0x1.0mm" H 3500 1400 50  0001 C CNN
F 3 "~" H 3500 1400 50  0001 C CNN
	1    3500 1400
	-1   0    0    1   
$EndComp
$Comp
L Connector:Conn_01x01_Male J7
U 1 1 5C293516
P 3500 1500
F 0 "J7" H 3473 1430 50  0000 R CNN
F 1 "Conn_01x01_Male" H 3473 1521 50  0000 R CNN
F 2 "TestPoint:TestPoint_Pad_1.0x1.0mm" H 3500 1500 50  0001 C CNN
F 3 "~" H 3500 1500 50  0001 C CNN
	1    3500 1500
	-1   0    0    1   
$EndComp
$Comp
L Connector:Conn_01x01_Male J8
U 1 1 5C293650
P 3500 1600
F 0 "J8" H 3473 1530 50  0000 R CNN
F 1 "Conn_01x01_Male" H 3473 1621 50  0000 R CNN
F 2 "TestPoint:TestPoint_Pad_1.0x1.0mm" H 3500 1600 50  0001 C CNN
F 3 "~" H 3500 1600 50  0001 C CNN
	1    3500 1600
	-1   0    0    1   
$EndComp
$Comp
L Connector:Conn_01x01_Male J9
U 1 1 5C2936C7
P 3500 1700
F 0 "J9" H 3473 1630 50  0000 R CNN
F 1 "Conn_01x01_Male" H 3473 1721 50  0000 R CNN
F 2 "TestPoint:TestPoint_Pad_1.0x1.0mm" H 3500 1700 50  0001 C CNN
F 3 "~" H 3500 1700 50  0001 C CNN
	1    3500 1700
	-1   0    0    1   
$EndComp
Wire Wire Line
	1350 1200 950  1200
Wire Wire Line
	950  1300 1350 1300
Wire Wire Line
	1350 1400 950  1400
Wire Wire Line
	950  1500 1350 1500
Wire Wire Line
	1350 1600 950  1600
Wire Wire Line
	950  1900 1350 1900
Wire Wire Line
	1700 3050 2200 3050
Text GLabel 2200 3050 2    50   Input ~ 0
A
Text GLabel 2200 3300 2    50   Input ~ 0
B
Text GLabel 2300 3550 2    50   Input ~ 0
SW
Wire Wire Line
	1600 3300 2200 3300
Wire Wire Line
	1500 3550 1500 2550
Wire Wire Line
	1500 3550 2300 3550
$Comp
L Device:Rotary_Encoder_Switch SW1
U 1 1 5C254D3E
P 4300 4800
F 0 "SW1" H 4300 5167 50  0000 C CNN
F 1 "Rotary_Encoder_Switch" H 4300 5076 50  0000 C CNN
F 2 "Rotary_Encoder:RotaryEncoder_Alps_EC12E-Switch_Vertical_H20mm" H 4150 4960 50  0001 C CNN
F 3 "~" H 4300 5060 50  0001 C CNN
	1    4300 4800
	1    0    0    -1  
$EndComp
$Comp
L Connector:Conn_01x01_Male J10
U 1 1 5C254FA2
P 3350 5400
F 0 "J10" H 3456 5578 50  0000 C CNN
F 1 "Conn_01x01_Male" H 3456 5487 50  0000 C CNN
F 2 "TestPoint:TestPoint_Pad_2.0x2.0mm" H 3350 5400 50  0001 C CNN
F 3 "~" H 3350 5400 50  0001 C CNN
	1    3350 5400
	1    0    0    -1  
$EndComp
$Comp
L Connector:Conn_01x01_Male J11
U 1 1 5C254FA9
P 3350 5650
F 0 "J11" H 3456 5828 50  0000 C CNN
F 1 "Conn_01x01_Male" H 3456 5737 50  0000 C CNN
F 2 "TestPoint:TestPoint_Pad_2.0x2.0mm" H 3350 5650 50  0001 C CNN
F 3 "~" H 3350 5650 50  0001 C CNN
	1    3350 5650
	1    0    0    -1  
$EndComp
$Comp
L Connector:Conn_01x01_Male J12
U 1 1 5C254FB0
P 3350 5900
F 0 "J12" H 3456 6078 50  0000 C CNN
F 1 "Conn_01x01_Male" H 3456 5987 50  0000 C CNN
F 2 "TestPoint:TestPoint_Pad_2.0x2.0mm" H 3350 5900 50  0001 C CNN
F 3 "~" H 3350 5900 50  0001 C CNN
	1    3350 5900
	1    0    0    -1  
$EndComp
$Comp
L Connector:Conn_01x01_Male J13
U 1 1 5C254FB7
P 3350 6150
F 0 "J13" H 3456 6328 50  0000 C CNN
F 1 "Conn_01x01_Male" H 3456 6237 50  0000 C CNN
F 2 "TestPoint:TestPoint_Pad_2.0x2.0mm" H 3350 6150 50  0001 C CNN
F 3 "~" H 3350 6150 50  0001 C CNN
	1    3350 6150
	1    0    0    -1  
$EndComp
Wire Wire Line
	3550 5400 3850 5400
Wire Wire Line
	3550 5650 3950 5650
Wire Wire Line
	3550 5900 3900 5900
Wire Wire Line
	3550 6150 3900 6150
Text GLabel 4050 5400 2    50   Input ~ 0
A
Text GLabel 4050 5650 2    50   Input ~ 0
B
Text GLabel 4050 5900 2    50   Input ~ 0
C
Text GLabel 4050 6150 2    50   Input ~ 0
SW
Wire Wire Line
	4000 4900 3950 4900
Wire Wire Line
	3950 4900 3950 5650
Connection ~ 3950 5650
Wire Wire Line
	3950 5650 4050 5650
Wire Wire Line
	4000 4800 3900 4800
Wire Wire Line
	3900 4800 3900 5900
Connection ~ 3900 5900
Wire Wire Line
	3900 5900 4050 5900
Wire Wire Line
	4000 4700 3850 4700
Wire Wire Line
	3850 4700 3850 5400
Connection ~ 3850 5400
Wire Wire Line
	3850 5400 4050 5400
Wire Wire Line
	4600 4900 4650 4900
Wire Wire Line
	4650 4900 4650 6050
Wire Wire Line
	4650 6050 3900 6050
Wire Wire Line
	3900 6050 3900 5900
Wire Wire Line
	3900 6150 3900 6300
Wire Wire Line
	3900 6300 4700 6300
Wire Wire Line
	4700 6300 4700 4700
Wire Wire Line
	4700 4700 4600 4700
Connection ~ 3900 6150
Wire Wire Line
	3900 6150 4050 6150
Wire Wire Line
	1600 2550 1600 3300
Wire Wire Line
	1700 2550 1700 3050
$Comp
L power:GND #PWR0102
U 1 1 5C2A3AC4
P 1650 5750
F 0 "#PWR0102" H 1650 5500 50  0001 C CNN
F 1 "GND" H 1655 5577 50  0000 C CNN
F 2 "" H 1650 5750 50  0001 C CNN
F 3 "" H 1650 5750 50  0001 C CNN
	1    1650 5750
	1    0    0    -1  
$EndComp
Wire Wire Line
	1650 5750 1650 5200
Connection ~ 1650 5200
Wire Wire Line
	1650 5200 1850 5200
$Comp
L topsulin:topsulin_topsulin_topsulin_topsulin_library_nRF51822_module_waveshare U1
U 1 1 5CD142A6
P 2050 1800
F 0 "U1" H 2050 2915 50  0000 C CNN
F 1 "topsulin_topsulin_library_nRF51822_module_waveshare" H 2050 2824 50  0000 C CNN
F 2 "" H 2050 1800 50  0001 C CNN
F 3 "" H 2050 1800 50  0001 C CNN
	1    2050 1800
	1    0    0    -1  
$EndComp
$EndSCHEMATC