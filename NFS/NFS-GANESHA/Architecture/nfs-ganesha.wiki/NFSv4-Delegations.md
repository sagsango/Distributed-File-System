One of the major features of NFSv4.0 is its ability to delegate file operations to the accessing client. The
intent of this document is to describe the extensions necessary to the Ganesha community code base to
support READ and WRITE delegations. This will allow compliant NFSv4.0 clients the ability to cache data
and metadata modifications without having to constantly contact the server. 
The Delegation feature is described in full by [RFC7530](https://tools.ietf.org/html/rfc7530)

As detailed in [RFC7530](https://tools.ietf.org/html/rfc7530), file delegations are an optional feature of the protocol. An NFS server can
choose to implement the feature or not; however, doing so, will allow compliant NFSv4.0 clients to
leverage delegations for aggressive caching of file data and/or metadata.
There are two flavors of file delegations: READ and WRITE:

1. For READ delegations, the server gives the client the assurance that no other client will have the
ability to write to the file for the duration of the delegation.
2. For WRITE delegations, the server gives the client the assurance that no other client has read or
write access to the file.

Delegations can be recalled by the server upon detection of conflicting access to the delegated file by
another client or due to other restrictions, such as lack of server resources. In order for the server to be
able to grant delegations, a callback path MUST exist between the server and client, such that the server
can have a method of recalling a delegated file. If this callback path cannot be established,
delegations cannot be granted. The specification suggests that a callback path be established and
tested with a CB_NULL operation, so it can be known early on whether delegations will be enabled or not.
Therefore, upon the receipt of a CB_RECALL, a client MUST either give up the (READ) delegation or start
pushing the cached content of a (WRITE) delegation, as soon as possible. The client is expected to
finalize the recalled delegation transaction by issuing a DELEGRETURN.

Delegations can also be revoked if the server has CB_RECALL'ed a delegated file, but the client either
fails to push its cached data in a timely fashion, a network partition between the client and server exists,
or the client simply ignores the CB_RECALL from the server. For READ delegations, this may not pose a
major issue, as the only guarantee the client had was that no other client would have the ability to write to
a READ delegated file. However, revocation of a WRITE delegated file can result in some or all of the
client cached data to be invalidated. Therefore, the client MUST start pushing any cached or modified
data/metadata quickly to the server, upon receipt of a CB_RECALL; failure to do so, risks the client cached
content to be invalidated and the delegation revoked. The server should also be more lenient as to how
much time (in lease periods) it may grant a client that is seen to be diligently pushing cached data back to
the server, before a revocation is performed.

Delegation of file operations to the client is expected to yield a performance improvement as the client
doesn't have to go over-the-wire (otw) for each operation it performs on the file. However, it should be
noted that these expected performance improvements are closely tied to the type of workloads being
exercised. Workloads that allow a client to exclusively access or modify a delegated file for extended
periods of time are likely to benefit the most by delegations. In contrast, workloads that allow multiple
clients to simultaneously access the same file will more than likely experience a degradation in
performance by the delegation overhead cycle (ie. File opened, delegation granted, conflicting access
detected, delegation recalled, etc).

In summary...
* File delegations will only be granted if the callback channel is operational. That is, the Ganesha
server will NOT grant any delegations if it has no reliable way to recall said delegation; this is
especially crucial when a WRITE delegation has been granted to a client that may have substantial
data cached locally, that needs to be pushed to the server's stable storage.

* As per [RFC7530](https://tools.ietf.org/html/rfc7530), any client that does not respond to a CB_RECALL w/in the lease period, risks
the delegation for the file being revoked by the server. However, if in response to the CB_RECALL,
the client starts flushing cached data (assuming large amounts of data), the server can be more
lenient as to much time (measured in lease periods) it may grant the client and to be able to finish
and issue DELEGRETURN.

## Design Details

### Delegation Grants

As described in [RFC7530](https://tools.ietf.org/html/rfc7530), an NFSv4 server grants a delegation in response to an OPEN
request. However, it is completely within the server's purview whether a delegation is granted or
not, so clients should not assume that a delegation will be granted. When a server indeed does
grant a delegation, additional state is created in the server in order to track the delegation (or
delegations, plural, in the case of READ delegations) on that file. This state remains in effect
while the delegation is outstanding and in the normal case, a client returns the delegation via the
DELEGRETURN op, which in turn destroys the server state associated with the delegation for that
file, for that client.

In order to be able to provide delegations, the server needs a way to communicate with the
client that was granted the delegation, that it is time to return control over to the server. This
communication channel is done via a callback RPC path. The callback path needs to be
established as part of a client original handshake with the server. The most natural place to do
this, from a protocol perspective, is in either SETCLIENTID or SETCLIENTID_CONFIRM. As
previously noted, _**if a callback path cannot be established from the server to the client,**_
_**delegations will not be possible**_. Performing this check early on is the right place since the
callback path is a clientid entity; moreover, it allows the server to create the callback path and
have it handy for future use. `nfs4_op_setclientid_confirm() → rpc_cb_null()`

The server will attempt to pro actively detect if a file has been delegated and recalled in rapid
succession for a tuneable set number of times. If this is the case, this would indicate the file is
being delegated to a client and it gets immediately recalled because it is either a hot file that is
being opened in conflicting ways from separate clients. In order to detect such behavior, the
server needs to maintain some statistics about how many times the file has been delegated,
recalled, and the average time a particular client held a delegation. With this information, the
server can make better decisions as to which files are worth the overhead of delegating to a
client or whether the drawbacks outweigh the benefits.

The server shall construct a unique token stateid for delegations. As per section 10.4 of
[RFC7530](https://tools.ietf.org/html/rfc7530), the delegation stateid is
separate and distinct from the OPEN stateid. The delegation stateid differs in that it is associated
with a client ID and may be used on behalf of all the open-owners for the given client. Once granted,
a delegation behaves in most ways like a lock in that it maintains an associated lease that is subject
to renewal via the RENEW operation. However, unlike a lock, a second client performing an operation
that conflicts in access mode to a delegated file, will cause the server to recall the delegation.

At the bottom of section 10.4.6, we find the following language:

> When the client holds a delegation, it cannot rely on operations,
except for RENEW, that take a stateid, to renew delegation leases
across callback path failures. The client that wants to keep
delegations in force across callback path failures must use RENEW
to do so.

The client may issue other operations that inherently update the lease, however, it is only via the
RENEW operation that the server can communicate back to the client if there is a problem with
the callback path. Therefore, a client MUST continue the use of RENEW to get callback path
status information and take corrective action if needed. This implementation will not be tolerant
of rogue or stuck clients that are not in compliance with the specification's recommendations.

The criteria for granting delegations can either be some mechanism defined as part of managing
the delegation state, or it can be something that can be left as an FSAL API, if the underlying
file-system can provide a richer set of policies on **when** to delegate a file. Initially,
however, ganesha will need _some_ set of heuristics to delegate a file based on statistical
data collected over time. _**We invite the community to help define this in a way that works
in a general way for all FSAL's**_

### Conflict Detection

Most of ganesha's supported back end file systems are clustered in nature. This provides ganesha
an opportunity to leverage the underlying file-system's cluster management software to detect
conflicting access to actively delegated files. Whether the access is via NFSv3, NFSv4.0, cifs,
or local node access, the underlying file-system already has the infrastructure to deal with
such events in order to maintain coherency across the cluster. 

### ... more to come ... =^)