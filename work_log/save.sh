#!/bin/bash
echo "Data Backup"

mv -f data data_`date '+%Y%m%d_%H.%M.%S'`
mkdir data