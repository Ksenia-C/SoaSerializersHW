# sudo docker build . --tag serializer/json:1.0 -f Dockerfiles/json.Dockerfile
# sudo docker run --name serializer-json -d -p 5004:5004 serializer/json:1.0

FROM serializer/boost:1.0

COPY serializers/json_serializer.hpp /serializers/json_serializer.hpp
RUN ln -fs /serializers/json_serializer.hpp /serializers/serializer.hpp
ENV LD_LIBRARY_PATH="/usr/local/lib:$LD_LIBRARY_PATH"

RUN cd boost_1_82_0 && \
    ./b2 install --with-json --with-container

RUN /usr/bin/g++ -lboost_json -lrestbed -I/restbed/distribution/include serializers/test_struct.cpp /rest_server.cpp -o /rest_ser 

CMD ["sh", "-c", "./rest_ser $PORT"] 
