GCCDIR = "/home/user/git/CFI-Project_group-B/gcc-install/bin"
CC = $(GCCDIR)/gcc
CXX = $(GCCDIR)/g++

# Flags for the C++ compiler: enable C++11 and all the warnings, -fno-rtti is required for GCC plugins
CXXFLAGS = -std=c++11 -Wall -fno-rtti -fPIC
# Workaround for an issue of -std=c++11 and the current GCC headers
CXXFLAGS += -Wno-literal-suffix

# Flags for the C compiler: enable C11 and all the warnings, -fno-rtti is required for GCC plugins
CFLAGS =  -std=c11 -Wall -fno-rtti -fPIC
# Workaround for an issue of -std=c++11 and the current GCC headers
CFLAGS += -Wno-literal-suffix


# Determine the plugin-dir and add it to the flags
PLUGINDIR=$(shell $(CXX) -print-file-name=plugin)
CXXFLAGS += -I$(PLUGINDIR)/include
CFLAGS += -I$(PLUGINDIR)/include

STATIC_CFLAGS = -std=c11 -Wall
STATIC_ARFLAGS = rcs

# Kernel module
obj-m = mydriver.o
KVERSION = $(shell uname -r)


all: clean libcfi_static.a main 
	$(CC) -o test main.o -L. -l:libcfi_static.a
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) mydriver.ko

main: plugin_cfi_pa_rtl.so
	$(CC) -g -fplugin=./plugin_cfi_pa_rtl.so -c -o main.o -fno-stack-protector main.c

plugin_cfi_pa_rtl.so: plugin_cfi_pa_rtl.o
		$(CXX) $(LDFLAGS) -shared -o $@ $<

plugin_cfi_pa_rtl.o : plugin_cfi_pa_rtl.cc
		$(CXX) $(CXXFLAGS) -fPIC -c -o $@ $<

libcfi_static.a: cfi_static/cfi_static.o
	$(AR) $(STATIC_ARFLAGS) $@ $<

cfi_static/cfi_static.o: cfi_static/cfi_static.c
	$(CC) $(STATIC_CFLAGS) -o $@ -c $<

mydriver.o: mydriver.c
	$(CC) -c -o mydriver.o mydriver.c

clean:
	rm -f plugin_cfi_pa_rtl.o plugin_cfi_pa_rtl.so main.o cfi_static/cfi_static.o libcfi_static.a test mydriver.ko mydriver.mod mydriver.mod.c mydriver.mod.o mydriver.o
		
#plugin_cfi_pa:
#	$(CXX) $(CXXFLAGS) -fPIC -c -o plugin_cfi_pa_rtl.o  plugin_cfi_pa_rtl.cc;
#	ld -pie -fPIC -shared -o plugin_cfi_pa_rtl.so plugin_cfi_pa_rtl.o;
#	$(CC) -g -fplugin=./plugin_cfi_pa_rtl.so main.c -o main.o -fno-stack-protector;

