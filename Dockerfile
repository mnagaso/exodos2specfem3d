#FROM debian:jessie-slim
FROM ubuntu:18.04

RUN apt-get update
RUN apt-get clean
RUN apt-get install -y build-essential checkinstall gfortran \
    libfreetype6-dev libpng-dev python-dev python3-dev vim git \
    libsm6 libfontconfig1 libxrender1 libxtst6 libxt-dev \
    cmake m4

#libglu1-mesa mesa-utils glxgears 
#RUN apt-get install zlib1g-devopenmpi-bin openmpi-doc libopenmpi-dev -y

# cmake
#RUN git clone https://gitlab.kitware.com/cmake/cmake.git /usr/local/src/cmake
#WORKDIR /usr/local/src/cmake
#RUN ./bootstrap --prefix=/usr/local && make -j5 && make install

# seacas
RUN git clone https://github.com/gsjaardema/seacas.git /usr/local/src/seacas
#WORKDIR /usr/local/src/seacas
#RUN mkdir /usr/local/src/seacas/TPL/hdf5 && mkdir /usr/local/src/seacas/TPL/netcdf

# install hdf5
COPY hdf5-1.10.5.tar.gz /usr/local/src/seacas/TPL/hdf5/ 
WORKDIR /usr/local/src/seacas/TPL/hdf5/
RUN tar -zxvf hdf5-1.10.5.tar.gz \
    && cd hdf5-1.10.5 \
    && ./configure --prefix=/usr/local/src/seacas/ --enable-build-mode=production --enable-static-exec \
    && make && make install

# install netcdf
COPY netcdf-c-4.6.3_mod.tar.gz /usr/local/src/seacas/TPL/netcdf/
WORKDIR /usr/local/src/seacas/TPL/netcdf/
RUN tar -zxvf netcdf-c-4.6.3_mod.tar.gz \
    && cd netcdf-c-4.6.3_mod \
    && ./configure --enable-netcdf-4 --enable-shared --disable-fsync --prefix=/usr/local/src/seacas/ --disable-dap --disable-cdmremote \
         CFLAGS="-I/usr/local/src/seacas/include" \
        LDFLAGS="-L/usr/local/src/seacas/lib" \
    && make && make install

WORKDIR /usr/local/src/seacas
RUN mkdir build && cd build && ../cmake-config && make && make install

ADD . /mesh
WORKDIR /mesh/exodos2specfem_IOSS
RUN make && ln -s /mesh/exodos2specfem_IOSS/bin/exodos2specfem_IOSS /usr/bin/
