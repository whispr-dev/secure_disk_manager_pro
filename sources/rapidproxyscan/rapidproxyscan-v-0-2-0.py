"""
RapidProxyScan - Advanced Optimizations Module
==============================================

This module implements the most sophisticated proxy scanning optimizations:
1. Lightweight custom endpoint creation
2. CDN-distributed endpoint selection for geographical optimization
3. DNS pre-resolution and aggressive caching
4. Header compression and bandwidth optimization
5. Binary protocol implementations
6. Special formatting for viewbot/Supreme environments

Requirements:
- Python 3.7+
- aiohttp
- beautifulsoup4
- colorama
"""

import aiohttp
import asyncio
import argparse
import time
import random
import os
import ipaddress
import socket
import re
import json
import csv
import gzip
import struct
import hashlib
import logging
from datetime import datetime
from bs4 import BeautifulSoup
from urllib.parse import urlparse
from colorama import Fore, Style, init
from concurrent.futures import ThreadPoolExecutor
from collections import defaultdict
from typing import Dict, List, Tuple, Set, Optional, Union, Any


# Initialize colorama
init()

# Setup logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s'
)

# ===========================================================================
# 1. LIGHTWEIGHT CUSTOM ENDPOINT CREATION
# ===========================================================================

class CustomEndpointManager:
    """Creates and manages ultra-lightweight endpoints for proxy testing"""
    
    def __init__(self):
        self.endpoints = {}
        self.endpoint_stats = defaultdict(lambda: {"success": 0, "failure": 0, "avg_time": 0})
        self.endpoint_health_lock = asyncio.Lock()
    
    async def create_endpoint_selection_map(self) -> Dict[str, List[str]]:
        """Create a mapping of regions to optimal endpoints"""
        # This maps geographical regions to the best endpoints for that region
        region_map = {
            "na": [],  # North America
            "eu": [],  # Europe
            "as": [],  # Asia
            "sa": [],  # South America
            "oc": [],  # Oceania
            "af": []   # Africa
        }
        
        # Populate with predefined lightweight endpoints optimized for each region
        region_map["na"] = [
            "https://cloudfront-na.example.com/echo",  # AWS CloudFront North America
            "https://na-ping.httpbin.org/ip",         # North America optimized
            "http://cdn-na.iptest.net/ping"           # NA CDN endpoint
        ]
        
        region_map["eu"] = [
            "https://cloudfront-eu.example.com/echo",  # AWS CloudFront Europe
            "https://eu-ping.httpbin.org/ip",         # Europe optimized
            "http://cdn-eu.iptest.net/ping"           # EU CDN endpoint
        ]
        
        region_map["as"] = [
            "https://cloudfront-ap.example.com/echo",  # AWS CloudFront Asia Pacific
            "https://ap-ping.httpbin.org/ip",         # Asia Pacific optimized
            "http://cdn-ap.iptest.net/ping"           # AP CDN endpoint
        ]
        
        # Add standard endpoints as fallbacks for all regions
        standard_endpoints = [
            "http://httpbin.org/ip",
            "https://api.ipify.org/?format=json",
            "http://ip-api.com/json/",
            "https://www.cloudflare.com/cdn-cgi/trace"
        ]
        
        # Add to all regions as fallbacks
        for region in region_map:
            region_map[region].extend(standard_endpoints)
        
        return region_map
    
    async def select_optimal_endpoint(self, proxy_info: dict) -> str:
        """Select the optimal endpoint based on proxy location and endpoint health"""
        # Default endpoints if we don't have location data
        if not proxy_info.get("country"):
            return random.choice([
                "http://httpbin.org/ip", 
                "https://api.ipify.org/?format=json"
            ])
        
        # Map country to region (simplified)
        country = proxy_info["country"].lower()
        
        # Map countries to regions (partial list)
        country_to_region = {
            # North America
            "us": "na", "ca": "na", "mx": "na",
            # Europe
            "gb": "eu", "de": "eu", "fr": "eu", "it": "eu", "es": "eu", "nl": "eu",
            # Asia
            "cn": "as", "jp": "as", "kr": "as", "in": "as", "sg": "as",
            # South America
            "br": "sa", "ar": "sa", "co": "sa", "cl": "sa",
            # Oceania
            "au": "oc", "nz": "oc",
            # Africa
            "za": "af", "ng": "af", "eg": "af"
        }
        
        region = country_to_region.get(country, "na")  # Default to NA if unknown
        
        # Get endpoints for this region
        region_map = await self.create_endpoint_selection_map()
        endpoints = region_map.get(region, region_map["na"])
        
        # Select based on health metrics
        async with self.endpoint_health_lock:
            # Sort endpoints by success rate and response time
            scored_endpoints = []
            for endpoint in endpoints:
                stats = self.endpoint_stats[endpoint]
                total = stats["success"] + stats["failure"]
                if total == 0:
                    # No data yet, give it a middle score
                    scored_endpoints.append((endpoint, 0.5))
                else:
                    success_rate = stats["success"] / total
                    # Normalize response time (0-1 scale, lower is better)
                    time_score = 1.0 - min(1.0, stats["avg_time"] / 5000.0) if stats["avg_time"] > 0 else 0.5
                    # Combined score (70% success rate, 30% speed)
                    score = (success_rate * 0.7) + (time_score * 0.3)
                    scored_endpoints.append((endpoint, score))
            
            # Sort by score (highest first)
            scored_endpoints.sort(key=lambda x: x[1], reverse=True)
            
            # Select randomly from top 3 to avoid overloading any single endpoint
            top_endpoints = [e[0] for e in scored_endpoints[:3]] if len(scored_endpoints) >= 3 else [e[0] for e in scored_endpoints]
            return random.choice(top_endpoints)
    
    async def update_endpoint_health(self, endpoint: str, success: bool, response_time: float):
        """Update health metrics for an endpoint"""
        async with self.endpoint_health_lock:
            stats = self.endpoint_stats[endpoint]
            
            if success:
                stats["success"] += 1
                # Update rolling average response time (last 20 requests)
                if stats["avg_time"] == 0:
                    stats["avg_time"] = response_time
                else:
                    stats["avg_time"] = (stats["avg_time"] * 19 + response_time) / 20
            else:
                stats["failure"] += 1
    
    async def create_micro_endpoint(self) -> str:
        """
        Create a micro-endpoint for testing (using serverless functions)
        This would create a specialized endpoint optimized for proxy testing
        
        Note: This is a placeholder for the concept - actual implementation would 
        involve deploying serverless functions on AWS Lambda, Cloudflare Workers, etc.
        """
        # In a real implementation, this would:
        # 1. Deploy a serverless function that returns minimal response
        # 2. Wait for deployment to complete
        # 3. Return the URL of the deployed function
        
        # For now, return a known lightweight endpoint
        return "https://echo.zuplo.io/"

logger = logging.getLogger("proxy_optimizer")

# Global constants
USER_AGENTS = [
    'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36',
    'Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/14.1.1 Safari/605.1.15',
    'Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36',
    'Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:89.0) Gecko/20100101 Firefox/89.0'
]

# Test URLs - chosen for fast response times and geographic distribution
TEST_ENDPOINTS = [
    'http://httpbin.org/ip',
    'https://api.ipify.org/?format=json',
    'http://ip-api.com/json/',
    'https://www.cloudflare.com/cdn-cgi/trace'
]

