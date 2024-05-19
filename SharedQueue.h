#ifndef SHAREDQUEUE_H
#define SHAREDQUEUE_H

#include <condition_variable>
#include <iostream>
#include <mutex>
#include <ostream>
#include <queue>

template <typename T> class SharedQueue {
    public:
    SharedQueue();  // Конструктор класса
    ~SharedQueue(); // Деструктор класса

    T &front();     // Возвращает ссылку на элемент в начале очереди
    T &pop_front(); // Удаляет и возвращает элемент в начале очереди

    void push_back(const T &item); // Добавляет элемент в конец очереди
    void push_back(T &&item);      // Добавляет rvalue элемент в конец очереди
    void clear();                  // Очищает очередь

    int size();                    // Возвращает количество элементов в очереди
    bool empty();                  // Проверяет, пуста ли очередь.

    private:
    std::deque<T> queue_;          // Очередь элементов.
    std::mutex mutex_;             // Мьютекс для обеспечения потокобезопасности
    std::condition_variable cond_; // Условная переменная для синхронизации потоков
};

template <typename T> SharedQueue<T>::SharedQueue()
{
}

template <typename T> SharedQueue<T>::~SharedQueue() = default;

template <typename T> T &SharedQueue<T>::front()
{
    std::unique_lock<std::mutex> mlock(mutex_);
    while (queue_.empty())
    {
        cond_.wait(mlock);
    }
    return queue_.front();
}

template <typename T> T &SharedQueue<T>::pop_front()
{
    std::unique_lock<std::mutex> mlock(mutex_);
    while (queue_.empty())
    {
        cond_.wait(mlock);
    }
    T &front = queue_.front();
    queue_.pop_front();
    return front;
}

template <typename T> void SharedQueue<T>::push_back(const T &item)
{
    std::unique_lock<std::mutex> mlock(mutex_);
    queue_.push_back(item);
    mlock.unlock();
    cond_.notify_one();
}

template <typename T> void SharedQueue<T>::push_back(T &&item)
{
    std::unique_lock<std::mutex> mlock(mutex_);
    queue_.push_back(std::move(item));
    mlock.unlock();
    cond_.notify_one();
}
template <typename T> int SharedQueue<T>::size()
{
    std::unique_lock<std::mutex> mlock(mutex_);
    int size = queue_.size();
    mlock.unlock();
    return size;
}

template <typename T> bool SharedQueue<T>::empty()
{
    return size() == 0;
}

// Очищает очередь.
template <typename T> void SharedQueue<T>::clear()
{
    std::unique_lock<std::mutex> mlock(mutex_);
    std::deque<T>().swap(queue_);
    mlock.unlock();
    cond_.notify_one();
}

#endif // SHAREDQUEUE_H
