#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <type_traits>
#include <iostream>
#include <string_view>
#include <fstream>

#include <msgpack.hpp>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/string.hpp>

#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>

#include <boost/json/src.hpp>
using namespace boost::json;

#include "test_data.pb.h"
#include <stdio.h>
#include <fcntl.h>
#include <assert.h>

#include "test_data.hh"
#include "avro/Encoder.hh"
#include "avro/Decoder.hh"
#include "avro/Stream.hh"
#include <memory>
#include "yaml-cpp/yaml.h"

struct TestData
{
    std::string small_str = "hello world";

    std::vector<int> small_array = {1, 2, 3, 4, 5};

    std::map<std::string, int> str_map = {{"a", 0}, {"b", 1}, {"c", 99}, {"d", 993}};

    uint64_t u64 = -1;

    double f64 = 16.0;
    MSGPACK_DEFINE(small_str, small_array, str_map, u64, f64);

    template <class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar &BOOST_SERIALIZATION_NVP(small_str);
        ar &BOOST_SERIALIZATION_NVP(small_array);
        ar &BOOST_SERIALIZATION_NVP(str_map);
        ar &BOOST_SERIALIZATION_NVP(u64);
        ar &BOOST_SERIALIZATION_NVP(f64);
    }
};

void tag_invoke(value_from_tag, value &jv, TestData const &d)
{

    jv = {
        {"small_str", d.small_str},
        {"small_array", value_from(d.small_array)},
        {"str_map", value_from(d.str_map)},
        {"u64", d.u64},
        {"f64", d.f64}};
}

template <class T>
void extract(object const &obj, T &t, string_view key)
{
    t = value_to<T>(obj.at(key));
}

TestData tag_invoke(value_to_tag<TestData>, value const &jv)
{
    TestData c;
    object const &obj = jv.as_object();
    extract(obj, c.small_str, "small_str");
    // extract(obj, c.small_array, "small_array");
    // extract(obj, c.str_map, "str_map");
    extract(obj, c.u64, "u64");
    extract(obj, c.f64, "f64");
    return c;
}

#include <time.h>

double one_test_time(struct timespec &begin, struct timespec &end)
{
    return (end.tv_sec - begin.tv_sec) * 1e3 + (end.tv_nsec - begin.tv_nsec) * 1e-6;
}

void text_ser(TestData &test)
{
    {
        std::ofstream ofs("simple_filename");
        boost::archive::text_oarchive oa(ofs);
        oa << test;
    }

    TestData other;
    {
        std::ifstream ifs("simple_filename");
        boost::archive::text_iarchive ia(ifs);
        ia >> other;
    }
    assert(other.u64 == test.u64);
}

void xml_ser(TestData &test)
{
    {
        std::ofstream ofs("xml_filename");
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(test);
    }

    TestData other;
    {
        std::ifstream ifs("xml_filename");
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(other);
    }
    assert(other.u64 == test.u64);
}

void proto_ser(TestData &test)
{
    // protoc --proto_path=. --cpp_out=. test_data.proto
    {
        TEST_DATA::Test_Data data;
        std::ofstream ofs("protobuf.txt", std::ios::out | std::ios::trunc | std::ios::binary);

        data.set_small_str(test.small_str);
        data.mutable_small_array()->Add(test.small_array.begin(), test.small_array.end());
        data.set_f64(test.f64);
        data.set_u64(test.u64);
        data.mutable_str_map()->insert(test.str_map.begin(), test.str_map.end());

        assert(data.SerializeToOstream(&ofs));
    }

    TEST_DATA::Test_Data another;
    {
        std::ifstream ifs("protobuf.txt", std::ios::in | std::ios::binary);
        assert(another.ParseFromIstream(&ifs));
    }
    assert(test.u64 == another.u64());
}

void json_ser(TestData &test)
{

    {
        std::ofstream ofs("json_filename");
        ofs << serialize(value_from(test));
        // ofs.flush();
    }

    TestData other;
    {
        std::ifstream ifs("json_filename");
        value jv = parse(ifs);
        TestData another(value_to<TestData>(jv));
    }
    assert(test.u64 == other.u64);
}

