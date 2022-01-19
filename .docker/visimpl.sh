#!/bin/bash
for var in "$@"
do
	if [[ $var = "--testFile" ]]
	then
		echo "Aborted: --testFile parameter only valid for SimPart, unable to launch both SimPart and StackViz with those arguments."
		exit 1
	fi
done

if pgrep -x "avahi-daemon" > /dev/null 2>&1
then
	sleep 1
else
	chmod -R -f 777 /tmp
	chmod -R -f 777 /etc/avahi/
	mkdir -p /var/run/avahi-daemon
	chmod -R -f 777 /var/run/avahi-daemon/
	echo '*' > /etc/mdns.allow
	service dbus start > /dev/null 2>&1
	avahi-daemon --daemonize --no-drop-root --no-rlimits
	sleep 1
fi
/usr/bin/visimpl $@ &
/usr/bin/stackviz $@ 
