# Jurassic Spark Analog Computer
---

## NEW CAPSTONE GROUP - Intro
---

Hello new capstone group! We hope you're excited to continue to develop on top of this project. We hope you find a solid enough foundation built here to continue to expand from, and are excited to see where this project goes in the future.

Look around this repository, and familiarize yourself with it's layout and components. 

## Overview
---

Jurassic Spark's Analog Computer is a device that was designed to be used as the core of a course on control theory that will be offered in the future at CU Boulder. It is an educational device featuring 12 swappable module ports, with four different modules (adder, integrator, subtractor, and gain), each of which are a foundational building block that allow a student to quickly create and test a control design of their own making. Featured is also an on-board 4 channel oscilloscope, allowing students to visualize and compare waveforms produced.

To demonstrate some sort of control, in addition to the analog computer, a tertiary demonstrative device was developed. This device features a ping pong ball in a tube, with a fan that blows up and down the ball. Students will learn to develop and test different methods of controlling the height of the ball, through a feedback loop they develop on the analog computer itself. 

This repository contains all the design files and documentation for the Jurassic Spark Analog Computer, and the demonstrative tertiary system that goes along with it.

## Repository Structure
--- 

A quick description of what exists in each directory:

- **documentation** - design write-ups and reference docs for each subsystem
- **hardware** - all PCB schematics and layouts (Altium `.SchDoc`/`.PcbDoc`), grouped per board: `modules/` (Adder, Gain, Integrator, Subtractor), `power/`, `oscilloscope-adc-hat/`, `mechatronic/`
- **mechanical** - enclosure/housing CAD (Parasolid `.x_t`, plus a `.3mf` for the printable module housing) for the module enclosures and full assemblies
- **mechatronic** - Arduino firmware for the mechatronic (ball-in-tube) system
- **oscilloscope** - all of the source code, along with a couple helper scripts, to the oscilloscope.
- **spice** - SPICE simulation files (`.wxsch`) for the individual analog computer modules (gain, integrator, adder/subtractor)

## Recommended Next Steps (to the new capstone team)
--- 

As you will find out, capstone moves quick, and you will never be able to get everything done that you want to get done. A few things worth picking up early:

- **Mechatronic/oscilloscope PCB bringup:** Unfortunately during our time, we were unable to make some quality of life revisions on the mechatronic PCB, and had to make a proto-board instead of a fully fledged PCB for the oscilloscope. Personally, I would begin here, ordering and verifying design, then reintegrating.
- **Case/power delivery issues:** We have noticed that particularly the gain module, for reasons we could not determine, do not get power delivered to them in certain module bays. It is particularly strange as it's only the gain modules that won't work in certain ports, when the adder/subtractor for example work as expected in those modules. May be worthwhile to consider the power delivery of our system and potentially rework it slightly, or at least figure out what the particular issue is with those certain ports. 
- **Test bluetooth exporting for Windows:** A known issue that we weren't able to fix in time is exporting screenshots to Windows. In short, Windows requires pairing with the device before the OBEX OPP BlueTooth protocol can proceed and a file can be sent. To fix this, an agent is created that establishes a pairing for the duration of the transfer, and unpairs at the end, however this fix is not tested. The fix can be found in the `windows-bluetooth-pairing-proposal` branch, and once tested can be merged. 
- **`deploy.fish`:** this is a script that was written in order to cross compile the oscilloscope app for the Pi Zero 2W. As it stands, it's not functional as a custom Docker image is used that existed on Logan's local machine. Qt Creator (Qt's own IDE) has support for cross compilation built into the IDE, and is recommended to use their IDE and set up cross compilation through that. However, this is kept here for reference, and might be nice to convert to a bash script to be kept separate from Qt Creator (if you have issues with Qt Creator). 
- **Oscilloscope considerations:** The oscilloscope stack of tech used here works well enough for this application, though it might be worth it if you end up needing better performance to take a deeper dive into particularly hardware, and the PCB associated with the custom ADC solution.


## Contact
---

Feel free to reach out to [Logan Caskey](caskeyvl@gmail.com) with any questions or issues.  

## Useful Links
---

[BOM](https://docs.google.com/spreadsheets/d/1X1U5svekkfRvvx4M4c1IeXdciezWEVZrIU-C1A8QEh0)
[Final Report (link to edit)](https://www.overleaf.com/4477574124qrsjspkmtzmq#01acd5)
[Images and Drawings](https://docs.google.com/document/d/1QA50hwxr3ZcNsiclymAVjyM4m0BCg9A4m2U9kV9aPGA/edit?usp=sharing)
