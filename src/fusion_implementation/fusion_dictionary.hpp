#pragma once

#include <memory>
#include <shared_mutex>
#include <tbb/concurrent_hash_map.h>
#include <unordered_set>
#include <utility>
#include <vector>

#include "../IDictionary.hpp"
#include "../trie_implementation/node.hpp"
#include "../hashmap_implementation/hashmap.hpp"


class Fusion_Dictionary : public IReversedDictionary
{
public:
    using delete_map_own = hashmap<int, std::vector<std::shared_ptr<Sub_node>>>;

    Fusion_Dictionary();
    Fusion_Dictionary(const dictionary_t& init);

    template <class Iterator>
    Fusion_Dictionary(Iterator begin, Iterator end)
    {
        _init({begin, end});
    }

    virtual void init(const dictionary_t& d) final;
    virtual result_t search(const char* word) const final;
    virtual void insert(int document_id, gsl::span<const char*> text) final;
    virtual void remove(int document_id) final;
    void _add_word(const char* word, int book);
    void _add_word(const char* word, int book,
                   std::shared_ptr<std::vector<std::shared_ptr<Sub_node>>> vect);

    // TODO private
    Node root_;

    delete_map_own book_Sub_nodes_own_;

private:
    void _init(const dictionary_t& d);
    void _search_word(const char* word, result_t& r) const;
    void _remove(int document_id);
};
