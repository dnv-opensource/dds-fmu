#include <iostream>
#include "TrigSubscriber.hpp"
#include "trig.h"

#include <thread>
#include <chrono>

#include "config.hpp"

int main(){

  std::cout << ddsfmu_version() << std::endl;

  using namespace std::chrono_literals;

  TrigSubscriber sub;

  sub.init(false);

  while(true){
    std::this_thread::sleep_for(1000ms);
    auto& trig = sub.read();
    std::cout << trig.sine() << std::endl;
  }
  return 0;
}
