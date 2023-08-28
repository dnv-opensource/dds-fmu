#include <chrono>
#include <filesystem>
#include <thread>


#include<gtest/gtest.h>


#include <DataMapper.hpp>
#include <DynamicPubSub.hpp>

TEST(DynamicPubSub, Initialization)
{
  auto resources = std::filesystem::current_path() / "resources";
  DataMapper data_mapper;
  DynamicPubSub pubsub;

  data_mapper.reset(resources);
  pubsub.reset(resources, &data_mapper);

  auto& dyn_write = data_mapper.data_ref("roundtrip", DataMapper::Direction::Write);
  auto& dyn_read = data_mapper.data_ref("roundtrip", DataMapper::Direction::Read);

  double d_val(3.14);

  dyn_write["val"] = d_val;
  pubsub.write();

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  pubsub.read();

  EXPECT_EQ(d_val, dyn_read["val"].value<double>());

}
