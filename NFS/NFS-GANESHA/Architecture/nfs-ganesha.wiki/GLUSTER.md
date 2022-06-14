# Exporting GlusterFS volumes via Ganesha

The FSAL_GLUSTER allows you to export GlusterFS volumes with NFS-Ganesha. It relies on `libgfapi` user-space library to access the data stored in GlusterFS volume. This library is distributed by most recent Linux distributions, and called `glusterfs-api` in Fedora and RHEL based systems.

Before using this feature make sure that GlusterFS volumes are created and ready to be exported. Refer to the [Gluster QuickStart Guide](http://docs.gluster.org/en/latest/Quick-Start-Guide/Quickstart/) to setup and create glusterfs volumes.

## Compiling NFS-GANESHA with the FSAL_GLUSTER
If using `cmake`,

~~~
$ mkdir build
$ cmake ../path/to/nfs-ganesha/src
$ make
$ sudo make install
~~~

You will need to have the `glusterfs-api-devel` package installed. By default, `cmake` should be able to detect the library. If the detection fails, try passing the `-DUSE_FSAL_GLUSTER=ON` option to the above `cmake` command.

## Configuring the specific stuff for FSAL_GLUSTER
To configure NFS-Ganesha to export GlusterFS volume, below are the minimal set of options required to be set in the configuration file (by default `/etc/ganesha/ganesha.conf`).

~~~
EXPORT
{
   # Export Id (mandatory, each EXPORT must have a unique Export_Id)
   Export_Id = 77;

   # Exported path (mandatory)
   Path = "/testvol"; # assuming 'testvol' is the Gluster volume name

   # Pseudo Path (required for NFS v4)
   Pseudo = "/testvol";

   # Required for access (default is None)
   # Could use CLIENT blocks instead
   Access_Type = RW;

   # Allow root access
   Squash = No_Root_Squash;

   # Security flavor supported
   SecType = "sys";

   # Exporting FSAL
   FSAL {
     Name = "GLUSTER";
     Hostname = "10.xx.xx.xx";  # IP of one of the nodes in the trusted pool
     Volume = "testvol";
   }
}
~~~
