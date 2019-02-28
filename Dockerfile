FROM debian:jessie-slim

RUN apt-get update
RUN apt-get clean
RUN apt-get install build-essential checkinstall gfortran -y
RUN apt-get install libfreetype6-dev libpng-dev python-dev python3-dev vim git -y
RUN apt-get install -y libsm6 libfontconfig1 libxrender1 libxtst6 libxt-dev
#libglu1-mesa mesa-utils glxgears 
#RUN apt-get install zlib1g-devopenmpi-bin openmpi-doc libopenmpi-dev -y

# cmake
RUN git clone https://gitlab.kitware.com/cmake/cmake.git /usr/local/src/cmake
WORKDIR /usr/local/src/cmake
RUN ./bootstrap --prefix=/usr/local && make -j5 && make install




# seacas
RUN git clone https://github.com/gsjaardema/seacas.git /usr/local/src/seacas
WORKDIR /usr/local/src/seacas 
RUN mkdir build && cd build && ../cmake-config && make && make install

WORKDIR /mesh
