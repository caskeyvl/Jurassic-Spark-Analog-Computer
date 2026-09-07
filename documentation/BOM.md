# Bill of Materials

This is pulled from the project's PDR (Preliminary Design Review) parts-costing tab, since a fully updated final BOM was never delivered by the finance manager side of the project.

Treat this as a real, working parts list — not a final invoice. Some items below were superseded as the design matured past PDR (called out under "Known changes since PDR"), and all prices are PDR-stage quotes/estimates rather than confirmed final purchase prices. Cross-reference against `hardware/` and `mechanical/` before ordering anything.

## Known changes since PDR

- **Oscilloscope ADC/mux**: the PDR plan used a 16-channel multiplexer (`CD74HC4067`) to route module signals into an ADC. The final design instead uses a single Waveshare ADS1263 ADC HAT (10-channel, native differential inputs) that reads all 4 module channels directly — no external mux needed. See [`../hardware/oscilloscope-adc-hat/`](../hardware/oscilloscope-adc-hat/) and `../oscilloscope/README.md`. The HAT's own part number/price isn't in the PDR sheet and needs to be added (Waveshare "ADS1263 ADC HAT").
- **Mechatronic sensing**: the PDR plan used 3x Microchip `MCP3204` 4-channel ADCs, presumably for analog ball-position sensing. The final mechatronic system instead uses a VL53L0x time-of-flight distance sensor (per the Arduino firmware in `../mechatronic/`) — the MCP3204 line below was not used in the final build. VL53L0x part/price needs to be added.
- **Custom oscilloscope ADC HAT (ADS4195)**: an earlier, ultimately abandoned attempt at a custom ADC HAT using an ADS4195 (signal integrity issues killed it — see [`../hardware/README.md`](../hardware/README.md)). Not reflected in this sheet at all and never a final cost, but the PCBs exist in `../hardware/oscilloscope-adc-hat/` for reference.
- **Display**: PDR lists a Waveshare 6.25" capacitive touch display, which lines up with the final report's description of "the LCD screen" — kept as-is below since nothing contradicts it, but worth a sanity-check against whatever's actually installed.

Anything not flagged above is assumed to still reflect the as-built design, but wasn't independently re-verified item-by-item against the final hardware — treat quantities in particular as a starting point to recount against actual stock/spares on hand.

## Analog Computer (modules, power distribution, housing)

