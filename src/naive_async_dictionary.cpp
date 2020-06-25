#include "naive_async_dictionary.hpp"



naive_async_dictionary::naive_async_dictionary(const dictionary_t& d)
{
  m_dic.init(d);
}

void naive_async_dictionary::init(const dictionary_t& d)
{
  m_dic.init(d);
}

std::future<result_t> naive_async_dictionary::search(const char* query)
{
  std::promise<result_t> p;
  p.set_value(m_dic.search(query));
  return p.get_future();
}


std::future<void> naive_async_dictionary::insert(int doc_id, gsl::span<const char*> text)
{
  std::promise<void> p;
  m_dic.insert(doc_id, text);
  p.set_value();
  return p.get_future();
}

std::future<void> naive_async_dictionary::remove(int doc_id)
{
  std::promise<void> p;
  m_dic.remove(doc_id);
  p.set_value();
  return p.get_future();
}

