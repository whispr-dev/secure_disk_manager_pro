#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <chrono>
#include <future>
#include <queue>
#include <regex>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <random>
#include <memory>
#include <functional>
#include <curl/curl.h>
#include <json/json.h>

// Forward declarations
class ProxyInfo;
class ProxyScanManager;
class OptimizerIntegration;

// =====================================================================
// CONFIGURATION CONSTANTS
// =====================================================================
namespace ProxyConfig {
    constexpr int DEFAULT_CHECK_INTERVAL = 30;
    constexpr int DEFAULT_CONNECTION_LIMIT = 200;
    constexpr int DEFAULT_VALIDATION_ROUNDS = 3;
    constexpr double DEFAULT_TIMEOUT = 5.0;
    constexpr int DEFAULT_EXPORT_INTERVAL = 300;
    constexpr int DEFAULT_PROXY_REFRESH_MIN_INTERVAL = 300;
    constexpr int DEFAULT_MAX_PROXIES_TO_KEEP = 10000;
    constexpr int DEFAULT_MAX_FETCH_ATTEMPTS = 5;
    constexpr int DEFAULT_FETCH_RETRY_DELAY = 5;
    
    const std::string DEFAULT_TEST_URL = "http://ip-api.com/json";
    
    const std::vector<std::string> DEFAULT_PROXY_SOURCES = {
        "https://raw.githubusercontent.com/TheSpeedX/PROXY-List/master/http.txt",
        "https://raw.githubusercontent.com/clarketm/proxy-list/master/proxy-list-raw.txt",
        "https://raw.githubusercontent.com/sunny9577/proxy-scraper/master/proxies.txt"
    };
    
    const std::vector<std::string> USER_AGENTS = {
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36",
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Edge/91.0.864.59 Safari/537.36",
        "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.114 Safari/537.36",
        "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.101 Safari/537.36",
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:89.0) Gecko/20100101 Firefox/89.0",
        "Mozilla/5.0 (Macintosh; Intel Mac OS X 10.15; rv:89.0) Gecko/20100101 Firefox/89.0"
    };
}

// =====================================================================
// UTILITY CLASSES AND STRUCTS
// =====================================================================

enum class ValidationMode {
    ANY,
    MAJORITY,
    ALL
};

enum class AnonymityLevel {
    UNKNOWN,
    TRANSPARENT,
    ANONYMOUS,
    ELITE
};

struct CurlResponse {
    std::string data;
    long response_code = 0;
    double total_time = 0.0;
    
    void clear() {
        data.clear();
        response_code = 0;
        total_time = 0.0;
    }
};

// CURL write callback function
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, CurlResponse* response) {
    size_t total_size = size * nmemb;
    response->data.append(static_cast<char*>(contents), total_size);
    return total_size;
}

// =====================================================================
// PROXY INFO CLASS
// =====================================================================

class ProxyInfo {
public:
    std::string host;
    int port;
    std::string protocol;
    std::string country;
    double avg_response_time;
    double reliability;
    AnonymityLevel anonymity;
    std::chrono::time_point<std::chrono::steady_clock> last_tested;
    
    ProxyInfo(const std::string& h, int p, const std::string& proto = "http", 
              const std::string& c = "Unknown")
        : host(h), port(p), protocol(proto), country(c), 
          avg_response_time(0.0), reliability(0.0), 
          anonymity(AnonymityLevel::UNKNOWN),
          last_tested(std::chrono::steady_clock::now()) {}
    
    std::string getAddress() const {
        return host + ":" + std::to_string(port);
    }
    
    std::string getUrl() const {
        return protocol + "://" + host + ":" + std::to_string(port);
    }
    
    Json::Value toJson() const {
        Json::Value json;
        json["host"] = host;
        json["port"] = port;
        json["protocol"] = protocol;
        json["country"] = country;
        json["avg_response_time"] = avg_response_time;
        json["reliability"] = reliability;
        json["anonymity"] = static_cast<int>(anonymity);
        
        auto time_since_epoch = last_tested.time_since_epoch();
        auto seconds = std::chrono::duration_cast<std::chrono::seconds>(time_since_epoch);
        json["last_tested"] = static_cast<int64_t>(seconds.count());
        
        return json;
    }
    
    std::string toString() const {
        return getAddress();
    }
    
    bool operator==(const ProxyInfo& other) const {
        return host == other.host && port == other.port && protocol == other.protocol;
    }
};

// Hash function for ProxyInfo to use in unordered containers
struct ProxyInfoHash {
    std::size_t operator()(const ProxyInfo& proxy) const {
        return std::hash<std::string>{}(proxy.getAddress());
    }
};

// =====================================================================
// OPTIMIZER INTEGRATION CLASS
// =====================================================================

class OptimizerIntegration {
private:
    std::mt19937 rng;
    
public:
    OptimizerIntegration() : rng(std::random_device{}()) {}
    
    void initialize() {
        std::cout << "[OPTIMIZER] Initializing optimizer integration..." << std::endl;
        // Initialize any resources required by the optimizer
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cout << "[OPTIMIZER] Initialization complete." << std::endl;
    }
    
    struct OptimizedTestConfig {
        std::string url;
        std::unordered_map<std::string, std::string> headers;
        bool is_fast_track;
        bool use_binary_protocol;
    };
    
    OptimizedTestConfig optimizeProxyTest(const ProxyInfo& proxy_info) {
        OptimizedTestConfig config;
        config.url = ProxyConfig::DEFAULT_TEST_URL;
        config.headers = getRandomHeaders();
        
        // Example optimization logic
        std::uniform_int_distribution<int> dist(0, 1);
        config.is_fast_track = dist(rng) == 1;
        config.use_binary_protocol = dist(rng) == 1;
        
        return config;
    }
    
