/*
  Copyright 2023, SINTEF Ocean
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <chrono>
#include <filesystem>
#include <thread>

#include <gtest/gtest.h>

#include "DataMapper.hpp"
#include "DynamicPubSub.hpp"

TEST(DynamicPubSub, Initialization) {
  auto resources = std::filesystem::current_path() / "resources";
  ddsfmu::DataMapper data_mapper;
  ddsfmu::DynamicPubSub pubsub;

  data_mapper.reset(resources);
  pubsub.reset(resources, &data_mapper);

  auto& dyn_write = data_mapper.data_ref("roundtrip", ddsfmu::DataMapper::Direction::Write);
  auto& dyn_read = data_mapper.data_ref("roundtrip", ddsfmu::DataMapper::Direction::Read);

  double d_val(3.14);

  dyn_write["val"] = d_val;
  pubsub.write();

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  pubsub.take();

  EXPECT_EQ(d_val, dyn_read["val"].value<double>());


  d_val = 1.8;
  dyn_write["val"] = d_val;
  pubsub.write();
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  d_val = 0.9;
  dyn_write["val"] = d_val;
  pubsub.write();
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  pubsub.take();

  EXPECT_EQ(d_val, dyn_read["val"].value<double>());
}

TEST(DynamicPubSub, Reinitialization) {
  auto resources = std::filesystem::current_path() / "resources";
  ddsfmu::DataMapper data_mapper;
  ddsfmu::DynamicPubSub pubsub;

  data_mapper.reset(resources);
  pubsub.reset(resources, &data_mapper);
  pubsub.reset(resources, &data_mapper);
}
