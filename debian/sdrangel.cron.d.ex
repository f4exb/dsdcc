#
# Regular cron jobs for the dsdcc package
#
0 4	* * *	root	[ -x /usr/bin/dsdcc_maintenance ] && /usr/bin/dsdcc_maintenance
