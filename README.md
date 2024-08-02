# coroutine_study

## How to build (OS version: Ubuntu 22.04+)
1. install build-essential, python3-dev audotools-dev libicu-dev libbz2-dev libssl-dev
```bash
sudo apt update
sudo apt install build-essential python3-dev autotools-dev libicu-dev libbz2-dev libssl-dev cmake -y
```

2. install boost 1.85
* download
```bash
wget https://boostorg.jfrog.io/artifactory/main/release/1.85.0/source/boost_1_85_0.tar.gz
```
* extract
```bash
tar -xvf boost_1_85_0.tar.gz
```
* build and install
```bash
cd boost_1_85_0
./bootstrap.sh
sudo ./b2 install
```
* verify installation
```bash
b2 --version
```

3. install VSCode
4. install "C/C++ Extension Pack", "CMake Tools" extensions for VSCode
5. how to build
    * VSCode -> CTRL + SHIFT + P -> CMake: Configure
    * VSCode -> CTRL + SHIFT + P -> CMake: Build Target -> all