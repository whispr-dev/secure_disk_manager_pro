"""
RapidProxyScan - Usage Guide
============================

This guide shows you how to use the RapidProxyScan tool to efficiently find, test,
and export working proxies with minimal configuration.
"""

# Basic Usage (Command Line)
"""
# 1. Install required dependencies
pip install aiohttp bs4 colorama

# 2. Save the RapidProxyScan code to a file named rapidproxyscan.py

# 3. Run with default settings (scan every 30 seconds)
python rapidproxyscan.py

# 4. Run with custom settings
python rapidproxyscan.py --interval 60 --connections 300 --validation-rounds 2 --anonymity

# 5. Run once and exit (for scheduled tasks/cron jobs)
python rapidproxyscan.py --single-run
"""

# ----------------------------------------
# Example: Custom Usage in Your Own Script
# ----------------------------------------

# Import the scanner functions
from rapidproxyscan import run_scanner, scan_proxy_list

# Option 1: Run the full scanner with custom settings
def run_custom_scanner():
    run_scanner(
        check_interval=60,          # Scan every 60 seconds
        connection_limit=150,       # Use 150 concurrent connections
        validation_rounds=2,        # Test each proxy twice
        validation_mode='any',      # Consider proxy valid if any test passes
        timeout=3.0,                # Connection timeout of 3 seconds
        check_anonymity=True,       # Check proxy anonymity level
        verbose=True,               # Show detailed logs
        single_run=False            # Run continuously
    )

# Option 2: Test a specific list of proxies
def test_specific_proxies():
    my_proxies = [
        "http://123.45.67.89:8080",
        "https://98.76.54.32:3128",
        "1.2.3.4:8000"
    ]
    
    working_proxies = scan_proxy_list(
        proxy_list=my_proxies,
        validation_rounds=3,
        timeout=5.0
    )
    
    print(f"Found {len(working_proxies)} working proxies:")
    for proxy in working_proxies:
        print(f"  - {proxy['url']} ({proxy['response_time']:.2f}ms, {proxy['reliability']:.1f}%)")

# ----------------------------------------
# Example: Integration with Elite Proxy Switcher
# ----------------------------------------

import subprocess
import os
import time

def update_elite_proxy_switcher():
    # 1. Run the scanner to find working proxies
    run_scanner(single_run=True)
    
    # 2. Find the most recent export file
    export_files = [f for f in os.listdir('.') if f.startswith('working_proxies_') and f.endswith('.txt')]
    if not export_files:
        print("No export files found")
        return
    
    latest_file = max(export_files, key=os.path.getmtime)
    
    # 3. Prepare the formatted proxy list for Elite Proxy Switcher
    with open(latest_file, 'r') as infile:
        proxies = infile.read().splitlines()
    
    # 4. Format proxies as needed by Elite Proxy Switcher
    formatted_proxies = []
    for proxy in proxies:
        # Ensure format is correct (http(s)://host:port)
        if '://' in proxy:
            formatted_proxies.append(proxy)
        else:
            # Default to http if protocol not specified
            formatted_proxies.append(f"http://{proxy}")
    
    # 5. Write to Elite Proxy Switcher import file
    eps_import_file = "elite_proxy_list.txt"
    with open(eps_import_file, 'w') as outfile:
        outfile.write('\n'.join(formatted_proxies))
    
    print(f"Exported {len(formatted_proxies)} proxies to {eps_import_file}")
    
    # 6. Optionally, auto-import to Elite Proxy Switcher (if installed)
    eps_path = r"C:\Elite Proxy Switcher\EPS.exe"
    if os.path.exists(eps_path):
        try:
            # Import the proxy list (check EPS documentation for correct command line options)
            subprocess.run([eps_path, f"/import:{os.path.abspath(eps_import_file)}"])
            print("Proxies automatically imported to Elite Proxy Switcher")
        except Exception as e:
            print(f"Failed to auto-import: {e}")
            print("Please import the list manually")

# ----------------------------------------
# Example: Scheduled Automation
# ----------------------------------------

def scheduled_proxy_updates():
    """Run this function to set up automatic proxy updates every hour"""
    while True:
        print(f"Starting proxy scan at {time.strftime('%Y-%m-%d %H:%M:%S')}")
        
        # Run scanner with single run mode
        run_scanner(
            check_interval=30,
            validation_rounds=2,
            timeout=4.0,
            single_run=True
        )
        
        # Update Elite Proxy Switcher if needed
        # update_elite_proxy_switcher()
        
        print(f"Scan completed at {time.strftime('%Y-%m-%d %H:%M:%S')}")
        print(f"Waiting for next scheduled scan...")
        
        # Wait for the next hour
        time.sleep(3600)  # 1 hour in seconds

