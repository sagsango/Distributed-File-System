#!/usr/bin/perl

# This is a filter for converting Subversion repositories to use
# the svn:riscosfiletype attribute instead of a ,xxx filename suffix
# when recording a RISC OS filetype.
#
# To use it, dump the repository to a text file using the
# svnadmin dump command and pipe the result into this script.  The
# resulting output will be a modified repository dump which can be
# converted back into a repository using the svnadmin load command.
#
# WARNING: this script makes some fairly simplistic assumptions
# about the structure of a dump file, and it is not guaranteed to
# work in all cases.  In order to use it safely you will need to:
# (a) keep a backup of the original repository, and
# (b) inspect the result carefully to determine whether it is correct.
#
# One situation it will certainly not handle correctly is where the
# same pathname exists both with and without a ,xxx suffix.  This
# must be resolved manually before the data is filtered.
#
# (C) 2006 Graham Shaw
# This file may be redistributed and/or modified under the terms of
# any version of the Subversion License as published by CollabNet
# (http://subversion.tigris.org/license-1.html).

$filetype = undef;
while (<STDIN>)
{
	if (/^Node-path: /)
	{
		if (/(.*),([0-9a-f]{3})\n$/)
		{
			$filetype = $2;
			$_ = "$1\n";
		}
		else
		{
			$filetype = undef;
		}
	}
	elsif (/^(Prop-content-length: )(.*)\n$/ && $filetype)
	{
		$n = $2 + 21;
		$_ = "$1$n\n";
	}
	elsif (/^(Content-length: )(.*)\n$/ && $filetype)
	{
		$n = $2 + 21;
		$_ = "$1$n\n";
	}
	elsif (/^PROPS-END\n$/ && $filetype)
	{
		print "K 18\n";
		print "svn:riscosfiletype\n";
		print "V 3\n";
		print "$filetype\n";
		$filetype = undef;
	}
	print $_;
}
