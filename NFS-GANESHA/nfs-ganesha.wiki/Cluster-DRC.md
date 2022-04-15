# Cluster DRC
## Introduction
The NFS reply cache, also known as the Duplicate Request/Reply Cache (DRC) helps a server to identify duplicate requests and avoid repeated processing of non-idempotent operations. Generally, idempotent operations can be safely repeated and will cause no harm, but non-idempotent operations, can only be executed once.
For example exclusive create can be a success on first attempt, but if the same operation is retried, it will fail because the file is already created.
NFS-Ganesha maintains a Duplicate Reply Cache in node's local memory. 
In a clustered environment, there are issues because the duplicate reply cache is not cluster-aware and in the case of recovery or fail-over, if the NFS request is replayed on another server, then that server has no knowledge of the reply cache and may result in incorrect behavior.

## Design
We are designing for a single failure scenarios. So each node identifies a backup node for storing DRC.
In the case of failover, the backedup DRC is accessed and operations can continue smoothly. 

Whenever NFS-Ganesha receives a request, it will look for the entry to be present in the local DRC. If the entry is present, then response will be sent from the DRC. But if this is a new request, then a new entry is added to the local DRC. The new entry is also sent to the backup node. The backup nodes stores the backup DRC in the Ganesha memory. On a server fail-over, a new server will take over the IP of the failed server. This new server will figure out the backup node for the failed server. It will then retrieve all the DRC entries from the backup node and load them into its own local cache.

This clustered DRC design is based on “best effort”. It does not make any guarantees that the all the DRC entries will synced to the backup node and will be available immediately after fail-over. It could happen that there are some DRC entries that never make it to the backup server before the server fails. In that case, replayed non-idempotent operations may fail. This design is trying to alleviate (not completely eliminate) the conditions when failures are encountered because of replaying non-idempotent operations.

## Design Details
### CMAL Interface
A new Abstraction layer called the Cluster Management Abstraction Layer would be added to NFS-Ganesha. This Abstraction layer would be similar to the FSAL (File System Abstraction Layer). The CMAL, as the name suggests would provide an abstraction for cluster manager support. So this would be a generic layer that would interface with different backend cluster managers. The backend cluster manager will be a dynamically linked layer. So the CMAL layer would have function pointers based on the CMAL that is dynamically linked. The backend CMAL would interface with the available cluster manager. 
* **init_cmal**

Initialize the CMAL interface. Set up backup node information.

* **add_drc_entry(xid, entry)**

Stores the DRC entry in the cluster. This functionality will be provided by the backend CMAL layer, which will talk to the available cluster manager. 

* **retrieve_drc_entries(ip_address)**

Retrieve all the DRC entries for a particular sever node.

* **shutdown_cmal()**

Shutdown the CMAL interface

 