# NEAT JSON BUILDER for RAPIDJSON

Rapid builder allows you to create JSON in a nice and readable way, thanks to c++11 feature - std::initializer_list. You may create std::string, rapidjson::Document, or rapidjson::Value with three easy API calls: BuildJson, BuildValue, or BuildDocument.
Rapid builder requires c++17 due to std::string_view usage.

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

The same json build code with regular rapidjson API:
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
const std::string json(buffer.GetString(), buffer.Size());
```

The same json build code with writer rapidjson API:
```c++
rapidjson::StringBuffer string_buffer;
rapidjson::Writer<rapidjson::StringBuffer> writer(string_buffer);
writer.StartObject();
writer.Key("username");
writer.String("my_username");
writer.Key("password");
writer.String("my_password");
writer.Key("validation-factors");
writer.StartArray();
writer.StartObject();
writer.Key("name");
writer.String("remote_address");
writer.Key("value");
writer.String("127.0.0.1");
writer.EndObject();
writer.StartObject();
writer.Key("name");
writer.String("remote_port");
writer.Key("value");
writer.Int(3333);
writer.EndObject();
writer.EndArray();
const std::string json(string_buffer.GetString(), string_buffer.Size());
```

So I think that you get the point of Rapid Builder.

Even more complex things possible with Rapid Builder:
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

```
BM_RapidbuilderCreateJson - This builder, create json text.
BM_RapidjsonCreateJson - Regular rapidjson API, create json text.
BM_RapidjsonWriterCreateJson - Writer rapidjson API, create json text.
BM_RapidbuilderCreateDocument - This builder, create json object to continue work with.
BM_RapidjsonCreateDocument - Regular rapidjson API, create json object to continue work with.
```

### GCC 9
```
----------------------------------------------------------------------
Benchmark                            Time             CPU   Iterations
----------------------------------------------------------------------
BM_RapidbuilderCreateJson            753 ns          750 ns       896000
BM_RapidjsonCreateJson               893 ns          879 ns       746667
BM_RapidjsonWriterCreateJson         686 ns          684 ns      1120000
BM_RapidbuilderCreateDocument        348 ns          353 ns      2036364
BM_RapidjsonCreateDocument           195 ns          197 ns      3733333
```

### Visual studio 2019 
```
------------------------------------------------------------------------
Benchmark                              Time             CPU   Iterations
------------------------------------------------------------------------
BM_RapidbuilderCreateJson           1575 ns         1569 ns       448000
BM_RapidjsonCreateJson              1829 ns         1800 ns       373333
BM_RapidjsonWriterCreateJson        1213 ns         1200 ns       560000
BM_RapidbuilderCreateDocument        686 ns          698 ns      1120000
BM_RapidjsonCreateDocument           426 ns          424 ns      1659259
```

MIT License
Copyright (c) 2021 pavel.sokolov@gmail.com / CEZEO software Ltd.. All rights reserved.