# Proxy sources
PROXY_SOURCES = [
    # Free proxy lists
    'https://www.freeproxylists.net/',
    'https://free-proxy-list.net/',
    'https://www.sslproxies.org/',
    'https://www.us-proxy.org/',
    'https://free-proxy-list.net/uk-proxy.html',
    'https://www.proxy-list.download/api/v1/get',
    'https://api.proxyscrape.com/v2/?request=getproxies&protocol=http&timeout=10000&country=all&ssl=all&anonymity=all',
    'https://raw.githubusercontent.com/TheSpeedX/PROXY-List/master/http.txt',
    'https://raw.githubusercontent.com/ShiftyTR/Proxy-List/master/http.txt',
    'https://raw.githubusercontent.com/clarketm/proxy-list/master/proxy-list.txt',
    'https://raw.githubusercontent.com/jetkai/proxy-list/main/online-proxies/txt/proxies.txt',
    'https://raw.githubusercontent.com/sunny9577/proxy-scraper/master/proxies.txt'
]

# Timeouts configuration
CONNECT_TIMEOUT = 2.0
READ_TIMEOUT = 5.0
ADVANCED_TIMEOUT_RANGES = {
    'fast': {'connect': 1.0, 'read': 3.0},
    'medium': {'connect': 2.0, 'read': 5.0},
    'slow': {'connect': 3.0, 'read': 8.0}
}

# DNS Cache to prevent redundant lookups
dns_cache = {}
dns_lock = asyncio.Lock()

class ProxyType:
    """Enum-like class for proxy types"""
    HTTP = 'http'
    HTTPS = 'https'
    SOCKS4 = 'socks4'
    SOCKS5 = 'socks5'
    
    @staticmethod
    def get_all():
        return [ProxyType.HTTP, ProxyType.HTTPS, ProxyType.SOCKS4, ProxyType.SOCKS5]

class ProxyInfo:
    """Class to store proxy information"""
    def __init__(self, host, port, proto=ProxyType.HTTP, country=None):
        self.host = host
        self.port = port
        self.protocol = proto
        self.country = country
        self.response_times = []
        self.success_count = 0
        self.fail_count = 0
        self.last_checked = None
        self.anonymity = None
    
    @property
    def address(self):
        return f"{self.host}:{self.port}"
    
    @property
    def url(self):
        return f"{self.protocol}://{self.host}:{self.port}"
    
    @property
    def avg_response_time(self):
        if not self.response_times:
            return float('inf')
        return sum(self.response_times) / len(self.response_times)
    
    @property
    def reliability(self):
        total = self.success_count + self.fail_count
        if total == 0:
            return 0
        return (self.success_count / total) * 100
    
    def to_dict(self):
        return {
            'host': self.host,
            'port': self.port,
            'protocol': self.protocol,
            'country': self.country,
            'avg_response_time': self.avg_response_time,
            'reliability': self.reliability,
            'anonymity': self.anonymity
        }
    
    def __str__(self):
        return f"{self.url} [{self.avg_response_time:.2f}ms, {self.reliability:.1f}%]"

