#!/bin/sh

# this can be called from another script as ./ethostfw <server-id> <port-number>

# you need to specify unique table name for each port
NAME=${1:-LIMITER}

# and unique server port as well
PORT=${2:-27960}

RATE=768/sec
BURST=128

# flush INPUT table:
#iptables -F INPUT

# insert our rule at the beginning of the INPUT chain:
iptables -I INPUT \
    -p udp --dport $PORT -m hashlimit \
    --hashlimit-mode srcip \
    --hashlimit-srcmask 32 \
    --hashlimit-above $RATE \
    --hashlimit-burst $BURST \
    --hashlimit-name $NAME \
    -j DROP