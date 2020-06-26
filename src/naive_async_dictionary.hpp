#pragma once

#include "IAsyncDictionary.hpp"
#include "naive_dictionary.hpp"


/// The naive implementation is blocking to ensure Sequential Consistency
class naive_async_dictionary : public IAsyncReversedDictionary
{
public:
  naive_async_dictionary() = default;
  naive_async_dictionary(const dictionary_t& d);

  void init(const dictionary_t& d) final;

  std::future<result_t> search(const char* word) const final;
  std::future<void>     insert(int document_id, gsl::span<const char*> text) final;
  std::future<void>     remove(int document_id) final;


private:
  naive_dictionary m_dic;
};