#!/bin/bash
chmod -R -f 777 /tmp
service dbus start > /dev/null 2>&1
service avahi-daemon start > /dev/null 2>&1
sleep 1
/usr/bin/visimpl $@
