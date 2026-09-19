# Four-relay libgpiod controller

A small C++17 HTTP service and browser UI for controlling four GPIO relays with
[libgpiod](https://libgpiod.readthedocs.io/). We're using the relays to turn our K3s on and off.

## Build

Install a libgpiod development package (including its C++ bindings), pkg-config,
and either CMake or Meson. For Debian/Raspberry Pi OS, install
`libgpiod-dev` when available. For Fedora, install `libgpiod-devel` and `libgpiod-c++`. This project uses the libgpiod 2.x C++ bindings (`gpiod.hpp`).

### Meson

```sh
meson setup build
meson compile -C build
meson compile -C build format
```

### CMake

```sh
cmake -S . -B build
cmake --build build
```

### Container

The repository includes an example [`compose.yaml`](../compose.yaml) that uses the
published multi-architecture image. Start it on the Linux GPIO host from the
repository root:

```sh
docker compose -f compose.yaml up -d
```

The Compose configuration maps `/dev/gpiochip0` into the container and exposes
port `8080`. Open `http://<board-ip>:8080/` in a browser. Change the `devices`,
`GPIO_CHIP`, and `RELAY_*_LINE` values in the root `compose.yaml` for a different
GPIO chip or line mapping.

## Run

The defaults are `/dev/gpiochip0`, GPIO line offsets `17, 27, 22, 23`, active-low
relays, and HTTP port `8080`. These connect relay-board IN1-IN4 to Raspberry Pi
GPIO17/GPIO27/GPIO22/GPIO23 on physical header pins 11/13/15/16:

```sh
sudo ./build/relay-gpio
```

Open `http://<board-ip>:8080/` in a browser. The static frontend is loaded from
`public/index.html` when running from the project directory. If installed with
Meson, use the installed data directory as the executable's working directory,
or serve the frontend through a separate web server.

Configure a board without changing source:

```sh
GPIO_CHIP=/dev/gpiochip0 \
RELAY_1_LINE=17 RELAY_2_LINE=27 RELAY_3_LINE=22 RELAY_4_LINE=23 \
RELAY_ACTIVE_LOW=1 RELAY_HTTP_PORT=8080 \
sudo ./build/relay-gpio
```

Verify GPIO line offsets with `gpioinfo` before connecting the relays. The relay
board control header is wired as DC+ to Pi physical pin 2 (5V), DC- to physical
pin 6 (GND), IN1 to GPIO17 (pin 11), IN2 to GPIO27 (pin 13), IN3 to GPIO22
(pin 15), and IN4 to GPIO23 (pin 16). Do not connect relay coils directly to a
GPIO pin; use a suitable relay module/driver, common ground, and an appropriately
powered supply.

## API

- `GET /api/relays` returns `{ "relays": [{ "id": 1, "on": false }, ...] }`.
- `POST /api/relays/1/on` turns relay 1 on.
- `POST /api/relays/1/off` turns relay 1 off.

The service initializes all relays off and releases GPIO lines when it exits. `RELAY_ACTIVE_LOW=1`
means the electrical low level is treated as on; set it to `0` for active-high relay modules.
For production use, place it behind authentication/TLS or restrict access to a
trusted network; the example server deliberately has no authentication.
