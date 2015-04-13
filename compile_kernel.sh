#!/bin/sh

VAR=`echo "$1"|sed 's/\./_/g'`

(echo "const char *$VAR="; cpp $1|grep -v '^# ' |sed 's/"/\\"/g'|sed 's/^/"/g'|sed 's/$/\\n"/g'; echo ";") > $2