class ProxyScanManager:
    """Main proxy scan manager class"""
    def __init__(self, args):
        self.args = args
        self.proxies = {}  # Dictionary to store proxy objects {address: ProxyInfo}
        self.session = None
        self.scrape_session = None
        self.fetch_lock = asyncio.Lock()
        self.export_lock = asyncio.Lock()
        self.dns_cache = {}
        self.test_tasks = []
        self.verified_proxies = set()
        self.proxy_stats = {
            'total_discovered': 0,
            'total_working': 0,
            'high_speed': 0,
            'medium_speed': 0,
            'low_speed': 0
        }
        self.last_fetch_time = 0
        self.consecutive_failures = defaultdict(int)
        self.region_stats = defaultdict(int)
    
    async def initialize(self):
        """Initialize the proxy scanner"""
        connector = aiohttp.TCPConnector(
            limit=self.args.connection_limit,
            ttl_dns_cache=300,
            ssl=False
        )
        timeout = aiohttp.ClientTimeout(
            total=30,
            connect=CONNECT_TIMEOUT,
            sock_connect=CONNECT_TIMEOUT,
            sock_read=READ_TIMEOUT
        )
        self.session = aiohttp.ClientSession(
            connector=connector,
            timeout=timeout,
            headers=self._get_random_headers()
        )
        
        self.scrape_session = aiohttp.ClientSession(
            headers=self._get_random_headers(),
            timeout=aiohttp.ClientTimeout(total=30)
        )
        
        print(f"{Fore.CYAN}RapidProxyScan initialized with {self.args.connection_limit} max connections{Style.RESET_ALL}")
        print(f"{Fore.YELLOW}Settings: Check interval: {self.args.check_interval}s, "
              f"Validation rounds: {self.args.validation_rounds}{Style.RESET_ALL}")

    async def run(self):
        """Run the proxy scanner"""
        await self.initialize()
        try:
            await self.start_scanning_loop()
        finally:
            await self.cleanup()
    
    async def cleanup(self):
        """Close sessions and cleanup resources"""
        if self.session:
            await self.session.close()
        if self.scrape_session:
            await self.scrape_session.close()
    
    async def start_scanning_loop(self):
        """Main loop for scanning"""
        try:
            while True:
                start_time = time.time()
                
                # Fetch new proxy lists
                await self.fetch_proxy_lists()
                
                # Scan all known proxies
                await self.scan_all_proxies()
                
                # Export results
                await self.export_results()
                
                # Calculate and display stats
                self.display_stats()
                
                # Wait for next iteration
                elapsed = time.time() - start_time
                wait_time = max(1, self.args.check_interval - elapsed)
                print(f"{Fore.BLUE}Completed scan cycle in {elapsed:.2f}s. "
                      f"Waiting {wait_time:.2f}s for next cycle...{Style.RESET_ALL}")
                
                if self.args.single_run:
                    break
                    
                await asyncio.sleep(wait_time)
        
        except KeyboardInterrupt:
            print(f"{Fore.YELLOW}Scan interrupted by user. Finishing...{Style.RESET_ALL}")
            await self.export_results(force=True)
    
    async def fetch_proxy_lists(self):
        """Fetch proxy lists from all sources"""
        async with self.fetch_lock:
            current_time = time.time()
            if current_time - self.last_fetch_time < 10 and not self.args.force_fetch:
                # Don't fetch too frequently
                return
            
            self.last_fetch_time = current_time
            print(f"{Fore.GREEN}Fetching proxy lists from {len(PROXY_SOURCES)} sources...{Style.RESET_ALL}")
            
            fetch_tasks = []
            for source in PROXY_SOURCES:
                fetch_tasks.append(self.fetch_from_source(source))
            
            results = await asyncio.gather(*fetch_tasks, return_exceptions=True)
            
            successful_sources = sum(1 for r in results if isinstance(r, int) and r > 0)
            total_proxies = sum((r for r in results if isinstance(r, int)), 0)
            
            print(f"{Fore.GREEN}Fetched {total_proxies} proxies from {successful_sources}/{len(PROXY_SOURCES)} "
                  f"sources. Total known proxies: {len(self.proxies)}{Style.RESET_ALL}")
    
    async def fetch_from_source(self, source):
        """Fetch proxies from a specific source"""
        try:
            parsed_url = urlparse(source)
            if parsed_url.netloc == 'raw.githubusercontent.com':
                return await self.fetch_from_raw_github(source)
            elif 'api' in parsed_url.netloc:
                return await self.fetch_from_api(source)
            else:
                return await self.fetch_from_html(source)
        except Exception as e:
            if self.args.verbose:
                print(f"{Fore.RED}Error fetching from {source}: {str(e)}{Style.RESET_ALL}")
            return 0
    
    async def fetch_from_raw_github(self, url):
        """Fetch proxies from raw GitHub content"""
        try:
            async with self.scrape_session.get(url) as response:
                if response.status != 200:
                    return 0
                
                content = await response.text()
                count = 0
                
                # Process line by line
                for line in content.splitlines():
                    line = line.strip()
                    if not line:
                        continue
                    
                    # Try to parse IP:PORT format
                    if ':' in line:
                        parts = line.split(':')
                        if len(parts) >= 2:
                            host = parts[0].strip()
                            port = parts[1].strip()
                            try:
                                port = int(port)
                                if self._is_valid_host(host) and 1 <= port <= 65535:
                                    self._add_proxy(host, port)
                                    count += 1
                            except ValueError:
                                continue
                
                return count
        except Exception as e:
            if self.args.verbose:
                print(f"{Fore.RED}Error fetching from GitHub {url}: {str(e)}{Style.RESET_ALL}")
            return 0
    
    async def fetch_from_api(self, url):
        """Fetch proxies from API endpoints"""
        try:
            async with self.scrape_session.get(url) as response:
                if response.status != 200:
                    return 0
                
                content_type = response.headers.get('Content-Type', '')
                count = 0
                
                if 'application/json' in content_type:
                    # JSON response
                    data = await response.json()
                    count = await self._parse_api_json(data)
                else:
                    # Assume plaintext list
                    text = await response.text()
                    count = self._parse_plaintext_list(text)
                
                return count
        except Exception as e:
            if self.args.verbose:
                print(f"{Fore.RED}Error fetching from API {url}: {str(e)}{Style.RESET_ALL}")
            return 0
    
    async def _parse_api_json(self, data):
        """Parse JSON API response"""
        count = 0
        
        # Try common formats
        if isinstance(data, list):
            for item in data:
                if isinstance(item, dict):
                    # Try to find common field names
                    host = item.get('ip') or item.get('host') or item.get('addr')
                    port = item.get('port')
                    if host and port:
                        try:
                            port = int(port)
                            if self._is_valid_host(host) and 1 <= port <= 65535:
                                proto = item.get('protocol', ProxyType.HTTP)
                                country = item.get('country')
                                self._add_proxy(host, port, proto, country)
                                count += 1
                        except (ValueError, TypeError):
                            continue
                elif isinstance(item, str) and ':' in item:
                    # IP:PORT format in list
                    parts = item.split(':')
                    if len(parts) >= 2:
                        host = parts[0].strip()
                        port = parts[1].strip()
                        try:
                            port = int(port)
                            if self._is_valid_host(host) and 1 <= port <= 65535:
                                self._add_proxy(host, port)
                                count += 1
                        except ValueError:
                            continue
        elif isinstance(data, dict):
            # Try to find proxies in dict
            for key, value in data.items():
                if isinstance(value, list):
                    for item in value:
                        if isinstance(item, dict):
                            host = item.get('ip') or item.get('host') or item.get('addr')
                            port = item.get('port')
                            if host and port:
                                try:
                                    port = int(port)
                                    if self._is_valid_host(host) and 1 <= port <= 65535:
                                        proto = item.get('protocol', ProxyType.HTTP)
                                        country = item.get('country')
                                        self._add_proxy(host, port, proto, country)
                                        count += 1
                                except (ValueError, TypeError):
                                    continue
        
        return count
    
    def _parse_plaintext_list(self, text):
        """Parse plaintext proxy list"""
        count = 0
        
        # Process line by line
        for line in text.splitlines():
            line = line.strip()
            if not line:
                continue
            
            # Try to parse IP:PORT format
            if ':' in line:
                parts = line.split(':')
                if len(parts) >= 2:
                    host = parts[0].strip()
                    port = parts[1].strip()
                    try:
                        port = int(port)
                        if self._is_valid_host(host) and 1 <= port <= 65535:
                            self._add_proxy(host, port)
                            count += 1
                    except ValueError:
                        continue
        
        return count
    
    async def fetch_from_html(self, url):
        """Fetch proxies from HTML pages"""
        try:
            async with self.scrape_session.get(url) as response:
                if response.status != 200:
                    return 0
                
                html = await response.text()
                soup = BeautifulSoup(html, 'html.parser')
                count = 0
                
                # Find tables that might contain proxy info
                tables = soup.find_all('table')
                for table in tables:
                    # Process each row in the table
                    rows = table.find_all('tr')
                    for row in rows:
                        cells = row.find_all('td')
                        if len(cells) >= 2:  # Need at least IP and port
                            # Try to find IP and port
                            ip = None
                            port = None
                            
                            # Common patterns for IP cells
                            for cell in cells:
                                text = cell.get_text().strip()
                                
                                # Check for IP address pattern
                                if not ip and re.match(r'^(\d{1,3}\.){3}\d{1,3}$', text):
                                    ip = text
                                
                                # Check for port pattern
                                elif not port and re.match(r'^\d{1,5}$', text):
                                    try:
                                        port = int(text)
                                    except ValueError:
                                        continue
                            
                            if ip and port and 1 <= port <= 65535:
                                if self._is_valid_host(ip):
                                    self._add_proxy(ip, port)
                                    count += 1
                
                # If we couldn't find proxies in tables, try regex for IP:PORT pattern
                if count == 0:
                    ip_port_pattern = re.compile(r'(\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}):(\d{1,5})')
                    matches = ip_port_pattern.findall(html)
                    
                    for ip, port_str in matches:
                        try:
                            port = int(port_str)
                            if self._is_valid_host(ip) and 1 <= port <= 65535:
                                self._add_proxy(ip, port)
                                count += 1
                        except ValueError:
                            continue
                
                return count
        except Exception as e:
            if self.args.verbose:
                print(f"{Fore.RED}Error fetching from HTML {url}: {str(e)}{Style.RESET_ALL}")
            return 0
    
    def _is_valid_host(self, host):
        """Check if a host string is a valid IP address"""
        try:
            ipaddress.ip_address(host)
            return True
        except ValueError:
            # Not an IP address, might be a hostname
            return re.match(r'^[a-zA-Z0-9][-a-zA-Z0-9.]*\.[a-zA-Z]{2,}$', host) is not None
    
    def _add_proxy(self, host, port, proto=ProxyType.HTTP, country=None):
        """Add a proxy to the list if it doesn't exist"""
        address = f"{host}:{port}"
        
        if address not in self.proxies:
            self.proxies[address] = ProxyInfo(host, port, proto, country)
            self.proxy_stats['total_discovered'] += 1
    
    async def scan_all_proxies(self):
        """Scan all known proxies in parallel"""
        if not self.proxies:
            print(f"{Fore.YELLOW}No proxies to scan.{Style.RESET_ALL}")
            return
        
        print(f"{Fore.CYAN}Scanning {len(self.proxies)} proxies...{Style.RESET_ALL}")
        
        # Group proxies by scan priority
        to_scan = {}
        for addr, proxy in self.proxies.items():
            # Skip proxies that have consistently failed and were just tested
            if self.consecutive_failures[addr] >= 3 and time.time() - (proxy.last_checked or 0) < 300:
                continue
            
            to_scan[addr] = proxy
        
        if not to_scan:
            print(f"{Fore.YELLOW}No proxies to scan after filtering.{Style.RESET_ALL}")
            return
        
        print(f"{Fore.CYAN}Testing {len(to_scan)} proxies with {self.args.validation_rounds} " 
              f"validation rounds...{Style.RESET_ALL}")
        
        test_tasks = []
        for _, proxy in to_scan.items():
            test_tasks.append(self.test_proxy(proxy))
        
        # Execute tests
        results = await asyncio.gather(*test_tasks, return_exceptions=True)
        
        working_count = sum(1 for r in results if r is True)
        print(f"{Fore.GREEN}Found {working_count} working proxies out of {len(test_tasks)}{Style.RESET_ALL}")
    
    async def test_proxy(self, proxy):
        """Test a single proxy with multiple validation rounds"""
        proxy.last_checked = time.time()
        
        # Multiple validation rounds
        success_count = 0
        
        for round_num in range(self.args.validation_rounds):
            try:
                success, response_time = await self._test_proxy_single_round(proxy)
                
                if success:
                    success_count += 1
                    proxy.success_count += 1
                    proxy.response_times.append(response_time)
                    # Keep only the last 5 response times
                    if len(proxy.response_times) > 5:
                        proxy.response_times = proxy.response_times[-5:]
                else:
                    proxy.fail_count += 1
                
                # Short circuit if we've met our success criteria
                if self.args.validation_mode == 'any' and success_count > 0:
                    break
                elif self.args.validation_mode == 'majority' and success_count > self.args.validation_rounds // 2:
                    break
            
            except Exception as e:
                if self.args.verbose:
                    print(f"{Fore.RED}Error testing {proxy.address}: {str(e)}{Style.RESET_ALL}")
                proxy.fail_count += 1
        
        # Determine final result based on validation mode
        result = False
        if self.args.validation_mode == 'any':
            result = success_count > 0
        elif self.args.validation_mode == 'majority':
            result = success_count > self.args.validation_rounds // 2
        elif self.args.validation_mode == 'all':
            result = success_count == self.args.validation_rounds
        
        # Update consecutive failures tracking
        if result:
            self.consecutive_failures[proxy.address] = 0
            self.verified_proxies.add(proxy.address)
        else:
            self.consecutive_failures[proxy.address] += 1
            if proxy.address in self.verified_proxies:
                self.verified_proxies.remove(proxy.address)
        
        return result
    
    async def _test_proxy_single_round(self, proxy):
        """Test a proxy with a single request"""
        # Choose test endpoint randomly
        test_url = random.choice(TEST_ENDPOINTS)
        
        # Decide on protocol (http/https)
        if proxy.protocol == ProxyType.HTTPS:
            proxy_url = f"https://{proxy.host}:{proxy.port}"
        else:
            proxy_url = f"http://{proxy.host}:{proxy.port}"
        
        # Decide on timeout based on proxy response history
        if not proxy.response_times:
            # Default timeouts for new proxies
            connect_timeout = CONNECT_TIMEOUT
            read_timeout = READ_TIMEOUT
        else:
            avg_time = proxy.avg_response_time / 1000  # Convert ms to seconds
            if avg_time < 1.0:
                # Fast proxy
                connect_timeout = ADVANCED_TIMEOUT_RANGES['fast']['connect']
                read_timeout = ADVANCED_TIMEOUT_RANGES['fast']['read']
            elif avg_time < 3.0:
                # Medium proxy
                connect_timeout = ADVANCED_TIMEOUT_RANGES['medium']['connect']
                read_timeout = ADVANCED_TIMEOUT_RANGES['medium']['read']
            else:
                # Slow proxy
                connect_timeout = ADVANCED_TIMEOUT_RANGES['slow']['connect']
                read_timeout = ADVANCED_TIMEOUT_RANGES['slow']['read']
        
        timeout = aiohttp.ClientTimeout(
            total=read_timeout + connect_timeout,
            connect=connect_timeout,
            sock_connect=connect_timeout,
            sock_read=read_timeout
        )
        
        try:
            start_time = time.time()
            
            async with self.session.get(
                test_url,
                proxy=proxy_url,
                timeout=timeout,
                headers=self._get_random_headers(),
                ssl=False
            ) as response:
                if response.status != 200:
                    return False, 0
                
                # Try to read the response body
                body = await response.text()
                if not body:
                    return False, 0
                
                # Calculate response time in milliseconds
                response_time = (time.time() - start_time) * 1000
                
                # Check if the response contains expected data
                # For ipify, httpbin, etc., we expect to see an IP address in the response
                if 'ip' in body.lower() or re.search(r'(\d{1,3}\.){3}\d{1,3}', body):
                    # Try to determine proxy anonymity level
                    if self.args.check_anonymity:
                        await self._check_proxy_anonymity(proxy, body)
                    
                    # Get country information if available and not already set
                    if not proxy.country and 'country' in body.lower():
                        try:
                            # Try to parse JSON responses
                            if response.headers.get('Content-Type', '').startswith('application/json'):
                                data = await response.json()
                                if 'country' in data:
                                    proxy.country = data['country']
                                elif 'country_code' in data:
                                    proxy.country = data['country_code']
                        except:
                            pass
                    
                    # Track region stats if country information is available
                    if proxy.country:
                        self.region_stats[proxy.country] += 1
                    
                    return True, response_time
                
                return False, 0
        
        except asyncio.TimeoutError:
            return False, 0
        except Exception as e:
            if self.args.verbose:
                print(f"{Fore.RED}Error testing {proxy.address}: {str(e)}{Style.RESET_ALL}")
            return False, 0

