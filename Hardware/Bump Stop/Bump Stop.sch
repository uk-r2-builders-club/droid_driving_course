EESchema Schematic File Version 4
LIBS:Bump Stop-cache
EELAYER 26 0
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
$Comp
L wemos_mini:WeMos_mini U1
U 1 1 5C9C8FA5
P 5250 3950
F 0 "U1" H 5250 4587 60  0000 C CNN
F 1 "WeMos_mini" H 5250 4481 60  0000 C CNN
F 2 "wemos-d1-mini:wemos-d1-mini-with-pin-header" H 5800 3250 60  0001 C CNN
F 3 "" H 5250 4481 60  0000 C CNN
	1    5250 3950
	1    0    0    -1  
$EndComp
$Comp
L Switch:SW_DIP_x04 SW1
U 1 1 5C9C9005
P 7050 4000
F 0 "SW1" H 7050 4467 50  0000 C CNN
F 1 "ADDRESS" H 7050 4376 50  0000 C CNN
F 2 "Buttons_Switches_THT:SW_DIP_x4_W7.62mm_Slide" H 7050 4000 50  0001 C CNN
F 3 "" H 7050 4000 50  0001 C CNN
	1    7050 4000
	1    0    0    -1  
$EndComp
$Comp
L Connector:Screw_Terminal_01x02 J3
U 1 1 5C9C9527
P 3400 6000
F 0 "J3" V 3273 6080 50  0000 L CNN
F 1 "PASS1" V 3364 6080 50  0000 L CNN
F 2 "TerminalBlocks_Phoenix:TerminalBlock_Phoenix_MPT-2.54mm_2pol" H 3400 6000 50  0001 C CNN
F 3 "~" H 3400 6000 50  0001 C CNN
	1    3400 6000
	0    1    1    0   
$EndComp
$Comp
L Connector:Screw_Terminal_01x02 J4
U 1 1 5C9C95A5
P 3900 6000
F 0 "J4" V 3773 6080 50  0000 L CNN
F 1 "PASS2" V 3864 6080 50  0000 L CNN
F 2 "TerminalBlocks_Phoenix:TerminalBlock_Phoenix_MPT-2.54mm_2pol" H 3900 6000 50  0001 C CNN
F 3 "~" H 3900 6000 50  0001 C CNN
	1    3900 6000
	0    1    1    0   
$EndComp
$Comp
L Connector:Screw_Terminal_01x02 J5
U 1 1 5C9C960F
P 4400 6000
F 0 "J5" V 4273 6080 50  0000 L CNN
F 1 "FAIL1" V 4364 6080 50  0000 L CNN
F 2 "TerminalBlocks_Phoenix:TerminalBlock_Phoenix_MPT-2.54mm_2pol" H 4400 6000 50  0001 C CNN
F 3 "~" H 4400 6000 50  0001 C CNN
	1    4400 6000
	0    1    1    0   
$EndComp
$Comp
L Connector:Screw_Terminal_01x02 J6
U 1 1 5C9C963F
P 4900 6000
F 0 "J6" V 4773 6080 50  0000 L CNN
F 1 "FAIL2" V 4864 6080 50  0000 L CNN
F 2 "TerminalBlocks_Phoenix:TerminalBlock_Phoenix_MPT-2.54mm_2pol" H 4900 6000 50  0001 C CNN
F 3 "~" H 4900 6000 50  0001 C CNN
	1    4900 6000
	0    1    1    0   
$EndComp
$Comp
L Connector:Screw_Terminal_01x02 J2
U 1 1 5C9C9A18
P 7000 4900
F 0 "J2" H 6920 4575 50  0000 C CNN
F 1 "RESET" H 6920 4666 50  0000 C CNN
F 2 "TerminalBlocks_Phoenix:TerminalBlock_Phoenix_MPT-2.54mm_2pol" H 7000 4900 50  0001 C CNN
F 3 "~" H 7000 4900 50  0001 C CNN
	1    7000 4900
	1    0    0    -1  
$EndComp
$Comp
L Connector:Screw_Terminal_01x03 J1
U 1 1 5C9C9B27
P 3200 3600
F 0 "J1" H 3120 3275 50  0000 C CNN
F 1 "LED" H 3120 3366 50  0000 C CNN
F 2 "TerminalBlocks_Phoenix:TerminalBlock_Phoenix_MPT-2.54mm_3pol" H 3200 3600 50  0001 C CNN
F 3 "~" H 3200 3600 50  0001 C CNN
	1    3200 3600
	-1   0    0    1   
$EndComp
$Comp
L power:GND #PWR04
U 1 1 5C9C9C25
P 5600 5800
F 0 "#PWR04" H 5600 5550 50  0001 C CNN
F 1 "GND" H 5605 5627 50  0000 C CNN
F 2 "" H 5600 5800 50  0001 C CNN
F 3 "" H 5600 5800 50  0001 C CNN
	1    5600 5800
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR01
U 1 1 5C9C9C5F
P 3750 4000
F 0 "#PWR01" H 3750 3750 50  0001 C CNN
F 1 "GND" H 3755 3827 50  0000 C CNN
F 2 "" H 3750 4000 50  0001 C CNN
F 3 "" H 3750 4000 50  0001 C CNN
	1    3750 4000
	1    0    0    -1  
