#!/bin/sh

make clean
chmod +x configure
./configure CFLAGS=-m32 CXXFLAGS=-m32 LDFLAGS=-m32 --build=x86_64-pc-linux-gnu --host=i686-pc-linux-gnu --enable-shared=no --enable-static=yes --enable-http --enable-ftp --disable-gopher --enable-file --disable-ldap --disable-dict --disable-telnet --disable-manual --enable-libgcc --enable-ipv6 --disable-ares --with-ssl --without-zlib --without-libidn -without-brotli 
make