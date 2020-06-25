#pragma once

#include "IAsyncDictionary.hpp"
#include "tree_dictionary.hpp"
#include <tbb/task_group.h>
#include "cptl/ctpl_stl.h"

/// The naive implementation is blocking to ensure Sequential Consistency
class Tree_Async_Dictionary : public IAsyncReversedDictionary
{
public:
  Tree_Async_Dictionary();
  Tree_Async_Dictionary(const dictionary_t& d);

  void init(const dictionary_t& d) final;

  std::future<result_t> search(const char* word) final;
  std::future<void>     insert(int document_id, gsl::span<const char*> text) final;
  std::future<void>     remove(int document_id) final;


  Tree_Dictionary m_dic;
private:

  tbb::task_group thread_pool_one;
  tbb::task_group thread_pool_two;
  int counter;
};