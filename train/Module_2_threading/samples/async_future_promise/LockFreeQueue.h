#ifndef LOCKFREE_QUEUE_H
#define LOCKFREE_QUEUE_H

#include <array>
#include <atomic>
#include <chrono>
#include <memory>
#include <type_traits>

template <typename T>
class LockFreeQueue {
 private:
  struct Node {
    std::atomic<T*> data{nullptr};
    std::atomic<Node*> next{nullptr};

    Node() = default;

    explicit Node(T item) { data.store(new T(std::move(item)), std::memory_order_relaxed); }
  };

  std::atomic<Node*> head_;
  std::atomic<Node*> tail_;
  std::atomic<size_t> size_{0};

  // Memory reclamation using hazard pointers (simplified)
  static constexpr size_t MAX_HAZARD_POINTERS = 16;
  thread_local static std::array<std::atomic<Node*>, MAX_HAZARD_POINTERS> hazardPointers;
  inline static std::atomic<size_t> hazardPointerIndex{0};

  Node* acquireHazardPointer(Node* node) {
    size_t index = hazardPointerIndex.fetch_add(1, std::memory_order_relaxed) % MAX_HAZARD_POINTERS;
    hazardPointers[index].store(node, std::memory_order_release);
    return node;
  }

  void releaseHazardPointer(Node* node) {
    for (auto& hp : hazardPointers) {
      if (hp.load(std::memory_order_acquire) == node) {
        hp.store(nullptr, std::memory_order_release);
        break;
      }
    }
  }

  bool isHazardous(Node* node) {
    for (const auto& hp : hazardPointers) {
      if (hp.load(std::memory_order_acquire) == node) {
        return true;
      }
    }
    return false;
  }

 public:
  LockFreeQueue() {
    Node* dummy = new Node();
    head_.store(dummy, std::memory_order_relaxed);
    tail_.store(dummy, std::memory_order_relaxed);
  }

  ~LockFreeQueue() {
    while (Node* node = head_.load(std::memory_order_relaxed)) {
      head_.store(node->next.load(std::memory_order_relaxed), std::memory_order_relaxed);
      delete node;
    }
  }

  void enqueue(T item) {
    Node* newNode = new Node(std::move(item));

    while (true) {
      Node* last = tail_.load(std::memory_order_acquire);
      Node* next = last->next.load(std::memory_order_acquire);

      if (last == tail_.load(std::memory_order_acquire)) {
        if (next == nullptr) {
          if (last->next.compare_exchange_weak(next, newNode, std::memory_order_release, std::memory_order_relaxed)) {
            tail_.compare_exchange_weak(last, newNode, std::memory_order_release, std::memory_order_relaxed);
            size_.fetch_add(1, std::memory_order_relaxed);
            break;
          }
        } else {
          tail_.compare_exchange_weak(last, next, std::memory_order_release, std::memory_order_relaxed);
        }
      }
    }
  }

  bool dequeue(T& result) {
    while (true) {
      Node* first = head_.load(std::memory_order_acquire);
      Node* last = tail_.load(std::memory_order_acquire);
      Node* next = first->next.load(std::memory_order_acquire);

      if (first == head_.load(std::memory_order_acquire)) {
        if (first == last) {
          if (next == nullptr) {
            return false;
          }
          tail_.compare_exchange_weak(last, next, std::memory_order_release, std::memory_order_relaxed);
        } else {
          if (!next) {
            continue;
          }

          T* data = next->data.load(std::memory_order_acquire);
          if (data == nullptr) {
            continue;
          }

          if (head_.compare_exchange_weak(first, next, std::memory_order_release, std::memory_order_relaxed)) {
            result = *data;
            delete data;
            size_.fetch_sub(1, std::memory_order_relaxed);

            if (!isHazardous(first)) {
              delete first;
            }

            return true;
          }
        }
      }
    }
  }

  bool empty() const { return size_.load(std::memory_order_acquire) == 0; }

  size_t size() const { return size_.load(std::memory_order_acquire); }
};

template <typename T>
thread_local std::array<std::atomic<typename LockFreeQueue<T>::Node*>, LockFreeQueue<T>::MAX_HAZARD_POINTERS>
    LockFreeQueue<T>::hazardPointers{};

#endif  // LOCKFREE_QUEUE_H
