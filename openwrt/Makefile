
OWRT_SVN_PATH = openwrt-trunk

owrt_checkout:
	svn co -r r15323 svn://svn.openwrt.org/openwrt/trunk $(OWRT_SVN_PATH)
	cd $(OWRT_SVN_PATH) && patch -p0 < ../openwrt-jz47x0-r15323.diff	
	touch $@ 
