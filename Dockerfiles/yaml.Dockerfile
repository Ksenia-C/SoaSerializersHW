# sudo docker build . --tag serializer/yaml:1.0 -f Dockerfiles/yaml.Dockerfile
# sudo docker run --name serializer-yaml -d -p 5007:5007 serializer/yaml:1.0

FROM serializer/common:1.0

COPY serializers/yaml_serializer.hpp /serializers/yaml_serializer.hpp
RUN ln -fs /serializers/yaml_serializer.hpp /serializers/serializer.hpp
ENV LD_LIBRARY_PATH="/usr/local/lib:$LD_LIBRARY_PATH"

RUN pacman -S --noconfirm yaml-cpp

RUN /usr/bin/g++ -lyaml-cpp -lrestbed -I/restbed/distribution/include serializers/test_struct.cpp /rest_server.cpp -o /rest_ser

CMD ["sh", "-c", "./rest_ser $PORT"] 