# ----------------------------------------
# Example: Using the Results API
# ----------------------------------------

import json

def load_and_use_working_proxies():
    """Example of loading and using the detailed proxy results"""
    
    # Find most recent detailed JSON export
    json_files = [f for f in os.listdir('.') if f.startswith('proxies_detailed_') and f.endswith('.json')]
    if not json_files:
        print("No detailed proxy data found")
        return
    
    latest_json = max(json_files, key=os.path.getmtime)
    
    # Load the proxies
    with open(latest_json, 'r') as f:
        proxies = json.load(f)
    
    # Example: Get the fastest proxy
    fastest_proxy = min(proxies, key=lambda p: p['avg_response_time'])
    print(f"Fastest proxy: {fastest_proxy['protocol']}://{fastest_proxy['host']}:{fastest_proxy['port']} "
          f"({fastest_proxy['avg_response_time']:.2f}ms)")
    
    # Example: Get highly anonymous proxies
    anon_proxies = [p for p in proxies if p.get('anonymity') == 'high']
    print(f"Found {len(anon_proxies)} highly anonymous proxies")
    
    # Example: Get proxies by country
    by_country = {}
    for proxy in proxies:
        country = proxy.get('country', 'Unknown')
        if country not in by_country:
            by_country[country] = []
        by_country[country].append(proxy)
    
    for country, country_proxies in by_country.items():
        print(f"{country}: {len(country_proxies)} proxies")

# ----------------------------------------
# Example: Web API Integration
# ----------------------------------------

def create_simple_proxy_api():
    """Example of creating a simple API to serve working proxies"""
    from http.server import BaseHTTPRequestHandler, HTTPServer
    import json
    
    class ProxyAPIHandler(BaseHTTPRequestHandler):
        def do_GET(self):
            if self.path == '/api/proxies':
                # Find most recent detailed JSON export
                json_files = [f for f in os.listdir('.') if f.startswith('proxies_detailed_') and f.endswith('.json')]
                if not json_files:
                    self.send_response(404)
                    self.send_header('Content-type', 'application/json')
                    self.end_headers()
                    self.wfile.write(json.dumps({'error': 'No proxy data available'}).encode())
                    return
                
                latest_json = max(json_files, key=os.path.getmtime)
                
                # Load the proxies
                with open(latest_json, 'r') as f:
                    proxies = json.load(f)
                
                self.send_response(200)
                self.send_header('Content-type', 'application/json')
                self.end_headers()
                self.wfile.write(json.dumps(proxies, indent=2).encode())
            else:
                self.send_response(404)
                self.send_header('Content-type', 'application/json')
                self.end_headers()
                self.wfile.write(json.dumps({'error': 'Not found'}).encode())
    
    # Start the server
    server_address = ('', 8000)
    httpd = HTTPServer(server_address, ProxyAPIHandler)
    print("Proxy API server started at http://localhost:8000/api/proxies")
    httpd.serve_forever()

# ----------------------------------------
# Commands Reference
# ----------------------------------------

"""
Command line options:

  --interval SECONDS       Interval between scan cycles (default: 30)
  --connections LIMIT      Maximum concurrent connections (default: 200)
  --validation-rounds N    Number of validation tests per proxy (default: 3)
  --validation-mode MODE   Success criteria: any, majority, all (default: majority)
  --timeout SECONDS        Connection timeout (default: 5.0)
  --anonymity              Enable proxy anonymity checking
  --force-fetch            Force fetch proxy lists on each cycle
  --verbose                Show detailed logs
  --single-run             Run once and exit

Output files:

  working_proxies_DATE.txt      - Simple list of working proxies with URLs
  working_proxies_DATE.csv      - CSV file with proxy details for spreadsheets
  proxies_detailed_DATE.json    - Detailed JSON with all proxy information
"""

# If this script is run directly, show basic usage
if __name__ == "__main__":
    print("RapidProxyScan Usage Guide")
    print("=========================")
    print("")
    print("To use RapidProxyScan, run the main script:")
    print("  python rapidproxyscan.py")
    print("")
    print("For more options and examples, see the documentation above.")