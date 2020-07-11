#pragma once

#include <vector>
#include <shared_mutex>
#include <algorithm>

struct Sub_node
{
    // TODO with vector no risk of duplicate ?
    using book_set = std::vector<int>;

    void insert(int book)
    {
        std::unique_lock l(m);
        books.emplace_back(book);
    }

    void erase(int book)
    {
        std::unique_lock l(m);
        books.erase(std::remove(books.begin(), books.end(), book), books.end());
    }

    void read_books(result_t& r)
    {
        std::shared_lock l(m);
        r.m_count = std::min(int(books.size()), MAX_RESULT_COUNT);
        std::copy_n(books.begin(), r.m_count, r.m_matched);
    }

    mutable std::shared_mutex m;
    book_set books;
};