#pragma once

#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>

namespace sop {

/// Thread-safe queue for multiple producers and consumers.
template <typename T>
class ConcurrentQueue {
   public:
    /// Enqueues an item and wakes one waiting receiver.
    void Send(T item) {
        {
            std::lock_guard lock{m_Mutex};
            m_Items.push(std::move(item));
        }
        m_CondVar.notify_one();
    }

    /// Blocks until an item is available or the wait is interrupted.
    /// Returns std::nullopt if no item is available after waking.
    std::optional<T> Recv() {
        std::unique_lock lock{m_Mutex};
        if (m_Items.empty()) {
            m_CondVar.wait(lock);
        }
        if (m_Items.empty()) {
            return std::nullopt;
        }
        T item = std::move(m_Items.front());
        m_Items.pop();
        return item;
    }

    /// Attempts to dequeue an item without blocking.
    /// Returns std::nullopt if the queue is empty.
    std::optional<T> TryRecv() {
        std::lock_guard lock{m_Mutex};
        if (m_Items.empty()) {
            return std::nullopt;
        }
        T item = std::move(m_Items.front());
        m_Items.pop();
        return item;
    }

    /// Wakes one receiver currently blocked in Recv().
    void WakeOne() {
        m_CondVar.notify_one();
    }

    /// Wakes all receivers currently blocked in Recv().
    void WakeAll() {
        m_CondVar.notify_all();
    }

   private:
    std::mutex m_Mutex;
    std::condition_variable m_CondVar;
    std::queue<T> m_Items;
};

}  // namespace sop
