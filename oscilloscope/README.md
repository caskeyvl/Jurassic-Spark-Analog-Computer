# Oscilloscope

A 4-channel oscilloscope built into the analog computer: a Raspberry Pi Zero 2W
with a Waveshare ADS1263 ADC HAT, running a Qt/QML app that displays the four
module signal channels, exports captures over Bluetooth, and drives the
built-in LCD.

Hardware for this subsystem — the ADC HAT schematic/layout — lives in
[`../hardware/oscilloscope-adc-hat/`](../hardware/oscilloscope-adc-hat/).

## How it works

**Acquisition (`datasource.cpp`)**: a dedicated thread (`adcThreadFunc`) talks
to the ADS1263 over SPI, cycling through 4 single-ended channels (AIN0–AIN3
vs. AINCOM) on a DRDY interrupt. Wiring:

| Signal | Pin |
|---|---|
| DRDY | BCM 17 |
| CS | BCM 22 |
| RST | BCM 18 |
| SPI | `/dev/spidev0.0`, Mode 1, 4 MHz |

AINCOM is wired to the 2.5V rail (the system's virtual 12V mid-rail). A
level-shifting board (see the hardware folder above) divides the module
signals' 0–24V physical range down to 0–5V at the ADC input; with the
ADS1263's ADC1 in bipolar mode and its internal 2.5V reference, that's a
±2.5V differential input mapped back out to the ±12V the UI displays.

Each sample gets an exponential moving average filter (`setFilterAlpha`,
1.0 = off) before landing in a mutex-protected ring buffer (`ScopeRing`,
default 9000 samples ≈ 5s window). A level trigger (`Trigger`) gates
rendering — nothing draws until the trigger channel crosses its level, and it
stays armed-off until `rearm()` is called. The QML side's 25Hz timer calls
`updateChannel()` on the main thread to copy the current view window into a
chart series.

The window is specified in seconds (`setWindowSeconds`), not samples —
`m_samplesPerView` is recalibrated automatically from the *measured* frame
rate every 200 frames, so the displayed time window stays accurate regardless
of the ADC's actual achieved throughput.

**No hardware fallback**: if `/dev/spidev0.0` isn't available (e.g. running
the UI on a desktop for development), the ADC thread falls back to a
synthetic signal generator so the UI still works without a Pi.

**Export**: `exportCsv()` writes the current view to CSV (per-channel columns
gated by an enabled-channels list). `BluetoothExporter` (`bluetoothexporter.cpp`)
handles device discovery and file transfer over BlueZ/D-Bus, using OBEX OPP —
no pairing required, works with any discoverable nearby device.

**UI**: QML (`qml/qmloscilloscope/`), Qt Charts for the trace, running on
Weston/Wayland on the Pi. `ScopeView.qml` is the scope display;
`SettingsPanel`/`TriggerSettingsPanel`/`AxisSettingsPanel`/`ExportDataPanel`
are the slide-out drawers.

## Building

```
cmake -B build -S .
cmake --build build
```

Requires Qt6 (Charts, Core, DBus, Gui, Qml, Quick) and a threads-capable
toolchain. `toolchain.cmake` is a CMake toolchain file for cross-compiling to
the Pi Zero 2W (armv6/armv7 depending on your Qt build).

`deploy.fish` was an attempt at a scripted cross-compile-and-rsync-to-Pi
pipeline, but depends on a Docker image that only ever existed on one
machine — **it doesn't work as committed**. Kept for reference. Qt Creator's
built-in cross-compilation support is the recommended path until this is
replaced with something that actually runs anywhere.

## Known issues

- **Voltage noise on the displayed trace**, worse at higher gain settings.
  Root cause: the level-shifting board shares analog/digital ground routing,
  and the SPI traces between the ADC and the Pi are relatively long. Neither
  the ADS1263's hardware FIR filter nor the software EMA filter fully hide
  it. A future PCB revision with a split analog/digital ground plane, shorter
  SPI traces, and more decoupling at the ADC's supply pins should reduce this
  substantially — this was root-caused but not yet re-fabricated.
- `deploy.fish` doesn't fail loudly on a bad cross-compile — see above.
- **Bluetooth export doesn't work to Windows receivers.** `BluetoothExporter`
  sends over OBEX OPP with no pairing step, matching Linux's behavior: an
  unpaired device just gets an "accept this file?" prompt. Windows' Bluetooth
  stack requires the two devices to already be paired before it'll accept an
  incoming OBEX transfer — there's no pairing flow in the app, so a Windows
  receiver has to be paired via Windows' own Bluetooth settings first, outside
  the app, or the send silently fails to prompt anything on their end. Fixing
  this properly means either adding a pairing flow, or documenting the
  workaround prominently in the export UI.

## Generating API docs

Header comments (`datasource.h`, `bluetoothexporter.h`) are Doxygen-formatted.
With [Doxygen](https://www.doxygen.nl/) installed:

```
cd oscilloscope
doxygen
```

Output goes to `docs/html/index.html` (gitignored — regenerate rather than
committing it). QML isn't included; it's documented here instead.
