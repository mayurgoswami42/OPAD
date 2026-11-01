#include <benchmark/benchmark.h>
#include "../cpp/parser/parser.hpp"
#include "../cpp/detector/detector.hpp"

static void BM_DETECTOR(benchmark::State &state)
{
    Parser parser("(date)(time)(log_type)(user_ip)(method)(path)(protocol)(version)(status)(message)", R"re((\d{4}-\d{2}-\d{2})\s+((?:\d{2}:){2}\d{2},\d{3})\s+(INFO|ERROR|WARN|FATAL)\s+((?:\d{1,3}\.){3}\d{1,3})\s+"(GET|POST|PUT|DELETE|PATCH)\s+(\S+)\s+((?:HTTP|HTTPS)/(\d(?:\.\d)?))"\s+(\d{3})\s+(\S+))re");
    std::unordered_map<std::string, std::string> parsed_log = parser.parse("2026-09-16 11:53:18,605 INFO 218.117.15.101 \"GET /fetch HTTP/1.1\" 404 -");

    Detector detector(50000, 5000, 30);

    for (auto _ : state)
    {
        for (int i = 0; i < 100000; ++i)
            ((detector.*detector.insert)(parsed_log));
    }

    state.SetItemsProcessed(state.iterations() * 100000);
}

BENCHMARK(BM_DETECTOR);