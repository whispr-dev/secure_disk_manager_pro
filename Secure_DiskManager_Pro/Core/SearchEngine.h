#ifndef SEARCH_ENGINE_H
#define SEARCH_ENGINE_H

#include <string>
#include <vector>

struct SearchResult {
    std::string path;
    double entropyScore;
};

class SearchEngine {
public:
    static std::vector<SearchResult> findMatchingFiles(
        const std::string& rootPath,
        const std::string& pattern,
        bool useEntropy = false
    );

    static void printResults(const std::vector<SearchResult>& results);
};

#endif