| Component | Spec | Qty | Package | Manufacturer | Mfr Part # | Distributor Part # | Price/Item | Total |
|---|---|---|---|---|---|---|---|---|
| 24V PSU | 24V, 156W, 6.5A, 89% efficient, adjustable output | 2 | Chassis mount | Mean Well USA | LRS-150-24 | 1866-3321-ND | $19.99 | $39.98 |
| 12V LED | 12V, 8mm red, voltage-protected | 2 | Panel mount | FILN | N/A | FL1A-08FW-1_R_12_5 | $11.99 | $23.98 |
| Buzzer | 12V, 95dB, 20mA max | 2 | Panel mount | HiLetgo | 3-01-1588 | 3-01-1588 | $7.59 | $15.18 |
| Main power port | 125V 10A fused power socket/switch, 4.9ft cable | 2 | Chassis mount | N/A | ASDS-ZH016 | B0B4256Y2S | $15.99 | $31.98 |
| Aluminum housing | 5052 aluminum sheet, 18"x24", 1/16" | 1 | Sheet | PCB Way | — | — | $65.00 | $39.99 |
| Banana plugs | Gold-plated 3.5mm/3mm | 1 | Banana plug | LINSYRC | B09MCKKBHJ | 732688553829 | $7.99 | $7.99 |
| XT30 connector (male) | 5A/contact solid-state connector | 1 | Pin solder | Amass | XT30PW-M | XT30PW-M | $10.30 | $10.30 |
| XT30 connector (female) | 5A/contact solid-state connector | 1 | Pin solder | Amass | XT30PW-F | XT30PW-F | $10.30 | $10.30 |
| Comparator (THT) | ~36V dual comparator, 5mV input offset | 10 | Thru-hole | Texas Instruments | LM393P | 296-1398-5-ND | $0.33 | $3.31 |
| Comparator (SMT) | ~36V dual comparator, 5mV input offset | 10 | SMT | Texas Instruments | LM393DR | 296-1015-1-ND | $0.15 | $1.52 |
| Dual op-amp | 36V RRIO mux-friendly op-amp | 25 | SMT | Texas Instruments | OPA196IDR | 595-OPA196IDR | $1.01 | $25.25 |
| 3D print | UV resin printed components | 1 | N/A | — | — | — | $15.00 | $15.00 |
| Wood plank | 0.5"x5.5"x48" unfinished red oak | 1 | Box | Home Depot | BRDNAT 1-2X6X4ROAK | 333192905 | $11.71 | $11.71 |
| PCB, power rails | — | 3 | SMD/SMT | JLCPCB | — | — | $5.00 | $15.00 |
| PCB, modules | — | 12 | SMD/SMT | JLCPCB | — | — | $2.00 | $24.00 |
| PCB, motherboard | — | 3 | SMD/SMT | JLCPCB | — | — | $5.00 | $15.00 |
| Capacitor 0.1uF | MLCC | 30 | SMD/SMT | Yageo | CC1206MRY5V9BB104 | 603-CC126MRY5V9BB104 | $0.11 | $3.36 |
| Resistor 100Ω | Thick film | 200 | SMD/SMT | Panasonic | ERJ-P08J101V | 667-ERJ-P08J101V | $0.03 | $5.60 |
| Resistor 1kΩ | Thick film | 100 | SMD/SMT | TE Connectivity | CRGS1206J1K0 | 279-CRGS1206J1K0 | $0.09 | $8.80 |
| Resistor 3kΩ | Thick film | 100 | SMD/SMT | Panasonic | ERJ-T08J302V | 667-ERJ-T08J302V | $0.06 | $6.30 |
| Resistor 100kΩ | Thick film | 100 | SMD/SMT | Vishay | CRCW1206100KFKEBC | 71-CRCW1206100KFKEBC | $0.03 | $2.70 |
| Resistor 330kΩ | Thick film | 100 | SMD/SMT | Yageo | RE1206DRE07330KL | 603-RE1206DRE07330KL | $0.05 | $4.50 |
| Rocker switch | Heavy duty KCD4 | 1 | Chassis mount | N/A | B0FD8LDTS6 | B0FD8LDTS6 | $15.99 | $15.99 |
| Potentiometer | 100kΩ precision, metal shaft | 10 | Pin solder | Jebhane | RK097 | RK097 | $1.00 | $10.00 |

**Section total: $347.74**

## Mechatronic (ball-in-tube demonstrator)

