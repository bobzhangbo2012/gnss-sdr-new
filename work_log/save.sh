#!/bin/bash
echo "Data Backup"

cp  ./myconf/mysdr.conf ./data/mysdr.conf
mv -f data data_`date '+%Y%m%d_%H.%M.%S'`
mkdir data