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

    const auto dicos = m_rev_dico.find(word);
    if (!dicos.has_value())
        return r;

    r.m_count = std::min(int(dicos.value().size()), MAX_RESULT_COUNT);
    std::copy_n(dicos.value().begin(), r.m_count, r.m_matched);
    return r;
}

void hashmap_dictionary::insert(int document_id, gsl::span<const char*> text)
{
    auto node = m_dico[document_id];
    if (node != nullptr)
        return;

    for (const char* word : text)
    {
        m_dico.insert_value(node, document_id, word);
        m_rev_dico.insert_value(word, document_id);
    }
 }

void hashmap_dictionary::remove(int document_id)
{
    const auto words = m_dico.find(document_id);
    if (!words.has_value())
        return;

    for (const auto& w : words.value())
        m_rev_dico.remove_value(w, document_id);
    m_dico.remove(document_id);
}