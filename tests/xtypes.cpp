#include <filesystem>
#include <functional>
#include <vector>

#include <gtest/gtest.h>
#include <xtypes/idl/idl.hpp>

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
