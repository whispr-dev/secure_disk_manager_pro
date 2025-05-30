"""
RapidProxyScan - High Performance Proxy Scanner
==============================================

A fast, efficient proxy scanner that:
1. Automatically fetches proxy lists from multiple sources
2. Tests proxies using asynchronous I/O for maximum performance
3. Performs triple validation for reliability
4. Exports working proxies as formatted text

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
from datetime import datetime
from bs4 import BeautifulSoup
from urllib.parse import urlparse
from colorama import Fore, Style, init
from concurrent.futures import ThreadPoolExecutor
from collections import defaultdict

# Initialize colorama
init()

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