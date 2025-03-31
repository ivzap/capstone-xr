## Instructions
1. Install NDI sdk from https://ndi.video/for-developers/ndi-sdk/
2. Install opencv via apt or brew


Mac
```
brew install opencv
```
Linux
```
sudo apt install libopencv-dev
```

3. install make, pkg-config

Mac
```
brew install pkg-config make
```

Linux

```
apt install pkg-config
```

### .pc files
pkg-config is used within the makefile to help the gcc compile and link to libraries
properly. This means it requires a config file per library i.e opencv, ndi, etc.

#### example .pc file:

```
prefix=/Library/ndi_sdk
exec_prefix=${prefix}
libdir=${prefix}/lib/macOS
includedir=${prefix}/include

Name: NDI
Description: NewTek NDI SDK
Version: 5.5.0  # Replace with your NDI version
Libs: -L${libdir} -lndi
Cflags: -I${includedir}
```

#### Here's a explanation of each field in .pc files:

prefix=/Library/ndi_sdk

This is the base directory where your NDI SDK is installed. All other paths are relative to this.

exec_prefix=${prefix}

This is typically the same as prefix unless you're doing cross-compilation or have architecture-specific files in a different location. It's where executable files would be found.

libdir=${prefix}/lib/macOS

Specifies where the NDI library files (.dylib on macOS) are located. The path is constructed relative to prefix.

includedir=${prefix}/include

Specifies where the NDI header files (.h files) are located, again relative to prefix.

Name: NDI

The name you'll use with pkg-config commands (e.g., pkg-config --libs NDI).

Description: NewTek NDI SDK

A human-readable description of what this package is.

Version: 5.5.0

The version number of your NDI SDK. Important for version checking.

Libs: -L${libdir} -lndi

The linker flags that will be output:

-L specifies the library directory (expands to /Library/ndi_sdk/lib/macOS)

-lndi tells the linker to link with libndi.dylib

Cflags: -I${includedir}

The compiler flags that will be output:

-I specifies the include directory (expands to /Library/ndi_sdk/include)


#### Tell pkg-config where the .pc file is by adding the following to .zshrc or .bashrc. 

```
export PKG_CONFIG_PATH=/Library/ndi_sdk/config:$PKG_CONFIG_PATH
```


now you should be able to make after you source the rc file.
```
make all
```




