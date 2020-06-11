#pragma once
#include "IDictionary.hpp"
#include "IAsyncDictionary.hpp"
#include <string>
#include <vector>
#include <memory>


class Scenario
{
public:
  struct param_t
  {
    /** Dataset and Text configuration **/

    /* Number of words in the language 𝓛 */
    std::size_t word_count;

    /* Initial number of documents */
    std::size_t doc_count;

    /* Word redoundancy: [0-1]: average number of documents (in pourcent) that contain the same word*/
    /* 10% = a word is (on avarage) available in 10% of the documents */
    float word_redoundancy;

    /* Word occupancy: [0-1]: pourcentage of the langage in documents  */
    /* 100% = every single word is (at least) in a document (on average)*/
    float word_occupancy;

    /***** Scenario config *****/

    /* Number of queries
     * For searches:
     *   Words will be picked randomly from 𝓛
     *   The hit/miss ratio is related to \p word_occupancy
     * For insertion/deletion
    */
    std::size_t n_queries;

    /* Ratio [0-1]: n_indel/n_queries
       10% means: 1 doc insertion/deletion for 9 searches
     */
    float ratio_indel;
  };


  ~Scenario();

  // Create a scenario from a word list
  Scenario(const param_t& params);

  // Prepare and populate the dictionary
  void prepare(IReversedDictionaryBase& dic) const;

  // Execute a scenario sync-way
  std::vector<result_t> execute(IReversedDictionary& dic) const;

  // Execute a scenario async-way
  std::vector<result_t> execute(IAsyncReversedDictionary& dic) const;

  // Get the scenario parameters
  const param_t& params() const;

  // Print the statistics of the scenario
  void display_stats() const;

  struct scenario_impl_t;
private:
  std::unique_ptr<scenario_impl_t> m_impl;
};




