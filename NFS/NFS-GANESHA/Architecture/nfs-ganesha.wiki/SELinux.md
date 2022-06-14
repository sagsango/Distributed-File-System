Beginning with NFS-Ganesha-2.7 Ganesha provides its own SELinux policy subpackage when building RPMs for RHEL, CentOS, and Fedora. This package is derived from the RHEL 7 and Fedora SELinux policies. The subpackage is only built on RHEL 8 and Fedora 30 and later, although it could be enabled for RHEL 7 and Fedora 29.

RHEL 8 and Fedora 30 (will) no longer include policies for NFS-Ganesha in their selinux-policy-* packages. To use NFS-Ganesha on these platforms you must install the nfs-ganesha-selinux subpackage.

The subpackage build can be enabled for RHEL 7 and Fedora 29. When installed it installs at priority 200, overriding the base policies which are installed at priority 100. At the time of this writing they are the same, so this is operationally a no-op.

If you observe AVC denials when running NFS-Ganesha using the nfs-ganesha-selinux subpackage, please report them by opening an issue at https://github.com/nfs-ganesha/nfs-ganesha/issues.

Here's how to collect the info necessary to resolve the AVC:
1. Set SELINUX to "permissive" in /etc/selinux/config and reboot. 
2. Run nfs-ganesha until the AVC is observed. 
3. Run `audit2allow` and attach the output to the issue. 
3. Attach the AVC log messages from /var/log/audit/audit.log to the issue. 