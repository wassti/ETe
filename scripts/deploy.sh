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
    files_to_remove=$(ssh $USER@$HOST ls -t $SNAPSHOT_DIR | tail -$remove_count)
    ssh $USER@$HOST cd $SNAPSHOT_DIR && rm $files_to_remove
fi

ssh $USER@$HOST mkdir -p $TMP_DIR
scp $TRAVIS_BUILD_DIR/*.7z $USER@$HOST:$TMP_DIR
ssh $USER@$HOST 7z x /tmp/etfdeploy/*.7z -aoa -o/home/etf/wolfet
ssh $USER@$HOST mv $TMP_DIR/* $SNAPSHOT_DIR
ssh $USER@$HOST rm -rf $TMP_DIR
ssh $USER@$HOST sudo systemctl restart etf.service

echo "Deployment successful"
