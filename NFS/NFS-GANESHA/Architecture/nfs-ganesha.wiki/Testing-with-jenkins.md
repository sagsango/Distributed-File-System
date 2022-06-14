Jenkins is an automation environment that can poll git repositories, download, build ganesha on a server, test the server with multiple scripts from multiple clients, then parse the results and report on the health of the server. There are a lot of pieces to put together though. Hopefully this page takes away some of the pain.

## Installing ##

Download latest (version > 1.500) from front page of http://jenkins-ci.org/

In my experience updating from one version to another through an installed jenkins instance is a disaster.

I think this was a simple process. If not we can add instructions.

## Configuring ##

First install plugins, then add VM clients, then create jobs which will run scripts on the clients.

### Plugins to install ###

* Git Plugin - This will allow us to poll git repositories and 
* Libvirt Slaves Plugin - This enables jenkins to start/stop VMs and ssh to them to start the jenkins client.

### Add VMs to list of known machines ###

To add a new node:
 1. click these "Manage Jenkins" -> "Manage Nodes" -> "New Node"
 2. Add a node name which will be displayed in jenkins to identify that VM and choose
   "Slave virtual computer running on a virtualization platform (via libvirt)"
 3. Fill out the fields similar to this and then repeat steps 1-4 for every VM you want to use:

![Configuring a new VM client](https://raw.github.com/bongiojp/ganesha_jenkins/master/pics/addclient_p3.png "Configuration of a VM client")

Here is the list of VM clients I've been using. Keep in mind the terms sonas13 and sonas20 are incorrect. sonas13 tests the ibm_next branch. sonas20 tests the ganesha 2.0 bulid with GPFS.
![List of VM clients already added](https://raw.github.com/bongiojp/ganesha_jenkins/master/pics/addclient_p1.png "Registered clients")

### Setting up new jobs ###

Most jobs should be created as muliconfiguration jobs. This way you can choose which VMs the script will run on.

![Choosing a multiconfiguration job](https://raw.github.com/bongiojp/ganesha_jenkins/master/pics/creatingmatrixjob.png)

#### Job to distribute build script ####
First, it's useful to have a job that makes all clients/servers pull a single repository that contains useful scripts.

![Choosing git repository to test and poll](https://raw.github.com/bongiojp/ganesha_jenkins/master/pics/distributescriptsjob_sourcecodemanagement.png)

![configuration matrix](https://raw.github.com/bongiojp/ganesha_jenkins/master/pics/distributescriptsjob_configurationmatrix.png)

![](https://raw.github.com/bongiojp/ganesha_jenkins/master/pics/distributescriptsjob_buildtriggers.png)

![](https://raw.github.com/bongiojp/ganesha_jenkins/master/pics/distributescriptsjob_build.png)

#### Job to build Ganesha on servers ####

![](https://raw.github.com/bongiojp/ganesha_jenkins/master/pics/buildganeshajob_sourcecodemanagement.png)

![](https://raw.github.com/bongiojp/ganesha_jenkins/master/pics/buildganeshajob_buildtriggers.png)

![](https://raw.github.com/bongiojp/ganesha_jenkins/master/pics/buildganeshajob_build.png)

#### Job to run tests with servers ####

![](https://raw.github.com/bongiojp/ganesha_jenkins/master/pics/testjob_configmatrix.png)
![](https://raw.github.com/bongiojp/ganesha_jenkins/master/pics/testjob_build.png)
![](https://raw.github.com/bongiojp/ganesha_jenkins/master/pics/testjob_post-buildactions.png)