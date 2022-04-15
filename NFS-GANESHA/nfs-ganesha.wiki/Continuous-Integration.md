There are several continuous integration platforms that do some degree of testing of nfs-ganesha.

Many of these tests are run on every gerrithub patch submission, others run under different circumstances which will be noted for each test case (otherwise assume the test is run on each patch submission).

# CEA #

CEA runs two series of tests:
* FSAL VFS compiled with ASAN and clang, with 9P client (cthon and a few other basic tests), and some pynfs tests for NFS 4.0 and 4.1
* FSAL PROXY with NFS clients (cthon on 4.1 and 4.0 client)

Both series are linked to each other, and will only +1 if all tests succeeded. The exact script being run should be available [here](https://github.com/nfs-ganesha/ci-tests/tree/CEA/CEA) but are known not to be up to date.

Anyone can retrigger a test by leaving a comment on gerrit that contains 'recheck CEA'

# Centos CI #

CentOS CI tests are visible [here](https://ci.centos.org/view/NFS-Ganesha/) ; the tests scripts are run straight from github [in here](https://github.com/nfs-ganesha/ci-tests/tree/centos-ci)

There are three independant tests on this platform:
* checkpatch
* cthon04 (on what?)
* gluster (what tests?)

Anyone can retrigger cthon04 and gluster by leaving a comment on gerrit that contains either 'recheck cthon04' or 'recheck gluster'. Something like 'infra problems - recheck cthon04, recheck gluster' will retrigger both.

# Ceph CI #

NFS-Ganesha is built with the Ceph and RGw FSALS on Ceph's on Ceph's CI infrastructure at https://shaman.ceph.com/builds/nfs-ganesha/. These packages are built every 12 hours and are built using the latest versions of Ceph's `master` branch and NFS-Ganesha's `next` branch. Custom packages of any version of Ceph and NFS-Ganesha can be built upon request. These packages will be consumed by teuthology running a NFS-Ganesha suite for Cephfs and RGW.

There are also stable NFS-Ganesha (starting with 2.5) packages with the RGW and Ceph FSAL's for versions of Ceph (starting with Luminous) here: http://download.ceph.com/nfs-ganesha/.

For more information on the Ceph CI reach out to Ali Maredia at amaredia@redhat.com.

# [Gandi CI for FreeBSD](https://ganesha-ci.gandi-cloud.net/#/) #

nfs-ganesha built with `FSAL_VFS`, built and run on FreeBSD server and linux client. We test use `nfstest_posix`, this CI is mostly for compatibility testing so correct compiling and basic functionality is the aim.

The underlying CI is buildbot, all configuration and infrastructure is kept in [here](https://github.com/nfs-ganesha/ci-tests/tree/gandi-ci)

For more information please reach out to ganesha@gandi.net
