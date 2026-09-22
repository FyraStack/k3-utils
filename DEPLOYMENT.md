# Kairos Raspberry Pi deployment

This deployment uses two containers:

- `relay-gpio` uses the Raspberry Pi GPIO device and exposes the relay HTTP API to the webui
- `webui` serves the browser UI and proxies API requests to `relay-gpio`

We're using Jadeite because:

1. Jade made it
2. It's cool
3. It's immutable
4. Quadlets!

## Install and start with Jadeite

On Jadeite, install the Quadlet units:

```sh
sudo mkdir -p /etc/containers/systemd
sudo cp node-config/kairos-relay.network /etc/containers/systemd/
sudo cp node-config/relay-gpio.container /etc/containers/systemd/
sudo cp node-config/webui.container /etc/containers/systemd/
sudo systemctl daemon-reload
sudo systemctl enable --now webui.service
```

See [`node-config/README.md`](node-config/README.md) for logs and configuration.

## Install and start with Compose

Copy this repository (or just `compose.yaml`) to the Pi, then run:

```sh
git clone https://github.com/FyraLabs/k3-utils.git
cd k3-utils
docker compose pull
docker compose up -d
docker compose ps
```

Open `http://<pi-address>:8080/`. To preview the UI without a relay API, use
`http://<pi-address>:8080/?fixtures` or `/fixtures`; controls then use in-memory
fixture state. To use a different host port:

```sh
WEBUI_PORT=8000 docker compose up -d
```

Do not publish port 8080 on `relay-gpio`: the UI is intentionally the only public
entry point. The relay API has no authentication, so restrict the UI port to a
trusted network or put it behind an authenticated reverse proxy/VPN.

## GPIO and wiring

The default relay mapping is:

| Relay input | GPIO line | Physical pin |
| ----------- | --------: | -----------: |
| IN1         |        17 |           11 |
| IN2         |        27 |           13 |
| IN3         |        22 |           15 |
| IN4         |        23 |           16 |

The default is active-low (`RELAY_ACTIVE_LOW=1`). A blank relay name hides that
relay from the web UI without changing its relay ID or GPIO mapping. Override
names and line mappings in an `.env` file beside `compose.yaml`, for example:

```dotenv
RELAY_1_NAME=K3s node 1
RELAY_2_NAME=K3s node 2
RELAY_3_NAME=K3s node 3
# Leave a name blank to hide that relay while preserving its relay ID/GPIO slot.
RELAY_4_NAME=
RELAY_1_KVM_URL=https://nanokvm-blessed-son.barking-kokanue.ts.net
RELAY_2_KVM_URL=
RELAY_3_KVM_URL=https://nanokvm-blessed-daughter.barking-kokanue.ts.net
RELAY_4_KVM_URL=https://nanokvm-chud-daughter.barking-kokanue.ts.net
RELAY_1_LINE=17
RELAY_2_LINE=27
RELAY_3_LINE=22
RELAY_4_LINE=23
RELAY_ACTIVE_LOW=1
GPIO_DEVICE=/dev/gpiochip0
```

Confirm the actual offsets on the host with `gpioinfo` before connecting hardware.
The container needs access to the selected GPIO character device; Compose maps it
as `/dev/gpiochip0` inside the relay container. Never drive relay coils directly
from Pi GPIO pins. Use a properly powered relay module/driver, common ground, and
appropriate electrical isolation for the switched load.

## Building or using another registry

Override either image without changing the Compose file:

```sh
RELAY_GPIO_IMAGE=registry.example/relay-gpio:tag \
WEBUI_IMAGE=registry.example/kairos-webui:tag \
docker compose up -d
```

The `relay-gpio` and `webui` images are built by separate GitHub Actions workflows.
Both must be published for `linux/arm64` (and optionally `linux/amd64`) before
running `docker compose pull` on the Pi.
