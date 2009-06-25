#
# Regular cron jobs for the inflash package
#
0 4	* * *	root	[ -x /usr/bin/inflash_maintenance ] && /usr/bin/inflash_maintenance
