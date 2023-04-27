
# sudo docker build . --tag serializer/text:1.0 -f Dockerfiles/text.Dockerfile     
# sudo docker run --name serializer-text -d -p 5002:5002 serializer/text:1.0

FROM serializer/boost:1.0

COPY serializers/text_serializer.hpp /serializers/text_serializer.hpp
RUN ln -fs /serializers/text_serializer.hpp /serializers/serializer.hpp
ENV LD_LIBRARY_PATH="/usr/local/lib:$LD_LIBRARY_PATH"

RUN cd boost_1_82_0 && \
    ./b2 install --with-serialization --with-container

RUN /usr/bin/g++ -lboost_serialization -lrestbed -I/restbed/distribution/include serializers/test_struct.cpp /rest_server.cpp -o /rest_ser 

CMD ["sh", "-c", "./rest_ser $PORT"] 
