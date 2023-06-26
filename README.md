# CFI-Project_group-B

## OS
This project was developed on *Ubuntu 22.04.2 LTS* with a *5.19.0-45-generic* kernel version 

## INSTALLATION AND SETUP
1 - Download the repository
```
$ git clone git@github.com:Internaut401/CFI-Project_group-B.git
```
2 - enter in the repo folder
 ```
$ cd CFI-Project_group-B/
```
3 - export the environment variable (!!! replace /home/user/git/ with your path to the folder !!!)
```
$ export BASEDIR=/home/user/git/CFI-Project_group-B
```
4 - download gcc-12.2.0 and extact it in the repo root folder
 ```
 $ wget https://ftp.gnu.org/gnu/gcc/gcc-12.2.0/gcc-12.2.0.tar.gz
 $ tar -xf gcc-12.2.0.tar.gz
```
5 - install pre-requisites
```
# enter in the extracted gcc folder
$ cd gcc-12.2.0/

# And now download the prerequisites
$ ./contrib/download_prerequisites

$sudo apt-get install gcc-multilib
```

6 - build gcc and install
```
# We are in gcc-12.2.0, go up one level
$ cd ..

# Now create the build directory, gcc-build is a sensible name
$ mkdir gcc-build
$ mkdir gcc-install

# Enter the build directory
$ cd gcc-build

# Define an installation path, it must be an absolute path!
$ export INSTALLDIR=$BASEDIR/gcc-install

# Configure GCC
$ ../gcc-12.2.0/configure --prefix=$INSTALLDIR --enable-languages=c,c++

# Build 'getconf _NPROCESSORS_ONLN' will return the number of threads
# we can use, in order to build GCC in parallel
$ make -j$(getconf _NPROCESSORS_ONLN)

# install
$ make install


# kernel module
# sometimes kernel module compilation could result in error. One common problem is:<br>
# https://askubuntu.com/questions/1348250/skipping-btf-generation-xxx-due-to-unavailability-of-vmlinux-on-ubuntu-21-04

# so try to:
$ apt install dwarves
$ cp /sys/kernel/btf/vmlinux /usr/lib/modules/`uname -r`/build/
```

### CHECK INSTALLATION
```
# Create a convenience variable for the path of GCC
$ export GCCDIR=$INSTALLDIR/bin
$ $GCCDIR/g++ --version
```

## NOTE
on linux exported environment variable are not persistent. To make them persistent you can add them to your bash shell configuration file (usually .bashrc or .zshrc file present in your home directory). Add the following line (replace /home/user/git/ with YOUR PATH): 
```
export BASEDIR=/home/user/git/CFI-Project_group-B
export INSTALLDIR=$BASEDIR/gcc-install
export GCCDIR=$INSTALLDIR/bin
```

### LOAD KERNEL MODULE
⚠️ TEMPORARY SOLUTION ⚠️
After make compilation, run:
```shell
sudo insmod mydriver.ko
sudo chmod 777 /dev/mydriver
```

If you want to modify the driver c code, you  need to reboot your machine before compile again.<br>
That's because is not implemented a clean procedure to completely unload the kernel module and its structures / files.
