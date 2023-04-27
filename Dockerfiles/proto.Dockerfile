# sudo docker build . --tag serializer/proto:1.0 -f Dockerfiles/proto.Dockerfile
# sudo docker run --name serializer-proto -d -p 5001:5001 serializer/proto:1.0

FROM serializer/common:1.0

COPY serializers/proto_serializer.hpp /serializers/proto_serializer.hpp
RUN ln -fs /serializers/proto_serializer.hpp /serializers/serializer.hpp
COPY test_data.proto test_data.proto
ENV LD_LIBRARY_PATH="/usr/local/lib:$LD_LIBRARY_PATH"

RUN pacman -S --noconfirm protobuf
RUN cd serializers && protoc --proto_path=.. --cpp_out=. test_data.proto

RUN /usr/bin/g++ -lprotobuf -lrestbed -I/restbed/distribution/include serializers/test_data.pb.cc serializers/test_struct.cpp /rest_server.cpp -o /rest_ser

CMD ["sh", "-c", "./rest_ser $PORT"] 
