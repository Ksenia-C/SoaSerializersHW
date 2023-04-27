# sudo docker build . --tag serializer/avro:1.0 -f Dockerfiles/avro.Dockerfile
# sudo docker run --name serializer-avro -d -p 5006:5006 serializer/avro:1.0

FROM serializer/common:1.0

COPY serializers/avro_serializer.hpp  /serializers/avro_serializer.hpp 
RUN ln -fs /serializers/avro_serializer.hpp /serializers/serializer.hpp
COPY test_data.json test_data.json
ENV LD_LIBRARY_PATH="/usr/local/lib:$LD_LIBRARY_PATH"

RUN pacman -Sy --noconfirm sudo

RUN useradd builduser -m && \
    passwd -d builduser && \
    printf 'builduser ALL=(ALL) ALL\n' | tee -a /etc/sudoers

RUN cd /home/builduser && \
    git clone https://aur.archlinux.org/avro-cpp.git && chown builduser -R avro-cpp && \
    sudo -u builduser bash -c 'cd ~/avro-cpp && makepkg -si --noconfirm --nocheck' 

RUN cd serializers && avrogencpp -i ../test_data.json -o test_data.hh -n c

RUN /usr/bin/g++ -lavrocpp -lrestbed -I/restbed/distribution/include serializers/test_struct.cpp /rest_server.cpp -o /rest_ser

CMD ["sh", "-c", "./rest_ser $PORT"]
