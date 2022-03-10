#!/bin/bash

text=$(python3 -c "import sys, urllib.parse as ul; \
		print(ul.quote(sys.argv[1]))" "$1")

qrencode -s 20 -l L -v 1 -o output.png "http://192.168.0.14:8000/$text"
