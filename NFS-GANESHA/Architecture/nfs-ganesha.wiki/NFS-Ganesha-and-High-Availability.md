NFS-Ganesha does not provide its own clustering support, but HA can be achieved using Linux HA.

1. FSAL Gluster:
*    NFS-Ganesha + GlusterFS based on Pacemaker is only available up to glusterfs 3.10. 
     [More information](http://docs.gluster.org/en/latest/Administrator%20Guide/NFS-Ganesha%20GlusterFS%20Integration/)  

*    NFS-Ganesha + GlusterFS based on CTDB is available from  [[https://github.com/gluster/storhaug]]

2. FSAL Ceph:
   With version 2.5.5 and above, RADOS_KV backend has been introduced. Work in underway for 2.7 to have clustering support in NFS-Ganesha for cephfs. But unless that is backported to previous release, you need a Linux HA solution. 
