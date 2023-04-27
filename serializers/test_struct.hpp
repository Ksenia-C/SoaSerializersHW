#include <vector>
#include <string>
#include <map>

namespace test_data
{

    struct TestData
    {
        std::string one_str;

        std::vector<int> one_array;

        std::map<std::string, int> one_map;

        uint64_t u64 = 0;

        double f64 = 0.0;

        size_t get_data_size() const;
    };

    TestData get_small_test();
    TestData get_big_test(size_t datasz = 100);
}