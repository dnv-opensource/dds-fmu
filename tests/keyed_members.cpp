/*
  Copyright 2023, SINTEF Ocean
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "hello_pubsub.hpp"
#include <gtest/gtest.h>


class KeyedDynamicType : public ::testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {
    if (pub) { delete pub; }
    if (sub) { delete sub; }
  }
  void Init(Permutation creator_pub, Permutation creator_sub) {
    pub = new HelloPubSub(creator_pub, true);
    sub = new HelloPubSub(creator_sub, false);
    pub->init();
    sub->init();
  }
  void Run() {
    std::thread threadPub(&HelloPubSub::runThread, pub, 5, 100);
    threadPub.join();
  }
  HelloPubSub *pub, *sub;
};



TEST_F(KeyedDynamicType, APIFastDDS_XX) {
  Init(Permutation::API_FASTDDS, Permutation::API_FASTDDS);
  Run();

  EXPECT_GE(sub->samples_received(), 1) << "Number of samples received by subscriber";
}

TEST_F(KeyedDynamicType, PubFastDDS_SubXTypes_XY) {
  Init(Permutation::API_FASTDDS, Permutation::API_XTYPES);
  Run();

  EXPECT_GE(sub->samples_received(), 1) << "Number of samples received by subscriber";
}

TEST_F(KeyedDynamicType, PubFastDDS_SubIDL_XZ) {
  Init(Permutation::API_FASTDDS, Permutation::API_XTYPES_IDL);
  Run();

  EXPECT_GE(sub->samples_received(), 1) << "Number of samples received by subscriber";
}

TEST_F(KeyedDynamicType, PubXTypes_SubFastDDS_YX) {
  Init(Permutation::API_XTYPES, Permutation::API_FASTDDS);
  Run();

  EXPECT_GE(sub->samples_received(), 1) << "Number of samples received by subscriber";
}

TEST_F(KeyedDynamicType, APIXTypes_YY) {
  Init(Permutation::API_XTYPES, Permutation::API_XTYPES);
  Run();

  EXPECT_GE(sub->samples_received(), 1) << "Number of samples received by subscriber";
}

TEST_F(KeyedDynamicType, PubXTypes_SubIDL_YZ) {
  Init(Permutation::API_XTYPES, Permutation::API_XTYPES_IDL);
  Run();

  EXPECT_GE(sub->samples_received(), 1) << "Number of samples received by subscriber";
}

TEST_F(KeyedDynamicType, PubIDL_SubFastDDS_ZX) {
  Init(Permutation::API_XTYPES_IDL, Permutation::API_FASTDDS);
  Run();

  EXPECT_GE(sub->samples_received(), 1) << "Number of samples received by subscriber";
}

TEST_F(KeyedDynamicType, PubIDL_SubXTypes_ZY) {
  Init(Permutation::API_XTYPES_IDL, Permutation::API_XTYPES);
  Run();

  EXPECT_GE(sub->samples_received(), 1) << "Number of samples received by subscriber";
}

TEST_F(KeyedDynamicType, APIIDL_ZZ) {
  Init(Permutation::API_XTYPES_IDL, Permutation::API_XTYPES_IDL);
  Run();

  EXPECT_GE(sub->samples_received(), 1) << "Number of samples received by subscriber";
}


TEST(KeyedTopics, ContentFilteredTopic) {
  HelloPubSub pub(Permutation::API_XTYPES_IDL, true);
  HelloPubSub sub(Permutation::API_XTYPES, false, 3);
  HelloPubSub sub2(Permutation::API_XTYPES_IDL, false, 4);
  pub.init();
  sub.init();
  sub2.init();

  std::thread threadPub(&HelloPubSub::runThread, &pub, 5, 100);
  threadPub.join();
}
