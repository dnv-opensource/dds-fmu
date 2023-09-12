#include <filesystem>
#include <functional>
#include <vector>

#include <gtest/gtest.h>
#include <xtypes/DynamicData.hpp>
#include <xtypes/idl/idl.hpp>

#include "DataMapper.hpp"
#include "model-descriptor.hpp"

TEST(Visitors, Principle) {
  std::string my_idl = R"~~~(
    struct Inner
    {
        uint32 my_uint32[3];
    };

    struct Outer
    {
        Inner my_inner;
    };

    module Space
    {
       struct Sun
       {
         int32 distance[3][2];
         Outer universe[2];
         string str;
       };

       module Atom
       {
         struct Bohr
         {
           boolean done;
         };
       };

       // Enum, Map, Sequence, String, wstring, Alias, Pair, Union
    };

)~~~";
  // Parse an idl
  eprosima::xtypes::idl::Context context;
  context.preprocess = false;
  context = eprosima::xtypes::idl::parse(my_idl, context);

  ASSERT_TRUE(context.success) << "Successful parsing";

  auto space_type = context.module().structure("Space::Sun");
  eprosima::xtypes::DynamicData data(space_type);

  // recursively iterate over all members of dynamic data
  std::cout << "Iterating: " << data.type().name() << " for structured names" << std::endl;

  std::vector<std::function<void(const std::int32_t&)>> int_writer_visitors;
  std::vector<std::function<void(std::int32_t&)>> int_reader_visitors;
  // need to store structured name for later use in modelDescription
  //   iterate visitors and

  data.for_each([&](eprosima::xtypes::DynamicData::WritableNode& node) {
    bool is_leaf = (node.type().is_primitive_type() || node.type().is_enumerated_type());
    bool is_string = node.type().kind() == eprosima::xtypes::TypeKind::STRING_TYPE;
    // missing: sequence type, wstring type, map type

    if (is_leaf || is_string) {
      std::string nested_name;
      ddsfmu::config::name_generator(nested_name, node);
      std::cout << nested_name << std::endl;

      if (node.type().kind() == eprosima::xtypes::TypeKind::UINT_32_TYPE) {
        int_reader_visitors.emplace_back(std::bind(
          ddsfmu::detail::reader_visitor<std::int32_t, std::uint32_t>, std::placeholders::_1, node.data()));

        int_writer_visitors.emplace_back(std::bind(
          ddsfmu::detail::writer_visitor<std::int32_t, std::uint32_t>, std::placeholders::_1, node.data()));

        std::cout << "valueReference " << int_reader_visitors.size() - 1 << " with name "
                  << nested_name << std::endl;
        std::cout << "valueReference " << int_writer_visitors.size() - 1 << " with name "
                  << nested_name << std::endl;
      }
    }
  });

  std::uint32_t val(32u);
  data["universe"][0]["my_inner"]["my_uint32"][0].value(val);
  EXPECT_EQ(val, data["universe"][0]["my_inner"]["my_uint32"][0].value<std::uint32_t>());

  std::int32_t fetch;
  // 0 is value reference, fetch would be reference value to read into
  int_reader_visitors[0](fetch);
  // reader_visitor is to be called in GetT or here: GetInt of cppfmu
  EXPECT_EQ(fetch, val);
  EXPECT_EQ(fetch, data["universe"][0]["my_inner"]["my_uint32"][0].value<std::uint32_t>());
  EXPECT_NE(fetch, data["universe"][0]["my_inner"]["my_uint32"][1].value<std::uint32_t>());

  std::uint32_t val2(42u);
  int_writer_visitors[1](val2); // 1 is value reference, val2 is const reference value to write
  // write_visitor is to be called in SetT or here: SetInt of cppfmu
  EXPECT_EQ(val2, data["universe"][0]["my_inner"]["my_uint32"][1].value<std::uint32_t>());

  //data["universe"][0]["my_inner"]["my_uint32"] equivalent with universe[0].my_inner.my_uint32
}


