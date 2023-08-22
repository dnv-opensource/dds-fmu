#include <filesystem>
#include <functional>
#include <vector>

#include <gtest/gtest.h>
#include <xtypes/idl/idl.hpp>

#include "model-descriptor.hpp"

#include <visitors.hpp>

//std::function<void(std::string&, const eprosima::xtypes::DynamicData::ReadableNode&)>



TEST(Visitors, Reader)
{

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
  eprosima::xtypes::idl::Context context = eprosima::xtypes::idl::parse(my_idl);

  ASSERT_TRUE(context.success) << "Successful parsing";

  auto space_type = context.module().structure("Space::Sun");
  eprosima::xtypes::DynamicData data(space_type);

  // recursively iterate over all members of dynamic data
  std::cout << "Iterating: " << data.type().name() << " for structured names" << std::endl;

  std::vector<std::function<void(const std::int32_t&)>> int_writer_visitors;
  std::vector<std::function<void(std::int32_t&)>> int_reader_visitors;
  // need to store structured name for later use in modelDescription
  //   iterate visitors and

  data.for_each([&](eprosima::xtypes::DynamicData::WritableNode& node)
  {
    bool is_leaf = (node.type().is_primitive_type() || node.type().is_enumerated_type());
    // missing: sequence type, string type, wstring type, map type

    if(is_leaf){
      std::string nested_name;
      name_generator(nested_name, node);
      std::cout << nested_name << std::endl;

      if (node.type().kind() == eprosima::xtypes::TypeKind::UINT_32_TYPE){

        int_reader_visitors.emplace_back(
            std::bind(reader_visitor<std::int32_t, std::uint32_t>, std::placeholders::_1, node.data()));

        int_writer_visitors.emplace_back(
            std::bind(writer_visitor<std::int32_t, std::uint32_t>, std::placeholders::_1, node.data()));

        /*int_reader_visitors.emplace_back(std::bind([&](
          std::int32_t& out,
          const eprosima::xtypes::ReadableDynamicDataRef& cref){
          out = cref.value<std::uint32_t>();
          }, std::placeholders::_1, node.data()));*/

        /*
          int_writer_visitors.emplace_back(std::bind([&](
          const std::int32_t& in,
          eprosima::xtypes::WritableDynamicDataRef& ref){
          ref.value(static_cast<std::uint32_t>(in));
          }, std::placeholders::_1, node.data()));
        */

        std::cout << "valueReference " << int_reader_visitors.size()-1 << " with name " << nested_name << std::endl;
        std::cout << "valueReference " << int_writer_visitors.size()-1 << " with name " << nested_name << std::endl;
      }

    }
  });

  std::uint32_t val(32u);
  data["universe"][0]["my_inner"]["my_uint32"][0].value(val);
  EXPECT_EQ(val, data["universe"][0]["my_inner"]["my_uint32"][0].value<std::uint32_t>());

  std::int32_t fetch;
  int_reader_visitors[0](fetch); // 0 is value reference, fetch would be reference value to read into
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
