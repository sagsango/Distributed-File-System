# What is Understand?
The Understand tool from SciTools Inc. can be used to analyze, navigate, edit, etc. NFS-Ganesha code. Refer to https://scitools.com/ and https://scitools.com/support/ for more information. Like many powerful tools, you need to spend some time learning how to use the tool before you will realize how much you can do with the tool.
# How can Understand analyze code very accurately?
Understand's analysis of software can be very accurate because SciTools Inc. provides a buildspy application. Buildspy captures many compilation arguments. Compilation arguments can greatly influence the code. Accurately including the build options in the analysis increases Understand's code analysis accuracy.
# Creating the Understand project for NFS-Ganesha
First decide on the NFS-Ganesha build options that you want. For example; the current NFS-Ganesha "src/COMPILING_HOWTO.txt" file identifies a cmake command similar to the one below:

    mkdir mybuild; cd mybuild; cmake -DCMAKE_BUILD_TYPE=Maintainer \
        -DBUILD_CONFIG=everything /work/repos/nfs-ganesha/src

If you've installed Understand from SciTools Inc. into /opt/Understand-4.0, the commands below will create an Understand project:

    /opt/Understand-4.0/bin/linux64/und -db /work/mybuild/nfs-ganesha.udb create \
        -languages c++ ; echo $?
    /opt/Understand-4.0/bin/linux64/und -db /work/mybuild/nfs-ganesha.udb settings \
        -c++Compiler 'GNU GCC' -c++IncludePath /usr/include -c++AddFoundFilesToProject on \
        -c++AddFoundSystemFiles on -c++PromptMissingIncludes on \
        -c++CreateReferencesToMacrosDuringMacroExpansion on  -c++SaveDuplicateReferences on \
        -c++SaveMacroExpansionText on -c++Includes /usr/include/linux; echo $?
    /opt/Understand-4.0/bin/linux64/buildspy/buildspy -db /tmp/temp.udb -cc gcc -cxx g++ \
        -cmd 'cmake -DDEBUG_SYMS=ON -DCMAKE_BUILD_TYPE=Maintainer -DBUILD_CONFIG=everything \
         -DCMAKE_C_COMPILER=/opt/Understand-4.0/bin/linux64/buildspy/gccwrapper \
         /work/repos/nfs-ganesha/src' ; echo $?
    /opt/Understand-4.0/bin/linux64/buildspy/buildspy -db /work/mybuild/nfs-ganesha.udb \
         -cc /usr/bin/cc -cxx /usr/bin/g++ -cmd 'make' ; echo $?
    
Notes:

    1. Zeros (from "echo $?") should follow each successful command.
    2. The one use above of /tmp/temp.udb is not accidental. We don't care to save
       any operations during the cmake so that command used a throw away database
       to ignore the cmake steps.
    3. If some includes are not found during analysis, you may need to add several
       paths not automatically identified by buildspy. For example; /usr/include/c++/6,
       /usr/include/c++/6/tr1, /usr/include/c++/6/x86_64-redhat-linux, etc.
    4. Even after doing the above, Understand still might not find a couple (e.g.
       stubs, endian) headers. I generally don't worry about those.
    5. Only one program at a time can open the Understand project so don't leave it
       open in the GUI while trying to use buildspy or the und command line program.

Once I've created a satisfactory Understand projects, I wipe out my build directory and then re-run cmake and the make to eliminate the buildspy hooks. Appropriate commands (executed within the build directory) for the example above are:

    rm -rf ../mybuild/[a-z]* ; echo $?
    cmake -DDEBUG_SYMS=ON -DCMAKE_BUILD_TYPE=Maintainer \
        -DBUILD_CONFIG=everything /work/repos/nfs-ganesha/src' ; echo $?
    make ; echo $?

# You can probably receive a free Understand license
If you are working on NFS-Ganesha (or other open source project), you can probable receive a free Understand license. Search for SciTools Inc Understand non-commercial license for additional information.