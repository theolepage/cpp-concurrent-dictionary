#include "tools.hpp"
#include <vector>
#include <algorithm>
#include <numeric>
#include <random>
#include <fstream>
#include <spdlog/spdlog.h>


extern const char* WordListPath;


std::vector<std::string> load_word_list(const char* filename = nullptr, bool shuffle = true)
{
  if (!filename)
    filename = WordListPath;

  std::vector<std::string> data;

  {
    std::ifstream f(filename);

    if (!f.good())
    {
      spdlog::error("Unable to load the resource file: '{}'", filename);
      std::abort();
    }

    for (std::string s; std::getline(f, s);)
      data.push_back(s);
  }

  if (shuffle)
  {
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(data.begin(), data.end(), g);
  }

  return data;
}





struct query_t
{
  enum op_type
  {
    search,
    insert,
    erase
  };

  op_type     op;
  int         arg;
};


struct Scenario::scenario_impl_t
{
  const std::string*                    words;
  std::vector<int>                      word_count;
  std::vector<int>                      doc_ids;
  std::vector<std::vector<const char*>> texts;
  std::vector<query_t>                  queries;
  Scenario::param_t                     param;
};


namespace
{
  /*
  std::string word_modify(const std::string& s, std::mt19937& gen)
  {
    std::string res;
    std::uniform_real_distribution<> rg;

    std::size_t pos = 0;
    std::size_t n = s.size();
    while (pos < n)
    {
      auto x = rg(gen);
      if (x < 0.8)
        res.push_back((char) s[pos++]);
      else if (x < 0.85) // deletion
        pos++;
      else if (x < 0.9) // insertion
        res.push_back((char) ('a' + 26 * ((x - 0.85) / 0.05)  ));
      else if (x < 0.95) // mutation
      {
        res.push_back((char) ('a' + 26 * ((x - 0.9) / 0.05) ));
        pos++;
      }
      else if (pos + 1 < n) //
      {
        res.append({s[pos+1], s[pos]});
        pos += 2;
      }
    }

    return res;
  }
  */

}

namespace
{

  void generate_dataset(Scenario::param_t p, Scenario::scenario_impl_t* dst)
  {
    Expects(0.f <= p.word_redoundancy && p.word_redoundancy <= 1.f);
    Expects(0.f <= p.word_occupancy && p.word_occupancy <= 1.f);
    Expects(p.doc_count > 0);
    Expects(p.word_count > 0);
    Expects(p.n_queries > 0);

    static auto k_word_full_list = load_word_list();

    p.word_count = std::min(p.word_count, k_word_full_list.size());

    std::random_device rd;
    std::mt19937 gen(rd());

    // 1. Generate the documents
    std::uniform_int_distribution doc_gen(0, int(p.doc_count - 1));

    std::vector<std::vector<const char*>> texts(p.doc_count);
    std::vector<int>                      wc(p.word_count, 0);
    {
      std::bernoulli_distribution   g1(p.word_occupancy);
      float                         m = p.word_redoundancy * p.doc_count;
      std::normal_distribution      g2(m, m / 10.f);

      for (std::size_t word_id = 0; word_id < p.word_count; ++word_id)
        if (g1(gen))
        {
          int c = std::ceil(g2(gen));
          for (int i = 0; i < c; ++i) {
            texts[doc_gen(gen)].emplace_back(k_word_full_list[word_id].c_str());
            wc[word_id]++;
          }
        }
    }

    // Create a new document and returns its id
    std::uniform_int_distribution word_gen(0, int(p.word_count - 1));

    auto generate_new_doc = [&, m = int(p.word_redoundancy * (p.doc_count - 1) + 1.f)]() {
      auto& text = texts.emplace_back();
      for (int i = 0; i < m; ++i)
      {
        int word_id = word_gen(gen);
        text.emplace_back(k_word_full_list[word_id].c_str());
        wc[word_id]++;
      }
      return int(texts.size() - 1);
    };

    // 2. Generate actions
    //std::size_t n_searches = (std::size_t) std::round(p.n_queries / (p.ratio_indel + 1.f));
    //std::size_t n_indels = n_queries - n_searches;
    //std::size_t n_texts = p.doc_count + std::ceil(p.ratio_indel * n_queries);


    std::vector<query_t> queries(p.n_queries);
    {
      std::uniform_real_distribution<float> g;

      for (std::size_t i = 0; i < p.n_queries; ++i)
      {
        float r = g(gen);
        if (r > p.ratio_indel)
          queries[i] = {query_t::search, word_gen(gen)};
        else if (r <= (0.5f * p.ratio_indel) && !texts.empty())
        {
          doc_gen.param(decltype(doc_gen)::param_type{0, int(texts.size() - 1)});
          queries[i] = {query_t::erase, doc_gen(gen)};
        }
        else
          queries[i] = {query_t::insert, generate_new_doc()};
      }
    }

    // 3. Generate random ids for documents
    std::vector<int> ids(texts.size());
    std::generate_n(ids.data(), ids.size(), [&gen, g = std::uniform_int_distribution()]() mutable { return g(gen); });


    // 4. store the result
    dst->words      = k_word_full_list.data();
    dst->word_count = std::move(wc);
    dst->doc_ids    = std::move(ids);
    dst->texts      = std::move(texts);
    dst->queries    = std::move(queries);
    dst->param      = p;
  }
} // namespace

