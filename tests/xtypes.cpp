#include <filesystem>
#include <functional>
#include <vector>

#include <gtest/gtest.h>
#include <xtypes/idl/idl.hpp>
#include <xtypes/xtypes.hpp>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastrtps/types/DynamicDataFactory.h>
#include <fastrtps/types/DynamicTypeBuilder.h>
#include <fastrtps/types/DynamicPubSubType.h>
#include <fastrtps/types/DynamicTypePtr.h>

#include "Converter.hpp"


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

  EXPECT_TRUE(context.success) << "IDL parsing successful";
  EXPECT_TRUE(context.module().has_structure("Space::Sun")) << "Has structure Space::Sun";

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

  EXPECT_TRUE(context.success) << "IDL parsing successful";

  // Create same
  eprosima::xtypes::StructType inner2 =
   eprosima::xtypes::StructType("Inner")
   .add_member(eprosima::xtypes::Member("my_uint32", eprosima::xtypes::primitive_type<uint32_t>()).key())
   .add_member(eprosima::xtypes::Member("yes", eprosima::xtypes::primitive_type<bool>()).optional());

  auto inner = context.module().structure("Inner");

  // These will fail.
  GTEST_SKIP(); // Remove this once fixed
  EXPECT_EQ(inner.member(0).is_key(), inner2.member(0).is_key()) << "my_uint32 is @key";          // false, true
  EXPECT_EQ(inner.member(1).is_optional(), inner2.member(1).is_optional()) << "yes is @optional"; // false, true

}


TEST(XTypes, DdsEnum)
{
  // Test case where struct has enum and to be registered with as type in fast-dds

  std::string my_idl = R"~~~(
  enum EnumState {
    SLEEP, OKAY, ALERT, FAILURE, DEAD
  };

  struct Message
  {
    string str;
    uint8 ui8;
    int8 i8;
    uint16 ui16;
    int16 i16;
    uint32 ui32;
    unsigned long ui32_2;
    int32 i32;
    long i32_2;
    int64 i64;
    long long i64_2;
    uint64 ui64;
    unsigned long long ui64_2;
    double d_val;
    float f_val;
    boolean enabled;
    char ch;
    EnumState status; // Fails with fastdds
  };
  )~~~";

  eprosima::xtypes::idl::Context context;
  context.log_level(eprosima::xtypes::idl::log::LogLevel::xDEBUG);
  context.print_log(true);
  context = eprosima::xtypes::idl::parse(my_idl, context);
  EXPECT_TRUE(context.success) << "IDL parsing successful";

  const eprosima::xtypes::DynamicType& msg_type(context.module().structure("Message"));

  namespace etypes = eprosima::fastrtps::types;

  etypes::DynamicTypeBuilder* builder = ddsfmu::Converter::create_builder(msg_type);
  ASSERT_NE(builder, nullptr) << "Successful DynamicTypeBuilder for struct with enum";

  etypes::DynamicType_ptr dyntype = builder->build();
  ASSERT_NE(dyntype, nullptr) << "Create DynamicType_ptr";

  etypes::DynamicPubSubType dyntype_sup(dyntype);

  dyntype_sup.setName("Menum");
  // WORKAROUND START
  dyntype_sup.auto_fill_type_information(false);  // True will not work with Cyclone DDS
  dyntype_sup.auto_fill_type_object(false);       // True causes seg fault with enums
  // WORKAROUND END

  eprosima::fastdds::dds::DomainParticipantQos participant_qos = eprosima::fastdds::dds::PARTICIPANT_QOS_DEFAULT;

  eprosima::fastdds::dds::DomainId_t domain_id(0);
  auto participant = eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->create_participant(domain_id, participant_qos);

  ASSERT_NE(participant, nullptr);

  participant->register_type(dyntype_sup);


}