$EndComp
$Comp
L power:+3V3 #PWR03
U 1 1 5C9C9CA0
P 5850 2850
F 0 "#PWR03" H 5850 2700 50  0001 C CNN
F 1 "+3V3" H 5865 3023 50  0000 C CNN
F 2 "" H 5850 2850 50  0001 C CNN
F 3 "" H 5850 2850 50  0001 C CNN
	1    5850 2850
	1    0    0    -1  
$EndComp
$Comp
L power:+5V #PWR02
U 1 1 5C9C9CE9
P 4400 3200
F 0 "#PWR02" H 4400 3050 50  0001 C CNN
F 1 "+5V" H 4415 3373 50  0000 C CNN
F 2 "" H 4400 3200 50  0001 C CNN
F 3 "" H 4400 3200 50  0001 C CNN
	1    4400 3200
	1    0    0    -1  
$EndComp
Wire Wire Line
	7350 4100 7450 4100
Wire Wire Line
	7450 4100 7450 4000
Wire Wire Line
	7350 3800 7450 3800
Wire Wire Line
	7350 3900 7450 3900
Connection ~ 7450 3900
Wire Wire Line
	7450 3900 7450 3800
Wire Wire Line
	7350 4000 7450 4000
Connection ~ 7450 4000
Wire Wire Line
	7450 4000 7450 3900
Wire Wire Line
	4900 5800 4900 5700
Wire Wire Line
	4900 5700 5600 5700
Wire Wire Line
	5600 5700 5600 5800
Wire Wire Line
	4400 5800 4400 5700
Wire Wire Line
	4400 5700 4900 5700
Connection ~ 4900 5700
Wire Wire Line
	3900 5700 4400 5700
Wire Wire Line
	3900 5700 3900 5800
Connection ~ 4400 5700
Wire Wire Line
	3400 5800 3400 5700
Wire Wire Line
	3400 5700 3900 5700
Connection ~ 3900 5700
Wire Wire Line
	3400 3500 4400 3500
Wire Wire Line
	4750 3600 4400 3600
Wire Wire Line
	4400 3200 4400 3500
Connection ~ 4400 3500
Wire Wire Line
	4400 3500 4400 3600
Wire Wire Line
	3400 3600 4200 3600
Wire Wire Line
	4200 3600 4200 3800
Wire Wire Line
	4200 3800 4750 3800
Wire Wire Line
	4800 5800 4800 5550
Wire Wire Line
	4800 5550 4300 5550
Wire Wire Line
	4300 5550 4300 5800
Wire Wire Line
	4300 5550 4300 5300
Connection ~ 4300 5550
Wire Wire Line
	3300 5800 3300 5550
Wire Wire Line
	3300 5550 3800 5550
Wire Wire Line
	3800 5550 3800 5800
Wire Wire Line
	3800 5550 4200 5550
Wire Wire Line
	4200 5550 4200 5100
Connection ~ 3800 5550
$Comp
L power:GND #PWR06
U 1 1 5CA3CA77
P 7450 4300
F 0 "#PWR06" H 7450 4050 50  0001 C CNN
F 1 "GND" H 7455 4127 50  0000 C CNN
F 2 "" H 7450 4300 50  0001 C CNN
F 3 "" H 7450 4300 50  0001 C CNN
	1    7450 4300
	1    0    0    -1  
$EndComp
Wire Wire Line
	7450 4300 7450 4100
Connection ~ 7450 4100
$Comp
L Device:R R1
U 1 1 5CA3D6F2
P 6100 3450
F 0 "R1" V 6000 3450 50  0000 L CNN
F 1 "10K" V 6100 3400 50  0000 L CNN
F 2 "Resistors_THT:R_Axial_DIN0204_L3.6mm_D1.6mm_P7.62mm_Horizontal" V 6030 3450 50  0001 C CNN
F 3 "~" H 6100 3450 50  0001 C CNN
	1    6100 3450
	1    0    0    -1  
$EndComp
$Comp
L Device:R R2
U 1 1 5CA3D78D
P 6300 3450
F 0 "R2" V 6200 3450 50  0000 L CNN
F 1 "10K" V 6300 3400 50  0000 L CNN
F 2 "Resistors_THT:R_Axial_DIN0204_L3.6mm_D1.6mm_P7.62mm_Horizontal" V 6230 3450 50  0001 C CNN
F 3 "~" H 6300 3450 50  0001 C CNN
	1    6300 3450
	1    0    0    -1  
$EndComp
$Comp
L Device:R R5
U 1 1 5CA3D817
P 6500 3450
F 0 "R5" V 6400 3450 50  0000 L CNN
F 1 "10k" V 6500 3400 50  0000 L CNN
F 2 "Resistors_THT:R_Axial_DIN0204_L3.6mm_D1.6mm_P7.62mm_Horizontal" V 6430 3450 50  0001 C CNN
F 3 "~" H 6500 3450 50  0001 C CNN
	1    6500 3450
	1    0    0    -1  
