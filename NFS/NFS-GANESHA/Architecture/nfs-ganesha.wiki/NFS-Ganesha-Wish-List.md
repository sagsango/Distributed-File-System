# Significant Features

## Dynamic Export Update (in Progress by ffilz)

We need to be able to modify exports without remove/add export.

## Complete NFS v4.1 Support

We fail 15 pynfs 4.1 test cases. Most of these are due to unimplemented features.

# Minor Issues and Code Bugs
## Review and cleanup Coverity Issues

## Directory larger than HWMark bug

This may or may not still be an issue...

It seems that in large directories, the aging out of cache inode entries results in ESTALE causing directory entries to become invisible.

## Issues With lookup_path

open_dir_by_path_walk - currently some FSALs are subject to NFS v3 mounts sneaking out of the export by use of symlink.

This function also needs to be verified that it closes all the doors.

Some FSALs do not correctly implement lookup_path method, and just return the root handle of the export, these SHOULD be changed, at least if NFS v3 support is desired (without the change, a v3 client that sends a mount for a sub-directory of an export will actually mount the root of the export). If fixed, they will need an equivalent function.

Needs equivalent function: FSAL_PT, FSAL_PROXY Needs Work to support sub-directory mounting: FSAL_GLUSTER, FSAL_CEPH, FSAL_ZFS Fixed in: FSAL_GPFS, FSAL_VFS, FSAL_XFS, FSAL_LUSTRE

## fsal_filesystem (github issue created)

It would be nice for all the FSALs convert to using these. Note that for the FSALs below, the filesytems will not exist in /etc/mtab, but they can still be maintained in the table of filesystems (which would help identify duplicate fsids).

FSAL_GLUSTER
FSAL_CEPH
FSAL_ZFS
FSAL_PROXY

Used in FSAL_VFS, FSAL_XFS, FSAL_GPFS

## Sub-mounted Filesystems

Ganesha sort of assumes an export exports a single filesystem. FSAL_VFS is a model of how we can export sub-mounted filesystems. Given use of FSAL_FILESYSTEM, the solution is actually pretty simple:

