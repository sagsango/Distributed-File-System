#
#    $Id: Makefile 34 2006-04-29 22:33:20Z ajw $
#

all:
	mkdir -p Apps/Mail
	cp -r ../!hmail Apps/Mail
	perl CreatePackage
	zip -r -9 -I ^.hmail-pkg/zip Apps RiscPkg Sprites SysVars -x *svn*

.PHONY: clean all


clean:
	-rm -rf ../hmail-pkg.zip Apps RiscPkg/Control
