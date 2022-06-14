## QA Process
### Working group:

* **Hooshang Dadgari** hdadgari@panasas.com  from Panasas
   * FSALs responsible for testing: VFS, PNFS, XFS
* **Philippe Deniel** philippe.deniel@cea.fr from CEA
   * FSALs responsible for testing: LUSTRE, 9P
* **Jeremy Bongio** jbongio@linux.vnet.ibm.com from IBM
   * FSALs responsible for testing: GPFS, VFS

ZFS, CEPH, PROXY ???

### Criteria


In order for a next release candidate to be acceptable the following tests must pass:
 1. cthon04 with nfsv3/4, udp/tcp, krb5/i/p   with the solaris client, linux knfs client, and dNFS client.
 2. pynfs nfs4.0 with less then 15 fails of 565 tests run.
 3. all Ganesha regression tests must pass on nfsv3/4.

The FSALs to be tested:
 * PNFS
 * 9P
 * GPFS
 * VFS
 * LUSTRE
 * PROXY
 * CEPH
 * XFS
 * ZFS

### testing results


| Branch        | General health  | repos link | tests link |
| --------------------- | ---------:| ------:|------:|
| lieb/next        | rough       | [link](https://github.com/lieb/nfs-ganesha/tree/next) | [tests](https://github.com/nfs-ganesha/nfs-ganesha/wiki/next-test-results) |
| phdeniel/stable_1_5_x          |  good     |[link](https://github.com/phdeniel/nfs-ganesha/tree/stable_1_5_x) | [tests](https://github.com/nfs-ganesha/nfs-ganesha/wiki/stable15-test-results) |

## Ganesha configuration

[config](https://github.com/nfs-ganesha/nfs-ganesha/wiki/ganesha-config)

## Machine configuration

| name        | used by FSALs  | link to config |
| --------------------- | ---------:| ------:|
| rhel63        | GPFS,       |  [config](https://github.com/nfs-ganesha/nfs-ganesha/wiki/rhel63-config) |
| BSD        |   | [config](https://github.com/nfs-ganesha/nfs-ganesha/wiki/bsd-config)  |
| Solaris        |   | [config](https://github.com/nfs-ganesha/nfs-ganesha/wiki/solaris-config)  |
|         |   |   |

kickstart scripts, rhel6, bsd, solaris, etc.   packages and version

## How to run specific tests

### Running NFS-Ganesha regression tests
```no-highlight
# mount an export from ganesha
mount -t nfs <ganesha_host>:/export/dir  /mnt/sigmund
git clone http://github.com/phdeniel/sigmund.git
./sigmund/run_test.sh

# Running Sigmund's tests in a "quiet" way
./sigmund/run_test.sh -q

# Running Sigmund's tests with JUint/XML output in /tmp
./sigmund/run_test.sh -j
```
More information is available here (on Sigmund's wiki) : [Sigmund's wiki](http://github.com/phdeniel/sigmund/wiki/Using-Sigmund)

### Running cthon04 (nfsv3/4 with root/nonroot)

```no-highlight
git clone https://github.com/bongiojp/cthon04.git
cd cthon04
sudo ./runcthon --server ${SERVER} --serverdir $HOSTFS/hudson/root/$NODE_NAME --onlyv3 --noudp
./runcthon --server ${SERVER} --serverdir $HOSTFS/hudson/root/$NODE_NAME --onlyv3 --noudp
sudo ./runcthon --server ${SERVER} --serverdir $HOSTFS/hudson/root/$NODE_NAME --onlyv4 --noudp
./runcthon --server ${SERVER} --serverdir $HOSTFS/hudson/root/$NODE_NAME --onlyv4 --noudp
```

### Running cthon04 with kerberos (will test krb5/krb5i/krb5p with nfsv3/4 and root/nonroot)
```no-highlight
git clone https://github.com/bongiojp/cthon04.git
cd cthon04
sudo ./runcthon --server ${SERVER} --serverdir $HOSTFS/hudson/root/$NODE_NAME --onlyv3 --onlykrb5 --noudp
./runcthon --server ${SERVER} --serverdir $HOSTFS/hudson/root/$NODE_NAME --onlyv3 --onlykrb5 --noudp
sudo ./runcthon --server ${SERVER} --serverdir $HOSTFS/hudson/root/$NODE_NAME --onlyv4 --onlykrb5 --noudp
./runcthon --server ${SERVER} --serverdir $HOSTFS/hudson/root/$NODE_NAME --onlyv4 --onlykrb5 --noudp
```
### Running cthon04 with solaris
```no-highlight
git clone https://github.com/bongiojp/cthon04.git
cd cthon04
git checkout --track remotes/origin/solaris
sudo ./runcthon --server $SERVER --serverdir $HOSTFS/hudson/root/$NODE_NAME --onlyv3 --noudp
./runcthon --server $SERVER --serverdir $HOSTFS/hudson/root/$NODE_NAME --onlyv3 --noudp
sudo ./runcthon --server $SERVER --serverdir $HOSTFS/hudson/root/$NODE_NAME --onlyv4 --noudp
./runcthon --server $SERVER --serverdir $HOSTFS/hudson/root/$NODE_NAME --onlyv4 --noudp
```

### Running pynfs
```no-highlight
git clone git://git.linux-nfs.org/projects/bfields/pynfs.git
cd pynfs
yes | python setup.py build
cd nfs4.0
sudo python testserver.py $SERVER:$HOSTFS --maketree all &> /tmp/pynfs4.0/root-results
# in the future ganesha/src/scripts/test_pynfs will have a script to run that chooses which tests to run.
```

### Running Oracle 11g dNFS tests
```no-highlight
  
```