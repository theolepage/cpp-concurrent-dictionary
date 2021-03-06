#pragma once
#include "../IDictionary.hpp"
#include <unordered_set>
#include <mutex>
#include <vector>

#include "hashmap.hpp"

class hashmap_dictionary : public IReversedDictionary
{
public:
  hashmap_dictionary() = default;
  hashmap_dictionary(const dictionary_t& init);

  template <class Iterator>
  hashmap_dictionary(Iterator begin, Iterator end) { _init({begin, end}); }

  virtual void     init(const dictionary_t& d) final;
  virtual result_t search(const char* word) const final;
  virtual void     insert(int document_id, gsl::span<const char*> text) final;
  virtual void     remove(int document_id) final;


private:
  void _init(const dictionary_t& d);

  hashmap<int, std::vector<std::string>> m_dico;
  hashmap<std::string, std::vector<int>> m_rev_dico;
};