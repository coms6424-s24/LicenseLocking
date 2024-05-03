#! /bin/bash

git clone https://github.com/fentec-project/CiFEr.git CiFEr
sudo apt update
sudo apt install -y unzip g++ make clang-format python-is-python3
sudo apt install --no-install-recommends -qq -y build-essential libgmp-dev libsodium-dev libprotobuf-c-dev git python3 python3-pip vim pkg-config
sudo pip3 install cmake==3.18 --break-system-packages
cd CiFEr/external/amcl
sudo ./setup_amcl.sh
cd ../..
cp ../CiFEr_CMakeLists.txt ./CMakeLists.txt
mkdir build
cd build
cmake -DCMAKE_C_COMPILER=gcc ..
make
sudo make install
sudo ldconfig