| Component | Spec | Qty | Package | Manufacturer | Mfr Part # | Distributor Part # | Price/Item | Total |
|---|---|---|---|---|---|---|---|---|
| Acrylic tube | 6" x 1.5" | 1 | N/A | Mexanixxity | B0B9C6951J | B0B9C6951J | $12.59 | $12.59 |
| Sheet metal | Aluminum, 8"x12" | 1 | N/A | Lswteiz | B09YGZRKD4 | B09YGZRKD4 | $10.99 | $10.99 |
| 3D print | UV resin printed components | 1 | N/A | — | — | — | $15.00 | $15.00 |
| Banana plugs | Gold-plated 3.5mm/3mm | 1 | Banana plug | LINSYRC | B09MCKKBHJ | 732688553829 | $7.99 | $7.99 |
| Potentiometer | 100kΩ precision, metal shaft | 5 | Pin solder | Jebhane | RK097 | RK097 | $1.00 | $5.00 |
| Rocker switch | Heavy duty KCD4 | 1 | Chassis mount | N/A | B0FD8LDTS6 | B0FD8LDTS6 | $15.99 | $15.99 |
| PCB, motherboard | — | 3 | SMD/SMT | JLCPCB | — | — | $5.00 | $15.00 |
| 12V LED | 12V, 8mm red, voltage-protected | 2 | Panel mount | FILN | N/A | FL1A-08FW-1_R_12_5 | $11.99 | $23.98 |
| Buzzer | 12V, 95dB, 20mA max | 2 | Panel mount | HiLetgo | 3-01-1588 | 3-01-1588 | $7.59 | $15.18 |
| Main power port | 125V 10A fused power socket/switch, 4.9ft cable | 2 | Chassis mount | N/A | ASDS-ZH016 | B0B4256Y2S | $15.99 | $31.98 |
| Wood plank | 0.5"x5.5"x12" unfinished red oak | 1 | Box | Home Depot | BRDNAT 1-2X6X4ROAK | 333192905 | $8.99 | $8.99 |
| ~~4-channel ADC~~ | ~~12-bit SPI 100kS/s differential ADC~~ | ~~3~~ | ~~SMT~~ | ~~Microchip~~ | ~~MCP3204T-CI/SL~~ | ~~MCP3204T-CI/SLTR-ND~~ | ~~$1.72~~ | ~~$5.16~~ |
| 12V brushless motor | 6000 RPM, w/ tachometer | 1 | N/A | Robot Shop | BLDC3650-24-8000 | — | $25.00 | $25.00 |
| Screw | M3x10mm | 1 | Box | MDFLY | B00ZWASZGO | B00ZWASZGO | $2.68 | $2.68 |
| Screw terminal block | 5mm, 2-pin | 1 | SMD/SMT | SynHHergyx | B0FNQX9MMT | B0FNQX9MMT | $4.99 | $4.99 |
| Propeller | — | 1 | N/A | Wurenji | 789616070286 | 789616070286 | $8.69 | $8.69 |

The struck-through ADC line was PDR's plan and was not used in the final build (see "Known changes" above) — it's kept here crossed out rather than deleted so its cost isn't silently lost from the section total below, which still includes it as originally quoted.

**Section total: $205.77**

## Oscilloscope / Screen

| Component | Spec | Qty | Package | Manufacturer | Mfr Part # | Distributor Part # | Price/Item | Total |
|---|---|---|---|---|---|---|---|---|
| Raspberry Pi Zero 2 W | Pi Zero 2 WH | 1 | Box | Raspberry Pi | B0DKKXS4RV | B0DKKXS4RV | $27.99 | $27.99 |
| Mini HDMI to HDMI cable | HDMI 2.1 | 1 | Cable | Cable Matters | B0DP3WLGMZ | B0DP3WLGMZ | $7.99 | $7.99 |
| ~~Multiplexer~~ | ~~16-channel digital/analog mux~~ | ~~1~~ | ~~Board~~ | ~~HiLetgo~~ | ~~CD74HC4067~~ | ~~CD74HC4067~~ | ~~$5.69~~ | ~~$5.69~~ |
| Display | 6.25" capacitive touch | 1 | Thru-hole | Waveshare | B0CW35VK1B | B0CW35VK1B | $79.99 | $79.99 |
| **Waveshare ADS1263 ADC HAT** | 10-channel, 32kSPS ADC — replaces the multiplexer above | — | — | Waveshare | — | — | — | **TBD** |

Same deal on the struck-through multiplexer line — not used in the final build, kept for the record. The ADS1263 HAT that replaced it is the one missing price/part number in this whole document; whoever picks this up should add it once they've got the actual purchase info.

**Section total (excluding the still-TBD ADS1263 HAT): $121.66**

## Grand total (as quoted at PDR, both superseded lines included, ADS1263 HAT excluded)

**$675.17**

This number is not the real final cost of the project — it predates the ADC redesign, the abandoned custom ADC HAT attempt, and whatever the finance manager's actual final purchasing looked like. It's a starting point, not a final figure.
