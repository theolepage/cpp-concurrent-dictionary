#include "hashmap_dictionary.hpp"
#include <algorithm>


hashmap_dictionary::hashmap_dictionary(const dictionary_t& d)
{
  this->_init(d);
}

void hashmap_dictionary::init(const dictionary_t& d)
{
  this->_init(d);
}

void hashmap_dictionary::_init(const dictionary_t& d)
{
  for (auto&& [id, text] : d)
    for (auto&& word : text)
    {
      m_dico[id].insert(word);
      m_rev_dico[word].insert(id);
    }
}

result_t hashmap_dictionary::search(const char* word) const
{
  std::lock_guard l(m);

  result_t r;

  auto itemptr = m_rev_dico.find(word);
  if (itemptr == m_rev_dico.end())
    return r;

  auto& item = itemptr->second;
  r.m_count = std::min(int(item.size()), MAX_RESULT_COUNT);
  std::copy_n(item.begin(), r.m_count, r.m_matched);
  return r;
}


void hashmap_dictionary::insert(int document_id, gsl::span<const char*> text)
{
  std::lock_guard l(m);

  for (auto&& word : text)
  {
    m_dico[document_id].insert(word);
    m_rev_dico[word].insert(document_id);
  }
}

void hashmap_dictionary::remove(int document_id)
{
  std::lock_guard l(m);

  /*
    std::lock_guard l(m);

  auto entry = m_dico.find(document_id);
  if (entry == std::nullopt)
    return;

  for (const auto& w : entry.value())
    m_rev_dico[w].erase(document_id);

  m_dico.erase(document_id);
  */

  auto entry = m_dico.find(document_id);
  if (entry == m_dico.end())
    return;

  for (const auto& w : entry->second)
    m_rev_dico[w].erase(document_id);

  m_dico.erase(entry);
}