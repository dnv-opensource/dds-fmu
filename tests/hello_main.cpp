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
