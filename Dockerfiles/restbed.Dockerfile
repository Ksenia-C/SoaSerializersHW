# sudo docker build . --tag serializer/common:1.0 -f Dockerfiles/restbed.Dockerfile

FROM archlinux:latest

ENV DEBIAN_FRONTEND=noninteractive

COPY rest_server.cpp rest_server.cpp
COPY serializers/interface.hpp serializers/interface.hpp
COPY serializers/test_struct.cpp serializers/test_struct.cpp
COPY serializers/test_struct.hpp serializers/test_struct.hpp

RUN pacman -Sy --noconfirm gcc wget git ca-certificates cmake base-devel

RUN git clone https://github.com/Corvusoft/restbed.git && \
    git clone https://github.com/corvusoft/asio-dependency restbed/dependency/asio && \
    mkdir restbed/build && \
    cd restbed/build && \
    cmake -DBUILD_SSL=NO -DBUILD_TESTS=NO .. && \
    make install && \
    cp ../distribution/library/librestbed.so* /usr/local/lib/
