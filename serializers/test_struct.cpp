#include "test_struct.hpp"

namespace test_data
{

    size_t TestData::get_data_size() const
    {
        size_t result = sizeof(*this) +
                        this->one_str.size() +
                        this->one_array.size() * sizeof(decltype(this->one_array)::value_type) +
                        this->one_map.size() * sizeof(decltype(this->one_map)::node_type);
        for (auto &[key, _] : this->one_map)
        {
            result += key.size();
        }
        return result;
    }

    TestData get_small_test()
    {
        return TestData{
            one_str :
                "fromTestData",
            one_array : {1, 2, 3, 4, 5},
            one_map : {
                {"a", 0}, {"b", 1}, {"c", 99}, {"d", 993}},
            u64 : (uint64_t)-1,
            f64 : 16.0
        };
    }

    TestData get_big_test(size_t datasz)
    {
        auto data = std::vector<int>(datasz, 0);
        std::map<std::string, int> map_data;
        for (size_t i = 0; i < datasz; ++i)
        {
            data[i] = i;
            map_data[std::string(i, 'a')] = i;
        }
        return TestData{
            one_str :
                std::string(datasz, '_'),
            one_array : std::move(data),
            one_map : std::move(map_data),
            u64 : uint64_t(rand()),
            f64 : 1.0 * rand()
        };
    }
} // namespace test_data
