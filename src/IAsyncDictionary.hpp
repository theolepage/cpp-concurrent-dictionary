#pragma once
#include "IDictionary.hpp"
#include <future>

class IAsyncReversedDictionary : public IReversedDictionaryBase
{
public:
  IAsyncReversedDictionary() = default;

  virtual std::future<result_t> search(const char* query) const                   = 0;
  virtual std::future<void>     insert(int document_id, gsl::span<const char*> text) = 0;
  virtual std::future<void>     remove(int document_id)                              = 0;
};
