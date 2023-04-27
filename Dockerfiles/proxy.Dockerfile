# sudo docker build . --tag serializer/proxy:1.1 -f Dockerfiles/proxy.Dockerfile     
# sudo docker run --name serializer-proxy -d -p 6000:6000 serializer/proxy:1.0

FROM serializer/common:1.0

COPY proxy_server.cpp proxy_server.cpp
COPY docker-compose.yaml docker-compose.yaml

ENV LD_LIBRARY_PATH="/usr/local/lib:$LD_LIBRARY_PATH"
RUN pacman -S --noconfirm yaml-cpp

RUN /usr/bin/g++ -lyaml-cpp -lrestbed -g -I/restbed/distribution/include /proxy_server.cpp -o /proxy_ser 

CMD ["sh", "-c", "./proxy_ser $PORT"] 
