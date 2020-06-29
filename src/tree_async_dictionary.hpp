#pragma once

#include "IAsyncDictionary.hpp"
#include "tree_dictionary.hpp"
#include <tbb/task_group.h>
#include "thread_pool.hpp"

/// The naive implementation is blocking to ensure Sequential Consistency
class Tree_Async_Dictionary : public IAsyncReversedDictionary
{
public:
  Tree_Async_Dictionary();
  Tree_Async_Dictionary(const dictionary_t& d);

  void init(const dictionary_t& d) final;

  std::future<result_t> search(const char* word) const final;
  std::future<void>     insert(int document_id, gsl::span<const char*> text) final;
  std::future<void>     remove(int document_id) final;


  Tree_Dictionary m_dic;
private:

  mutable Thread_Pool thread_pool_;
};