#pragma once

#include <vector>

#include "IDictionary.hpp"

class Node
{
public:
    Node(char letter) : letter_(letter) {}

    void add_child(char letter)
    {
        children_.emplace_back(Node(letter));
    }

    bool operator==(char l) const
    {
        return letter_ == l;
    }

    std::vector<int> get_books()
    {
        return books_;
    }

private:
    char letter_;
    std::vector<Node> children_;
    std::vector<int> books_;
    bool is_leaf;
};

class Tree_Dictionary : public IReversedDictionary
{
public:
  Tree_Dictionary() = default;
  Tree_Dictionary(const dictionary_t& init);

  template <class Iterator>
  Tree_Dictionary(Iterator begin, Iterator end) { _init({begin, end}); }

  virtual void     init(const dictionary_t& d) final;
  virtual result_t search(const char* word) const final;
  virtual void     insert(int document_id, gsl::span<const char*> text) final;
  virtual void     remove(int document_id) final;


private:
  void _init(const dictionary_t& d);

  Node root_;
};