# sudo docker build . --tag serializer/boost:1.0 -f Dockerfiles/boost.Dockerfile

FROM serializer/common:1.0

RUN wget https://boostorg.jfrog.io/artifactory/main/release/1.82.0/source/boost_1_82_0.tar.bz2 
RUN tar --bzip2 -xf boost_1_82_0.tar.bz2

RUN cd boost_1_82_0 && \
    ./bootstrap.sh --prefix=/usr/local
