Build instructions
------------------

## Dependencies

Environment:

1. g++ 4.8+, MinGW-w64, MinGW or Microsoft Visual Studio 15+
2. CMake

Libraries:

1. `boost::locale`
2. `boost::filesystem`
3. `libpng` 1.4+
4. `libjpeg`
5. `zlib` (comes with `libpng`)
6. `iconv` (comes with POSIX)
7. `openssl`



## Compiling for GNU/Linux or Cygwin with g++ or MinGW

1. Download required dependencies (the ones with `-dev` or `-devel` suffix). On
   Cygwin most of them are available on [Cygwin Ports](http://cygwinports.org/).

2. Run following:

        mkdir build
        cd build
        cmake -DCMAKE_BUILD_TYPE=Release .. # for debug build, change to Debug
        make -j8

3. The executables should appear in `build/` directory.



## Cross compiling for Windows with MinGW-w64

In this case, the project requires MinGW-w64. There are several ways to install
it, but I believe using [`mxe`](http://mxe.cc/) is the most pleasant way.

1. Install `mxe`:

        git clone https://github.com/mxe/mxe.git

2. Build `arc_unpacker`'s dependencies using `mxe`:

        cd mxe
        make libiconv zlib libpng jpeg boost openssl -j8 JOBS=8

3. In the `arc_unpacker`'s directory run following:

        mkdir build-mxe
        cd build-mxe
        ~/path/to/mxe/usr/bin/i686-w64-mingw32.static-cmake
        make -j8

4. The executables should appear in `build-mxe/` directory.



## Compiling for Windows with Microsoft Visual Studio 15+

1. Install [CMake](https://cmake.org/download/).

2. Download and install or compile dependencies:

    - [`boost`](http://sourceforge.net/projects/boost/files/boost-binaries/)
    - [`zlib`](http://www.zlib.net/)
    - [`libpng`](http://www.libpng.org/pub/png/libpng.html)
    - [`libjpeg`](http://sourceforge.net/projects/libjpeg-turbo/)
    - [`openssl`](https://slproweb.com/products/Win32OpenSSL.html)
    - [`iconv`](https://github.com/win-iconv/win-iconv)

    Depending on your environment, you might need to patch `iconv()` signature
    in `iconv.h` to use `char**` rather than `const char**`. This is to match
    glibc, which is arc_unpacker's compiler of choice.

3. In the `arc_unpacker`'s directory run following:

        mkdir build-vs
        cd build-vs
        cmake --verbose \
            -G "Visual Studio 14" \
            -DBOOST_ROOT="C:/pkg/boost_1_60_0" \
            -DBOOST_LIBRARYDIR="C:/pkg/boost_1_60_0/bin.v2/libs" \
            -DZLIB_INCLUDE_DIR="C:/pkg/zlib-1.2.8;C:/pkg/builds/zlib" \
            -DZLIB_LIBRARY="C:/pkg/builds/zlib/Debug/zlibd.lib" \
            -DPNG_PNG_INCLUDE_DIR="C:/pkg/libpng-1.6.20" \
            -DPNG_LIBRARY="C:/pkg/builds/png/Debug/libpng16_staticd.lib" \
            -DJPEG_LIBRARY="C:/pkg/libjpeg-turbo/lib/jpeg.lib" \
            -DJPEG_INCLUDE_DIR="C:/pkg/libjpeg-turbo/include" \
            -DOPENSSL_ROOT_DIR="C:/pkg/OpenSSL-Win32" \
            -DOPENSSL_INCLUDE_DIR="C:/pkg/OpenSSL-Win32/include" \
            -DICONV_LIBRARY="C:/pkg/win-iconv/build/Debug/iconv.lib" \
            -DICONV_INCLUDE_DIR="C:/pkg/win-iconv" \
            ..

    The paths in `-D` switches should refer to locations where you installed
    the dependencies.

4. The `.vcxproj` files should appear in `build-vs/` directory. After you
   compile the project with Visual Studio, the executables should appear in
   `build-vs/` directory.