# ===========================================================================
# 2. DNS PRE-RESOLUTION AND CACHING
# ===========================================================================

class EnhancedDNSCache:
    """Advanced DNS cache with pre-resolution and TTL management"""
    
    def __init__(self, ttl: int = 300):
        self.cache = {}  # hostname -> (ip, timestamp)
        self.ttl = ttl
        self.lock = asyncio.Lock()
        self.popular_domains = set([
            "httpbin.org",
            "api.ipify.org",
            "ip-api.com",
            "cloudflare.com",
            "google.com",
            "aws.amazon.com",
            "example.com"
        ])
        self.resolvers = [
            "1.1.1.1",  # Cloudflare
            "8.8.8.8",  # Google
            "9.9.9.9",  # Quad9
            "208.67.222.222"  # OpenDNS
        ]
    
    async def pre_resolve_popular_domains(self):
        """Pre-resolve popular domains used for testing"""
        logger.info("Pre-resolving common domains...")
        
        for domain in self.popular_domains:
            await self.resolve(domain)
        
        logger.info(f"Pre-resolved {len(self.popular_domains)} domains")
    
    async def resolve(self, hostname: str) -> str:
        """Resolve hostname to IP with caching"""
        # Check cache first
        async with self.lock:
            cache_entry = self.cache.get(hostname)
            if cache_entry:
                ip, timestamp = cache_entry
                if time.time() - timestamp < self.ttl:
                    return ip
        
        # Not in cache or expired, resolve it
        try:
            # This is a simplified implementation
            # In production, use proper async DNS resolution
            loop = asyncio.get_running_loop()
            ip = await loop.run_in_executor(None, socket.gethostbyname, hostname)
            
            # Cache the result
            async with self.lock:
                self.cache[hostname] = (ip, time.time())
            
            return ip
        except socket.gaierror:
            logger.warning(f"Failed to resolve {hostname}")
            return None
    
    async def resolve_with_multiple_providers(self, hostname: str) -> str:
        """Resolve using multiple DNS providers for redundancy"""
        # Try built-in resolver first
        result = await self.resolve(hostname)
        if result:
            return result
        
        # If that fails, try specific DNS servers
        # This would require a more complex implementation with DNS library
        # For the concept, we'll return None to indicate failure
        return None
    
    def get_cached_domains(self) -> List[str]:
        """Get list of currently cached domains"""
        return list(self.cache.keys())



    async def _check_proxy_anonymity(self, proxy, response_body=None):
        """Check proxy anonymity level"""
        # This requires making a request to a specialized service that reports the details
        # of the connecting client. For basic functionality, we'll use a simplified approach.
        try:
            anonymity_test_url = "https://httpbin.org/headers"
            
            if proxy.protocol == ProxyType.HTTPS:
                proxy_url = f"https://{proxy.host}:{proxy.port}"
            else:
                proxy_url = f"http://{proxy.host}:{proxy.port}"
            
            async with self.session.get(
                anonymity_test_url,
                proxy=proxy_url,
                timeout=aiohttp.ClientTimeout(total=5),
                headers=self._get_random_headers(),
                ssl=False
            ) as response:
                if response.status != 200:
                    return
                
                data = await response.json()
                headers = data.get('headers', {})
                
                # Check for proxy headers
                proxy_headers = [
                    'Via', 'Forwarded', 'X-Forwarded-For', 'X-Proxy-Id',
                    'X-Real-Ip', 'Proxy-Connection', 'X-ProxyUser-Ip'
                ]
                
                disclosed_ip = False
                disclosed_proxy = False
                
                # Check if original IP is exposed
                for header in headers:
                    header_value = headers[header]
                    
                    # Look for IP addresses in header values
                    ip_matches = re.findall(r'(\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3})', header_value)
                    if ip_matches:
                        disclosed_ip = True
                    
                    # Check for proxy disclosure
                    if header.lower() in [h.lower() for h in proxy_headers]:
                        disclosed_proxy = True
                
                # Determine anonymity level
                if not disclosed_ip and not disclosed_proxy:
                    proxy.anonymity = "high"
                elif not disclosed_ip and disclosed_proxy:
                    proxy.anonymity = "medium"
                else:
                    proxy.anonymity = "low"
        
        except Exception as e:
            if self.args.verbose:
                print(f"{Fore.RED}Error checking anonymity for {proxy.address}: {str(e)}{Style.RESET_ALL}")
    
    async def export_results(self, force=False):
        """Export verified proxies to file"""
        async with self.export_lock:
            if not self.verified_proxies and not force:
                return
            
            # Export with formatted timestamp
            timestamp = datetime.now().strftime('%Y%m%d_%H%M%S')
            
            # Export as TXT
            txt_filename = f"working_proxies_{timestamp}.txt"
            with open(txt_filename, 'w') as f:
                for addr in sorted(self.verified_proxies):
                    proxy = self.proxies[addr]
                    f.write(f"{proxy.url}\n")
            
            # Export as JSON with detailed info
            json_filename = f"proxies_detailed_{timestamp}.json"
            detailed_data = []
            for addr in sorted(self.verified_proxies):
                proxy = self.proxies[addr]
                detailed_data.append(proxy.to_dict())
            
            with open(json_filename, 'w') as f:          
                json.dump(detailed_data, f, indent=2)
            
            # Export as CSV for spreadsheet compatibility
            csv_filename = f"working_proxies_{timestamp}.csv"
            with open(csv_filename, 'w', newline='') as f:
                writer = csv.writer(f)
                writer.writerow(['Protocol', 'Host', 'Port', 'Response Time (ms)', 'Reliability (%)', 'Country', 'Anonymity'])
                
                for addr in sorted(self.verified_proxies):
                    proxy = self.proxies[addr]
                    writer.writerow([
                        proxy.protocol,
                        proxy.host,
                        proxy.port,
                        f"{proxy.avg_response_time:.2f}",
                        f"{proxy.reliability:.1f}",
                        proxy.country or 'Unknown',
                        proxy.anonymity or 'Unknown'
                    ])
            
            # Count proxies by speed category
            speeds = {
                'high_speed': 0,
                'medium_speed': 0,
                'low_speed': 0
            }
            
            for addr in self.verified_proxies:
                proxy = self.proxies[addr]
                if proxy.avg_response_time < 500:  # Less than 500ms
                    speeds['high_speed'] += 1
                elif proxy.avg_response_time < 1500:  # Between 500ms and 1.5s
                    speeds['medium_speed'] += 1
                else:  # More than 1.5s
                    speeds['low_speed'] += 1
            
            self.proxy_stats.update({
                'high_speed': speeds['high_speed'],
                'medium_speed': speeds['medium_speed'],
                'low_speed': speeds['low_speed'],
                'total_working': len(self.verified_proxies)
            })
            
            print(f"{Fore.GREEN}Exported {len(self.verified_proxies)} verified proxies to:")
            print(f"  - {txt_filename} (Simple list)")
            print(f"  - {csv_filename} (CSV format)")
            print(f"  - {json_filename} (Detailed JSON){Style.RESET_ALL}")
    
    def display_stats(self):
        """Display statistics about the proxy scan"""
        if not self.proxies:
            print(f"{Fore.YELLOW}No statistics available yet.{Style.RESET_ALL}")
            return
        
        uptime = time.time() - self.last_fetch_time if self.last_fetch_time else 0
        
        # Calculate overall statistics
        total_discovered = self.proxy_stats['total_discovered']
        total_working = self.proxy_stats['total_working']
        success_rate = (total_working / total_discovered * 100) if total_discovered > 0 else 0
        
        print(f"\n{Fore.CYAN}======= PROXY SCAN STATISTICS ======={Style.RESET_ALL}")
        print(f"{Fore.WHITE}Total discovered proxies: {total_discovered}")
        print(f"Working proxies: {total_working} ({success_rate:.1f}%)")
        print(f"Speed categories:")
        print(f"  {Fore.GREEN}High speed (<500ms): {self.proxy_stats['high_speed']}")
        print(f"  {Fore.YELLOW}Medium speed (<1.5s): {self.proxy_stats['medium_speed']}")
        print(f"  {Fore.RED}Low speed (>1.5s): {self.proxy_stats['low_speed']}{Style.RESET_ALL}")
        
        # Display top countries if available
        if self.region_stats:
            print(f"\n{Fore.CYAN}Top countries:{Style.RESET_ALL}")
            sorted_regions = sorted(self.region_stats.items(), key=lambda x: x[1], reverse=True)
            for country, count in sorted_regions[:5]:
                print(f"  {country}: {count} proxies")
        
        print(f"{Fore.CYAN}====================================={Style.RESET_ALL}\n")
    
    def _get_random_headers(self):
        """Generate random headers to avoid detection"""
        return {
            'User-Agent': random.choice(USER_AGENTS),
            'Accept': 'text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8',
            'Accept-Language': 'en-US,en;q=0.5',
            'Connection': 'keep-alive',
            'Upgrade-Insecure-Requests': '1',
            'Cache-Control': 'max-age=0',
            'TE': 'Trailers',
        }
