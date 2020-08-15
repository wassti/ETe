#!/bin/bash

set -euxo

MAX_FILE_HISTORY=10
SNAPSHOT_DIR=/home/etf/www_files/ete-linux-i386
TMP_DIR=/tmp/etfdeploy
DEPLOY_DIR=/home/etf/wolfet
USER=etf
HOST=etf2.org
SERVICE_NAME=etf.service

ssh-keyscan -H $HOST >> ~/.ssh/known_hosts

ssh $USER@$HOST bash <<EOF
file_count=\$(ls $SNAPSHOT_DIR | wc -l)
if (( \$file_count > $MAX_FILE_HISTORY )); then
    remove_count=\$((file_count - MAX_FILE_HISTORY + 1))
    files_to_remove=\$(ls -t $SNAPSHOT_DIR | tail -\$remove_count | tr '\n' ' ')
    # remove last whitespace
    files_to_remove=\${files_to_remove%?}
    cd $SNAPSHOT_DIR && rm \$files_to_remove
fi
EOF


ssh $USER@$HOST mkdir -p $TMP_DIR
scp $TRAVIS_BUILD_DIR/*.7z $USER@$HOST:$TMP_DIR
ssh $USER@$HOST 7z x $TMP_DIR/*.7z -aoa -o$DEPLOY_DIR
ssh $USER@$HOST mv $TMP_DIR/* $SNAPSHOT_DIR
ssh $USER@$HOST rm -rf $TMP_DIR
ssh $USER@$HOST sudo systemctl restart $SERVICE_NAME

echo "Deployment successful"