When you get an fsal lookup call, check if st_dev has changed. If so, call lookup_dev to get the filesystem and associate the fsal_obj_handle (in your FSAL's private handle structure) with the new filesystem. You may have to do some work for getattr (to report the correct fsid for each handle) and for producing the wire handle (to get the right fsid into your wire handle).

Fixed in FSAL_VFS, FSAL_XFS, FSAL_GPFS. May be an issue for: FSAL_GLUSTER, FSAL_CEPH, FSAL_PROXY

## Multiple Filesystems in MDCACHE

Not all FSALs encode an fsid into their handle. Those that do not MAY not have unique handles across multiple exported filesystems.

## Verify Fix - ESTALE during mount into PseudoFS

If an ESTALE occurs during building the PseudoFS, there is a deadlock issue. I don't know if this still happens.

## Deal with FSID spoofing in filehandles (github issue created)

Currently nothing is done to prevent a rogue client from spoofing handles and inserting an fsid into a handle with an export_id that was not intended to export that filesystem. It may be possible to access filesystems that weren't even intended for export at all. FSAL_VFS provides a particular exposure here since the root filesystem is almost certainly exportable by FSAL_VFS.

FSAL_VFS keeps a mapping between it's exports and filesystems and it will be easy to validate that a given handle with it's export_id and fsid represent a properly exported filesystem. Any FSAL that uses the fsal_filesystem code in the same way would be able to do the same (maybe there is some room for common code here...).

## Changing FSALin POSIX Namespace

We have several FSALs that export POSIX (/etc/mtab) namespace. It is possible to have filesystems nested in a way that would require crossing filesystem AND FSAL junctions at the same time, for example, assume the following exports of filesystem roots:

Path = /gpfs0; Pseudo = /gpfs0; FSAL { Name = GPFS; } Path = /gpfs0/some/path/ext4; Pseudo = /gpfs0/some/path/ext4; FSAL { Name = VFS; }

In order for the export junction at /gpfs0/some/path/ext4 to work properly, there must be both a GPFS inode that lookup will find, and a VFS inode. If it was the other way around, VFS could trivially create such a node (since it's name_to_handle_at will properly work). Since the node would ONLY be used to hang a junction off, it could be attached to a fake fsal_filesystem (since the fsal_filesystem would be owned by FSAL_GPFS). FSAL_GPFS would have to do something a little different.

The solution is when name_to_handle_at (or equivalent) cannot produce a handle, just build a handle using the fsid of the sub-mounted filesystem in the current export (and FSAL). The FSAL then allows LOOKUP to return this handle to the client (or to cache_inode_readdir) and allows GETATTR on it.
This allows cache_inode and the client to instantiate an inode in the parent export that can serve as a mount point for the new export (with new FSAL).

Fixed for FSAL_VFS, FSAL_XFS
Still an issue for FSAL_GPFS

One issue that still arises is how mounted_on_fileid should be set. I'm unclear how AIX depends on mounted_on_fileid, and I don't know if any other client depends on it. Sadly, we can't get the mountpoint inode in /gpfs0 to get an inode number that is unique in /gpfs0 to use.

## NFS v3 Nested Export Behavior

Currently export junctions are invisible to NFS v3. That actually could result in some unexpected behavior. I think we could have an option where the export junctions ARE visible to NFS v3 and then NFS v3 would just quietly cross the junctions with all the export permission changes implied (getting an NFS3ERR_ACCESS in place of NFS4ERR_WRONGSEC if security flavor was incompatible).

That might go hand in hand with another option I have proposed in the past.

Use the Pseudo path for mount to export matching instead of Path. That would actually allow deprecating Tag...

## Get Dynamic FS Info

We need to check other FSALs to make sure we call statfs appropriately so sub-mounted filesystems work, or if a filesystem should have some subdivision (like a file set) that has different accounting.

Fixed in FSAL_VFS, FSAL_XFS, FSAL_GPFS

## GET QUOTA / SET QUOTA

Similar to get dynamic fs info, get/set quota needs to deal with sub-mounted filesystems.

## MAXREAD and MAXWRITE

We should also examine how things like MaxRead/MaxWrite get reported, those maybe should actually be per-filesystem if there isn't an export override.

We should also convert the export config for these to use the new "option was set" notation.

## Handle ESTALE on a Junction

If the root inode of an export goes stale, we unexport, but we don't handle the case if an inode that is used only as a junction goes stale, for example, consider the following exports:

Path = /export1; Pseudo = /export1;
Path = /export2; Pseudo = /export1/some/path/export2;

Assume that at startup, /export1/some/path/export2 directory exists such that it can anchor the export junction to export2. If that directory is removed, we could end up with an ESTALE. We need to be able to clean up the mess.

## What Export Config Problems Could be Fatal

In 1.5, IBM discovered that it was a pain if Ganesha bailed out because an exported filesystem had not come on line. For 2.1, we should also consider that. For some enterprise environments that will try to keep Ganesha running, we may want to be more relaxed about lots of export config errors.

They will now be more fixable without restarting Ganesha.

## FSAL_VFS rmdir and EBUSY

The rmdir function can return EBUSY. This will turn into an NFS3ERR_JUKEBOX or NFS4ERR_DELAY, either of which will put the client into a soft lockup retrying an operation that won't be succeeding anytime soon.

## Write a pynfs Test Case for mounted_on_fileid

Handling of mounted_on_fileid in NFS v4 READDIR results should be tested that it works when there is an NFS4ERR_WRONGSEC issue at a junction.

## How Does SUID/SGID Interact With Us

If a directory has suid or sgid bit set, that implies certain inheritance-like behavior. How does that interact with Ganesha? Is there anything we need to do for correct semantics?

## Verify Numeric IDs are Working

RFC 3530 bis and RFC 5661 indicate that in absence of idmapping, the client and server may exchange user ids as numeric strings without an @domain suffix, i.e. sprintf(owner_attr, "%d", owner_uid).

## Reject ACLS We Can't Enforce

Per RFC 3530 bis 6.2.1, if we are presented an ACL with DENY ACES that we can not enforce, we MUST reject the ACL. It is ok to accept ACEs where a bit we don't enforce isn't specified. For example, we don't enforce ACE4_READ_ATTR. If that bit is not set in an ALLOW ACE, that is ok, but if that bit is set in a DENY ACE, we MUST reject the ACL.

## SETATTR May Need Owner Override

Scenario: Client opens file O_RDWR while it has permissions Then client changes the mode of the file to disallow write Then client does a setattr(size) to truncate the file

## We Currently Squash Attributes, There Was Some Discussion to Drop Instead

If client is root squashed, and then does a chown, we currently squash the chown, and operation may succeed. There was some discussion AFTER we got that code in that maybe we should instead, just drop changing the owner and owner_group attributes if the credentials were squashed.

A recent change skips squashing if the anonymous uid is 0.

## Need Synchronization to Prevent Multiple Ops on a Single State Owner or Stateid

I don't think we can guarantee we will never get multiple ops on the same state_owner or stateid, there are some issues if we are processing simultaneously since seqid is incremented and such.

## Oddities When Exporting the Same Path via Multiple Pseudo Paths

There are some oddities that can occur if using different Pseudo to export the same Path multiple times. I think this can be left as a documentation issue, but consider the following example:

Path = /export/exp1; Pseudo = /class-a/exp1 Path = /export/exp1; Pesudo = /class-b/exp1 Path = /export/exp1/some/path/exp2; Pseudo = /class-a/exp1/some/path/exp2

In this case, exp2 will actually appear as if it ALSO had:

Path = /export/exp1/some/path/exp2; Pseudo = /class-b/exp1/some/path/exp2

This is because the physical node, /export/exp1/some/path/exp2 can have only one export junction, but that node appears in both exp1 exports.

And in fact, the following export would raise problems because of this:

Path = /export/exp1/some/path/exp2; Pseudo = /class-b/exp1/some/path/exp2

Thus using different Pseudo to get different classes of service, as Tag was originally introduced for, should not have additional exports nested underneath.

One solution, if path just contained exports, would be to do the following

Path = /export/exp1; Pseudo = /class-a/exp1 Path = /export/exp1; Pesudo = /class-b/exp1 Path = /export/exp1/some/path; Pseudo = /class-a/exp1/some/path; FSAL { Name = PSEUDO; } Path = /export/exp1/some/path/exp2; Pseudo =
/class-a/exp1/some/path/class-a/exp2
Path = /export/exp1/some/path/exp2; Pseudo =
/class-a/exp1/some/path/class-b/exp2

Now the client never sees the original path directory with exp2 in it, and instead sees a directory with class-a and class-b sub-directories, each of which has exp2 in them.
