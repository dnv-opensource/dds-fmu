/*
  Copyright 2023, SINTEF Ocean
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "hello_pubsub.hpp"

int main(int argc, char**argv){

  if (argc > 1) {
    HelloPubSub pub(Permutation::API_XTYPES_IDL, true);
    pub.init();
    pub.runPub(5, 100);
  } else {
    //HelloPubSub sub(Permutation::API_XTYPES_IDL, false, 0);
    //sub.init();
    HelloPubSub sub2(Permutation::API_XTYPES_IDL, false, 1);
    sub2.init();
    sub2.runSub(5, 100);
    //sub.printInfo();
  }

  return 0;
}