void Scenario::display_stats() const
{
  auto word_used = std::count_if(m_impl->word_count.begin(), m_impl->word_count.end(), [](int c) { return c > 0; });
  auto [w0, w1]  = std::minmax_element(m_impl->word_count.begin(), m_impl->word_count.end());
  auto w3 = std::accumulate(m_impl->word_count.begin(), m_impl->word_count.end(), 0) / float(m_impl->param.word_count);

  spdlog::info("===== Generated dataset =========");
  spdlog::info("\t Word count: used={} total={}", word_used, m_impl->param.word_count);
  spdlog::info("\t Document counts: {} (required: {})", m_impl->texts.size(), m_impl->param.doc_count);
  spdlog::info("\t Occupancy: {}% (required: {}%)", 100 * float(word_used) / float(m_impl->param.word_count),
               100 * m_impl->param.word_occupancy);
  spdlog::info("\t Redoundancy (word per text):");
  spdlog::info("\t\t min={}", *w0);
  spdlog::info("\t\t max={}", *w1);
  spdlog::info("\t\t avg={} ({}% required={})", w3, 100 * w3 / m_impl->texts.size(), 100 * m_impl->param.word_redoundancy);

  int n_search = 0;
  int n_deletion = 0;
  int n_insertion = 0;
  for (auto&& q : m_impl->queries)
    if (q.op == query_t::search) n_search++;
    else if (q.op == query_t::insert) n_insertion++;
    else n_deletion++;

  float total = n_search + n_deletion + n_insertion;
  float r = m_impl->param.ratio_indel;
  spdlog::info("\t Queries: total={}", total);
  spdlog::info("\t\t searches={}  ({}% req={}%)", n_search, n_search / total * 100, 100 * (1 - r));
  spdlog::info("\t\t insertions={} ({}% req={}%)", n_insertion, n_insertion / total * 100, 100 * 0.5 * r);
  spdlog::info("\t\t deletions={} ({}% req={}%)", n_deletion, n_deletion / total * 100, 100 * 0.5 * r);
 
}


Scenario::~Scenario()
{
}


Scenario::Scenario(const param_t& params)
  : m_impl(std::make_unique<scenario_impl_t>())
{
  generate_dataset(params, m_impl.get());
  this->display_stats();
}


const Scenario::param_t& Scenario::params() const
{
  return m_impl->param;
}

