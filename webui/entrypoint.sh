#!/bin/sh
set -eu

: "${RELAY_1_NAME-Relay 1}"
: "${RELAY_2_NAME-Relay 2}"
: "${RELAY_3_NAME-Relay 3}"
: "${RELAY_4_NAME-Relay 4}"
: "${RELAY_1_KVM_URL-}"
: "${RELAY_2_KVM_URL-}"
: "${RELAY_3_KVM_URL-}"
: "${RELAY_4_KVM_URL-}"

export RELAY_1_NAME RELAY_2_NAME RELAY_3_NAME RELAY_4_NAME \
  RELAY_1_KVM_URL RELAY_2_KVM_URL RELAY_3_KVM_URL RELAY_4_KVM_URL
envsubst '$RELAY_1_NAME $RELAY_2_NAME $RELAY_3_NAME $RELAY_4_NAME $RELAY_1_KVM_URL $RELAY_2_KVM_URL $RELAY_3_KVM_URL $RELAY_4_KVM_URL' \
  < /etc/nginx/templates/config.js.template \
  > /usr/share/nginx/html/config.js

exec nginx -g 'daemon off;'
