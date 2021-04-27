#!/bin/sh
server_command="socat /dev/ttyS0,raw,echo=0 TCP-LISTEN:15000"
echo "Running server command: $server_command"
ssh nas-1@nas1 "${server_command}"
