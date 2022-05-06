#
#    $Id$
#

release: clean
	make -C !Wiresalmon
	zip -r -9 -I wiresalmon/zip !Wiresalmon COPYING -x *svn* -x *.o.*

.PHONY: clean release


clean:
	-rm -rf wiresalmon.zip
