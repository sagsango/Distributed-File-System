#
#    $Id$
#

all:
	mkdir -p Apps/Development
	cp -r ../!SVN Apps/Development
	perl CreatePackage
	zip -r -9 -I ^.svn-pkg/zip Apps RiscPkg Sprites SysVars -x */svn*


.PHONY: clean all


clean:
	-rm -rf ../svn-pkg.zip Apps RiscPkg/Control SysVars
