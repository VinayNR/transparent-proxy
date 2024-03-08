#pragma once

#include <mutex>
#include <condition_variable>

template <typename T>
struct node {
    T value;
    node *next;
};

template <typename T>
class Queue {
    private:
        node<T> *head_;
        node<T> *tail_;

        mutable std::mutex queue_lock;
        std::condition_variable queue_condition;
        
    public:
        Queue();
        
        void enqueue(T &val);
        T dequeue();
        bool isEmpty();
};

/*
* Source Implementation for Queue
*/
template <typename T>
Queue<T>::Queue() {
    head_ = nullptr;
    tail_ = nullptr;
}

template <typename T>
inline bool Queue<T>::isEmpty() {
    return head_ == nullptr;
}

template <typename T>
void Queue<T>::enqueue(T &val) {
    // add the lock here
    std::lock_guard<std::mutex> lock(queue_lock);

    // make a new node
    struct node<T> *new_node = new node<T>;
    new_node->value = val;
    new_node->next = nullptr;

    // insert the new node
    if (tail_ == nullptr) {
        head_ = new_node;
    }
    else {
        tail_->next = new_node;
        
    }
    tail_ = new_node;

    // notify one other threads that there is work on the queue
    queue_condition.notify_one();
}

template <typename T>
T Queue<T>::dequeue() {
    // add the lock here
    // std::cout << "Inside deque" << std::endl;
    std::unique_lock<std::mutex> lock(queue_lock);

    if (head_ == nullptr) {
        // wait for work only if the queue is empty
        queue_condition.wait(
            lock,
            [this]{return !this->isEmpty();}
        );
    }

    T result = head_->value;

    // clean up memory
    node<T> *temp = head_;
    head_ = head_->next;

    if (head_ == nullptr) {
        tail_ = nullptr;
    }

    // delete the variable
    delete temp;

    return result;
}