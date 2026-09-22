#!/bin/sh
set -eu

# Build the configured Jadeite application image and a Pi 4 bootable raw image.

VERSION="${VERSION:-0.1.0}"
IMAGE="${IMAGE:-ghcr.io/fyrastack/k3-utils/node:${VERSION}-rpi4}"
JADEITE_IMAGE="${JADEITE_IMAGE:-ghcr.io/nothingneko/jadeite:0.0.3-rpi4}"
RELAY_GPIO_IMAGE="${RELAY_GPIO_IMAGE:-ghcr.io/fyrastack/k3-utils/relay-gpio:main}"
WEBUI_IMAGE="${WEBUI_IMAGE:-ghcr.io/fyrastack/k3-utils/webui:main}"
OUTPUT_DIR="${OUTPUT_DIR:-build-rpi4}"
AURORABOOT_IMAGE="${AURORABOOT_IMAGE:-quay.io/kairos/auroraboot:v0.25.2}"
PUSH_IMAGE="${PUSH_IMAGE:-1}"
AUTH_FILE="${REGISTRY_AUTH_FILE:-}"

if [ -z "${AUTH_FILE}" ]; then
    if [ -n "${XDG_RUNTIME_DIR:-}" ] && [ -f "${XDG_RUNTIME_DIR}/containers/auth.json" ]; then
        AUTH_FILE="${XDG_RUNTIME_DIR}/containers/auth.json"
    elif [ -f "${HOME}/.config/containers/auth.json" ]; then
        AUTH_FILE="${HOME}/.config/containers/auth.json"
    elif [ -f "${HOME}/.docker/config.json" ]; then
        AUTH_FILE="${HOME}/.docker/config.json"
    fi
fi

if [ -n "${AUTH_FILE}" ] && [ ! -f "${AUTH_FILE}" ]; then
    printf '%s\n' "Registry auth file not found: ${AUTH_FILE}" >&2
    exit 1
fi

podman build \
    --platform linux/arm64 \
    --build-arg JADEITE_IMAGE="${JADEITE_IMAGE}" \
    --build-arg RELAY_GPIO_IMAGE="${RELAY_GPIO_IMAGE}" \
    --build-arg WEBUI_IMAGE="${WEBUI_IMAGE}" \
    --tag "${IMAGE}" \
    --file node-config/Containerfile \
    node-config

if [ "${PUSH_IMAGE}" = "1" ]; then
    podman push "${IMAGE}"
fi

mkdir -p "${OUTPUT_DIR}"
if [ -n "${AUTH_FILE}" ]; then
    podman run --rm --privileged \
        -v "$(pwd)/${OUTPUT_DIR}:/aurora" \
        -v "${AUTH_FILE}:/run/containers/auth.json:ro" \
        -e REGISTRY_AUTH_FILE=/run/containers/auth.json \
        "${AURORABOOT_IMAGE}" \
        --debug \
        --set disable_http_server=true \
        --set disable_netboot=true \
        --set disk.efi=true \
        --set "container_image=docker://${IMAGE}" \
        --set "state_dir=/aurora"
else
    podman run --rm --privileged \
        -v "$(pwd)/${OUTPUT_DIR}:/aurora" \
        "${AURORABOOT_IMAGE}" \
        --debug \
        --set disable_http_server=true \
        --set disable_netboot=true \
        --set disk.efi=true \
        --set "container_image=docker://${IMAGE}" \
        --set "state_dir=/aurora"
fi

printf '%s\n' "Pi 4 image written to ${OUTPUT_DIR}"