void Scenario::prepare(IReversedDictionaryBase& dic) const
{
  std::map<int, gsl::span<const char*>> data;

  for (std::size_t i = 0; i < m_impl->texts.size(); ++i)
    data[this->m_impl->doc_ids[i]] = gsl::make_span(this->m_impl->texts[i].data(), this->m_impl->texts[i].size());

  dic.init(data);
}


std::vector<result_t> Scenario::execute(IReversedDictionary& dic) const
{
  std::vector<result_t> results;
  results.reserve(this->m_impl->queries.size());

  for (auto&& q : this->m_impl->queries)
  {
    switch (q.op)
    {
    case query_t::search: results.push_back(dic.search(this->m_impl->words[q.arg].c_str())); break;
    case query_t::insert: dic.insert(this->m_impl->doc_ids[q.arg], this->m_impl->texts[q.arg]); break;
    case query_t::erase:  dic.remove(this->m_impl->doc_ids[q.arg]); break;
    }
  }

  return results;
}


std::vector<result_t> Scenario::execute(IAsyncReversedDictionary& dic,
                                int max_parallel_queries,
                                int max_parallel_read,
                                int max_parallel_write) const
{
  assert(max_parallel_queries > 0);
  assert(max_parallel_read > 0);
  assert(max_parallel_write > 0);

  std::vector<result_t> results;
  results.reserve(this->m_impl->param.n_queries);

  int READ_BUFFER_SIZE = max_parallel_read;
  int WRITE_BUFFER_SIZE = max_parallel_write;

  std::vector<std::future<result_t>> read_buffer(READ_BUFFER_SIZE);
  std::vector<std::future<void>>     write_buffer(WRITE_BUFFER_SIZE);

  int read_buffer_pos  = 0;
  int write_buffer_pos = 0;
  int qcount           = max_parallel_queries; // Number of rooms (queries) remaining

  for (auto&& q : this->m_impl->queries)
  {
    if (qcount == 0)
    {
      int m = std::max(READ_BUFFER_SIZE, WRITE_BUFFER_SIZE);
      for (int i = 0; i < m && qcount == 0; ++i)
      {
        int rp = (read_buffer_pos + i) % READ_BUFFER_SIZE;
        int wp = (write_buffer_pos + i) % WRITE_BUFFER_SIZE;

        if (read_buffer[rp].valid())
        {
          results.push_back(read_buffer[rp].get());
          qcount++;
        }
        if (write_buffer[wp].valid())
        {
          write_buffer[wp].get();
          qcount++;
        }
      }
    }


    if (q.op == query_t::search)
    {
      if (read_buffer[read_buffer_pos].valid())
      {
        results.push_back(read_buffer[read_buffer_pos].get());
        qcount++;
      }
    }
    else
    {
      if (write_buffer[write_buffer_pos].valid())
      {
        write_buffer[write_buffer_pos].get();
        qcount++;
      }
    }

    switch (q.op)
    {
    case query_t::search:
      read_buffer[read_buffer_pos] = dic.search(m_impl->words[q.arg].c_str()); // non-blocking
      read_buffer_pos = (read_buffer_pos + 1) % READ_BUFFER_SIZE;
      break;
    case query_t::insert:
      write_buffer[write_buffer_pos] = dic.insert(m_impl->doc_ids[q.arg], m_impl->texts[q.arg]);
      write_buffer_pos = (write_buffer_pos + 1) % WRITE_BUFFER_SIZE;
      break;
    case query_t::erase:
      write_buffer[write_buffer_pos] = dic.remove(m_impl->doc_ids[q.arg]);
      write_buffer_pos = (write_buffer_pos + 1) % WRITE_BUFFER_SIZE;
      break;
    }
    qcount--;
  }

  for (int i = 0; i < READ_BUFFER_SIZE; ++i)
  {
    if (read_buffer[read_buffer_pos].valid())
      results.push_back(read_buffer[read_buffer_pos].get());
    read_buffer_pos = (read_buffer_pos + 1) % READ_BUFFER_SIZE;
  }
  return results;
}
