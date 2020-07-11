#pragma once

#include "../IAsyncDictionary.hpp"
#include <tbb/task_group.h>
#include "thread_pool.hpp"

template <typename DICTIONARY>
class Async_Dictionary : public IAsyncReversedDictionary
{
public:
    Async_Dictionary() {}

    Async_Dictionary(const dictionary_t& d)
    {
        m_dic.init(d);
    }

    void init(const dictionary_t& d)
    {
        m_dic.init(d);
    }

    std::future<result_t>search(const char* query) const
    {
        auto p = new std::promise<result_t>;
        auto futur = p->get_future();
        thread_pool_.push([this, query, p]() {
            p->set_value(this->m_dic.search(query));
            delete p;
        });
        return futur;
    }

    std::future<void> insert(int doc_id, gsl::span<const char*> text)
    {
        auto p = new std::promise<void>;
        auto futur = p->get_future();
        thread_pool_.push([this, doc_id, text, p]() {
            this->m_dic.insert(doc_id, text);
            p->set_value();
            delete p;
        });
        return futur;
    }

    std::future<void> remove(int doc_id)
    {
        auto p = new std::promise<void>;
        auto futur = p->get_future();
        thread_pool_.push([this, doc_id, p]()
        {
            this->m_dic.remove(doc_id);
            p->set_value();
            delete p;
        });
        return futur;
    }

  DICTIONARY m_dic;
private:

  mutable Thread_Pool thread_pool_;
};