TEST(DataMapper, Visitors) {
  // This test assumes that msg_read is the first listed <fmu_out> in ddsfmu_mapping, and that msg_write is the first <fmu_in>
  // It will fail otherwise

  ddsfmu::DataMapper data_mapper;
  data_mapper.reset(std::filesystem::current_path() / "resources");

  auto& dyn_data = data_mapper.data_ref("msg_read", ddsfmu::DataMapper::Direction::Read);
  auto& dyn_data2 = data_mapper.data_ref("msg_write", ddsfmu::DataMapper::Direction::Write);

  bool with_enum = true;
  try {
    dyn_data["status"] = 1;
  } catch (const std::runtime_error&) { with_enum = false; }

  // We want to pretend we got data on dds (read data) (manually set dynamic data)
  dyn_data["str"] = std::string("Hello");         // str: 0
  dyn_data["ui8"] = std::uint8_t(255);            // int: 0
  dyn_data["i8"] = std::int8_t(-127);             // int: 1
  dyn_data["ui16"] = std::uint16_t(65565);        // int: 2
  dyn_data["i16"] = std::int16_t(-32766);         // int: 3
  dyn_data["i32"] = std::int32_t(-100000);        // int: 4
  dyn_data["i32_2"] = std::int32_t(-100001);      // int: 5
  dyn_data["ui32"] = std::uint32_t(2147483648);   // dbl: 0
  dyn_data["ui32_2"] = std::uint32_t(2147483649); // dbl: 1
  dyn_data["i64"] = std::int64_t(-4294967296);    // dbl: 2
  dyn_data["i64_2"] = std::int64_t(-4294967297);  // dbl: 3
  dyn_data["ui64"] = std::uint64_t(4294967296);   // dbl: 4
  dyn_data["ui64_2"] = std::uint64_t(4294967297); // dbl: 5
  dyn_data["d_val"] = 3.14;                       // dbl: 6
  dyn_data["f_val"] = 1.81f;                      // dbl: 7
  dyn_data["enabled"] = true;                     // bool: 0
  dyn_data["ch"] = '!';                           // str: 1
  if (with_enum) {
    dyn_data["status"] = 1; // int: 6
  }

  std::cout << "Did set dynamic data" << std::endl;

  // fmi getters to fetch from datamapper into local vars)

  double ui32, ui32_2, i64, i64_2, ui64, ui64_2, d_val, f_val; // 8 double
  std::string str, ch;                                         // 2 string
  bool enabled;                                                // 1 bool
  std::int32_t ui8, i8, ui16, i16, i32, i32_2, status;         // 7 int

  std::int32_t dbl_idx(0), str_idx(0), int_idx(0), bool_idx(0);

  data_mapper.get_int(int_idx++, ui8);
  data_mapper.get_int(int_idx++, i8);
  data_mapper.get_int(int_idx++, ui16);
  data_mapper.get_int(int_idx++, i16);
  data_mapper.get_int(int_idx++, i32);
  data_mapper.get_int(int_idx++, i32_2);
  if (with_enum) { data_mapper.get_int(int_idx++, status); }

  data_mapper.get_double(dbl_idx++, ui32);
  data_mapper.get_double(dbl_idx++, ui32_2);
  data_mapper.get_double(dbl_idx++, i64);
  data_mapper.get_double(dbl_idx++, i64_2);
  data_mapper.get_double(dbl_idx++, ui64);
  data_mapper.get_double(dbl_idx++, ui64_2);
  data_mapper.get_double(dbl_idx++, d_val);
  data_mapper.get_double(dbl_idx++, f_val);

  data_mapper.get_string(str_idx++, str);
  data_mapper.get_string(str_idx++, ch);
  data_mapper.get_bool(bool_idx++, enabled);

  std::cout << "Did fetch dynamic data" << std::endl;

  EXPECT_EQ(str, dyn_data["str"].value<std::string>());
  EXPECT_EQ(ui8, dyn_data["ui8"].value<std::uint8_t>());
  EXPECT_EQ(i8, dyn_data["i8"].value<std::int8_t>());
  EXPECT_EQ(ui16, dyn_data["ui16"].value<std::uint16_t>());
  EXPECT_EQ(i16, dyn_data["i16"].value<std::int16_t>());
  EXPECT_EQ(i32, dyn_data["i32"].value<std::int32_t>());
  EXPECT_EQ(i32_2, dyn_data["i32_2"].value<std::int32_t>());
  EXPECT_EQ(static_cast<std::uint32_t>(ui32), dyn_data["ui32"].value<std::uint32_t>());
  EXPECT_EQ(static_cast<std::uint32_t>(ui32_2), dyn_data["ui32_2"].value<std::uint32_t>());
  EXPECT_DOUBLE_EQ(i64, dyn_data["i64"].value<std::int64_t>());
  EXPECT_DOUBLE_EQ(i64_2, dyn_data["i64_2"].value<std::int64_t>());
  EXPECT_DOUBLE_EQ(ui64, dyn_data["ui64"].value<std::uint64_t>());
  EXPECT_DOUBLE_EQ(ui64_2, dyn_data["ui64_2"].value<std::uint64_t>());
  EXPECT_DOUBLE_EQ(d_val, dyn_data["d_val"].value<double>());
  EXPECT_DOUBLE_EQ(f_val, dyn_data["f_val"].value<float>());
  EXPECT_EQ(enabled, dyn_data["enabled"].value<bool>());
  EXPECT_EQ(ch, std::string(1, dyn_data["ch"].value<char>()));
  if (with_enum) { EXPECT_EQ(status, dyn_data["status"].value<std::uint32_t>()); }

  std::cout << "Did verify dynamic data" << std::endl;

  // Set these data (write data) using setters into dds outbound data buffers (Direction::Write)

  // This only usable if the msg_read and msg_write placement assumptions at the start of this function hold.
  int_idx = data_mapper.int_offset();
  dbl_idx = data_mapper.real_offset();
  str_idx = data_mapper.string_offset();
  bool_idx = data_mapper.bool_offset();

  data_mapper.set_int(int_idx++, ui8);
  data_mapper.set_int(int_idx++, i8);
  data_mapper.set_int(int_idx++, ui16);
  data_mapper.set_int(int_idx++, i16);
  data_mapper.set_int(int_idx++, i32);
  data_mapper.set_int(int_idx++, i32_2);
  if (with_enum) { data_mapper.set_int(int_idx++, status); }
  data_mapper.set_double(dbl_idx++, ui32);
  data_mapper.set_double(dbl_idx++, ui32_2);
  data_mapper.set_double(dbl_idx++, i64);
  data_mapper.set_double(dbl_idx++, i64_2);
  data_mapper.set_double(dbl_idx++, ui64);
  data_mapper.set_double(dbl_idx++, ui64_2);
  data_mapper.set_double(dbl_idx++, d_val);
  data_mapper.set_double(dbl_idx++, f_val);
  data_mapper.set_string(str_idx++, str);
  data_mapper.set_string(str_idx++, ch);
  data_mapper.set_bool(bool_idx++, !enabled); // not identical

  std::cout << "Did set dynamic data 2" << std::endl;

  EXPECT_FALSE(dyn_data == dyn_data2) << "Dynamic data 'msg_read' not yet equal to 'msg_write'";

  data_mapper.set_bool(bool_idx - 1, enabled); // reset previous
  EXPECT_TRUE(dyn_data == dyn_data2)
    << "Dynamic data read and written to same data structures shall be equal";

  std::cout << "Did confirm dynamic data equality" << std::endl;

  // test writer visitor with FMIBoolean (int32) , FMIString (char*)

  // Test set_string with char*, not string, usage pattern being used in cpp-fmu
  const char* something = "yeah";
  std::string other;
  data_mapper.set_string(data_mapper.string_offset(), something); // msg_read str
  dyn_data["str"] = std::string(something);
  data_mapper.get_string(0, other); // msg_write str

  EXPECT_EQ(dyn_data["str"].value<std::string>(), dyn_data2["str"].value<std::string>());
  EXPECT_EQ(other, std::string(something));
  EXPECT_TRUE(dyn_data == dyn_data2)
    << "Dynamic data read and written to same data structures shall be equal";

  // Test set_bool with int32, not bool, usage pattern being used in cpp-fmu
  std::int32_t my_bool(1);
  bool my_other_bool;
  data_mapper.set_bool(data_mapper.bool_offset(), static_cast<bool>(my_bool));
  dyn_data["enabled"] = bool(my_bool);
  data_mapper.get_bool(0, my_other_bool);
  EXPECT_EQ(dyn_data["enabled"].value<bool>(), dyn_data2["enabled"].value<bool>());
  EXPECT_EQ(bool(my_bool), my_other_bool);
  EXPECT_TRUE(dyn_data == dyn_data2)
    << "Dynamic data read and written to same data structures shall be equal";
}
