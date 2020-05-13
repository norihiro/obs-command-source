#! /bin/bash

obs_dir=../obs-studio
mkdir ${obs_dir}/plugins/obs-command-source
git archive --format tar HEAD . | (cd ${obs_dir}/plugins/obs-command-source && tar x)
echo 'add_subdirectory(obs-command-source)' >> ${obs_dir}/plugins/CMakeLists.txt

cd ${obs_dir}/build
make -j4

echo '[command-source] Checking plugin file..'
test -f plugins/obs-command-source/command-source.so

make install DESTDIR=/tmp/obs-command-source-root
echo '[command-source] Checking plugin file in installed directory..'
test -f "$(find /tmp/obs-command-source-root -name 'command-source.so' | head -n 1)"
