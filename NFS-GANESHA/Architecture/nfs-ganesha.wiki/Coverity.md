# Static code analysis with Coverity

[Coverity](http://www.coverity.com/) is a popular static code analysis tool which supports different programming languages such as Java, C and C++. As it is quite a powerful tool, it does not only checks for simple mistakes (like uninitialized variables or memory leaks) but it can also detect more complex patterns and code paths which may eventually lead to security threats and even thread deadlocks. On top of that, it ships with a web front-end to manage issues.

The NFS-Ganesha project as started using Coverity during the 2.0 development phase (around august 2013). At first, the total number of defects reached 364 which wasn't very high compared to the roughly 180,000 LoC that makes NFS-Ganesha. Among these 364 defects, there is a fair high number false positive issues (intentional or bad guesses made by Coverity) accounting for about a third of the total defects base. The development team has put a lot of effort to improve the overall code's quality and today we have got only a few issues left. The following chart shows the defects trend over time:

![NFS-Ganesha Coverity defects trend](https://38.media.tumblr.com/c5effdb73c2cebc492579dfdb723f224/tumblr_nenu1pMzeM1tt4h2fo1_1280.png)

# The analysis process

So far, we're using two different Coverity instances: one based on the commercial version (courtesy of [Bull](http://www.bull.com/)) and one using the free [online version](https://scan.coverity.com/projects/2187?tab=overview) available to all. There are some differences between these two versions, that's why we keep both.

NFS-Ganesha maintains two branches within its github repository, one for the development version (next) and one for the stable release (master). Both are analysed regularly, each time a new merge is made upstream.

## Stable branch

The stable branch isn't updated very often and we don't push analysis on the online tool.

## Development branch

The development branch is analysed on a weekly basis, see the [DevPolicy](https://github.com/nfs-ganesha/nfs-ganesha/wiki/DevPolicy) wiki page for further information, and both tools are run against each release. A summary report is sent to the development mailing list.

## Access to the online Coverity

Developers interested in joining the effort may ask to get privileged access directly from [here](https://scan.coverity.com/projects/2187?tab=overview).

## Analysis environment

Here's a description of the build environment currently used to run Coverity against NFS-Ganesha's code base (both tools).

### Base

* RHEL 6.5

### Manually added dependencies

* cmake: 2.8.11.2
* lustre: 2.5.3
* mooshika: 0.6
* glusterfs: 3.5.0
* LTTng: 2.5.0
* jemalloc: 3.4.1
* ZFS: version provided in [contrib/](https://github.com/nfs-ganesha/nfs-ganesha/tree/next/contrib/libzfswrap)

## Build script

Here's how nfs-ganesha is built for analysis, I use custom paths for many libraries otherwise it's pretty straightforward:

```
#!/bin/bash
git submodule update
if [ -d build ]; then
    rm -rf build
fi

mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Maintainer \
      -DBUILD_CONFIG=everything \
      -DDEBUG_SYMS=ON \
      -DDEBUG_SAL=ON \
      -DUSE_TIRPC_IPV6=ON \
      -DUSE_ADMIN_TOOLS=ON \
      -DUSE_DBUS=ON \
      -DUSE_LTTNG=ON \
      -DUSE_FSAL_LUSTRE=ON \
      -DUSE_FSAL_LUSTRE_UP=OFF \
      -DUSE_FSAL_PT=ON \
      -DUSE_FSAL_CEPH=ON \
      -DUSE_FSAL_ZFS=ON \
      -DUSE_FSAL_GLUSTER=ON \
      -DUSE_FSAL_PROXY=ON \
      -DUSE_9P_RDMA=ON \
      -D_USE_9P_RDMA=ON \
      -DPROXY_HANDLE_MAPPING=ON \
      -D_NO_XATTRD=OFF \
      -DUSE_CB_SIMULATOR=ON \
      -DLUSTRE_PREFIX=$LUSTRE_PATH/usr \
      -DLIBIBVERBS_PREFIX=$LIBIBVERBS_BR/usr \
      -DLIBRDMACM_PREFIX=$LIBRDMACM_BR/usr \
      -DMOOSHIKA_PREFIX=/homes/favrebut/local \
      -DGLUSTER_PREFIX=/homes/favrebut/local \
      -DCEPH_PREFIX=/homes/favrebut/local \
      -DZFS_PREFIX=/homes/favrebut/local \
      ../src
make -j4
```