$EndComp
$Comp
L Device:R R6
U 1 1 5CA3D8B4
P 6700 3450
F 0 "R6" V 6600 3450 50  0000 L CNN
F 1 "10k" V 6700 3400 50  0000 L CNN
F 2 "Resistors_THT:R_Axial_DIN0204_L3.6mm_D1.6mm_P7.62mm_Horizontal" V 6630 3450 50  0001 C CNN
F 3 "~" H 6700 3450 50  0001 C CNN
	1    6700 3450
	1    0    0    -1  
$EndComp
$Comp
L Device:R R3
U 1 1 5CA3D988
P 4600 5100
F 0 "R3" V 4600 5100 50  0000 C CNN
F 1 "10K" V 4700 5100 50  0000 C CNN
F 2 "Resistors_THT:R_Axial_DIN0204_L3.6mm_D1.6mm_P7.62mm_Horizontal" V 4530 5100 50  0001 C CNN
F 3 "~" H 4600 5100 50  0001 C CNN
	1    4600 5100
	0    1    1    0   
$EndComp
$Comp
L Device:R R4
U 1 1 5CA3DA4D
P 4600 5300
F 0 "R4" V 4600 5300 50  0000 C CNN
F 1 "10k" V 4700 5300 50  0000 C CNN
F 2 "Resistors_THT:R_Axial_DIN0204_L3.6mm_D1.6mm_P7.62mm_Horizontal" V 4530 5300 50  0001 C CNN
F 3 "~" H 4600 5300 50  0001 C CNN
	1    4600 5300
	0    1    1    0   
$EndComp
Wire Wire Line
	6100 3600 6100 3800
Wire Wire Line
	6100 3800 6750 3800
Wire Wire Line
	6300 3600 6300 3900
Wire Wire Line
	6300 3900 6750 3900
Wire Wire Line
	6500 3600 6500 4000
Wire Wire Line
	6500 4000 6750 4000
Wire Wire Line
	6700 3600 6700 4100
Wire Wire Line
	6700 4100 6750 4100
Wire Wire Line
	5750 3600 5850 3600
Wire Wire Line
	5850 3600 5850 3000
Connection ~ 5850 3000
Wire Wire Line
	5850 3000 5850 2850
Wire Wire Line
	4450 5100 4200 5100
Wire Wire Line
	4450 5300 4300 5300
Wire Wire Line
	4750 5300 4950 5300
Wire Wire Line
	4950 4900 4950 5100
Wire Wire Line
	4750 5100 4950 5100
Connection ~ 4950 5100
Wire Wire Line
	4950 5100 4950 5300
$Comp
L power:+3V3 #PWR05
U 1 1 5CA4F070
P 4950 4900
F 0 "#PWR05" H 4950 4750 50  0001 C CNN
F 1 "+3V3" H 4965 5073 50  0000 C CNN
F 2 "" H 4950 4900 50  0001 C CNN
F 3 "" H 4950 4900 50  0001 C CNN
	1    4950 4900
	1    0    0    -1  
$EndComp
Wire Wire Line
	5850 3000 6100 3000
Wire Wire Line
	6700 3000 6700 3300
Wire Wire Line
	6500 3000 6500 3300
Connection ~ 6500 3000
Wire Wire Line
	6500 3000 6700 3000
Wire Wire Line
	6300 3000 6300 3300
Connection ~ 6300 3000
Wire Wire Line
	6300 3000 6500 3000
Wire Wire Line
	6100 3000 6100 3300
Connection ~ 6100 3000
Wire Wire Line
	6100 3000 6300 3000
Wire Wire Line
	6100 3800 5750 3800
Connection ~ 6100 3800
Wire Wire Line
	6300 3900 5750 3900
Connection ~ 6300 3900
Wire Wire Line
	6500 4000 5750 4000
Connection ~ 6500 4000
Wire Wire Line
	6700 4100 5750 4100
Connection ~ 6700 4100
Wire Wire Line
	3400 3700 3750 3700
Wire Wire Line
	3750 3700 3750 4000
Connection ~ 3750 3700
Wire Wire Line
	3750 3700 4750 3700
Wire Wire Line
	6800 4900 6700 4900
Wire Wire Line
	6700 4900 6700 4300
Wire Wire Line
	6700 4300 5750 4300
Wire Wire Line
	6800 5000 6450 5000
Wire Wire Line
	6450 5000 6450 5250
$Comp
L power:GND #PWR07
U 1 1 5CABAEEE
P 6450 5250
F 0 "#PWR07" H 6450 5000 50  0001 C CNN
F 1 "GND" H 6455 5077 50  0000 C CNN
F 2 "" H 6450 5250 50  0001 C CNN
F 3 "" H 6450 5250 50  0001 C CNN
	1    6450 5250
	1    0    0    -1  
$EndComp
Wire Wire Line
	4750 4000 4200 4000
Wire Wire Line
	4200 4000 4200 5100
Connection ~ 4200 5100
Wire Wire Line
	4750 3900 4300 3900
Wire Wire Line
	4300 3900 4300 5300
Connection ~ 4300 5300
$EndSCHEMATC
