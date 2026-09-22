# Kairos web UI

The web UI is normally served in front of the `relay-gpio` API container.

## Fixture mode

Open the UI with the `fixtures` query parameter:

```text
http://localhost:8080/?fixtures
```

Fixture mode renders four relays with in-memory state. Clicking the controls
only updates the page locally.

Normal mode omits the fixture marker and uses the relay API through nginx.

## Relay names

Set these environment variables when starting the container:

```sh
podman run --rm -p 8080:8080 \
  -e RELAY_1_NAME='K3s node 1' \
  -e RELAY_2_NAME='K3s node 2' \
  -e RELAY_3_NAME='K3s node 3' \
  -e RELAY_4_NAME='K3s node 4' \
  localhost/kairos-webui:test
```

The variables are `RELAY_1_NAME` through `RELAY_4_NAME`. They are substituted
when the container starts, so the same image can be reused with different names.
If a name is blank or contains only whitespace, that relay is hidden from the UI.
Its ID and GPIO position are not renumbered. Compose and Jadeite Quadlet
deployments expose the same variables.

Each relay can also have a NanoKVM URL using `RELAY_1_KVM_URL` through
`RELAY_4_KVM_URL`. A nonblank URL adds a `KVM` button that opens in a new tab;
blank URLs hide the button. For example:

```dotenv
RELAY_1_KVM_URL=https://nanokvm-blessed-son.barking-kokanue.ts.net
RELAY_2_KVM_URL=
RELAY_3_KVM_URL=https://nanokvm-blessed-daughter.barking-kokanue.ts.net
RELAY_4_KVM_URL=https://nanokvm-chud-daughter.barking-kokanue.ts.net
```
