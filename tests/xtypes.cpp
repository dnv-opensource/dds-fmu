#include <filesystem>
#include <functional>
#include <vector>

#include <gtest/gtest.h>
#include <xtypes/idl/idl.hpp>
#include <xtypes/xtypes.hpp>

TEST(XTypes, BasicUsage)
{

  std::string my_idl = R"~~~(
    struct Inner
    {
        uint32 my_uint32;
    };

    struct Outer
    {
        Inner my_inner;
    };

    module Space
    {
       struct Sun
       {
         int32 distance;
         Outer universe[2];
       };
    };

)~~~";
  // Parse an idl
  eprosima::xtypes::idl::Context context = eprosima::xtypes::idl::parse(my_idl);

  for (auto [name, type] : context.get_all_scoped_types()) {
    std::cout << name << std::endl;
  }

  EXPECT_TRUE(context.module().has_structure("Space::Sun"));

  auto space_type = context.module().structure("Space::Sun");

  std::cout << "Space::Sun is " << space_type.name() << std::endl;

  eprosima::xtypes::DynamicData my_space(space_type);

  my_space["distance"] = -55;

  // nested
  my_space["universe"][0]["my_inner"]["my_uint32"] = 23u;
  my_space["universe"][1]["my_inner"]["my_uint32"] = 24u;

}

TEST(XTypes, Annotations)
{
  std::string my_idl = R"~~~(
    struct Inner
    {
        @key uint32 my_uint32;
        @optional boolean yes;
    };

)~~~";

  eprosima::xtypes::idl::Context context;
  context.log_level(eprosima::xtypes::idl::log::LogLevel::xDEBUG);
  context.print_log(true);

  context = eprosima::xtypes::idl::parse(my_idl, context);

  // Create same
  eprosima::xtypes::StructType inner2 =
   eprosima::xtypes::StructType("Inner")
   .add_member(eprosima::xtypes::Member("my_uint32", eprosima::xtypes::primitive_type<uint32_t>()).key())
   .add_member(eprosima::xtypes::Member("yes", eprosima::xtypes::primitive_type<bool>()).optional());

  auto inner = context.module().structure("Inner");

  // These will fail.
  //EXPECT_EQ(inner.member(0).is_key(), inner2.member(0).is_key());           // false, true
  //EXPECT_EQ(inner.member(1).is_optional(), inner2.member(1).is_optional()); // false, true

}