    void postTestProcessing(ProxyInfo& proxy_info, bool success, double response_time, const std::string& endpoint) {
        if (success) {
            proxy_info.reliability = std::min(100.0, proxy_info.reliability + 5.0);
            if (proxy_info.avg_response_time == 0.0) {
                proxy_info.avg_response_time = response_time;
            } else {
                proxy_info.avg_response_time = (proxy_info.avg_response_time + response_time) / 2.0;
            }
        } else {
            proxy_info.reliability = std::max(0.0, proxy_info.reliability - 10.0);
        }
        proxy_info.last_tested = std::chrono::steady_clock::now();
    }
    
    std::vector<std::string> exportSpecializedFormats(const std::vector<ProxyInfo>& proxies_to_export) {
        std::cout << "[OPTIMIZER] Exporting " << proxies_to_export.size() 
                  << " proxies to specialized formats..." << std::endl;
        // Placeholder for specialized export logic
        return {};
    }
    
private:
    std::unordered_map<std::string, std::string> getRandomHeaders() {
        std::uniform_int_distribution<size_t> dist(0, ProxyConfig::USER_AGENTS.size() - 1);
        
        return {
            {"User-Agent", ProxyConfig::USER_AGENTS[dist(rng)]},
            {"Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8"},
            {"Accept-Language", "en-US,en;q=0.5"},
            {"Accept-Encoding", "gzip, deflate, br"},
            {"DNT", "1"},
            {"Connection", "keep-alive"},
            {"Upgrade-Insecure-Requests", "1"},
            {"Cache-Control", "max-age=0"}
        };
    }
};

// =====================================================================
// PROXY SCAN MANAGER CLASS
// =====================================================================

class ProxyScanManager {
private:
    // Configuration
    int check_interval;
    int connection_limit;
    int validation_rounds;
    ValidationMode validation_mode;
    double timeout;
    bool check_anonymity;
    bool force_fetch;
    bool verbose;
    bool single_run;
    
    std::vector<std::string> proxy_sources;
    int proxy_test_workers;
    int proxy_fetch_interval;
    double proxy_test_timeout;
    int proxy_test_retries;
    int max_proxies_to_keep;
    int proxy_refresh_min_interval;
    std::string test_url;
    int export_interval;
    int max_fetch_attempts;
    int fetch_retry_delay;
    
    // Runtime state
    std::unordered_map<std::string, std::unique_ptr<ProxyInfo>> proxies;
    std::unordered_set<std::string> verified_proxies;
    std::queue<ProxyInfo*> proxy_test_queue;
    
    // Threading and synchronization
    std::mutex proxies_mutex;
    std::mutex verified_proxies_mutex;
    std::mutex queue_mutex;
    std::condition_variable queue_cv;
    std::atomic<bool> shutdown_requested{false};
    std::vector<std::thread> worker_threads;
    
    // CURL and optimizer
    OptimizerIntegration optimizer;
    
public:
    ProxyScanManager(int check_interval = ProxyConfig::DEFAULT_CHECK_INTERVAL,
                     int connection_limit = ProxyConfig::DEFAULT_CONNECTION_LIMIT,
                     int validation_rounds = ProxyConfig::DEFAULT_VALIDATION_ROUNDS,
                     ValidationMode validation_mode = ValidationMode::MAJORITY,
                     double timeout = ProxyConfig::DEFAULT_TIMEOUT,
                     bool check_anonymity = false,
                     bool force_fetch = false,
                     bool verbose = false,
                     bool single_run = false,
                     const std::vector<std::string>& proxy_sources = ProxyConfig::DEFAULT_PROXY_SOURCES,
                     int proxy_test_workers = 50,
                     int proxy_fetch_interval = 60,
                     double proxy_test_timeout = ProxyConfig::DEFAULT_TIMEOUT,
                     int proxy_test_retries = 3,
                     int max_proxies_to_keep = ProxyConfig::DEFAULT_MAX_PROXIES_TO_KEEP,
                     int proxy_refresh_min_interval = ProxyConfig::DEFAULT_PROXY_REFRESH_MIN_INTERVAL,
                     const std::string& test_url = ProxyConfig::DEFAULT_TEST_URL,
                     int export_interval = ProxyConfig::DEFAULT_EXPORT_INTERVAL,
                     int max_fetch_attempts = ProxyConfig::DEFAULT_MAX_FETCH_ATTEMPTS,
                     int fetch_retry_delay = ProxyConfig::DEFAULT_FETCH_RETRY_DELAY)
        : check_interval(check_interval), connection_limit(connection_limit),
          validation_rounds(validation_rounds), validation_mode(validation_mode),
          timeout(timeout), check_anonymity(check_anonymity), force_fetch(force_fetch),
          verbose(verbose), single_run(single_run), proxy_sources(proxy_sources),
          proxy_test_workers(proxy_test_workers), proxy_fetch_interval(proxy_fetch_interval),
          proxy_test_timeout(proxy_test_timeout), proxy_test_retries(proxy_test_retries),
          max_proxies_to_keep(max_proxies_to_keep), proxy_refresh_min_interval(proxy_refresh_min_interval),
          test_url(test_url), export_interval(export_interval), max_fetch_attempts(max_fetch_attempts),
          fetch_retry_delay(fetch_retry_delay) {}
    
    ~ProxyScanManager() {
        cleanup();
    }
    