# ===========================================================================
# 3. HEADER COMPRESSION AND BANDWIDTH OPTIMIZATION
# ===========================================================================

class RequestOptimizer:
    """Optimizes HTTP requests for minimal bandwidth usage"""
    
    def __init__(self):
        self.compressed_headers = {
            "User-Agent": "cps",  # Compressed Proxy Scanner
            "Accept": "*/*",
            "Connection": "keep-alive"
        }
        
        # Map of full header names to short versions for compression
        self.header_compression_map = {
            "User-Agent": "u",
            "Accept": "a", 
            "Accept-Language": "al",
            "Accept-Encoding": "ae",
            "Connection": "c",
            "Cache-Control": "cc",
            "Content-Type": "ct",
            "Content-Length": "cl",
            "Host": "h",
            "Origin": "o",
            "Referer": "r"
        }
    
    def get_minimal_headers(self) -> Dict[str, str]:
        """Get minimal HTTP headers for testing"""
        return self.compressed_headers
    
    def compress_request_data(self, data: str) -> bytes:
        """Compress request data using gzip"""
        return gzip.compress(data.encode())
    
    def create_binary_test_packet(self) -> bytes:
        """Create a binary protocol test packet (more efficient than HTTP)"""
        # Format: [4-byte magic number][1-byte version][1-byte command][4-byte timestamp]
        # This is a minimal binary protocol for testing proxy connectivity
        
        magic = b"CPST"  # Compressed Proxy Scanner Test
        version = 1
        command = 1  # 1 = ping
        timestamp = int(time.time()) & 0xFFFFFFFF
        
        return struct.pack("!4sBBI", magic, version, command, timestamp)
    
    def optimize_url_for_proxy_test(self, url: str) -> str:
        """Optimize URL for proxy testing by removing unnecessary components"""
        # Remove query parameters except essential ones
        if "?" in url:
            base, query = url.split("?", 1)
            essential_params = ["format=json"]
            
            if any(param in query for param in essential_params):
                # Keep only essential parameters
                new_query = "&".join(p for p in query.split("&") if any(e in p for e in essential_params))
                return f"{base}?{new_query}"
            else:
                return base
        
        return url

