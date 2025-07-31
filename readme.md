# NEAT JSON BUILDER for RAPIDJSON

The **Rapid Builder** provides a clean and readable way to create JSON using `std::initializer_list` (C++11 feature). With just three easy-to-use API calls—`BuildJson`, `BuildValue`, or `BuildDocument`—you can generate `std::string`, `rapidjson::Document`, or `rapidjson::Value` objects.

> **Note:** Rapid Builder requires **C++17** because it uses `std::string_view` and `std::variant`.

---

## Example JSON

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

---

## Rapid Builder Usage

```c++
const auto json = json::build({
    {"username", "my_username"},
    {"password", "my_password"},
    {"validation-factors", json::array({
        {{"name", "remote_address"}, {"value", "127.0.0.1"}},
        {{"name", "remote_port"}, {"value", 3333}}
    })}
});
```

---

## Equivalent Using Regular RapidJSON API

*(verbose, but functionally identical)*

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

---

## Writer API Example

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
**Nlohmann JSON library uses a similar approach** to build JSON, and I have included it in the benchmark section for comparison.

---

## More Complex Examples

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

---

## Limitations

1. **Do not use temporary variables!**
   Rapid Builder internally uses `StringRef()` from RapidJSON. Temporaries will be destroyed before the JSON is fully built, leading to undefined behavior.

   ```c++
   // ❌ BAD EXAMPLE
   const auto bad = json::build({
       {std::string("field"), std::string("value")}
   });
   ```

2. Containers can be directly used as arrays, but you cannot use `rapidjson::Value` in the same way.

---

## Benchmarks

Rapid Builder was tested against `rapidjson` and `nlohmann/json`.

### Google Benchmark

```
BM_RapidbuilderCreateJson        - This builder (create JSON text)
BM_RapidjsonCreateJson           - Regular rapidjson API (create JSON text)
BM_NlohmannCreateJson             - Nlohmann JSON (create JSON text)
BM_RapidjsonWriterCreateJson     - Writer API (create JSON text)

BM_RapidbuilderCreateDocument    - This builder (create JSON object)
BM_NlohmannCreateDocument        - Nlohmann JSON (create JSON object)
BM_RapidjsonCreateDocument       - Regular rapidjson API (create JSON object)
```

### GCC 9 (Linux)

```
2022-01-29T14:07:48+03:00
Running ./bench
Run on (20 X 3600 MHz CPU s)
Load Average: 0.52, 0.58, 0.59
------------------------------------------------------------------------
Benchmark                              Time             CPU   Iterations
------------------------------------------------------------------------
BM_RapidbuilderCreateJson            849 ns          837 ns       746667
BM_RapidjsonCreateJson               956 ns          963 ns       746667
BM_NlohmannCreateJson               4604 ns         4604 ns       149333
BM_RapidjsonWriterCreateJson         717 ns          715 ns       896000
BM_RapidbuilderCreateDocument        394 ns          392 ns      1792000
BM_NlohmannCreateDocument           3333 ns         3299 ns       203636
BM_RapidjsonCreateDocument           180 ns          180 ns      3733333
```

### Visual Studio 2019 (Windows)

```
2022-01-29T14:10:11+03:00
Running bench.exe
Run on (20 X 3600 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x10)
  L1 Instruction 32 KiB (x10)
  L2 Unified 256 KiB (x10)
  L3 Unified 20480 KiB (x1)
------------------------------------------------------------------------
Benchmark                              Time             CPU   Iterations
------------------------------------------------------------------------
BM_RapidbuilderCreateJson           1565 ns         1569 ns       448000
BM_RapidjsonCreateJson              2269 ns         2295 ns       320000
BM_NlohmannCreateJson              11585 ns        11719 ns        64000
BM_RapidjsonWriterCreateJson        1193 ns         1200 ns       560000
BM_RapidbuilderCreateDocument       1168 ns         1172 ns       640000
BM_NlohmannCreateDocument           9259 ns         9208 ns        74667
BM_RapidjsonCreateDocument           849 ns          837 ns       746667
```

---

## Build Instructions

This project uses **Conan** as a dependency manager and **CMake** as a build system.

```bash
cd repo_folder
cmake -DCMAKE_BUILD_TYPE=Release .
make
./bin/bench
./bin/tests
```

---

## Usage

Simply copy `builder.h` and `builder.cpp` into your project.

---

## License

MIT License
Copyright (c) 2022
**Pavel Sokolov (pavel.sokolov@gmail.com)** / **CEZEO Software Ltd.**

```
      .:+oooooooooooooooooooooooooooooooooooooo: `/ooooooooooo/` :ooooo+/-`
   `+dCEZEOCEZEOCEZEOCEZEOCEZEOCEZEOCEZEOCEZEOEZshCEZEOCEZEOEZ#doCEZEOEZEZNs.
  :CEZEON#ddddddddddddddddddddddddddddddNCEZEO#h.:hdddddddddddh/.yddddCEZEO#N+
 :NEZ#N+.        .-----------.`       `+CEZEOd/   .-----------.        `:CEZEO/
 hEZEZ/         :CEZEOCEZEOEZNd.    `/dCEZEO+`   sNCEZEOCEZEO#Ny         -CEZEO
 hEZEZ/         :#NCEZEOCEZEONd.   :hEZEZNo`     oNCEZEOCEZEO#Ny         -CEZEO
 :NEZ#No.`       `-----------.`  -yNEZ#Ns.       `.-----------.`       `/CEZEO/
  :CEZEONCEZEOd/.ydCEZEOCEZEOdo.sNCEZEOCEZEOCEZEOCEZEOCEZEOCEZEOCEZEOEZNEZEZN+
   `+dCEZEOEZEZdoCEZEOCEZEOEZ#N+CEZEOCEZEOCEZEOCEZEOCEZEOCEZEOCEZEOCEZEOEZ#s.
      .:+ooooo/` :+oooooooooo+. .+ooooooooooooooooooooooooooooooooooooo+/.
```
