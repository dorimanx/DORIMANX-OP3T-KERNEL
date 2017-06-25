#!/bin/sh

# This is dorimanx upload release file.
# You will need ncftp installed to use this.
# http://www.ncftp.com/download/ (i have used the 64bit linux)
# download extract, then run: make install and all set.
# You must have ftp user/pass set in /root/ftp_login_mirror1.cfg + /root/ftp_login_mirror1.cfg to access your server!
# set it like this:
# vi /root/ftp_login_mirrorX.cfg (must be root!) dont add the #

#host ip.of.YOUR-server
#user john
#pass password

# Save and chmod 700 /root/ftp_login_mirrorX.cfg
# have fun.

# Before you start make sure READY-KERNEL/ contain only new update.

if [ -e /root/ftp_login_mirror1.cfg ]; then
	cp READY-KERNEL/*OP3-T*.zip ../OP3T-DOWNLOADS/N-KERNEL/Kernels/
	ncftpput -f /root/ftp_login_mirror1.cfg /OP3T/Kernel/ READY-KERNEL/*OP3-T*.zip
fi;