    void initialize() {
        std::cout << "[SCANNER] Initializing CURL..." << std::endl;
        curl_global_init(CURL_GLOBAL_DEFAULT);
        
        std::cout << "[SCANNER] Initializing optimizer..." << std::endl;
        optimizer.initialize();
        
        std::cout << "[SCANNER] Fetching initial proxy lists..." << std::endl;
        fetchProxyLists();
        
        std::cout << "[SCANNER] Scanner initialized successfully." << std::endl;
    }
    
    void cleanup() {
        std::cout << "[SCANNER] Initiating cleanup..." << std::endl;
        
        shutdown_requested = true;
        queue_cv.notify_all();
        
        // Wait for all worker threads to finish
        for (auto& thread : worker_threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
        worker_threads.clear();
        
        curl_global_cleanup();
        std::cout << "[SCANNER] Cleanup complete." << std::endl;
    }
    
    void run() {
        try {
            std::cout << "[SCANNER] Initializing scanner..." << std::endl;
            initialize();
            
            std::cout << "[SCANNER] Starting scanning loop..." << std::endl;
            startScanningLoop();
            
        } catch (const std::exception& e) {
            std::cerr << "[SCANNER] Error during scanning: " << e.what() << std::endl;
        } catch (...) {
            std::cerr << "[SCANNER] Unknown error during scanning" << std::endl;
        }
    }
    
    // Public API methods for OS integration
    std::vector<ProxyInfo> getVerifiedProxies() {
        std::lock_guard<std::mutex> lock(verified_proxies_mutex);
        std::vector<ProxyInfo> result;
        
        for (const auto& address : verified_proxies) {
            std::lock_guard<std::mutex> proxy_lock(proxies_mutex);
            auto it = proxies.find(address);
            if (it != proxies.end()) {
                result.push_back(*(it->second));
            }
        }
        
        return result;
    }
    
    size_t getVerifiedProxyCount() {
        std::lock_guard<std::mutex> lock(verified_proxies_mutex);
        return verified_proxies.size();
    }
    
    void exportResults(bool force = false, const std::string& output_dir = "exported_proxies", 
                      const std::string& filename_prefix = "proxies") {
        std::lock_guard<std::mutex> lock(verified_proxies_mutex);
        
        if (verified_proxies.empty() && !force) {
            std::cout << "[EXPORT] No verified proxies to export." << std::endl;
            return;
        }
        
        // Create output directory
        std::filesystem::create_directories(output_dir);
        
        // Generate timestamp
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y%m%d_%H%M%S");
        std::string timestamp = ss.str();
        
        // Export to text file
        std::string txt_filename = output_dir + "/" + filename_prefix + "_" + timestamp + ".txt";
        std::ofstream txt_file(txt_filename);
        if (txt_file.is_open()) {
            for (const auto& address : verified_proxies) {
                txt_file << address << std::endl;
            }
            txt_file.close();
            std::cout << "[EXPORT] Exported " << verified_proxies.size() 
                      << " verified proxies to " << txt_filename << std::endl;
        }
        
        // Export to JSON file
        std::string json_filename = output_dir + "/" + filename_prefix + "_detailed_" + timestamp + ".json";
        std::ofstream json_file(json_filename);
        if (json_file.is_open()) {
            Json::Value json_array(Json::arrayValue);
            
            std::lock_guard<std::mutex> proxy_lock(proxies_mutex);
            for (const auto& address : verified_proxies) {
                auto it = proxies.find(address);
                if (it != proxies.end()) {
                    json_array.append(it->second->toJson());
                }
            }
            
            Json::StreamWriterBuilder builder;
            builder["indentation"] = "  ";
            std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
            writer->write(json_array, &json_file);
            json_file.close();
            
            std::cout << "[EXPORT] Exported detailed proxy info to " << json_filename << std::endl;
        }
    }

private:
    void startScanningLoop() {
        auto last_fetch_time = std::chrono::steady_clock::now();
        auto last_export_time = std::chrono::steady_clock::now();
        
        // Start worker threads
        for (int i = 0; i < proxy_test_workers; ++i) {
            worker_threads.emplace_back(&ProxyScanManager::proxyTestWorker, this);
        }
        
        while (!shutdown_requested) {
            auto current_time = std::chrono::steady_clock::now();
            
            // Fetch new proxies periodically
            auto time_since_fetch = std::chrono::duration_cast<std::chrono::seconds>(current_time - last_fetch_time);
            if (time_since_fetch.count() >= proxy_fetch_interval || force_fetch) {
                fetchProxyLists();
                last_fetch_time = current_time;
                force_fetch = false;
            }
            
            // Queue proxies for testing
            queueProxiesForTesting();
            
            // Export results periodically
            auto time_since_export = std::chrono::duration_cast<std::chrono::seconds>(current_time - last_export_time);
            if (time_since_export.count() >= export_interval) {
                exportResults();
                last_export_time = current_time;
            }
            
            if (single_run) {
                // Wait for all tests to complete
                std::this_thread::sleep_for(std::chrono::seconds(5));
                exportResults(true);
                break;
            }
            
            std::this_thread::sleep_for(std::chrono::seconds(check_interval));
        }
    }
    
    void fetchProxyLists() {
        std::cout << "[FETCH] Attempting to fetch proxy lists from " << proxy_sources.size() << " sources." << std::endl;
        
        int fetched_count = 0;
        for (const auto& source_url : proxy_sources) {
            std::cout << "[FETCH] Fetching from: " << source_url << std::endl;
            
            CURL* curl = curl_easy_init();
            if (!curl) continue;
            
            CurlResponse response;
            
            curl_easy_setopt(curl, CURLOPT_URL, source_url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, static_cast<long>(timeout));
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
            curl_easy_setopt(curl, CURLOPT_USERAGENT, ProxyConfig::USER_AGENTS[0].c_str());
            
            CURLcode res = curl_easy_perform(curl);
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response.response_code);
            curl_easy_cleanup(curl);
            
            if (res == CURLE_OK && response.response_code == 200) {
                auto new_proxies = parseProxyList(response.data, source_url);
                
                std::lock_guard<std::mutex> lock(proxies_mutex);
                for (const auto& [host, port] : new_proxies) {
                    std::string address = host + ":" + std::to_string(port);
                    if (proxies.find(address) == proxies.end()) {
                        proxies[address] = std::make_unique<ProxyInfo>(host, port);
                        fetched_count++;
                    }
                }
                
                std::cout << "[FETCH] Fetched " << new_proxies.size() << " proxies from " << source_url << std::endl;
            } else {
                std::cerr << "[FETCH] Failed to fetch from " << source_url << " (HTTP " << response.response_code << ")" << std::endl;
            }
        }
        
        std::cout << "[FETCH] Total new proxies added: " << fetched_count << std::endl;
    }
    
