#!/bin/sh
ls -la *.patch
for i in *.patch; do sed -i "s/CORE\/*/drivers\/staging\/qcacld-2.0\/CORE\//g" $i; done
