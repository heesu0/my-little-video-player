#ifndef __MEDIA_QUEUE_H__
#define __MEDIA_QUEUE_H__

#include <memory>
#include <queue>

template <typename T> class MediaQueue
{
public:
    void push(std::shared_ptr<T> in)
    {
        queue_.emplace(std::move(in));
    }

    std::shared_ptr<T> front()
    {
        if (queue_.empty())
        {
            return nullptr;
        }

        auto element = queue_.front();
        queue_.pop();

        return element;
    }

    int size()
    {
        return static_cast<int>(queue_.size());
    }

    void flush()
    {
        queue_ = std::queue<std::shared_ptr<T>>();
    }

private:
    std::queue<std::shared_ptr<T>> queue_;
};

#endif
