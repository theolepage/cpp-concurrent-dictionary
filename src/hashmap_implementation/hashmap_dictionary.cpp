#include "hashmap_dictionary.hpp"
#include <algorithm>
#include <iostream>

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
    {
        for (auto&& word : text)
        {
            m_dico.insert_value(id, word);
            m_rev_dico.insert_value(word, id);
        }
    }
}

result_t hashmap_dictionary::search(const char* word) const
{
    result_t r;

    const auto dicos = m_rev_dico.find_value_copy(word);
    if (!dicos.has_value())
        return r;

    r.m_count = std::min(int(dicos.value().size()), MAX_RESULT_COUNT);
    std::copy_n(dicos.value().begin(), r.m_count, r.m_matched);
    return r;
}

void hashmap_dictionary::insert(int document_id, gsl::span<const char*> text)
{
    if (m_dico.find_node_unlocked(document_id) != nullptr)
        return;

    auto node = m_dico.create_node(document_id);
    for (const char* word : text)
    {
        node->get_value()->emplace_back(word);

        m_rev_dico.insert_value(word, document_id);
    }

    node->get_mutex().unlock();
 }

void hashmap_dictionary::remove(int document_id)
{
    const auto words = m_dico.find_value_copy(document_id);
    if (!words.has_value())
        return;

    for (const auto& w : words.value())
        m_rev_dico.remove_value(w, document_id);
    m_dico.remove(document_id);
}
