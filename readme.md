# NEAT JSON BUILDER for RAPIDJSON

The **Rapid Builder** provides a clean and readable way to create JSON using `std::initializer_list` (a C++11 feature). With three API calls - `json::build`, `json::build_value`, and `json::build_document` - you can generate `std::string`, `rapidjson::Value`, or `rapidjson::Document` objects.

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

So you get the point of Rapid Builder.
**Nlohmann JSON uses a similar style** to build JSON, and it is included in the benchmark section for comparison.

---

## More Complex Examples

```c++
std::vector<int> values{1, 2, 3, 4};
const auto rapid_json_object =
    json::build({{"object", {{"some", "other"}, {"int", 0}}},
                 {"from vector", json::array(values)},
                 {"array of objects",
                  {{"string", json::array({string_field_name, json::array({0, 1, 2}), "2"}),
                    {{"name", "value"}, {"bool", false}}}}});
```

---

## Limitations

1. **The builder borrows its input: strings, object names and `json::value` objects passed to it must outlive the build call.**

   Strings are stored as `std::string_view`, object fields of initializer lists keep references to their values, and a `json::value` used inside another builder input is referenced, not copied. This is what makes the builder allocation-free for one-shot calls.

   Safe one-shot usage:

   ```c++
   const auto json = json::build({
       {"username", "my_username"},
       {"password", "my_password"}
   });
   ```

   Not intended for persisted builder input:

   ```c++
   // BAD: the temporary string dies immediately, json::value keeps a view of it.
   // json::value(std::string&&) is deleted, so this does not compile.
   const json::value bad = std::string("value");
   ```

2. **If the library is used to build a `rapidjson::Value` or `rapidjson::Document`, the input parameters must outlive the returned RapidJSON object.**

   String values and object member names are added to RapidJSON as `rapidjson::StringRef`, without copying.

   Intended usage:

   ```c++
   std::string field_name = "field";
   std::string field_value = "value";

   const json::value source = json::object(std::vector<std::pair<std::string, json::value>>{
       {field_name, field_value}
   });

   const auto doc = json::build_document(source);
   // `source` and `field_value` must stay alive while `doc` is used.
   ```

   Not intended:

   ```c++
   // BAD: the temporary input dies right after the call
   const auto doc = json::build_document(
       json::object(std::vector<std::pair<std::string, json::value>>{{"field", "value"}}));
   ```

3. **Persisted trees should be built with `json::value`, `json::object(...)`, and `json::array(...)`.**

   - `json::array({...})` copies the initializer list items into the returned value, because initializer list items die at the end of the expression. Inside a one-shot `build` call use nested `{{...}}` syntax instead: it is not copied.
   - `json::object(map)` and `json::array(container)` borrow names and values from an lvalue container. For an rvalue container, `json::value` elements are moved, or copied when const. Rvalue object keys of type `std::string_view`, `const char*`, or `char*` remain borrowed; other key types are copied into owned strings. String values remain borrowed.

4. Containers can be directly used as arrays, but you cannot use `rapidjson::Value` in the same way.

5. NaN and infinite numbers are written as `null`: they are not valid JSON numbers.

6. A two-element list whose first element is a string is an object field: `{"key", {{"a", "b"}}}` is `{"key":{"a":"b"}}`. Use `json::array({"a", "b"})` for an array of two strings.

## Performance Notes

- `json::build(..., std::string& out)` writes into an existing string and reuses its capacity. The writer and its buffer are kept per thread between the calls.
- `json::build_document(..., rapidjson::Document& document)` builds into an existing document with its allocator, e.g. a `rapidjson::MemoryPoolAllocator` over a user buffer, instead of the default 64KB chunk.
- Arrays and objects of documents are allocated with the exact capacity.
- Define `RAPIDJSON_SSE2` (or `RAPIDJSON_SSE42` / `RAPIDJSON_NEON`) for all translation units to scan strings with SIMD in the writer. The CMake option `RAPIDJSON_BUILDER_SSE2` does this for x86-64.

---

## Benchmarks

Rapid Builder was tested against `rapidjson` and `nlohmann/json`.

### Google Benchmark

```
RapidBuilder_CreateJson          - This builder (create JSON text)
RapidJson_CreateJson             - Regular rapidjson API (create JSON text)
Nlohmann_CreateJson              - Nlohmann JSON (create JSON text)
RapidJsonWriter_CreateJson       - Writer API (create JSON text)

RapidBuilder_CreateDocument      - This builder (create JSON object)
Nlohmann_CreateDocument          - Nlohmann JSON (create JSON object)
RapidJson_CreateDocument         - Regular rapidjson API (create JSON object)
```

To reproduce (median of 10 repetitions):

```bash
./bin/bench --benchmark_repetitions=10 --benchmark_report_aggregates_only=true
```

### Visual Studio 2022 (Windows)

MSVC 19.44 (VS 17.14), Release, RapidJSON 1.1.0, `RAPIDJSON_SSE2` enabled.

```
2026-10-28T15:17:43+04:00
Running bench.exe
Run on (8 X 3110 MHz CPU s)
CPU Caches:
  L1 Data 48 KiB (x8)
  L1 Instruction 64 KiB (x8)
  L2 Unified 2560 KiB (x8)
  L3 Unified 8192 KiB (x2)
-----------------------------------------------------------------------------
Benchmark                                   Time             CPU   Iterations
-----------------------------------------------------------------------------
RapidJsonWriter_CreateJson_median        1325 ns         1177 ns           10
RapidBuilder_CreateJson_median           1594 ns         1378 ns           10
RapidJson_CreateJson_median              2347 ns         1962 ns           10
Nlohmann_CreateJson_median              10544 ns         8894 ns           10
RapidJson_CreateDocument_median          1222 ns         1038 ns           10
RapidBuilder_CreateDocument_median       1344 ns         1184 ns           10
Nlohmann_CreateDocument_median           7884 ns         6975 ns           10
```

Relative to the regular RapidJSON code in the same run (CPU time):

| Benchmark | Rapid Builder | Reference | Ratio |
|---|---|---|---|
| Create JSON text | 1378 ns | Writer API: 1177 ns | 1.17x |
| Create JSON text | 1378 ns | Regular rapidjson API: 1962 ns | 0.70x |
| Create JSON object | 1184 ns | Regular rapidjson API: 1038 ns | 1.14x |

Rapid Builder is within 20% of the hand-written Writer and DOM code, faster than the regular rapidjson API for JSON text, and 6x faster than nlohmann/json.

### Previous results (2022, before the allocation rework)

#### GCC 9 (Linux)

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

#### Visual Studio 2019 (Windows)

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

Simply copy `builder.h` and `builder.cpp` into your project. Requires RapidJSON 1.1.0 or later.

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
