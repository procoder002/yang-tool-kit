#include "yang_subs_mgr.h"

YangSubsMgr* YangSubsMgr::ycmPtr = nullptr;

YangSubsMgr::YangSubsMgr() {

  std::shared_ptr<Subscriber> s1 = std::make_shared<Subscriber>("device1");
  std::shared_ptr<Subscriber> s2 = std::make_shared<Subscriber>("device2");
  std::shared_ptr<Subscriber> s3 = std::make_shared<Subscriber>("device3");

  s1->subscribe("/network-device");
  s2->subscribe("/network-device/port");
  s3->subscribe("/network-device/settings");

  registerSubscriber(s1);
  registerSubscriber(s2);
  registerSubscriber(s3);
}

YangSubsMgr::~YangSubsMgr() {
}

void Subscriber::notify(const std::string& xpath, std::ostringstream& payload) {
  _gtest_config.push_back(payload.str());

  cout << id << " has been notified, New Subtree Path: " << xpath << endl;
  //cout << payload << std::endl;
}


std::string Subscriber::gtest_get_all_notification_str() {
  string full_config_str;
  for(auto& config: _gtest_config)  full_config_str += config;

  return full_config_str;
}

void YangSubsMgr::registerSubscriber(std::shared_ptr<Subscriber> sub) {

  for(auto path: sub->get_paths()) {
    xpathSubscriberMap[path].insert(sub);
  }
}

void YangSubsMgr::deregisterSubscriber(std::shared_ptr<Subscriber> sub)
{
  for(auto path: sub->get_paths()) {
    xpathSubscriberMap[path].erase(sub);
  }
}
