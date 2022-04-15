https://github.com/nfs-ganesha/nfs-ganesha/tree/next

## Sigmund test results
clients: centos5 32 bit, centos6.4 64 bit

| FSAL        | Server Config | Pass    | Fail  | Date |
| ----------- |--------:|------:| -----:| ---------------------:|
| GPFS (ibm)  | rhel63 | ?           | ?  |   |
| VFS (ibm)   |  |           |       |   |
| VFS (panas) |  |           |       |   |
| 9P (CEA)    |  |            |       |   |
| XFS (panas) |  |            |       |   |
| PNFS (panas)|  |            |       |   |
| LUSTRE (CEA)|  |            |       |   |

## cthon04 NFSv3 test results

| FSAL        | Server Config | NFSv3 Pass    |NFSv3 Fail  | NFSv4 Pass  | NFSv4 Fail | Date |
| ----------- |--------:|-------------:| -----:| -----:| -------:| ---------:| ---------------------:|
| GPFS (ibm)        | rhel63  | b,g,s,l      |        | b,g,s,l        |           | 4/17/2013 |
| VFS (ibm)        |  |            |       |       |         |  |
| VFS (panas)        |  |            |       |       |         |  |
| 9P (CEA)          |  |             |       |       |         | |
| XFS (panas) |  |            |       |   |      | |
| PNFS (panas)|  |            |       |   |      | |
| LUSTRE (CEA)|  |            |       |   |      | |

## Pynfs test results

| FSAL        | Server Config |Asked (of 643)| Pass  | Fail  | Warning| Skipped | Date |
| ----------- |--------:|-------------:| -----:| -----:| -----:| -------:| ---------------------:|
| GPFS (ibm)  | rhel63 | 565           | 504   | 55    | 2    | 8       | 4/17/2013 |
| VFS (ibm)        |  |            |       |       |         |  |
| VFS (panas)        |  |            |       |       |         |  |
| 9P (CEA)          |  |             |       |       |         | |
| XFS (panas) |  |            |       |   |      | |
| PNFS (panas)|  |            |       |   |      | |
| LUSTRE (CEA)|  |            |       |   |      | |

## dNFS test results

| FSAL        | Server Config | Result    | Date |
| ----------- |--------:|------:| ---------------------:|
| GPFS (ibm)      |rhel63  | pass           | 4/17/2013 |
| VFS (ibm)        |rhel63  |       pass          | 4/17/2013 |
  