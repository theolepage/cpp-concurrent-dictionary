#include <functional>
#include "tools.hpp"
#include "naive_dictionary.hpp"
#include "naive_async_dictionary.hpp"
#include "tree_dictionary.hpp"
#include "tree_async_dictionary.hpp"
#include "hashmap_dictionary.hpp"

#include <benchmark/benchmark.h>


class BMScenario : public ::benchmark::Fixture
{
public:
  void SetUp(benchmark::State& )
    {
      if (!m_scenario)
      {
        Scenario::param_t params;
        params.word_count       = 10000;
        params.doc_count        = 100;
        params.word_redoundancy = 0.3f;
        params.word_occupancy   = 0.9f;
        params.n_queries        = 1000000;
        params.ratio_indel      = 0.0;


        m_scenario = std::make_unique<Scenario>(params);
      }
    }

protected:
  static std::unique_ptr<Scenario> m_scenario;
};

std::unique_ptr<Scenario> BMScenario::m_scenario;



BENCHMARK_DEFINE_F(BMScenario, Naive_NoAsync)(benchmark::State& st)
{
  naive_dictionary dic;
  m_scenario->prepare(dic);

  for (auto _ : st)
    m_scenario->execute(dic);

  st.SetItemsProcessed(st.iterations() * m_scenario->params().n_queries);
}

BENCHMARK_DEFINE_F(BMScenario, Tree_NoAsync)(benchmark::State& st)
{
    Tree_Dictionary dic;
    m_scenario->prepare(dic);

    for (auto _ : st)
        m_scenario->execute(dic);

    st.SetItemsProcessed(st.iterations() * m_scenario->params().n_queries);
}

BENCHMARK_DEFINE_F(BMScenario, Hashmap_NoAsync)(benchmark::State& st)
{
  hashmap_dictionary dic;
  m_scenario->prepare(dic);

  for (auto _ : st)
    m_scenario->execute(dic);

  st.SetItemsProcessed(st.iterations() * m_scenario->params().n_queries);
}

BENCHMARK_DEFINE_F(BMScenario, Naive_Async)(benchmark::State& st)
{
  naive_async_dictionary dic;
  m_scenario->prepare(dic);

  for (auto _ : st)
    m_scenario->execute(dic);

  st.SetItemsProcessed(st.iterations() * m_scenario->params().n_queries);
}

BENCHMARK_DEFINE_F(BMScenario, Tree_Async)(benchmark::State& st)
{
  Tree_Async_Dictionary dic;
  m_scenario->prepare(dic);

  for (auto _ : st)
    m_scenario->execute(dic);

  st.SetItemsProcessed(st.iterations() * m_scenario->params().n_queries);
}

BENCHMARK_REGISTER_F(BMScenario, Naive_NoAsync)
    ->Unit(benchmark::kMillisecond) //
    ->UseRealTime();

BENCHMARK_REGISTER_F(BMScenario, Naive_Async)
    ->Unit(benchmark::kMillisecond) //
    ->UseRealTime();
    
BENCHMARK_REGISTER_F(BMScenario, Tree_NoAsync)
    ->Unit(benchmark::kMillisecond) //
    ->UseRealTime();

BENCHMARK_REGISTER_F(BMScenario, Tree_Async)
    ->Unit(benchmark::kMillisecond) //
    ->UseRealTime();

BENCHMARK_REGISTER_F(BMScenario, Hashmap_NoAsync)
    ->Unit(benchmark::kMillisecond) //
    ->UseRealTime();

BENCHMARK_MAIN();
