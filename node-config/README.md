# Jadeite deployment

This directory contains both the runtime Quadlet units and a Pi 4 image build.
The Pi 4 artifact is a bootable raw disk image, which is the format AuroraBoot
uses for Raspberry Pi. The ISO is intended for PC/UEFI targets and not Pi.

- `relay-gpio`: uses `/dev/gpiochip0` to exposes the relay API
- `webui`: serves the UI and publishes port `8080`.

We're using Podman Quadlets because they're peak

## Build a configured Pi 4 image

First, authenticate to ghcr.io with Podman:

```sh
podman login ghcr.io
```

When prompted, put in your GitHub username and a personal access token with `write:packages` permission for the FyraStack/k3-utils repository

Find your user id:

```
id
```

Make sure you have a loop-device configured:

```sh
sudo modprobe loop max_loop=16
```

Then verify that it's available:

```sh
ls -l /dev/loop-control /dev/loop0 /dev/loop1
sudo losetup -f
```

You should see `/dev/loop0` in the list.

Build from the repository root with Podman:

```sh
chmod +x node-config/build-pi4.sh
sudo -E env \
  REGISTRY_AUTH_FILE=/run/user/1000/containers/auth.json \ # replace 1000 with your user id from above
  PUSH_IMAGE=1 \
  sh node-config/build-pi4.sh
```

The script builds a `linux/arm64` Jadeite derivative based on
`ghcr.io/nothingneko/jadeite:0.0.3-rpi4`, embeds the relay and web UI Quadlet
units, pushes the derivative image so AuroraBoot can consume it, and writes a
bootable raw image under `build-rpi4/`. It then starts both containers
automatically on first boot. The relay and web UI images must be available for
ARM64 before the Pi boots.

Override the image names or output directory when needed:

```sh
JADEITE_IMAGE=ghcr.io/nothingneko/jadeite:0.0.3-rpi4 \
IMAGE=ghcr.io/fyrastack/k3-utils/node:pi4 \
RELAY_GPIO_IMAGE=ghcr.io/fyrastack/k3-utils/relay-gpio:main \
WEBUI_IMAGE=ghcr.io/fyrastack/k3-utils/webui:main \
OUTPUT_DIR=build-rpi4 sh node-config/build-pi4.sh
```

Flash the resulting image to the Pi 4 storage using your normal imaging tool.
The build requires Podman, privileged container support, registry push access
for `IMAGE`, and network access to GHCR plus the Kairos AuroraBoot image. The
script passes your Podman/Docker registry auth file into AuroraBoot, so private
GHCR images can be pulled. It checks `REGISTRY_AUTH_FILE`, then the rootless
Podman auth file, Docker config, and finally `~/.docker/config.json`.

`PUSH_IMAGE=0` skips publishing, but the exact image still must already be
available from a registry that AuroraBoot can access. A local-only image cannot
be consumed by the AuroraBoot container.

## Install

Copy the units to the system Quadlet directory as root:

```sh
sudo mkdir -p /etc/containers/systemd
sudo cp node-config/kairos-relay.network /etc/containers/systemd/
sudo cp node-config/relay-gpio.container /etc/containers/systemd/
sudo cp node-config/webui.container /etc/containers/systemd/
sudo systemctl daemon-reload
sudo systemctl enable --now webui.service
```

The web UI is available at `http://<pi-address>:8080/`. To preview it without
running the relay container, open `http://<pi-address>:8080/?fixtures`; fixture
mode keeps four relay states in browser memory and never calls the relay API.
Check status and logs with:

```sh
sudo systemctl status relay-gpio.service webui.service
sudo journalctl -u relay-gpio.service -u webui.service -f
```

Jadeite’s persistent `/var/lib/containers` layout keeps the pulled images across
reboots and OS updates.

## Image and GPIO overrides

Edit the `Image=` and `Environment=` lines in the units before copying them. The
relay mapping defaults to GPIO17/27/22/23, physical pins 11/13/15/16, and active-low
logic. Verify offsets with `gpioinfo` on the target image.

The relay image must be published for `linux/arm64`. Jadeite’s own build recipes
use separate `rpi3` and `rpi4` images; this configuration targets the Pi 4 base.

The API has no authentication. Don't expose this outside of Tailscale
