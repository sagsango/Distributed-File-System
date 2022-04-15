https://github.com/phdeniel/nfs-ganesha/tree/stable_1_5_x

## Regression test results

| FSAL        | OS config |Pass    | Fail  | Top sha1 |
| ----------- |--------:|------:| -----:| ---------------------:|
| GPFS        | rhel63 |?           | ?  | a1a8740  |
| VFS         | |            |       |  a1a8740 |
| PROXY       |  |           |       |  a1a8740 |
| XFS         |  |           |       |  a1a8740 |
| CEPH        |  |           |       |  a1a8740 |
| LUSTRE      | |            |       |  a1a8740 |

## cthon04 NFSv3 test results

| FSAL        | OS config |NFSv3 Pass    |NFSv3 Fail  | NFSv4 Pass  | NFSv4 Fail | Top sha1 |
| ----------- |--------:| -------------:| -----:| -----:| -------:| ---------:| ---------------------:|
| GPFS        | rhel63 | b,g,s,l           |    | b,g,s,l    |        | a1a8740 |
| VFS         | |           |       |       |         | a1a8740 |
| PROXY       |  |          |       |       |         | a1a8740 |
| XFS         |   |         |       |       |         | a1a8740 |
| CEPH        |  |          |       |       |         | a1a8740 |
| LUSTRE      | |           |       |       |         | a1a8740 |

## Pynfs test results

| FSAL        | OS config |Asked (of 643)| Pass  | Fail  | Skipped | Top sha1 |
| ----------- |--------:|-------------:| -----:| -----:| -------:| ---------------------:|
| GPFS        | rhel63|565           |    | 7    |        | a1a8740 |
| VFS         | |565           |       |       |         | a1a8740 |
| PROXY       | |565           |       |       |         | a1a8740 |
| XFS         | |565           |       |       |         | a1a8740 |
| CEPH        | |565           |       |       |         | a1a8740 |
| LUSTRE      | |565           |       |       |         | a1a8740 |

## dNFS test results

| FSAL        | Pass    | Top sha1 |
| ----------- |------:| ---------------------:|
| GPFS        | yes           | a1a8740 |
| VFS         |            | a1a8740 |
| PROXY       |            | a1a8740 |
| XFS         |            | a1a8740 |
| CEPH        |            | a1a8740 |
| LUSTRE      |            | a1a8740 |