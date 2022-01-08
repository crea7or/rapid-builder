# NEAT JSON BUILDER for RAPIDJSON

Rapid builder allows you to create JSON in a nice and readable way, thanks to c++11 feature - std::initializer_list. You may create std::string, rapidjson::Document, or rapidjson::Value with three easy API calls: BuildJson, BuildValue, or BuildDocument.
Rapid builder requires c++17 due to std::string_view usage. The builder is even faster than direct use of regular rapidjson API due to std::initializer_list and constexpr strlen in std::string_view by 14%.

Example json:
```json
{
  "password": "my_password",
  "username": "my_username",
  "validation-factors": [
    {
      "name": "remote_address",
      "value": "127.0.0.1"
    },
    {
      "name": "remote_port",
      "value": 3333
    }
  ]
}
```

Rapid builder code:
```c++ 
 const auto json = json::build(
      {{"username", "my_username"},
       {"password", "my_password"},
       {"validation-factors", json::array({
                    {{"name", "remote_address"},
                     {"value", "127.0.0.1"}},
                    {{"name", "remote_port"},
                     {"value", 3333}}
          })
      }});
```

The same json build code with plain rapidjson API:
```c++
rapidjson::Document doc;
auto& allocator = doc.GetAllocator();
doc.AddMember("username", "my_username", allocator);
doc.AddMember("password", "my_password", allocator);
rapidjson::Value arr(rapidjson::kArrayType);
rapidjson::Value valid_factor_addr(rapidjson::kObjectType);
valid_factor_addr.AddMember("name", "remote_address", allocator);
valid_factor_addr.AddMember("value", "127.0.0.1", allocator);
arr.PushBack(valid_factor_addr, allocator);
rapidjson::Value valid_factor_port(rapidjson::kObjectType);
valid_factor_port.AddMember("name", "remote_port", allocator);
valid_factor_port.AddMember("value", 3333, allocator);
arr.PushBack(valid_factor_port, allocator);
doc.AddMember("validation-factors", arr, allocator);
rapidjson::StringBuffer buffer;
rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
doc.Accept(writer);
const auto json = buffer.GetString();
```

So I think that you get the point of Rapid Builder.

Even more complex things possible:
```c++
  std::vector<int> values{1,2,3,4};
  const auto rapid_json_object = 
        json::build({{"object", {{"some", "other"}, {"int", 0}}},
                     {"from vector", json::array(values)},
                     {"array of objects",
                       {{"string", json::array({string_field_name, json::array({0, 1, 2}), "2"}),
                       {{"name", "value"}, {"bool", false}}}}
                     }});
```

Currently, you can use a container as JSON array source, but can't use rapidjson::Value in the same way. Another important limitation is about using temporal objects while building JSON documents or values - don't do that! Internally, this code uses StringRef() from rapidjson and value may die before it will be used. 

Don't use temporary variables in the builder! Strings in the example below will be destroyed before building the JSON and you'll get garbage in result!
```c++ 
 const auto bad_sample = json::build({{std::string("some field"), std::string("some value")}});
```

Rapid builder can be modified to be used for almost any other JSON library internally.

Tested in Visual Studio 2019 (Open as Folder) and gcc under ubuntu. This project uses Conan as dependency manager and CMake as a build system.
Build command:
```
cd repo_folder
cmake -DCMAKE_BUILD_TYPE=Release .
make
./bin/bench
./bin/tests
```

## Google benchmark

BM_JsonBuilderCreate - This builder. BM_RapidJsonCreate - direct use rapidjson API to build the json.

### GCC 9
```
----------------------------------------------------------------------
Benchmark                            Time             CPU   Iterations
----------------------------------------------------------------------
BM_JsonBuilderCreate               782 ns          767 ns       896000
BM_JsonBuilderCreate               761 ns          767 ns       896000
BM_JsonBuilderCreate               750 ns          750 ns       896000
BM_JsonBuilderCreate               759 ns          767 ns       896000
BM_JsonBuilderCreate               760 ns          750 ns       896000
BM_JsonBuilderCreate               799 ns          802 ns       896000
BM_JsonBuilderCreate               797 ns          802 ns       896000
BM_JsonBuilderCreate               799 ns          802 ns       896000
BM_JsonBuilderCreate_mean          776 ns          776 ns            8
BM_JsonBuilderCreate_median        771 ns          767 ns            8
BM_JsonBuilderCreate_stddev       20.7 ns         22.8 ns            8
BM_JsonBuilderCreate_cv           2.67 %          2.94 %             8
BM_RapidJsonCreate                 882 ns          879 ns       746667
BM_RapidJsonCreate                 892 ns          900 ns       746667
BM_RapidJsonCreate                 914 ns          921 ns       746667
BM_RapidJsonCreate                 894 ns          879 ns       746667
BM_RapidJsonCreate                 893 ns          900 ns       746667
BM_RapidJsonCreate                 895 ns          900 ns       746667
BM_RapidJsonCreate                 902 ns          900 ns       746667
BM_RapidJsonCreate                 895 ns          900 ns       746667
BM_RapidJsonCreate_mean            896 ns          897 ns            8
BM_RapidJsonCreate_median          894 ns          900 ns            8
BM_RapidJsonCreate_stddev         9.27 ns         13.4 ns            8
BM_RapidJsonCreate_cv             1.03 %          1.49 %             8
```

### Visual studio 2019 
```
----------------------------------------------------------------------
Benchmark                            Time             CPU   Iterations
----------------------------------------------------------------------
BM_JsonBuilderCreate              1667 ns         1688 ns       407273
BM_JsonBuilderCreate              1672 ns         1650 ns       407273
BM_JsonBuilderCreate              1665 ns         1688 ns       407273
BM_JsonBuilderCreate              1668 ns         1650 ns       407273
BM_JsonBuilderCreate              1669 ns         1688 ns       407273
BM_JsonBuilderCreate              1672 ns         1650 ns       407273
BM_JsonBuilderCreate              1665 ns         1688 ns       407273
BM_JsonBuilderCreate              1668 ns         1650 ns       407273
BM_JsonBuilderCreate_mean         1668 ns         1669 ns            8
BM_JsonBuilderCreate_median       1668 ns         1669 ns            8
BM_JsonBuilderCreate_stddev       2.67 ns         20.5 ns            8
BM_JsonBuilderCreate_cv           0.16 %          1.23 %             8
BM_RapidJsonCreate                1923 ns         1925 ns       373333
BM_RapidJsonCreate                1925 ns         1925 ns       373333
BM_RapidJsonCreate                1911 ns         1925 ns       373333
BM_RapidJsonCreate                1902 ns         1883 ns       373333
BM_RapidJsonCreate                1909 ns         1925 ns       373333
BM_RapidJsonCreate                1901 ns         1883 ns       373333
BM_RapidJsonCreate                1912 ns         1925 ns       373333
BM_RapidJsonCreate                1896 ns         1883 ns       373333
BM_RapidJsonCreate_mean           1910 ns         1910 ns            8
BM_RapidJsonCreate_median         1910 ns         1925 ns            8
BM_RapidJsonCreate_stddev         10.3 ns         21.7 ns            8
BM_RapidJsonCreate_cv             0.54 %          1.13 %             8
```

MIT License
Copyright (c) 2021 pavel.sokolov@gmail.com / CEZEO software Ltd.. All rights reserved.