async def async_dns_resolve(host):
    """Asynchronously resolve DNS with caching"""
    global dns_cache
    
    async with dns_lock:
        if host in dns_cache:
            return dns_cache[host]
    
    try:
        # This is a simplified approach, a real implementation would use asyncio-friendly DNS resolution
        loop = asyncio.get_running_loop()
        ip_addresses = await loop.run_in_executor(None, socket.gethostbyname, host)
        
        async with dns_lock:
            dns_cache[host] = ip_addresses
        
        return ip_addresses
    except socket.gaierror:
        return None


async def main():
    """Main entry point"""
    parser = argparse.ArgumentParser(description='RapidProxyScan - High Performance Proxy Scanner')
    
    parser.add_argument('--interval', dest='check_interval', type=int, default=30,
                        help='Interval between scan cycles in seconds (default: 30)')
    
    parser.add_argument('--connections', dest='connection_limit', type=int, default=200,
                        help='Maximum number of concurrent connections (default: 200)')
    
    parser.add_argument('--validation-rounds', dest='validation_rounds', type=int, default=3,
                        help='Number of validation rounds for each proxy (default: 3)')
    
    parser.add_argument('--validation-mode', dest='validation_mode', 
                        choices=['any', 'majority', 'all'], default='majority',
                        help='Validation success criteria (default: majority)')
    
    parser.add_argument('--timeout', dest='timeout', type=float, default=5.0,
                        help='Timeout for proxy connections in seconds (default: 5.0)')
    
    parser.add_argument('--anonymity', dest='check_anonymity', action='store_true',
                        help='Enable proxy anonymity checking')
    
    parser.add_argument('--force-fetch', dest='force_fetch', action='store_true',
                        help='Force fetch proxy lists on each cycle')
    
    parser.add_argument('--verbose', dest='verbose', action='store_true',
                        help='Enable verbose output')
    
    parser.add_argument('--single-run', dest='single_run', action='store_true',
                        help='Run once and exit')
    
    args = parser.parse_args()
    
    # Apply global timeouts based on args
    global CONNECT_TIMEOUT, READ_TIMEOUT
    CONNECT_TIMEOUT = args.timeout / 2
    READ_TIMEOUT = args.timeout
    
    scanner = ProxyScanManager(args)
    await scanner.run()


if __name__ == "__main__":
    # Initialize colorama for Windows compatibility
    init()
    
    # Print banner
    print(f"""
{Fore.CYAN}=================================================
 ____             _     _ ____                      
|  _ \ __ _ _ __ (_) __| |  _ \ _ __ _____  ___   _ 
| |_) / _` | '_ \| |/ _` | |_) | '__/ _ \ \/ / | | |
|  _ < (_| | |_) | | (_| |  __/| | | (_) >  <| |_| |
|_| \_\\__,_| .__/|_|\__,_|_|   |_|  \___/_/\_\\__, |
           |_|                                |___/ 
 ____                                             
/ ___|  ___ __ _ _ __  _ __   ___ _ __           
\___ \ / __/ _` | '_ \| '_ \ / _ \ '__|          
 ___) | (_| (_| | | | | | | |  __/ |             
|____/ \___\__,_|_| |_|_| |_|\___|_|  v1.0       
=================================================
{Style.RESET_ALL}""")
    
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print(f"{Fore.YELLOW}Scanner interrupted. Exiting...{Style.RESET_ALL}")
    except Exception as e:
        print(f"{Fore.RED}Error: {str(e)}{Style.RESET_ALL}")


# Additional utility functions for standalone usage

def run_scanner(check_interval=30, connection_limit=200, validation_rounds=3, 
               validation_mode='majority', timeout=5.0, check_anonymity=False,
               force_fetch=False, verbose=False, single_run=False):
    """Run the scanner with specified settings from another Python script"""
    class Args:
        pass
    
    args = Args()
    args.check_interval = check_interval
    args.connection_limit = connection_limit
    args.validation_rounds = validation_rounds
    args.validation_mode = validation_mode
    args.timeout = timeout
    args.check_anonymity = check_anonymity
    args.force_fetch = force_fetch
    args.verbose = verbose
    args.single_run = single_run
    
    # Apply global timeouts based on args
    global CONNECT_TIMEOUT, READ_TIMEOUT
    CONNECT_TIMEOUT = timeout / 2
    READ_TIMEOUT = timeout
    
    scanner = ProxyScanManager(args)
    asyncio.run(scanner.run())


def scan_proxy_list(proxy_list, validation_rounds=3, timeout=5.0):
    """Scan a specific list of proxies and return working ones"""
    class Args:
        pass
    
    args = Args()
    args.check_interval = 30  # Not used in this case
    args.connection_limit = 200
    args.validation_rounds = validation_rounds
    args.validation_mode = 'majority'
    args.timeout = timeout
    args.check_anonymity = False
    args.force_fetch = False
    args.verbose = False
    args.single_run = True
    
    # Apply global timeouts based on args
    global CONNECT_TIMEOUT, READ_TIMEOUT
    CONNECT_TIMEOUT = timeout / 2
    READ_TIMEOUT = timeout
    
    async def run_specific_scan():
        scanner = ProxyScanManager(args)
        await scanner.initialize()
        
        # Add the proxies to scan
        for proxy_str in proxy_list:
            try:
                if '://' in proxy_str:
                    # Handle URL format (http://host:port)
                    parts = proxy_str.split('://')
                    proto = parts[0]
                    host_port = parts[1].split(':')
                    host = host_port[0]
                    port = int(host_port[1]) if len(host_port) > 1 else 80
                else:
                    # Handle host:port format
                    host_port = proxy_str.split(':')
                    host = host_port[0]
                    port = int(host_port[1]) if len(host_port) > 1 else 80
                    proto = ProxyType.HTTP
                
                scanner._add_proxy(host, port, proto)
            except Exception as e:
                print(f"Error parsing proxy {proxy_str}: {e}")
        
        # Scan the proxies
        await scanner.scan_all_proxies()
        
        # Export results
        await scanner.export_results(force=True)
        
        # Return working proxies
        working = []
        for addr in scanner.verified_proxies:
            proxy = scanner.proxies[addr]
            working.append({
                'url': proxy.url,
                'host': proxy.host,
                'port': proxy.port,
                'protocol': proxy.protocol,
                'response_time': proxy.avg_response_time,
                'reliability': proxy.reliability,
                'country': proxy.country,
                'anonymity': proxy.anonymity
            })
        
        await scanner.cleanup()
        return working
    
    return asyncio.run(run_specific_scan())

# ===========================================================================
# 4. VIEWBOT/SUPREME SPECIALIZED FORMATTING
# ===========================================================================

