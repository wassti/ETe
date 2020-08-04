#!/bin/bash

set -euxo

MAX_FILE_HISTORY=10
SNAPSHOT_DIR=/home/etf/www_files/ete-linux-i386
TMP_DIR=/tmp/etfdeploy
USER=etf
HOST=etf2.org

ssh-keyscan -H $HOST >> ~/.ssh/known_hosts

file_count=$(ssh $USER@$HOST ls $SNAPSHOT_DIR | wc -l)
if (( file_count > MAX_FILE_HISTORY )); then
    remove_count=$((file_count - MAX_FILE_HISTORY))
    ssh -T $USER@$HOST << EOSSH
rm "$(ls -t $SNAPSHOT_DIR | tail -$remove_count)"
EOSSH
fi

ssh etf@etf2.org mkdir -p $TMP_DIR
scp $TRAVIS_BUILD_DIR/*.7z etf@etf2.org:$TMP_DIR
ssh etf@etf2.org 7z x /tmp/etfdeploy/*.7z -aoa -o/home/etf/wolfet
ssh etf@etf2.org mv $TMP_DIR/* $SNAPSHOT_DIR
ssh etf@etf2.org rm -rf $TMP_DIR
ssh etf@etf2.org sudo systemctl restart etf.service

echo "Deployment successful"
