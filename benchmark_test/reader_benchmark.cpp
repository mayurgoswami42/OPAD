#include <benchmark/benchmark.h>
#include <vector>

#include "../cpp/io_manager/reader.hpp"

static void BM_READER(benchmark::State &state)
{
    Reader reader(false);
    
    for (auto _ : state)
        std::vector<std::string> lines = reader.read_logs("../../benchmark_test/sample.log");

    state.SetItemsProcessed(state.iterations() * 20000);
}

BENCHMARK(BM_READER);