class ViewbotFormatter:
    """Specialized formatter for viewbot/Supreme integration"""
    
    def __init__(self):
        self.speed_categories = {
            "ultra": [],    # <200ms
            "fast": [],     # 200-500ms
            "medium": [],   # 500-1000ms
            "slow": []      # >1000ms
        }
        
        self.reliability_categories = {
            "platinum": [],  # 100% success
            "gold": [],      # 90-99% success
            "silver": [],    # 75-89% success
            "bronze": []     # <75% success
        }
        
        self.stealth_categories = {
            "elite": [],     # High anonymity
            "anonymous": [], # Medium anonymity
            "transparent": [] # Low anonymity
        }
        
        # Keep track of persistent proxies (seen in multiple scans)
        self.persistent_proxies = set()
        self.persistence_counter = defaultdict(int)
        
        # History of all seen proxies
        self.proxy_history = {}
        
        # Fast track list - proxies that can skip some validation steps
        self.fast_track_proxies = set()
    
    def categorize_proxy(self, proxy: dict):
        """Categorize a proxy by speed, reliability, and stealth"""
        # Speed categorization
        response_time = proxy.get("avg_response_time", 1000)
        if response_time < 200:
            self.speed_categories["ultra"].append(proxy)
        elif response_time < 500:
            self.speed_categories["fast"].append(proxy)
        elif response_time < 1000:
            self.speed_categories["medium"].append(proxy)
        else:
            self.speed_categories["slow"].append(proxy)
        
        # Reliability categorization
        reliability = proxy.get("reliability", 0)
        if reliability >= 100:
            self.reliability_categories["platinum"].append(proxy)
        elif reliability >= 90:
            self.reliability_categories["gold"].append(proxy)
        elif reliability >= 75:
            self.reliability_categories["silver"].append(proxy)
        else:
            self.reliability_categories["bronze"].append(proxy)
        
        # Stealth categorization
        anonymity = proxy.get("anonymity", "unknown")
        if anonymity == "high":
            self.stealth_categories["elite"].append(proxy)
        elif anonymity == "medium":
            self.stealth_categories["anonymous"].append(proxy)
        else:
            self.stealth_categories["transparent"].append(proxy)
        
        # Update persistence tracking
        address = f"{proxy['host']}:{proxy['port']}"
        self.persistence_counter[address] += 1
        
        if self.persistence_counter[address] >= 3:
            self.persistent_proxies.add(address)
            # Add to fast track list if also reliable
            if reliability >= 85:
                self.fast_track_proxies.add(address)
        
        # Update history
        self.proxy_history[address] = {
            "last_seen": time.time(),
            "times_seen": self.persistence_counter[address],
            "reliability": reliability,
            "response_time": response_time,
            "anonymity": anonymity
        }
    
    def is_fast_track_proxy(self, proxy_address: str) -> bool:
        """Check if a proxy is in the fast track list"""
        return proxy_address in self.fast_track_proxies
    
    def get_persistent_proxies(self) -> List[str]:
        """Get list of persistent proxies"""
        return list(self.persistent_proxies)
    
    def export_for_viewbot(self, filename: str, filters: Dict[str, Any] = None):
        """Export specialized format for viewbot integration"""
        # Default filters
        if filters is None:
            filters = {
                "min_reliability": 75,
                "max_response_time": 1000,
                "min_anonymity": "medium"
            }
        
        # Collect proxies matching filters
        matching_proxies = []
        
        # Helper to check if proxy matches filters
        def matches_filters(proxy):
            if proxy.get("reliability", 0) < filters["min_reliability"]:
                return False
            if proxy.get("avg_response_time", 9999) > filters["max_response_time"]:
                return False
            
            anonymity = proxy.get("anonymity", "unknown")
            if filters["min_anonymity"] == "high" and anonymity != "high":
                return False
            if filters["min_anonymity"] == "medium" and anonymity not in ["high", "medium"]:
                return False
            
            return True
        
        # First add persistent + fast proxies
        for category in ["ultra", "fast"]:
            for proxy in self.speed_categories[category]:
                address = f"{proxy['host']}:{proxy['port']}"
                if address in self.persistent_proxies and matches_filters(proxy):
                    proxy["persistent"] = True
                    proxy["fast_track"] = address in self.fast_track_proxies
                    matching_proxies.append(proxy)
        
        # Then add other good proxies
        for category in ["medium"]:
            for proxy in self.speed_categories[category]:
                address = f"{proxy['host']}:{proxy['port']}"
                if address not in self.persistent_proxies and matches_filters(proxy):
                    proxy["persistent"] = False
                    proxy["fast_track"] = False
                    matching_proxies.append(proxy)
        
        # Sort by combined score (reliability * speed * anonymity bonus)
        def score_proxy(proxy):
            reliability = proxy.get("reliability", 0)
            speed_factor = 1.0 - min(1.0, proxy.get("avg_response_time", 1000) / 1000.0)
            
            anonymity_bonus = 1.0
            if proxy.get("anonymity") == "high":
                anonymity_bonus = 1.5
            elif proxy.get("anonymity") == "medium":
                anonymity_bonus = 1.2
            
            persistence_bonus = 1.5 if proxy.get("persistent", False) else 1.0
            
            return (reliability/100.0) * speed_factor * anonymity_bonus * persistence_bonus
        
        matching_proxies.sort(key=score_proxy, reverse=True)
        
        # Export in viewbot format
        with open(filename, 'w') as f:
            f.write("# RAPIDPROXYSCAN ELITE EXPORT FOR VIEWBOT/SUPREME\n")
            f.write(f"# Generated: {time.strftime('%Y-%m-%d %H:%M:%S')}\n")
            f.write(f"# Total proxies: {len(matching_proxies)}\n")
            f.write("#\n")
            f.write("# FORMAT: [protocol]://[host]:[port] [speed_ms] [reliability%] [anonymity] [persistence]\n")
            f.write("#\n")
            
            for proxy in matching_proxies:
                protocol = proxy.get("protocol", "http")
                host = proxy.get("host", "")
                port = proxy.get("port", 80)
                response_time = proxy.get("avg_response_time", 0)
                reliability = proxy.get("reliability", 0)
                anonymity = proxy.get("anonymity", "unknown")
                persistent = "P" if proxy.get("persistent", False) else "-"
                fast_track = "F" if proxy.get("fast_track", False) else "-"
                
                flags = f"{persistent}{fast_track}"
                
                f.write(f"{protocol}://{host}:{port} {response_time:.1f} {reliability:.1f} {anonymity} {flags}\n")
        
        logger.info(f"Exported {len(matching_proxies)} proxies in viewbot format to {filename}")
        return len(matching_proxies)
    
    def export_for_supreme(self, filename: str):
        """Export specialized format for Supreme bot integration"""
        # Supreme format prioritizes elite proxies with consistent success rates
        
        # Select only the highest quality proxies
        supreme_proxies = []
        
        # First add platinum+gold elite proxies
        for proxy in self.reliability_categories["platinum"] + self.reliability_categories["gold"]:
            if proxy.get("anonymity") == "high":
                supreme_proxies.append(proxy)
        
        # Then add persistent proxies with good reliability
        for address in self.persistent_proxies:
            if address.split(":")[0] in [p["host"] for p in supreme_proxies]:
                continue  # Already added
                
            history = self.proxy_history.get(address, {})
            if history.get("reliability", 0) >= 80:
                host, port = address.split(":")
                supreme_proxies.append({
                    "host": host,
                    "port": int(port),
                    "protocol": "https",  # Supreme prefers HTTPS
                    "reliability": history.get("reliability", 0),
                    "avg_response_time": history.get("response_time", 1000),
                    "anonymity": history.get("anonymity", "unknown"),
                    "persistent": True
                })
        
        # Sort by reliability first, then speed
        supreme_proxies.sort(key=lambda p: (p.get("reliability", 0), -p.get("avg_response_time", 1000)), reverse=True)
        
        # Export in Supreme format
        with open(filename, 'w') as f:
            f.write("# RAPIDPROXYSCAN SUPREME EXPORT\n")
            f.write(f"# Generated: {time.strftime('%Y-%m-%d %H:%M:%S')}\n")
            f.write(f"# Total proxies: {len(supreme_proxies)}\n")
            f.write("#\n")
            f.write("# FORMAT: [host]:[port]\n")
            f.write("#\n")
            
            # Supreme format is just host:port
            for proxy in supreme_proxies:
                f.write(f"{proxy['host']}:{proxy['port']}\n")
        
        logger.info(f"Exported {len(supreme_proxies)} elite proxies in Supreme format to {filename}")
        return len(supreme_proxies)
    
    def export_specialized_formats(self, base_dir: str = "."):
        """Export all specialized formats"""
        timestamp = time.strftime('%Y%m%d_%H%M%S')
        
        # Ensure directory exists
        os.makedirs(base_dir, exist_ok=True)
        
        # Export for different tools
        viewbot_file = os.path.join(base_dir, f"viewbot_proxies_{timestamp}.txt")
        supreme_file = os.path.join(base_dir, f"supreme_proxies_{timestamp}.txt")
        
        viewbot_count = self.export_for_viewbot(viewbot_file)
        supreme_count = self.export_for_supreme(supreme_file)
        
        # Export statistics
        stats_file = os.path.join(base_dir, f"proxy_stats_{timestamp}.json")
        with open(stats_file, 'w') as f:
            stats = {
                "timestamp": timestamp,
                "total_proxies": sum(len(c) for c in self.speed_categories.values()),
                "speed_categories": {k: len(v) for k, v in self.speed_categories.items()},
                "reliability_categories": {k: len(v) for k, v in self.reliability_categories.items()},
                "stealth_categories": {k: len(v) for k, v in self.stealth_categories.items()},
                "persistent_proxies": len(self.persistent_proxies),
                "fast_track_proxies": len(self.fast_track_proxies),
                "exports": {
                    "viewbot": viewbot_count,
                    "supreme": supreme_count
                }
            }
            json.dump(stats, f, indent=2)
        
        return {
            "viewbot": viewbot_file,
            "supreme": supreme_file,
            "stats": stats_file
        }


