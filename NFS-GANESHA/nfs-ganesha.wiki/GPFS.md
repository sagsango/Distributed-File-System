## Planning and Installing

NFS Ganesha is part of Spectrum Scale (formerly GPFS) Protocol Stack. It can be [installed using the `spectrumscale` deployment toolkit](http://www-01.ibm.com/support/knowledgecenter/STXKQY_4.2.0/com.ibm.spectrum.scale.v4r2.adm.doc/bl1adm_spectrumscale.htm?lang=en), the [GUI](http://www-01.ibm.com/support/knowledgecenter/STXKQY_4.2.0/com.ibm.spectrum.scale.v4r2.ins.doc/bl1ins_installingprotocolsgui.htm?lang=en) or manually. Advantages of using the deployment toolkit are explained [here](http://www-01.ibm.com/support/knowledgecenter/STXKQY_4.2.0/com.ibm.spectrum.scale.v4r2.ins.doc/bl1ins_installingprotocols.htm?lang=en).

Before deploying NFS Ganesha we need to make a number of decisions regarding various options at our disposal. An overview of considerations regarding NFS deployment on GPFS can be found [here](http://www-01.ibm.com/support/knowledgecenter/STXKQY_4.2.0/com.ibm.spectrum.scale.v4r2.ins.doc/bl1ins_planningnfs.htm?lang=en).

## Configuring

* [Configure NFS](https://www-01.ibm.com/support/knowledgecenter/STXKQY_4.2.0/com.ibm.spectrum.scale.v4r2.adm.doc/bl1adm_configces.htm) as part of CES (Cluster Export Services) 
* [Manage CES](https://www-01.ibm.com/support/knowledgecenter/STXKQY_4.2.0/com.ibm.spectrum.scale.v4r2.adm.doc/bl1adm_managingprotocolservices.htm).  Cluster Export Service (CES) uses NFS Ganesha for NFSv3 and NFSv4 file sharing.
* [Manage NFS exports](https://www-01.ibm.com/support/knowledgecenter/STXKQY_4.2.0/com.ibm.spectrum.scale.v4r2.adm.doc/bl1adm_ManagingNFSservices.htm)
* Understand [ACL administration](https://www-01.ibm.com/support/knowledgecenter/STXKQY_4.2.0/com.ibm.spectrum.scale.v4r2.adm.doc/bl1adm_admnfsaclg.htm) related to NFSv4 and GPFS
* Read about [various topics](https://www-01.ibm.com/support/knowledgecenter/STXKQY_4.2.0/com.ibm.spectrum.scale.v4r2.adm.doc/bl1adm_nfsinter.htm) concerning NFS Ganesha on GPFS and details on [ID mapping](http://www-01.ibm.com/support/knowledgecenter/STXKQY_4.2.0/com.ibm.spectrum.scale.v4r2.adm.doc/bl1adm_configfileauthentication.htm?lang=en) and [authentication](http://www-01.ibm.com/support/knowledgecenter/STXKQY_4.2.0/com.ibm.spectrum.scale.v4r2.ins.doc/bl1ins_authconcept.htm)

## Monitoring

* The integrated performance monitoring tool (details for v4.2.0 can be found [here](https://www-01.ibm.com/support/knowledgecenter/STXKQY_4.2.0/com.ibm.spectrum.scale.v4r2.ins.doc/bl1ins_manualinstallationofPMTl.htm)) gathers performance data and stores them in a database
* Spectrum Scale GUI provides performance and event monitoring for NFS Ganesha. Performance statistics can be [aggregated](http://www-01.ibm.com/support/knowledgecenter/STXKQY_4.2.0/com.ibm.spectrum.scale.v4r2.adv.doc/bl1adv_perfmoningui.htm?lang=en) at the cluster, server and export (share) level

## Community and Support

* [Sizing guidance](https://www.ibm.com/developerworks/community/wikis/home?lang=en#!/wiki/General%20Parallel%20File%20System%20%28GPFS%29/page/Sizing%20Guidance%20for%20Protocol%20Node) for NFS Ganesha as Spectrum Scale protocol-serving node
* [Spectrum Scale FAQ](http://www-01.ibm.com/support/knowledgecenter/STXKQY/gpfsclustersfaq.html)
* [Spectrum Scale Community Forum](https://www.ibm.com/developerworks/community/forums/html/forum?id=11111111-0000-0000-0000-000000000479&ps=25)
