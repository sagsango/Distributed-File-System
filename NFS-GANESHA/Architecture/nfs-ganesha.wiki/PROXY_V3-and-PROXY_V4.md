## Configuring a NFS-GANESHA server as an NFS v3 or v4 proxy

NFS-GANESHA has different backend modules, each of them dedicated to address a specific namespace. These backends are called FSALs (which stands for "File System Abstraction Layer"). Two of these PROXY_V3 and PROXY_V4 act as an NFS proxy: they act as an NFS v3/v4 client to some other backend server and turn NFS-GANESHA into an NFS proxy. Importantly, NFS-GANESHA itself can still serve v3 or v4 clients, so you can use the `PROXY` FSALs to turn NFS-GANESHA into a "v4 to v3" proxy or "v3 to v4".

### Step 1: Compiling NFS-GANESHA with the PROXY_V3 / PROXY_V4 FSALs

The PROXY_V3 and PROXY_V4 FSALs default to `ON` in the CMake config, so they should be built by default. You can control them via the `USE_FSAL_PROXY_V3` and `USE_FSAL_PROXY_V4` options. Note that the V3 proxy also requires `USE_NFS3` to be `ON`.

### Step 2 : Writing the configuration file

Assume you have a backend NFS server on a host at `192.168.0.100` which has an export of `/home`. Before trying to setup your proxy, ensure you can successfully mount the backend from your NFS-GANESHA host. For example:

```
sudo mount -v -t nfs -o proto=tcp,vers=3,nolock 192.168.0.100:/home ${HOME}/my-test-mount
```

Note: adding `nolock` is important for this test to avoid interfering with NLM on the same host as NFS-GANESHA.

Assuming you can successfully mount and interact with the backend server (so your firewall rules and permissions allow it), you can now proxy the backend.

Configuring one of the proxy FSALs is as simple as making an `EXPORT` with that information in your config:

```
EXPORT {
  Export_Id = 1234; # This can be anything as long as it is unique
  Path = "/home";
  Pseudo = "/home";

  Access_Type = RW;
  Squash = No_Root_Squash;
  SecType = "sys";
  Transports = "TCP";

  FSAL {
    Name = PROXY_V3; # Or PROXY_V4 for V4 backend servers.
    Srv_Addr = 192.168.0.100;
  }
}
```

Note that you *must* use `PROXY_V3` for NFSv3 backends and `PROXY_V4` for NFSv41 backends. The underlying code only implements one client version.

At this point, you should now be able to mount the "/home" directory against the NFS-GANESHA server and all the NFS operations will be proxied to the backend:

```
sudo mount -v -t nfs -o proto=tcp,vers=3 nfs-ganesha-host:/home ${HOME}/my-proxied-home
```

Even though the `PROXY` FSALs only speak to the *backend* server via one of NFSv3 or NFSv4, NFS-GANESHA can still serve both NFSv3 and NFSv41 clients. `PROXY_V3` supports this transparently, so the above mount line could be:

```
sudo mount -v -t nfs -o proto=tcp,vers=4.1 nfs-ganesha-host:/home ${HOME}/my-proxied-v4-home
```

to use NFSv41 as the client protocol. `PROXY_V4` requires the use of "handle mapping" (see next section) to permit NFSv3 clients to work with an NFSv41 backend. Note that NFSv41 *features* that aren't translatable to V3 won't work via `PROXY_V3` but basic operations will.

### Path vs Pseudo: Exporting with a different path

In the first example, the parameters `path` and `pseudo` were set to the same value of `/home`. When proxying different backends, however, you may end up with several servers each with their own `/home` export that you wish to proxy. The `PROXY` FSALs both use the `path` as the "backend path", but then allow NFS-GANESHA to (re-)export this via `pseudo`. Here's an example of exporting two different home directories as `home-1` and `home-2`:

```
EXPORT {
  Export_Id = 1234;
  Path = "/home";
  Pseudo = "/home-1";

  Access_Type = RW;
  Squash = No_Root_Squash;
  SecType = "sys";
  Transports = "TCP";

  FSAL {
    Name = PROXY_V3;
    Srv_Addr = 192.168.0.100;
  }
}

EXPORT {
  Export_Id = 4567;
  Path = "/home";
  Pseudo = "/home-2";

  Access_Type = RW;
  Squash = No_Root_Squash;
  SecType = "sys";
  Transports = "TCP";

  FSAL {
    Name = PROXY_V3;
    Srv_Addr = 192.168.0.101;
  }
}
```

Note that the backend IP (`Srv_Addr`) is different as are the values for `pseudo`.

### Unmaintained/Untested: Handle Mapping / Re-exporting NFSv3 from `PROXY_V4`

*As of this writing (01-Nov-2021) the handle mapping code is known to be buggy*

The `PROXY_V4` FSAL has a feature ("handle mapping") that can try to directly serve NFSv3 clients from an NFSv4 backend. Because file handles in NFSv4 are *larger* than in NFSv3, the code keeps a SQLite database of "FH3 <=> FH4" mappings. To enable handle mapping, you need SQLite 3.0+ installed and need to enable the `PROXYV4_HANDLE_MAPPING` option in CMake (it defaults to `OFF`).

There are additional configuration flags for handle mapping:

* HandleMap_DB_Dir (the filesystem path for where the sqlite database will be stored)
* HandleMap_Tmp_Dir (a temporary directory, required by sqlite)
* HandleMap_DB_Count (the number of maps to make. Defaults to 1 per thread)

### Unimplemented: GSSRPC / KRB5

The `PROXY_V4` FSAL also has the beginnings of KRB5 support via libgssrpc (see the code under `export.c` under the `_USE_GSSRPC_` ifdefs). However, the configuration values don't seem to be used anywhere in the code, so they don't work.