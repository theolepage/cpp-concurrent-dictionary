#include "tree_async_dictionary.hpp"
#include <iostream>
#include "cptl/ctpl_stl.h"

Tree_Async_Dictionary::Tree_Async_Dictionary()
{

}

Tree_Async_Dictionary::Tree_Async_Dictionary(const dictionary_t& d)
{
  m_dic.init(d);
}

void Tree_Async_Dictionary::init(const dictionary_t& d)
{
  m_dic.init(d);
}

// TODO put back const if needed
std::future<result_t> Tree_Async_Dictionary::search(const char* query)
{
  // TODO Find a way to move promise inside the lambda, current problem say it's const
  auto p = new std::promise<result_t>;
  auto futur = p->get_future();
  if (counter % 2 == 0)
  {
    thread_pool_one.run([this, query, p]()
    {
      p->set_value(this->m_dic.search(query));
      delete p;
    });
  }
  else
  {
    thread_pool_two.run([this, query, p]()
    {
      p->set_value(this->m_dic.search(query));
      delete p;
    });
  }
  ++counter;

  return futur;
}


std::future<void> Tree_Async_Dictionary::insert(int doc_id, gsl::span<const char*> text)
{
  std::promise<void> p;
  m_dic.insert(doc_id, text);
  p.set_value();
  return p.get_future();
}

std::future<void> Tree_Async_Dictionary::remove(int doc_id)
{
  std::promise<void> p;
  m_dic.remove(doc_id);
  p.set_value();
  return p.get_future();
}