    std::vector<std::pair<std::string, int>> parseProxyList(const std::string& content, const std::string& source_url) {
        std::vector<std::pair<std::string, int>> proxies;
        std::regex proxy_regex(R"((\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}):(\d+))");
        std::sregex_iterator iter(content.begin(), content.end(), proxy_regex);
        std::sregex_iterator end;
        
        for (; iter != end; ++iter) {
            std::smatch match = *iter;
            try {
                std::string host = match[1].str();
                int port = std::stoi(match[2].str());
                proxies.emplace_back(host, port);
            } catch (const std::exception&) {
                // Skip invalid entries
            }
        }
        
        return proxies;
    }
    
    void queueProxiesForTesting() {
        auto current_time = std::chrono::steady_clock::now();
        
        std::lock_guard<std::mutex> lock(proxies_mutex);
        for (auto& [address, proxy_info] : proxies) {
            auto time_since_test = std::chrono::duration_cast<std::chrono::seconds>(current_time - proxy_info->last_tested);
            if (time_since_test.count() >= proxy_refresh_min_interval) {
                {
                    std::lock_guard<std::mutex> queue_lock(queue_mutex);
                    proxy_test_queue.push(proxy_info.get());
                }
                queue_cv.notify_one();
                proxy_info->last_tested = current_time; // Mark as queued
            }
        }
    }
    
    void proxyTestWorker() {
        while (!shutdown_requested) {
            ProxyInfo* proxy_info = nullptr;
            
            {
                std::unique_lock<std::mutex> lock(queue_mutex);
                queue_cv.wait(lock, [this] { return !proxy_test_queue.empty() || shutdown_requested; });
                
                if (shutdown_requested) break;
                
                if (!proxy_test_queue.empty()) {
                    proxy_info = proxy_test_queue.front();
                    proxy_test_queue.pop();
                }
            }
            
            if (proxy_info) {
                testProxy(*proxy_info);
            }
        }
    }
    
    void testProxy(ProxyInfo& proxy_info) {
        auto start_time = std::chrono::high_resolution_clock::now();
        
        // Get optimized test configuration
        auto test_config = optimizer.optimizeProxyTest(proxy_info);
        
        CURL* curl = curl_easy_init();
        if (!curl) return;
        
        CurlResponse response;
        std::string proxy_url = proxy_info.getUrl();
        
        curl_easy_setopt(curl, CURLOPT_URL, test_config.url.c_str());
        curl_easy_setopt(curl, CURLOPT_PROXY, proxy_url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, static_cast<long>(proxy_test_timeout));
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, test_config.headers["User-Agent"].c_str());
        
        // Set headers
        struct curl_slist* headers = nullptr;
        for (const auto& [key, value] : test_config.headers) {
            std::string header = key + ": " + value;
            headers = curl_slist_append(headers, header.c_str());
        }
        if (headers) {
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        }
        
        CURLcode res = curl_easy_perform(curl);
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response.response_code);
        curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &response.total_time);
        
        if (headers) {
            curl_slist_free_all(headers);
        }
        curl_easy_cleanup(curl);
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        double response_time = duration.count();
        
        bool success = (res == CURLE_OK && response.response_code == 200);
        
        if (success) {
            // Check anonymity if enabled
            if (check_anonymity) {
                proxy_info.anonymity = checkProxyAnonymity(proxy_info, response);
            }
            
            std::lock_guard<std::mutex> lock(verified_proxies_mutex);
            verified_proxies.insert(proxy_info.getAddress());
            
            if (verbose) {
                std::cout << "[VERIFIED] " << proxy_info.getAddress() 
                          << " (" << response_time << "ms)" << std::endl;
            }
        } else if (verbose) {
            std::cout << "[FAILED] " << proxy_info.getAddress() 
                      << " (CURL error: " << res << ")" << std::endl;
        }
        
        // Post-test processing through optimizer
        optimizer.postTestProcessing(proxy_info, success, response_time, test_config.url);
    }
    
    AnonymityLevel checkProxyAnonymity(const ProxyInfo& proxy_info, const CurlResponse& response) {
        // Simplified anonymity check based on response content
        // In a real implementation, this would be more sophisticated
        try {
            Json::Value json_data;
            Json::Reader reader;
            
            if (reader.parse(response.data, json_data)) {
                std::string detected_ip = json_data.get("query", "").asString();
                
                // If the detected IP matches the proxy IP, it's likely highly anonymous
                if (detected_ip == proxy_info.host) {
                    return AnonymityLevel::ELITE;
                }
                // If different IP detected, check for transparency indicators
                else if (!detected_ip.empty()) {
                    return AnonymityLevel::ANONYMOUS;
                }
            }
        } catch (const std::exception&) {
            // If we can't parse the response, assume transparent
        }
        
        return AnonymityLevel::TRANSPARENT;
    }
};

