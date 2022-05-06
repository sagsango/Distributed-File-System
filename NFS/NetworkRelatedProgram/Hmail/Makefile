#
#    $Id: Makefile 34 2006-04-29 22:33:20Z ajw $
#

release: clean
	make -C !hmail
	zip -r -9 -I hmail/zip !hmail COPYING -x *svn*
	make -C pkg

.PHONY: clean release


clean:
	-rm -rf hmail.zip
	make -C pkg clean
