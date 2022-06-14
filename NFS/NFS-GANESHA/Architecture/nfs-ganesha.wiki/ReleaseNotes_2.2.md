# Release Notes V2.2
These are the release notes for Version 2.2 of the NFS-Ganesha file server.
NFS-Ganesha is a user mode NFS and Plan 9 Remote File Service server that provides many features not available in the kernel based NFS servers.

This version is the result of an 10 month effort by an active developer community.  There is a lot of new code, a whole lot of improved code, and lots of new features and capabilities.

## Overview

The following is a short brief overview of these changes:

* Ganesha supports granting delegations

* There have been numerous config changes

* Ganesha now includes systemd scripts

* Improved packaging for RPM and Debian

* Major stability improvements

* non-QT based python tools

* Support for Ganesha to be a pNFS DS only, no MDS

* SECINFO in preferred order

* LTTng support

* NFS v4.2 support

* Major improvements in 9p support

* Code cleanup (checkpatch and Coverity)

* ntirpc improvements

* FSAL_GLUSTER updated with pNFS and ACL support and more

## Software Availability
The software is available in source form from our [https://github.com/nfs-ganesha/nfs-ganesha Github site].  Source tarballs are also available on the Github site.

The source includes '''ntirpc''' which is a new TIRPC library based on the original Sun implementation but extended significantly to support bi-directional RPC and thread safe operation.  It is automatically pulled as a submodule when NFS Ganesha is cloned.  If you use tarballs, make sure you also get the correct ntirpc tarball.

We use signed, annotated tags to mark our official releases.  Our release tarballs are only made available using these signed tags.

; NOTE
: The fingerprint of the GPG key used to sign this release is '''bd0530f5'''.

NFS-Ganesha has been packaged for the Fedora distribution.  Packages are available for Fedora starting with Fedora 20.  These packages will be upgraded to V2.2 very soon after our release.  These RPMs are available from the Fedora distribution sites.  The packages are also built for EPEL6 and EPEL7.

There are Debian package control files in the source but these are out of date.  We are looking for volunteers to pick up Debian and other Debian based distribution support.

## Operating System Support
The primary platform for NFS-Ganesha is Linux.  Any kernel later than 2.6.39 is required to fully support the VFS FSAL.  This requirement does not apply to configurations using other FSALs.  We have not recently tested with kernels older than 3.8 but that should not be a problem for users with currently supported Linux distributions.

The primary development and use platform for NFS-Ganesha is the x86_64.  This is mainly because these 64 bit platforms are the norm for enterprise deployments.  It has also been built and tested on the ARMv8 (64 bit ARM) and 64 bit PPC.  The server should work on 32 bit platforms although we have not tested it in a while.  The server uses a lot of memory for its caches so a 32 bit address space would limit scalability.

## NFS Services
NFS Ganesha supports NFSv3, NLM4, and MNTv1 and v3.  Minimal MNTv1 support is present to handle some clients that still unmount NVSv3 exports using MNTv1.  The server requires a running portmapper/STATD for NFSv3 at this time.

NFSv4.0, v4.1 and V4.2 are supported.  pNFS is also supported where the FSAL is the backend for a distributed filesystem that can support pNFS.  The server can act as both Metadata Server (MDS) and Data Server (DS) these distributed filesystems.  Multiple (clustered) MDSs are not currently supported but they are on the roadmap for a future release.  There can be as many DS instances as makes sense for the filesystem cluster. Generating and managing layouts is the responsibility of the FSAL and its underlying filesystem.  The server core treats layouts as opaque data and so will mediate any layout type the FSAL chooses.  Both files and object layouts have been tested.

## Plan 9 Remote File Services
The server supports the 9P.2000L protocol.  This is a light(er) weight remote filesystem protocol that originated with AT&T Bell Labs Plan 9, their "next generation" UNIX.  The primary client for this service is the 9P client in Linux.