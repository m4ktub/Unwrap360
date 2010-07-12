#
# Regular cron jobs for the unwrap360 package
#
0 4	* * *	root	[ -x /usr/bin/unwrap360_maintenance ] && /usr/bin/unwrap360_maintenance
