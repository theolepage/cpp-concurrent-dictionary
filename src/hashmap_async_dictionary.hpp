#pragma once

#include "IAsyncDictionary.hpp"
#include "hashmap_dictionary.hpp"
#include <tbb/task_group.h>
#include "thread_pool.hpp"

class hashmap_async_dictionary : public IAsyncReversedDictionary
{
public:
  hashmap_async_dictionary();
  hashmap_async_dictionary(const dictionary_t& d);

  void init(const dictionary_t& d) final;

  std::future<result_t> search(const char* word) const final;
  std::future<void>     insert(int document_id, gsl::span<const char*> text) final;
  std::future<void>     remove(int document_id) final;

  hashmap_dictionary m_dic;
private:
  mutable Thread_Pool thread_pool_;
};