// =====================================================================
// OS INTEGRATION API
// =====================================================================

class ProxyScannerAPI {
private:
    std::unique_ptr<ProxyScanManager> scanner;
    std::thread scanner_thread;
    std::atomic<bool> is_running{false};
    
public:
    ProxyScannerAPI() = default;
    
    ~ProxyScannerAPI() {
        stop();
    }
    
    void start(const std::vector<std::string>& proxy_sources = ProxyConfig::DEFAULT_PROXY_SOURCES,
               int workers = 50, bool verbose = false) {
        if (is_running) return;
        
        scanner = std::make_unique<ProxyScanManager>(
            30,  // check_interval
            200, // connection_limit
            3,   // validation_rounds
            ValidationMode::MAJORITY,
            5.0, // timeout
            true, // check_anonymity
            false, // force_fetch
            verbose,
            false, // single_run (continuous mode for OS integration)
            proxy_sources,
            workers
        );
        
        scanner_thread = std::thread([this]() {
            scanner->run();
        });
        
        is_running = true;
        std::cout << "[API] Proxy scanner started in background." << std::endl;
    }
    
    void stop() {
        if (!is_running) return;
        
        is_running = false;
        if (scanner) {
            scanner.reset(); // This will trigger cleanup in destructor
        }
        
        if (scanner_thread.joinable()) {
            scanner_thread.join();
        }
        
        std::cout << "[API] Proxy scanner stopped." << std::endl;
    }
    
    std::vector<ProxyInfo> getWorkingProxies() {
        if (!scanner) return {};
        return scanner->getVerifiedProxies();
    }
    
    size_t getWorkingProxyCount() {
        if (!scanner) return 0;
        return scanner->getVerifiedProxyCount();
    }
    
    std::vector<std::string> getWorkingProxyList() {
        std::vector<std::string> result;
        if (!scanner) return result;
        
        auto proxies = scanner->getVerifiedProxies();
        result.reserve(proxies.size());
        
        for (const auto& proxy : proxies) {
            result.push_back(proxy.getUrl());
        }
        
        return result;
    }
    
    std::vector<std::string> getWorkingProxyAddresses() {
        std::vector<std::string> result;
        if (!scanner) return result;
        
        auto proxies = scanner->getVerifiedProxies();
        result.reserve(proxies.size());
        
        for (const auto& proxy : proxies) {
            result.push_back(proxy.getAddress());
        }
        
        return result;
    }
    
    // Get proxies filtered by criteria
    std::vector<ProxyInfo> getProxiesByCountry(const std::string& country) {
        std::vector<ProxyInfo> result;
        if (!scanner) return result;
        
        auto all_proxies = scanner->getVerifiedProxies();
        for (const auto& proxy : all_proxies) {
            if (proxy.country == country) {
                result.push_back(proxy);
            }
        }
        
        return result;
    }
    
    std::vector<ProxyInfo> getProxiesByAnonymity(AnonymityLevel level) {
        std::vector<ProxyInfo> result;
        if (!scanner) return result;
        
        auto all_proxies = scanner->getVerifiedProxies();
        for (const auto& proxy : all_proxies) {
            if (proxy.anonymity == level) {
                result.push_back(proxy);
            }
        }
        
        return result;
    }
    
    std::vector<ProxyInfo> getFastestProxies(size_t count = 10) {
        if (!scanner) return {};
        
        auto all_proxies = scanner->getVerifiedProxies();
        
        // Sort by response time
        std::sort(all_proxies.begin(), all_proxies.end(), 
                  [](const ProxyInfo& a, const ProxyInfo& b) {
                      return a.avg_response_time < b.avg_response_time;
                  });
        
        if (all_proxies.size() > count) {
            all_proxies.resize(count);
        }
        
        return all_proxies;
    }
    
    std::vector<ProxyInfo> getMostReliableProxies(size_t count = 10) {
        if (!scanner) return {};
        
        auto all_proxies = scanner->getVerifiedProxies();
        
        // Sort by reliability
        std::sort(all_proxies.begin(), all_proxies.end(), 
                  [](const ProxyInfo& a, const ProxyInfo& b) {
                      return a.reliability > b.reliability;
                  });
        
        if (all_proxies.size() > count) {
            all_proxies.resize(count);
        }
        
        return all_proxies;
    }
    
    void exportProxies(const std::string& output_dir = "exported_proxies",
                       const std::string& filename_prefix = "proxies") {
        if (scanner) {
            scanner->exportResults(true, output_dir, filename_prefix);
        }
    }
    
    bool isRunning() const {
        return is_running;
    }
};

// =====================================================================
// SPECIALIZED EXPORT FORMATS
// =====================================================================

class ProxyExporter {
public:
    static void exportForEliteProxySwitcher(const std::vector<ProxyInfo>& proxies, 
                                           const std::string& filename = "elite_proxy_list.txt") {
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "[EXPORT] Failed to open " << filename << " for writing." << std::endl;
            return;
        }
        
        for (const auto& proxy : proxies) {
            file << proxy.getUrl() << std::endl;
        }
        
