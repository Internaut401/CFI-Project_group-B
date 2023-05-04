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


all: clean plugin_cfi_pa_rtl.so
	$(CC) -g -fplugin=./plugin_cfi_pa_rtl.so main.c -o main.o -fno-stack-protector;

plugin_cfi_pa_rtl.so: plugin_cfi_pa_rtl.o
		$(CXX) $(LDFLAGS) -shared -o $@ $<

plugin_cfi_pa_rtl.o : plugin_cfi_pa_rtl.cc
		$(CXX) $(CXXFLAGS) -fPIC -c -o $@ $<

clean:
	rm -f plugin_cfi_pa_rtl.o plugin_cfi_pa_rtl.so main.o
		
#plugin_cfi_pa:
#	$(CXX) $(CXXFLAGS) -fPIC -c -o plugin_cfi_pa_rtl.o  plugin_cfi_pa_rtl.cc;
#	ld -pie -fPIC -shared -o plugin_cfi_pa_rtl.so plugin_cfi_pa_rtl.o;
#	$(CC) -g -fplugin=./plugin_cfi_pa_rtl.so main.c -o main.o -fno-stack-protector;

