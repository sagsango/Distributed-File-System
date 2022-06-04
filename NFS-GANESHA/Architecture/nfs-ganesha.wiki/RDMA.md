**The RDMA effort for Ganesha has been long abandoned, the current code may or may not compile but is most likely not functional and certainly is not supported by the active development team. If anyone develops an interest in actively developing and supporting RDMA, we would welcome you.**

***

At the moment we have an NFS-RDMA prototype developed by Dominique Martinet using SoftiWARP, Mooshika, Ganesha, and libntirpc.

SoftiWARP is a software implementation of the iWARP protocol. This offers a common environment for developers. SoftiWARP main website: https://www.gitorious.org/softiwarp

Mooshika is an RDMA abstraction layer that is used in libntirpc.

First install some dependencies (assuming rhel6):
<pre>
    yum install -y autoconf libtool librdmacm libibverbs
</pre>

Build/install mooshika:
<pre>
    git clone https://github.com/martinetd/mooshika.git
    cd mooshika
    git checkout -b next remotes/origin/next
    ./autogen.sh
    ./configure
    make
    make install
</pre>

Build/install softiwarp:
<pre>
    git clone https://git.gitorious.org/softiwarp/kernel.git

    git clone https://git.gitorious.org/softiwarp/userlib.git
</pre>

Build/install Ganesha with patched libntirpc:
<pre>
</pre>

To start using softiwarp, load the modules:
<pre>
    modprobe ib_uverb
    modprobe ib_uverbs
    modprobe rdma_ucm
    modprobe siw         # software iwarp module
</pre>

To see if an RDMA device is detectable, run this Mooshika tool:
<pre>
    # rcat -vvvvv -S 0.0.0.0
    librdmacm: couldn't read ABI version.
    librdmacm: assuming: 4
    CMA: unable to get RDMA device list
    ERROR: rcat.c (368), main: error: msk_init(&trans, &attr) failed (returned 19: No such device).
</pre>

To NFS mount using RDMA:
<pre>
    mount -t nfs -o vers=4,port=20049,rdma localhost:/home /mnt/temp
</pre>

To run a test server and test client:
<pre>
    mooshika/src/tests/nfsv4_client -s    # this starts a test server
    mooshika/src/tests/nfsv4_client -c    # this starts a test client
</pre>
