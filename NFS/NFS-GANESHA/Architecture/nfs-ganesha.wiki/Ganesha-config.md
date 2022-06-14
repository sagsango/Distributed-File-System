```no-highlight
FSAL
{
  Max_FS_calls = 0;
  GPFS {
        FSAL_Shared_Library = "/usr/lib/libfsalgpfs.so.4.2.0";
  }
}
FileSystem
{
  Umask = 0000 ;
  xattr_access_rights = 0400;
}
GPFS
{
}
CacheInode_Hash
{
}
CacheInode
{
    Attr_Expiration_Time = 600 ;
    Symlink_Expiration_Time = 600 ;
    Directory_Expiration_Time = 600 ;
    Use_Getattr_Directory_Invalidation = 0;
}
CacheInode_GC_Policy
{
    LRU_Run_Interval = 90;
    FD_HWMark_Percent = 60;
    FD_LWMark_Percent =  20;
    FD_Limit_Percent = 90 ;
    Reaper_Work = 15000 ;
}
NFS_Worker_Param
{
}
NFS_Core_Param
{
        Nb_Worker = 128 ;
	MNT_Port = 32767 ;
	NLM_Port = 32769 ;
	Nb_Max_Fd = -1 ; # -1 is not the default value                                                                                                                                                               
	Stats_File_Path = "/tmp/ganesha.stats" ;
	Stats_Update_Delay = 600 ;
	Clustered = TRUE ;
	NFS_Protocols="3,4";
}
NFS_DupReq_Hash
{
    Index_Size = 17 ;
    Alphabet_Length = 10 ;
}
NFS_IP_Name
{
    Index_Size = 17 ;
    Alphabet_Length = 10 ;
    Expiration_Time = 3600 ;
}
SNMP_ADM
{
    snmp_agentx_socket = "tcp:localhost:761";
    product_id = 2;
    snmp_adm_log = "/tmp/snmp_adm.log";
    export_cache_stats    = TRUE;
    export_requests_stats = TRUE;
    export_maps_stats     = FALSE;
    export_nfs_calls_detail = FALSE;
    #export_cache_inode_calls_detail = FALSE;                                                                                                                                                                        
    export_fsal_calls_detail = FALSE;
}
STAT_EXPORTER
{
    Access = "localhost";
    Port = "10401";
}
NFSv4
{
    Lease_Lifetime = 90 ;
    FH_Expire = FALSE ;
    Returns_ERR_FH_EXPIRED = TRUE ;
}
NFSv4_ClientId_Cache
{
    Index_Size = 17 ;
    Alphabet_Length = 10 ;
}
NFS_KRB5
{
}
EXPORT
{
  Export_Id = 77 ;
  FSAL="GPFS";
  Path = "/ibm/gpfs0";
  Root_Access = "*";
  RW_Access = "*";
  Pseudo = "/ibm/gpfs0";
  Anonymous_uid = -2 ;
  Anonymous_gid = -2 ;
  Make_All_Users_Anonymous = FALSE;
  NFS_Protocols = "3,4" ;
  Transport_Protocols = "UDP,TCP" ;
  SecType = "sys,krb5,krb5i,krb5p";
  MaxRead = 32768;
  MaxWrite = 32768;
  Filesystem_id = 192.168 ;
  Cache_Data =  FALSE;
  Tag = "gpfs0";
  Use_NFS_Commit = TRUE;
  Use_Ganesha_Write_Buffer = FALSE;
  Use_FSAL_UP = TRUE;
  FSAL_UP_Type = "DUMB";
}
```