        file.close();
        std::cout << "[EXPORT] Exported " << proxies.size() 
                  << " proxies to " << filename << " (Elite Proxy Switcher format)" << std::endl;
    }
    
    static void exportForViewbot(const std::vector<ProxyInfo>& proxies, 
                                const std::string& filename = "viewbot_proxies.txt") {
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "[EXPORT] Failed to open " << filename << " for writing." << std::endl;
            return;
        }
        
        // Viewbot typically wants IP:PORT format
        for (const auto& proxy : proxies) {
            file << proxy.getAddress() << std::endl;
        }
        
        file.close();
        std::cout << "[EXPORT] Exported " << proxies.size() 
                  << " proxies to " << filename << " (Viewbot format)" << std::endl;
    }
    
    static void exportForSupreme(const std::vector<ProxyInfo>& proxies, 
                                const std::string& filename = "supreme_proxies.txt") {
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "[EXPORT] Failed to open " << filename << " for writing." << std::endl;
            return;
        }
        
        // Supreme bot format: IP:PORT:USERNAME:PASSWORD (we don't have auth, so just IP:PORT)
        for (const auto& proxy : proxies) {
            file << proxy.getAddress() << std::endl;
        }
        
        file.close();
        std::cout << "[EXPORT] Exported " << proxies.size() 
                  << " proxies to " << filename << " (Supreme format)" << std::endl;
    }
    
    static void exportToCSV(const std::vector<ProxyInfo>& proxies, 
                           const std::string& filename = "proxies.csv") {
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "[EXPORT] Failed to open " << filename << " for writing." << std::endl;
            return;
        }
        
        // CSV header
        file << "Host,Port,Protocol,Country,ResponseTime,Reliability,Anonymity" << std::endl;
        
        for (const auto& proxy : proxies) {
            file << proxy.host << ","
                 << proxy.port << ","
                 << proxy.protocol << ","
                 << proxy.country << ","
                 << proxy.avg_response_time << ","
                 << proxy.reliability << ","
                 << static_cast<int>(proxy.anonymity) << std::endl;
        }
        
        file.close();
        std::cout << "[EXPORT] Exported " << proxies.size() 
                  << " proxies to " << filename << " (CSV format)" << std::endl;
    }
    
    static void exportToJSON(const std::vector<ProxyInfo>& proxies, 
                            const std::string& filename = "proxies.json") {
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "[EXPORT] Failed to open " << filename << " for writing." << std::endl;
            return;
        }
        
        Json::Value json_array(Json::arrayValue);
        for (const auto& proxy : proxies) {
            json_array.append(proxy.toJson());
        }
        
        Json::StreamWriterBuilder builder;
        builder["indentation"] = "  ";
        std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
        writer->write(json_array, &file);
        
        file.close();
        std::cout << "[EXPORT] Exported " << proxies.size() 
                  << " proxies to " << filename << " (JSON format)" << std::endl;
    }
};

// =====================================================================
// UTILITY FUNCTIONS FOR OS INTEGRATION
// =====================================================================

namespace ProxyUtils {
    
    void runSpeedTest(const std::string& filepath) {
        std::cout << "[SPEEDTEST] Running speed test on proxies from: " << filepath << std::endl;
        
        std::ifstream file(filepath);
        if (!file.is_open()) {
            std::cerr << "[SPEEDTEST] Error: File not found at " << filepath << std::endl;
            return;
        }
        
        std::vector<std::string> proxy_addresses;
        std::string line;
        while (std::getline(file, line)) {
            if (!line.empty()) {
                proxy_addresses.push_back(line);
            }
        }
        file.close();
        
        std::cout << "[SPEEDTEST] Testing " << proxy_addresses.size() << " proxies..." << std::endl;
        
        // Create a temporary scanner for speed testing
        ProxyScanManager speed_tester(
            30,   // check_interval
            100,  // connection_limit
            1,    // validation_rounds (just one for speed test)
            ValidationMode::ANY,
            3.0,  // timeout
            false, // check_anonymity
            false, // force_fetch
            true,  // verbose
            true   // single_run
        );
        
        // Here you would implement actual speed testing logic
        // For now, this is a placeholder that simulates the functionality
        
        std::cout << "[SPEEDTEST] Speed test completed." << std::endl;
    }
    
    void cleanOldFiles(const std::string& directory = "exported_proxies", int days_old = 7) {
        std::cout << "[CLEANUP] Cleaning files older than " << days_old << " days from " << directory << std::endl;
        
        namespace fs = std::filesystem;
        
        if (!fs::exists(directory) || !fs::is_directory(directory)) {
            std::cout << "[CLEANUP] Directory does not exist: " << directory << std::endl;
            return;
        }
        
        auto now = std::chrono::system_clock::now();
        auto cutoff_time = now - std::chrono::hours(24 * days_old);
        
        int deleted_count = 0;
        try {
            for (const auto& entry : fs::directory_iterator(directory)) {
                if (entry.is_regular_file()) {
                    auto file_time = fs::last_write_time(entry.path());
                    auto file_time_sys = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                        file_time - fs::file_time_type::clock::now() + std::chrono::system_clock::now()
                    );
                    
                    if (file_time_sys < cutoff_time) {
                        fs::remove(entry.path());
                        deleted_count++;
                        std::cout << "[CLEANUP] Deleted: " << entry.path().filename() << std::endl;
                    }
                }
            }
        } catch (const fs::filesystem_error& e) {
            std::cerr << "[CLEANUP] Error: " << e.what() << std::endl;
        }
        
