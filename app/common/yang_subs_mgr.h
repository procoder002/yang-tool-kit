#ifndef YANG_SUBS_MGR_H
#define YANG_SUBS_MGR_H

#include <bits/stdc++.h>
#include "utility.h"

#define SPACE   4

using namespace std;

/**
 * all data types in the auto-generated file uses this wrapper class
 * Holds the actuals value of the data with additional information
 * 1. Whether the data exists (isPresent) and
 * 2. if the data is operational data
 */

template<typename T>
class YTEntity {
  public:
    T   value;
    bool isPresent = false;
    bool isOperationalData  = false;
    YTEntity() {}

    T& get() { return value; }

    YTEntity(T newValue) {
      value = newValue;  // initialize to zero
      isPresent = false;
    }

    void operator = (T newValue) {
      value = newValue;
      isPresent = true;
    }

    void set_value(const char* newValue) {
      std::stringstream ss(newValue);
      ss >> value;
      isPresent = true;
    }

    bool operator == (const string& obj) const {
      std::stringstream ss;
      ss << value;
      return ss.str() == obj;
    }

    bool operator == (const YTEntity& obj) const {
      return value == obj.value;
    }

    bool operator != (const YTEntity& obj) const {
      return  value != obj.value;
    }

    bool operator < (const YTEntity& obj) const {
      return value < obj.value;
    }

    void clear() {
      value.clear();
      isPresent = false;
    }

    bool empty() {
      return value.empty();
    }
};

struct YTContainer {
  bool isPresent = false;
  bool isOperationalData  = false;

  // copy constructor
};


class Subscriber {

  public:
    explicit Subscriber(const string& s): id(s) {}
    //virtual ~Subscriber() {}

    void subscribe(const string&  xpath) {
        subscription_xpaths.push_back(xpath);
    }

    string get_id() { return id; }
    vector<string> get_paths() { return subscription_xpaths; }

    template<typename T>
    void notify(const std::string& xpath, T newValue) {
      std::cout << id << " has been notified, Path: " << xpath << " newvalue: " << newValue << std::endl;
    }

    virtual void notify(const std::string& xpath, std::ostringstream& payload);
    
    string gtest_get_all_notification_str();
    vector<string> gtest_get_all_notification() { return _gtest_config; };

    void gtest_reset_notification() { _gtest_config.clear(); }

  private:
    string id;
    vector<string> subscription_xpaths;

    vector<string>  _gtest_config;
    int             _gtest_count = 0;
};


class YangSubsMgr {
  public:
    static YangSubsMgr* instance() {
      if(ycmPtr == nullptr) {
        ycmPtr = new YangSubsMgr();
      }
      return ycmPtr;
    }

    static void reset() {
      delete ycmPtr;
      ycmPtr = nullptr;
    }

    void registerSubscriber(std::shared_ptr<Subscriber> sub);
    void deregisterSubscriber(std::shared_ptr<Subscriber> sub);


    // xpath to list of subscriber mapping
    std::unordered_map< std::string, std::set< std::shared_ptr<Subscriber> >>   xpathSubscriberMap;

  private:
    YangSubsMgr();
    ~YangSubsMgr();
    static YangSubsMgr* ycmPtr;
};

#endif // YANG_SUBS_MGR_H
