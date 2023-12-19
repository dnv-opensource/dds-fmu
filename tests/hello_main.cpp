/*
  Copyright 2023, SINTEF Ocean
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "hello_pubsub.hpp"

int main(int argc, char**argv){

  HelloPubSub pub(Permutation::API_XTYPES_IDL, true);
  //HelloPubSub sub(Permutation::API_XTYPES_IDL, false, 0);
  HelloPubSub sub2(Permutation::API_XTYPES_IDL, false, 1);
  pub.init();
  //sub.init();
  sub2.init();

  //sub.printInfo();

  std::thread threadPub(&HelloPubSub::runThread, &pub, 5, 100);
  threadPub.join();

  return 0;
}