        std::cout << "[CLEANUP] Deleted " << deleted_count << " old files." << std::endl;
    }
    
    void analyzeProxyStats(const std::string& directory = "exported_proxies") {
        std::cout << "[STATS] Analyzing proxy statistics from " << directory << std::endl;
        
        namespace fs = std::filesystem;
        
        if (!fs::exists(directory) || !fs::is_directory(directory)) {
            std::cout << "[STATS] Directory does not exist: " << directory << std::endl;
            return;
        }
        
        int total_files = 0;
        int total_proxies = 0;
        std::unordered_map<std::string, int> country_counts;
        std::vector<double> response_times;
        
        try {
            for (const auto& entry : fs::directory_iterator(directory)) {
                if (entry.is_regular_file() && entry.path().extension() == ".json") {
                    total_files++;
                    
                    std::ifstream file(entry.path());
                    if (file.is_open()) {
                        Json::Value json_data;
                        Json::Reader reader;
                        
                        if (reader.parse(file, json_data) && json_data.isArray()) {
                            total_proxies += json_data.size();
                            
                            for (const auto& proxy : json_data) {
                                std::string country = proxy.get("country", "Unknown").asString();
                                country_counts[country]++;
                                
                                double response_time = proxy.get("avg_response_time", 0.0).asDouble();
                                if (response_time > 0.0) {
                                    response_times.push_back(response_time);
                                }
                            }
                        }
                        file.close();
                    }
                }
            }
        } catch (const fs::filesystem_error& e) {
            std::cerr << "[STATS] Error: " << e.what() << std::endl;
            return;
        }
        
        std::cout << "[STATS] Analysis Results:" << std::endl;
        std::cout << "[STATS] - Total files analyzed: " << total_files << std::endl;
        std::cout << "[STATS] - Total proxies found: " << total_proxies << std::endl;
        
        if (!response_times.empty()) {
            std::sort(response_times.begin(), response_times.end());
            double avg_response = std::accumulate(response_times.begin(), response_times.end(), 0.0) / response_times.size();
            double median_response = response_times[response_times.size() / 2];
            
            std::cout << "[STATS] - Average response time: " << avg_response << "ms" << std::endl;
            std::cout << "[STATS] - Median response time: " << median_response << "ms" << std::endl;
            std::cout << "[STATS] - Fastest response time: " << response_times.front() << "ms" << std::endl;
            std::cout << "[STATS] - Slowest response time: " << response_times.back() << "ms" << std::endl;
        }
        
        if (!country_counts.empty()) {
            std::cout << "[STATS] - Proxies by country:" << std::endl;
            
            // Sort countries by proxy count
            std::vector<std::pair<std::string, int>> sorted_countries(country_counts.begin(), country_counts.end());
            std::sort(sorted_countries.begin(), sorted_countries.end(), 
                     [](const auto& a, const auto& b) { return a.second > b.second; });
            
            for (const auto& [country, count] : sorted_countries) {
                std::cout << "[STATS]   " << country << ": " << count << " proxies" << std::endl;
            }
        }
    }
    
    void setupAutoUpdate(int interval_hours = 1) {
        std::cout << "[AUTO-UPDATE] Setting up auto-update every " << interval_hours << " hours." << std::endl;
        
        // This would typically integrate with your OS's task scheduler
        // For now, this is a placeholder that shows the concept
        
        static std::thread update_thread;
        static std::atomic<bool> auto_update_running{false};
        
        if (auto_update_running) {
            std::cout << "[AUTO-UPDATE] Auto-update is already running." << std::endl;
            return;
        }
        
        auto_update_running = true;
        update_thread = std::thread([interval_hours]() {
            ProxyScannerAPI api;
            
            while (auto_update_running) {
                std::cout << "[AUTO-UPDATE] Starting scheduled proxy scan..." << std::endl;
                
                api.start();
                std::this_thread::sleep_for(std::chrono::minutes(10)); // Run for 10 minutes
                api.stop();
                
                std::cout << "[AUTO-UPDATE] Scheduled scan completed. Sleeping for " << interval_hours << " hour(s)." << std::endl;
                std::this_thread::sleep_for(std::chrono::hours(interval_hours));
            }
        });
        
        update_thread.detach();
        std::cout << "[AUTO-UPDATE] Auto-update setup completed." << std::endl;
    }
}

// =====================================================================
// COMMAND LINE INTERFACE
// =====================================================================

class ProxyScannerCLI {
public:
    static void printUsage() {
        std::cout << "RapidProxyScan C++ - Advanced Proxy Scanner\n"
                  << "===========================================\n\n"
                  << "Usage: rapidproxyscan [OPTIONS]\n\n"
                  << "Options:\n"
                  << "  --check-interval SECONDS     Interval between scan cycles (default: 30)\n"
                  << "  --connections LIMIT           Maximum concurrent connections (default: 200)\n"
                  << "  --validation-rounds N         Number of validation tests per proxy (default: 3)\n"
                  << "  --validation-mode MODE        Success criteria: any, majority, all (default: majority)\n"
                  << "  --timeout SECONDS             Connection timeout (default: 5.0)\n"
                  << "  --anonymity                   Enable proxy anonymity checking\n"
                  << "  --force-fetch                 Force fetch proxy lists on each cycle\n"
                  << "  --verbose                     Show detailed logs\n"
                  << "  --single-run                  Run once and exit\n"
                  << "  --workers N                   Number of test workers (default: 50)\n"
                  << "  --export-dir DIR              Export directory (default: exported_proxies)\n"
                  << "\nUtility Commands:\n"
                  << "  --speed-test FILE             Run speed test on proxies from file\n"
                  << "  --clean-old                   Clean old exported files\n"
                  << "  --analyze                     Analyze proxy statistics\n"
                  << "  --setup-auto-update           Setup automatic updates\n"
                  << "  --help                        Show this help message\n\n"
                  << "Examples:\n"
                  << "  rapidproxyscan --verbose --workers 100\n"
                  << "  rapidproxyscan --single-run --export-dir /tmp/proxies\n"
                  << "  rapidproxyscan --speed-test proxy_list.txt\n"
                  << std::endl;
    }
    