# ===========================================================================
# 5. INTEGRATION WITH MAIN SCANNER
# ===========================================================================

class OptimizerIntegration:
    """Integrates all optimizations with the main scanner"""
    
    def __init__(self):
        self.dns_cache = EnhancedDNSCache()
        self.endpoint_manager = CustomEndpointManager()
        self.request_optimizer = RequestOptimizer()
        self.viewbot_formatter = ViewbotFormatter()
        self.initialization_complete = False
        
        # Performance metrics
        self.metrics = {
            "dns_cache_hits": 0,
            "dns_cache_misses": 0,
            "endpoint_selections": defaultdict(int),
            "connection_reuse_count": 0,
            "optimized_requests_sent": 0,
            "binary_protocol_tests": 0
        }
    
    async def initialize(self):
        """Initialize all optimization components"""
        if self.initialization_complete:
            return
        
        logger.info("Initializing advanced optimization components...")
        
        # Pre-resolve common domains
        await self.dns_cache.pre_resolve_popular_domains()
        
        # Initialize custom endpoints
        endpoints = await self.endpoint_manager.create_endpoint_selection_map()
        logger.info(f"Initialized with {sum(len(e) for e in endpoints.values())} optimized endpoints")
        
        self.initialization_complete = True
        logger.info("Advanced optimizations initialized successfully")
    
    async def optimize_proxy_test(self, session, proxy_info):
        """Apply all optimizations to a proxy test"""
        if not self.initialization_complete:
            await self.initialize()
        
        # 1. Select optimal endpoint based on proxy location
        endpoint = await self.endpoint_manager.select_optimal_endpoint(proxy_info)
        self.metrics["endpoint_selections"][endpoint] += 1
        
        # 2. Optimize the URL
        optimized_url = self.request_optimizer.optimize_url_for_proxy_test(endpoint)
        
        # 3. Get minimal headers
        headers = self.request_optimizer.get_minimal_headers()
        
        # 4. Check if proxy is in fast track list
        address = f"{proxy_info['host']}:{proxy_info['port']}"
        is_fast_track = self.viewbot_formatter.is_fast_track_proxy(address)
        
        # 5. Use binary protocol for ultra-fast testing if supported
        use_binary_protocol = is_fast_track and random.random() < 0.3  # 30% chance for fast track proxies
        
        # Return the optimized test configuration
        return {
            "url": optimized_url,
            "headers": headers,
            "is_fast_track": is_fast_track,
            "use_binary_protocol": use_binary_protocol,
        }
    
    async def post_test_processing(self, proxy_info, success, response_time, endpoint):
        """Process results after a proxy test"""
        # Update endpoint health metrics
        await self.endpoint_manager.update_endpoint_health(endpoint, success, response_time)
        
        # If test was successful, categorize the proxy
        if success:
            self.viewbot_formatter.categorize_proxy(proxy_info)
    
    def export_specialized_formats(self, base_dir="."):
        """Export all specialized formats for downstream tools"""
        return self.viewbot_formatter.export_specialized_formats(base_dir)
    
    def get_metrics(self):
        """Get performance metrics"""
        return self.metrics
    
    def get_fast_track_proxies(self):
        """Get list of proxies that can be fast-tracked"""
        return self.viewbot_formatter.get_persistent_proxies()


# ===========================================================================
# USAGE EXAMPLE
# ===========================================================================

async def demo_usage():
    """Demonstrate usage of the optimization module"""
    optimizer = OptimizerIntegration()
    await optimizer.initialize()
    
    # Example proxy info
    proxy_info = {
        "host": "123.45.67.89",
        "port": 8080,
        "protocol": "http",
        "country": "us",
        "avg_response_time": 350,
        "reliability": 95,
        "anonymity": "high"
    }
    
    # Optimize a proxy test
    session = None  # Would be aiohttp.ClientSession in real usage
    test_config = await optimizer.optimize_proxy_test(session, proxy_info)
    
    print("Optimized test configuration:")
    print(f"URL: {test_config['url']}")
    print(f"Headers: {test_config['headers']}")
    print(f"Fast track: {test_config['is_fast_track']}")
    print(f"Binary protocol: {test_config['use_binary_protocol']}")
    
    # Simulate test result
    await optimizer.post_test_processing(
        proxy_info, 
        success=True, 
        response_time=350,
        endpoint=test_config['url']
    )
    
    # Export specialized formats
    export_paths = optimizer.export_specialized_formats()
    print(f"Exported specialized formats: {export_paths}")
    
    # Get metrics
    metrics = optimizer.get_metrics()
    print(f"Performance metrics: {metrics}")


if __name__ == "__main__":
    # Run the demo
    asyncio.run(demo_usage())


# Example usage of the standalone functions
if __name__ == "__main__":
    # Run the scanner with default settings
    run_scanner()
    
    # Alternatively, scan a specific list of proxies
    # proxies = [
    #     "http://123.45.67.89:8080",
    #     "https://98.76.54.32:3128",
    #     "1.2.3.4:8000"
    # ]
    # working_proxies = scan_proxy_list(proxies)
    # print(f"Found {len(working_proxies)} working proxies")