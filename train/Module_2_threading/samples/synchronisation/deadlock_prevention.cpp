/*
Build a resource management system that prevents deadlocks when multiple threads need to acquire multiple resources
simultaneously, simulating a database transaction system.

🔍 Practice
Using the code below, create multiple threads that need to access different combinations of resources:
* Thread A needs resources "account1", "account2", "ledger"
* Thread B needs resources "account2", "account1", "audit_log"
* Thread C needs resources "ledger", "audit_log", "account1"
Without proper ordering, these threads could deadlock. Test your deadlock prevention manager with these scenarios.

✅ Success Checklist
* All resource acquisitions follow consistent ordering regardless of request order
* No deadlocks occur even with complex resource dependency patterns
* Exception safety ensures locks are released if transactions fail
* System handles high concurrency without performance degradation
*/
#include <algorithm>
#include <chrono>
#include <functional>
#include <iostream>
#include <map>
#include <mutex>
#include <thread>
#include <vector>

class Resource {
 private:
  std::mutex mutex_;
  std::string name_;
  int value_;

 public:
  // primary constructor
  explicit Resource(const std::string& name, int initialValue = 0) : name_(name), value_(initialValue) {}
  // non‑copyable because mutex is not copyable
  Resource(const Resource&) = delete;
  Resource& operator=(const Resource&) = delete;

  // movable: mutex is default‑constructed in the target
  Resource(Resource&& other) noexcept : name_(std::move(other.name_)), value_(other.value_) {}
  Resource& operator=(Resource&& other) noexcept {
    if (this != &other) {
      name_ = std::move(other.name_);
      value_ = other.value_;
      // mutex_ remains default-constructed
    }
    return *this;
  }

  void lock() { mutex_.lock(); }
  void unlock() { mutex_.unlock(); }
  bool try_lock() { return mutex_.try_lock(); }

  void modify(int delta) {
    value_ += delta;
    std::cout << "Resource " << name_ << " modified to: " << value_ << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }

  int getValue() const { return value_; }
  const std::string& getName() const { return name_; }
};
template <typename T>
T& operator<<(T& stream, const Resource& r) {
  stream << "Resource: " << r.getName();
  stream << " value: " << r.getValue();
  return stream;
}

class DeadlockPreventionManager {
 private:
  std::map<std::string, Resource*> resources_;

 public:
  void addResource(Resource* resource) { resources_[resource->getName()] = resource; }

  // Ordered lock acquisition to prevent deadlocks
  void executeTransaction(const std::vector<std::string>& resourceNames,
                          std::function<void(std::vector<Resource*>&)> transaction) {
    // Sort resource names to ensure consistent ordering
    std::vector<std::string> sortedNames = resourceNames;
    std::sort(sortedNames.begin(), sortedNames.end());

    // Acquire resources in sorted order
    std::vector<Resource*> acquiredResources;
    for (const auto& name : sortedNames) {
      auto it = resources_.find(name);
      if (it != resources_.end()) {
        it->second->lock();
        acquiredResources.push_back(it->second);
      }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    try {
      // Execute the transaction
      transaction(acquiredResources);
    } catch (...) {
      // Release locks in reverse order on exception
      for (auto it = acquiredResources.rbegin(); it != acquiredResources.rend(); ++it) {
        (*it)->unlock();
      }
      throw;
    }

    // Release locks in reverse order
    for (auto it = acquiredResources.rbegin(); it != acquiredResources.rend(); ++it) {
      (*it)->unlock();
    }
  }
};

void build_resources(std::vector<Resource>& resources, DeadlockPreventionManager& dpm) {
  resources.emplace_back("account1", 1000);
  resources.emplace_back("account2", 2000);
  resources.emplace_back("ledger", 0);
  resources.emplace_back("audit_log", 0);

  dpm.addResource(&resources[0]);
  dpm.addResource(&resources[1]);
  dpm.addResource(&resources[2]);
  dpm.addResource(&resources[3]);
}
void thread_mgmt() {
  std::vector<Resource> resources;
  DeadlockPreventionManager dpm;
  build_resources(resources, dpm);
  std::vector<std::thread> threads;
  std::vector<std::string> operation_1{"account1", "account2", "ledger"};
  std::vector<std::string> operation_2{"account2", "account1", "audit_log"};
  std::vector<std::string> operation_3{"ledger", "audit_log", "account1"};
  threads.emplace_back([&dpm, &operation_1] {
    dpm.executeTransaction(operation_1, [](auto& resources) {
      resources[0]->modify(-100);
      resources[1]->modify(100);
      resources[2]->modify(1);
    });
  });
  threads.emplace_back([&dpm, &operation_2] {
    dpm.executeTransaction(operation_2, [](auto& resources) {
      resources[0]->modify(-50);
      resources[1]->modify(50);
      resources[2]->modify(10);
    });
  });
  threads.emplace_back([&dpm, &operation_3] {
    dpm.executeTransaction(operation_3, [](auto& resources) {
      resources[0]->modify(9);
      resources[1]->modify(20);
      resources[2]->modify(20);
    });
  });

  for (auto& t : threads) {
    t.join();
  }
  std::cout << "All transactions completed successfully -- final status" << std::endl;
  for (auto& resource : resources) {
    std::cout << resource << std::endl;
  }
}
int main() {
  thread_mgmt();
  return 0;
}