# wiring harness

WireViz definition and resulting artifacts for a K3 power harness. An 8-pin EPS12V connector supplies four +12V/COM pairs. Four separated SPDT relay connector blocks switch the +12V conductor for each pair through COM to NO; each NC terminal is exposed and intentionally left unconnected. J1-J4 are the four 2-pin K3 outputs.

The relay control header is wired to a Raspberry Pi 40-pin header as follows: DC+ to physical pin 2 (5V), DC- to physical pin 6 (GND), IN1 to GPIO17 / physical pin 11, IN2 to GPIO27 / physical pin 13, IN3 to GPIO22 / physical pin 15, and IN4 to GPIO23 / physical pin 16. The inputs are active-low, matching the relay controller configuration.

## how to use

Make sure you have wireviz installed on your machine. I just use pipx, so `pipx install wireviz`. I'm gonna package it in Terra at some point.

to render:

```sh
cd harness
wireviz k3-harness.yml
```

this should render the png, svg, bom, and html.

## actions

whenever you push to main, the png and svg outputs should regenerate as artifact outputs. same thing for releases.
