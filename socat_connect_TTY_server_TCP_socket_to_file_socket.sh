#!/bin/sh
# Server:
echo "Server command: socat /dev/ttyS0,raw,echo=0 TCP-LISTEN:15000"

# Client:
target="/tmp/vmware_com2_socket"
if [ ! -S "${target}" ]; then
    echo "Named pipe ${target} must be an existing local file socket!"
    exit 1
fi

echo "Target pipe: ${target}"
socat tcp:nas1:15000 "${target}"