    static int run(int argc, char* argv[]) {
        // Default values
        int check_interval = ProxyConfig::DEFAULT_CHECK_INTERVAL;
        int connection_limit = ProxyConfig::DEFAULT_CONNECTION_LIMIT;
        int validation_rounds = ProxyConfig::DEFAULT_VALIDATION_ROUNDS;
        ValidationMode validation_mode = ValidationMode::MAJORITY;
        double timeout = ProxyConfig::DEFAULT_TIMEOUT;
        bool check_anonymity = false;
        bool force_fetch = false;
        bool verbose = false;
        bool single_run = false;
        int workers = 50;
        std::string export_dir = "exported_proxies";
        
        // Parse command line arguments
        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];
            
            if (arg == "--help") {
                printUsage();
                return 0;
            }
            else if (arg == "--speed-test" && i + 1 < argc) {
                ProxyUtils::runSpeedTest(argv[++i]);
                return 0;
            }
            else if (arg == "--clean-old") {
                ProxyUtils::cleanOldFiles();
                return 0;
            }
            else if (arg == "--analyze") {
                ProxyUtils::analyzeProxyStats();
                return 0;
            }
            else if (arg == "--setup-auto-update") {
                ProxyUtils::setupAutoUpdate();
                return 0;
            }
            else if (arg == "--check-interval" && i + 1 < argc) {
                check_interval = std::stoi(argv[++i]);
            }
            else if (arg == "--connections" && i + 1 < argc) {
                connection_limit = std::stoi(argv[++i]);
            }
            else if (arg == "--validation-rounds" && i + 1 < argc) {
                validation_rounds = std::stoi(argv[++i]);
            }
            else if (arg == "--validation-mode" && i + 1 < argc) {
                std::string mode = argv[++i];
                if (mode == "any") validation_mode = ValidationMode::ANY;
                else if (mode == "majority") validation_mode = ValidationMode::MAJORITY;
                else if (mode == "all") validation_mode = ValidationMode::ALL;
            }
            else if (arg == "--timeout" && i + 1 < argc) {
                timeout = std::stod(argv[++i]);
            }
            else if (arg == "--workers" && i + 1 < argc) {
                workers = std::stoi(argv[++i]);
            }
            else if (arg == "--export-dir" && i + 1 < argc) {
                export_dir = argv[++i];
            }
            else if (arg == "--anonymity") {
                check_anonymity = true;
            }
            else if (arg == "--force-fetch") {
                force_fetch = true;
            }
            else if (arg == "--verbose") {
                verbose = true;
            }
            else if (arg == "--single-run") {
                single_run = true;
            }
        }
        
        // Run the scanner
        try {
            ProxyScanManager scanner(
                check_interval, connection_limit, validation_rounds, validation_mode,
                timeout, check_anonymity, force_fetch, verbose, single_run,
                ProxyConfig::DEFAULT_PROXY_SOURCES, workers
            );
            
            scanner.run();
            return 0;
            
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            return 1;
        } catch (...) {
            std::cerr << "Unknown error occurred." << std::endl;
            return 1;
        }
    }
};

// =====================================================================
// MAIN ENTRY POINT
// =====================================================================

int main(int argc, char* argv[]) {
    return ProxyScannerCLI::run(argc, argv);
}

// =====================================================================
// EXAMPLE USAGE FOR OS INTEGRATION
// =====================================================================

/*
// Example 1: Basic API usage in your OS
void example_basic_usage() {
    ProxyScannerAPI scanner;
    
    // Start scanning in background
    scanner.start({}, 100, true); // default sources, 100 workers, verbose
    
    // Let it run for a while
    std::this_thread::sleep_for(std::chrono::minutes(5));
    
    // Get working proxies
    auto proxies = scanner.getWorkingProxyList();
    std::cout << "Found " << proxies.size() << " working proxies:" << std::endl;
    for (const auto& proxy : proxies) {
        std::cout << "  " << proxy << std::endl;
    }
    
    // Export results
    scanner.exportProxies();
    
    // Stop scanner
    scanner.stop();
}

// Example 2: Get fastest proxies for critical operations
void example_get_fastest_proxies() {
    ProxyScannerAPI scanner;
    scanner.start();
    
    // Let it collect some proxies
    std::this_thread::sleep_for(std::chrono::minutes(2));
    
    // Get the 5 fastest proxies
    auto fastest = scanner.getFastestProxies(5);
    
    std::cout << "Top 5 fastest proxies:" << std::endl;
    for (const auto& proxy : fastest) {
        std::cout << "  " << proxy.getUrl() 
                  << " (" << proxy.avg_response_time << "ms)" << std::endl;
    }
    
    scanner.stop();
}

// Example 3: Export in specialized formats
void example_specialized_export() {
    ProxyScannerAPI scanner;
    scanner.start();
    
    std::this_thread::sleep_for(std::chrono::minutes(3));
    
    auto proxies = scanner.getWorkingProxies();
    
    // Export for different tools
    ProxyExporter::exportForEliteProxySwitcher(proxies);
    ProxyExporter::exportForViewbot(proxies);
    ProxyExporter::exportForSupreme(proxies);
    ProxyExporter::exportToCSV(proxies);
    ProxyExporter::exportToJSON(proxies);
    
    scanner.stop();
}
*/