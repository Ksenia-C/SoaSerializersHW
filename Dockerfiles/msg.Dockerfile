# sudo docker build . --tag serializer/msg:1.0 -f Dockerfiles/msg.Dockerfile
# sudo docker run --name serializer-msg -d -p 5008:5008 serializer/msg:1.0

FROM serializer/common:1.0

COPY serializers/msg_serializer.hpp /serializers/msg_serializer.hpp
RUN ln -fs /serializers/msg_serializer.hpp /serializers/serializer.hpp
ENV LD_LIBRARY_PATH="/usr/local/lib:$LD_LIBRARY_PATH"

RUN pacman -Sy --noconfirm msgpack-cxx 

RUN /usr/bin/g++ -lrestbed -I/restbed/distribution/include serializers/test_struct.cpp /rest_server.cpp -o /rest_ser

CMD ["sh", "-c", "./rest_ser $PORT"] 

