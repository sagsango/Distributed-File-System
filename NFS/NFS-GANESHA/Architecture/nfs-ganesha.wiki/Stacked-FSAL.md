A Stacked FSAL is a way of layering one FSAL on top of another FSAL, allowing a transformation of the data between the lower and upper layers.  So, for example, if a clustered filesystem had a Data Server (DS) that was a little bit of distributed metadata over a local filesystem, it could be implemented as a stacked FSAL on top of FSAL VFS, intercepting metadata operations and redirecting them to the cluster, and allowing data operations to pass through to FSAL VFS underneath for processing.

There are currently 2 stacked FSALs in tree:

1. NULL
2. MDCACHE

## NULL FSAL
NULL does nothing by itself, hence it's name.  It just stacks over a FSAL and passes all operations directly through.  It has two purposes:

1. To provide an example of a stackable FSAL that can be used as a basis for future work
2. To provide the ability to test FSAL stacking in Ganesha.

It's configured like any other FSAL, and the only bit of config it supports (the FSAL it's stacking over) is listed in config_samples/config.txt.


## MDCACHE FSAL
MDCACHE is an entirely different beast.  Ganesha has always had an inode/metadata caching layer in it.  It was historically called cache_inode, and lived in it's own directory, with it's own APIs. Protocol layer code called cache_inode_*, which in turn called into the FSALs via the FSAL API using exports and handles.

However, the cache_inode API was functionally very similar to the FSAL API, and there was a desire to enable FSALs with no metadata caching layer in Ganesha, since they may share memory space with an MDS that already caches.  To achieve this, cache_inode was transformed into a stacked FSAL that could live on top of another FSAL, and use the FSAL API.  This allowed the protocol layers to operate via the FSAL API, and therefore be agnostic as to whether caching was done in Ganesha or outside.

The result is FSAL_MDCACHE, which is a stackable FSAL that provides 2 features for other FSALS:
1. The handle cache.  The SAL depends on handles living in memory with the same pointer value for the lifetime of it's refcount.  MDCACHE provides a LRU-based cache of handles, so that the pointers are stable.
2. Metadata cache.  The two parts of it are the dentry cache, speeding up lookups, and the inode metadata cache, allowing faster metadata lookups, particularly for things like owner and mode that are used multiple times per op.

At this time, there are no NFS Ganesha FSALs that are uncached. MDCACHE auto-stacks at the top of any stack of FSALs.  If a FSAL wants to be uncached, this would need to be changed, and that FSAL would need to provide it's own handle cache.

For config compatibility, MDCACHE is still configured via the CACHE_INODE section in the config file.
