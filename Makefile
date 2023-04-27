
ROOT_PATH = $(shell pwd)

build_text:
	ln -sf  $(shell pwd)/serializers/text_serializer.hpp $(shell pwd)/serializers/serializer.hpp && \
	/usr/bin/g++ -fdiagnostics-color=always -g -lboost_serialization -lrestbed -I${ROOT_PATH}/restbed/distribution/include -fsanitize=undefined,address serializers/test_struct.cpp  rest_server.cpp -o rest_ser

build_xml:
	ln -sf  $(shell pwd)/serializers/xml_serializer.hpp $(shell pwd)/serializers/serializer.hpp && \
	/usr/bin/g++ -fdiagnostics-color=always -g -lboost_serialization -lrestbed -I${ROOT_PATH}/restbed/distribution/include -fsanitize=undefined,address serializers/test_struct.cpp  rest_server.cpp -o rest_ser

build_json:
	ln -sf  $(shell pwd)/serializers/json_serializer.hpp $(shell pwd)/serializers/serializer.hpp && \
	/usr/bin/g++ -fdiagnostics-color=always -g -lboost_json -lrestbed -I${ROOT_PATH}/restbed/distribution/include -fsanitize=undefined,address serializers/test_struct.cpp  rest_server.cpp -o rest_ser

build_proto:
	ln -sf  $(shell pwd)/serializers/proto_serializer.hpp $(shell pwd)/serializers/serializer.hpp && \
	protoc --proto_path=. --cpp_out=./serializers test_data.proto && \
	/usr/bin/g++ -fdiagnostics-color=always -g -lprotobuf -lrestbed -I${ROOT_PATH}/restbed/distribution/include -fsanitize=undefined,address serializers/test_data.pb.cc serializers/test_struct.cpp  rest_server.cpp -o rest_ser

build_avro:
	ln -sf  $(shell pwd)/serializers/avro_serializer.hpp $(shell pwd)/serializers/serializer.hpp && \
	avrogencpp -i test_data.json -o serializers/test_data.hh -n c && \
	/usr/bin/g++ -fdiagnostics-color=always -g -lavrocpp -lrestbed -I${ROOT_PATH}/restbed/distribution/include -fsanitize=undefined,address serializers/test_struct.cpp  rest_server.cpp -o rest_ser

build_yaml:
	ln -sf  $(shell pwd)/serializers/yaml_serializer.hpp $(shell pwd)/serializers/serializer.hpp && \
	/usr/bin/g++ -fdiagnostics-color=always -g -lyaml-cpp -lrestbed -I${ROOT_PATH}/restbed/distribution/include -fsanitize=undefined,address serializers/test_struct.cpp  rest_server.cpp -o rest_ser

build_msg:
	ln -sf  $(shell pwd)/serializers/msg_serializer.hpp $(shell pwd)/serializers/serializer.hpp && \
	/usr/bin/g++ -fdiagnostics-color=always -g -lrestbed -I${ROOT_PATH}/restbed/distribution/include -fsanitize=undefined,address serializers/test_struct.cpp  rest_server.cpp -o rest_ser


run:
	./rest_ser 5000

