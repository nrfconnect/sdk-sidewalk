#!/bin/bash
#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#
west list | tail -n +2 > /tmp/west_list_result
while read line
do
        echo "$line"
        repo_path=$(echo $line | awk '{print $2}')
        revision=$(echo $line | awk '{print $3}')
        repo_remote=$(echo $line | awk '{print $4}')

        cd $ZEPHYR_BASE/../$repo_path
        git fetch -n -o=--depth=1 $repo_remote $revision&
done < "/tmp/west_list_result"
wait
