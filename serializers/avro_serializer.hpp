#include "interface.hpp"

#include "test_data.hh"
#include "avro/Encoder.hh"
#include "avro/Decoder.hh"
#include "avro/Stream.hh"

#include <fstream>
#include <fcntl.h>
#include <assert.h>

namespace serializer
{
    struct TestDataAvro
    {
        TestDataAvro() = default;
        TestDataAvro(const TestData &test)
        {
            impl.f64 = test.f64;
            impl.u64 = test.u64;
            impl.one_array = test.one_array;
            impl.one_str = test.one_str;
            impl.one_map = test.one_map;
        }

        c::TestDataAvro impl;
    };
    class Serializer : public SerializerInterface<TestDataAvro>
    {
    public:
        Serializer() : SerializerInterface("avro") {}

    private:
        std::pair<double, double>
        estimate_one_run(TestDataAvro &test, int clock_mode) override
        {
            std::pair<double, double> result;
            struct timespec begin, end;

            {

                std::unique_ptr<avro::OutputStream> out = avro::fileOutputStream(SDATA_FILE_NAME.c_str());
                clock_gettime(clock_mode, &begin);

                avro::EncoderPtr e = avro::binaryEncoder();
                e->init(*out);
                avro::encode(*e, test.impl);
                out->flush();
                clock_gettime(clock_mode, &end);
            }
            result.first = one_test_time(begin, end);

            c::TestDataAvro other;
            {
                std::unique_ptr<avro::InputStream> in = avro::fileInputStream(SDATA_FILE_NAME.c_str());
                clock_gettime(clock_mode, &begin);
                avro::DecoderPtr d = avro::binaryDecoder();
                d->init(*in);

                avro::decode(*d, other);
                clock_gettime(clock_mode, &end);
            }
            result.second = one_test_time(begin, end);

            assert(test.impl.u64 == other.u64);
            return result;
        }
    };
}