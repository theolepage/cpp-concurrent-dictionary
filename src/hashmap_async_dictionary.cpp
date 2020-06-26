#include "hashmap_async_dictionary.hpp"
#include <iostream>

hashmap_async_dictionary::hashmap_async_dictionary() : thread_pool_(100)
{
}

hashmap_async_dictionary::hashmap_async_dictionary(const dictionary_t& d) : thread_pool_(100)
{
  m_dic.init(d);
}

void hashmap_async_dictionary::init(const dictionary_t& d)
{
  m_dic.init(d);
}

std::future<result_t> hashmap_async_dictionary::search(const char* query) const
{
  // TODO Find a way to move promise inside the lambda, current problem say it's const
  auto p = new std::promise<result_t>;
  auto futur = p->get_future();
  thread_pool_.push([this, query, p]()
                    {
                      p->set_value(this->m_dic.search(query));
                      delete p;
                    }
  );
  return futur;
}


std::future<void> hashmap_async_dictionary::insert(int doc_id, gsl::span<const char*> text)
{
  auto p = new std::promise<void>;
  auto futur = p->get_future();
  thread_pool_.push([this, doc_id, &text, p]()
                    {
                      this->m_dic.insert(doc_id, text);
                      p->set_value();
                      delete p;
                    }
  );
  return futur;
}

std::future<void> hashmap_async_dictionary::remove(int doc_id)
{
  auto p = new std::promise<void>;
  auto futur = p->get_future();
  thread_pool_.push([this, doc_id, p]()
                    {
                      this->m_dic.remove(doc_id);
                      p->set_value();
                      delete p;
                    }
  );
  return futur;
}

