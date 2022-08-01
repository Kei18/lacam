FROM ubuntu:18.04
RUN apt-get -y update
RUN apt-get -y install wget libssl-dev build-essential

# update g++ version
RUN apt-get -y install software-properties-common
RUN add-apt-repository ppa:ubuntu-toolchain-r/test
RUN apt-get -y install g++-8
RUN update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-8 80

# install cmake
WORKDIR /tmp
RUN wget https://github.com/Kitware/CMake/archive/refs/tags/v3.20.2.tar.gz -O cmake.tar.gz
RUN tar zxvf cmake.tar.gz
RUN cd CMake-3.20.2 && ./bootstrap && make -j4 && make install && cd ..
RUN hash -r
RUN rm -rf CMake-3.20.2

WORKDIR /workspace