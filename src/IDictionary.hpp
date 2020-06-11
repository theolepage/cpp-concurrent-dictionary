#pragma once

#include <vector>
#include <utility>
#include <map>
#include <gsl/gsl-lite.hpp>

constexpr int MAX_RESULT_COUNT = 10;

// Structure of a match
struct match_t
{
  match_t() = default;
  match_t(int id) : m_id(id) {}

  // The document id having a match
  int id() const { return m_id; }

  bool operator==(const match_t& other) const { return m_id == other.m_id; }
  bool operator!=(const match_t& other) const { return m_id != other.m_id; }

  int m_id;
};

// Structure for a result set
struct result_t
{
  // Number of document that have a match in the database
  int     count() const { return m_count; }

  // Access to the i-th match
  match_t item(int i) const { return m_matched[i]; }

  gsl::span<const match_t> items() const { return gsl::make_span(m_matched, m_count); }

  bool operator==(const result_t& other) const { return this->items() == other.items(); }
  bool operator!=(const result_t& other) const { return this->items() != other.items(); }

  int     m_count = 0;
  match_t m_matched[MAX_RESULT_COUNT];
};


// Each entry points to a contiguous array of c-string pointers
using text_t = gsl::span<const char*>;
using dictionary_t = std::map<int, gsl::span<const char*>>;



class IReversedDictionaryBase
{
public:
  // Reset the database from \p d
  virtual void init(const dictionary_t& d) = 0;
};

class IReversedDictionary : public IReversedDictionaryBase
{
public:

  /// Each of these methods SHALL be able to run concurrently
  /// \{

  /// Search the documents containing \p word in the database
  virtual result_t search(const char* word) const                             = 0;

  /// Insert a new document in the database
  /// If it already exists, nothing to do
  /// The same word may appear several time in text
  virtual void     insert(int document_id, gsl::span<const char*> text) = 0;

  /// Remove a document
  virtual void     remove(int document_id)                                    = 0;

  /// \}
};
