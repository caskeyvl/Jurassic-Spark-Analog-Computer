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

## Building (desktop, for development)

```
cmake -B build -S .
cmake --build build
```

Requires Qt6 (Charts, Core, DBus, Gui, Qml, Quick) and a threads-capable
toolchain. This builds and runs on a regular desktop Linux box using the
synthetic-signal fallback (see above) — useful for UI/logic work without a
Pi or the ADC hardware attached.

## Cross-compiling and deploying to the Pi

The target is a Raspberry Pi Zero 2W running **Raspberry Pi OS Bookworm,
64-bit (aarch64)** — not 32-bit. `toolchain.cmake` + `deploy.fish` implement
this; getting the pieces below right the first time is most of the actual
work, so it's written up in full here rather than left to be
re-discovered.

There are two one-time setup pieces, then a repeatable build+deploy step.

### 1. A sysroot pulled from a real Pi

`toolchain.cmake` expects a sysroot at `~/sysroots/rpi-bookworm-arm64`
holding the target's `/usr` (Qt6 libraries, headers, and — critically — the
target's own `Qt6Config.cmake` under
`usr/lib/aarch64-linux-gnu/cmake/Qt6/`), plus `/opt` and a `lib -> usr/lib`
symlink matching Raspberry Pi OS's merged-`/usr` layout:

```
mkdir -p ~/sysroots/rpi-bookworm-arm64
rsync -avz --rsync-path="sudo rsync" <pi-host>:/usr/ ~/sysroots/rpi-bookworm-arm64/usr/
rsync -avz --rsync-path="sudo rsync" <pi-host>:/opt/ ~/sysroots/rpi-bookworm-arm64/opt/
ln -s usr/lib ~/sysroots/rpi-bookworm-arm64/lib
```

Pulling it straight off a running Pi (rather than, say, `apt-get install`-ing
target packages some other way) guarantees the libraries you link against
are *exactly* what's on-device. This is a real rsync of a real filesystem —
expect it to be large (~2GB) and slow, budget real time for the first pull.
Re-run it if the Pi's Qt6/system packages ever get updated.

### 2. A cross-compile Docker image

The container needs the `aarch64-linux-gnu` cross-compiler *and* a
**host-native** (x86_64) Qt6 install — `moc`/`rcc`/`qmlimportscanner` have to
run on the machine doing the building even though the binary they help
produce targets arm64, which is what `toolchain.cmake`'s `QT_HOST_PATH` /
`Qt6HostInfo` settings are pointing at.

The one thing that actually matters here, learned the hard way: **don't**
try to install target-architecture (arm64) Qt6 packages into the container
via `dpkg --add-architecture arm64` + apt multiarch — that path is fragile
and kept breaking. Target Qt6 comes entirely from the sysroot above; the
container only ever needs Qt6 for its own (host) architecture:

```dockerfile
FROM debian:bookworm
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    cmake ninja-build pkg-config rsync openssh-client \
    gcc-aarch64-linux-gnu g++-aarch64-linux-gnu binutils-aarch64-linux-gnu \
    qt6-base-dev qt6-base-dev-tools qt6-declarative-dev-tools \
    qt6-tools-dev-tools qt6-charts-dev \
    libgl-dev libglx-dev libegl1 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /work
```

```
docker build -t qt-cross-bookworm:dev -f Dockerfile .
```

### 3. Build and deploy

With both of the above in place, edit `PI_HOST` near the top of
`deploy.fish` to your Pi's SSH host (user@hostname), then:

```
./deploy.fish
```

It cross-compiles inside the Docker image against the sysroot (using
`toolchain.cmake`), then rsyncs the resulting binary to the Pi. Known rough
edge: it doesn't check the Docker build's exit status or consistently bail
on a missing toolchain file, so a failed/skipped build can rsync a stale
binary and still report success — worth fixing before relying on it blindly.
Qt Creator's built-in cross-compilation support is a fine alternative once
its kit is pointed at the same sysroot + toolchain file.

### On the Pi itself

Raspberry Pi OS Bookworm (64-bit), running the app under a **Weston/Wayland**
compositor session (matches the software stack described above) with SSH
and mDNS (`.local` hostname resolution) enabled so `deploy.fish` can find it.
A full SD card image capturing this exact setup — so a new team can flash it
directly instead of reconstructing it by hand — is planned; check with
whoever's holding this repo for where to get it once it exists.

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
  the app, or the send silently fails to prompt anything on their end.

  A fix is written and builds clean, but is **not merged** — it needs testing
  against real Windows hardware first, which wasn't available while writing
  it. See the `windows-bluetooth-pairing-proposal` branch: adds a
  `BluetoothPairingAgent` (BlueZ `Agent1`, `NoInputNoOutput` → "Just Works"
  pairing) and routes `sendFile()` through a pair-if-needed step before the
  existing OBEX flow. Before merging: build it, pair against an actual
  Windows machine, confirm the transfer completes, and confirm the
  existing Linux/Android/macOS path still works unchanged.

## Generating API docs

Header comments (`datasource.h`, `bluetoothexporter.h`) are Doxygen-formatted.
With [Doxygen](https://www.doxygen.nl/) installed:

```
cd oscilloscope
doxygen
```

Output goes to `docs/html/index.html` (gitignored — regenerate rather than
committing it). QML isn't included; it's documented here instead.