std::pair<double, double> avro_ser(TestData &test, int clock_mod = CLOCK_REALTIME)
{
    std::pair<double, double> result;
    struct timespec begin, end;

    {
        std::unique_ptr<avro::OutputStream> out = avro::fileOutputStream("avro_file");
        clock_gettime(clock_mod, &begin);

        avro::EncoderPtr e = avro::binaryEncoder();
        e->init(*out);
        c::TestData data;
        data.f64 = test.f64;
        data.u64 = test.u64;
        data.small_array = test.small_array;
        data.small_str = test.small_str;
        data.str_map = test.str_map;

        avro::encode(*e, data);
        out->flush();
        clock_gettime(clock_mod, &end);
    }

    c::TestData other;
    {

        std::unique_ptr<avro::InputStream> in = avro::fileInputStream("avro_file");
        avro::DecoderPtr d = avro::binaryDecoder();
        d->init(*in);

        avro::decode(*d, other);
    }
    assert(other.u64 == test.u64);

    // https://aur.archlinux.org/avro-cpp.git
    // /home/ksenia/pkgs/avro-cpp
    // avrogencpp -i test_data.json -o test_data.hh -n c
    return result;
}

std::pair<double, double> yaml_ser(TestData &test, int clock_mod = CLOCK_REALTIME)
{
    std::pair<double, double> result;
    struct timespec begin, end;
    {
        std::ofstream ofs("yaml_file");
        YAML::Emitter out(ofs);
        clock_gettime(clock_mod, &begin);

        out << YAML::BeginMap;
        out << YAML::Key << "small_str";
        out << YAML::Value << test.small_str;
        out << YAML::Key << "small_array";
        out << YAML::Value << test.small_array;
        out << YAML::Key << "str_map";
        out << YAML::Value << test.str_map;
        out << YAML::Key << "u64";
        out << YAML::Value << test.u64;
        out << YAML::Key << "f64";
        out << YAML::Value << test.f64;
        out << YAML::EndMap;
        ofs.flush();
        clock_gettime(clock_mod, &end);
    }
    result.first = one_test_time(begin, end);

    TestData other;
    {
        clock_gettime(clock_mod, &begin);

        YAML::Node config = YAML::LoadFile("yaml_file");

        other.small_str = config["small_str"].as<typeof(other.small_str)>();
        other.small_array = config["small_array"].as<typeof(other.small_array)>();
        other.str_map = config["str_map"].as<typeof(other.str_map)>();
        other.u64 = config["u64"].as<typeof(other.u64)>();
        other.f64 = config["f64"].as<typeof(other.f64)>();
        clock_gettime(clock_mod, &end);
    }
    result.second = one_test_time(begin, end);

    assert(other.u64 == test.u64);
    return result;
}

std::pair<double, double> msg_ser(TestData &test, int clock_mod = CLOCK_REALTIME)
{
    std::pair<double, double> result;
    struct timespec begin, end;

    {
        std::ofstream ofs("msgpack_file");
        clock_gettime(clock_mod, &begin);
        msgpack::pack(ofs, test);
        clock_gettime(clock_mod, &end);
        // https://github.com/mikeloomisgg/cppack.git
    }
    result.first = one_test_time(begin, end);

    TestData other;
    other.u64 = 6.0;
    {
        std::ifstream ifs("msgpack_file", std::ios::binary | std::ios::in);
        std::stringstream buffer(std::stringstream::binary | std::stringstream::in | std::stringstream::out);
        buffer << ifs.rdbuf();

        clock_gettime(clock_mod, &begin);
        msgpack::object_handle oh = msgpack::unpack(buffer.str().data(), buffer.str().size());
        msgpack::object const &obj = oh.get();

        other = obj.as<TestData>();
        clock_gettime(clock_mod, &end);
    }
    result.second = one_test_time(begin, end);

    assert(other.u64 == test.u64);
    return result;
}

int main()
{
    freopen("output.txt", "w", stdout);
    TestData test;
    // text_ser(test);
    // xml_ser(test);
    // proto_ser(test);
    // json_ser(test);
    // avro_ser(test);
    // yaml_ser(test);
    msg_ser(test, CLOCK_REALTIME);
    msg_ser(test, CLOCK_PROCESS_CPUTIME_ID);
}
