
socat -d -v -x PTY,link=$2,wait-slave,raw $1,raw
