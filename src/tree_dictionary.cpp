#include "tree_dictionary.hpp"
#include "IDictionary.hpp"

Tree_Dictionary::Tree_Dictionary(const dictionary_t& d)
{
  this->_init(d);
}

void Tree_Dictionary::init(const dictionary_t& d)
{
  this->_init(d);
}

std::optional<std::vector> recur_search(const char* word, Node root)
{
    if (*word == '\0')
    {
        return node.get_books();
    }
    for (const Node& node : root)
    {
        if (*word == node)
        {
            return recur_search(word + 1, node);
        }
    }
}

std::optional<std::vector&> _search(const char* word)
{
    return recur_search(word, root);
}

void Tree_Dictionary::_init(const dictionary_t& d)
{
    for (const auto& [book, words] : d)
    {
        for (const auto& word : words)
        {
            std::optional<std::vector&> result_search = _search(word);
            if (!result_search.has_value())
            {
                create_word(word, book);
            }
            else
            {
                result_search.emplace_back(book);
            }
        }
    }
}

result_t naive_dictionary::search(const char* word) const
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



void naive_dictionary::insert(int document_id, gsl::span<const char*> text)
{
  std::lock_guard l(m);

  for (auto&& word : text)
  {
    m_dico[document_id].insert(word);
    m_rev_dico[word].insert(document_id);
  }
}


void naive_dictionary::remove(int document_id)
{
  std::lock_guard l(m);

  auto entry = m_dico.find(document_id);
  if (entry == m_dico.end())
    return;

  for (const auto& w : entry->second)
    m_rev_dico[w].erase(document_id);

  m_dico.erase(entry);
}