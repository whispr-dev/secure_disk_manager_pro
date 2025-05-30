"""
RapidProxyScan - Quick Start Guide
=================================

A high-performance proxy scanner that automatically fetches, tests, and exports working proxies
with ultra-fast concurrent processing and triple validation for reliable results.

Follow these steps to get up and running in just a few minutes!
"""

# ===================================================
# STEP 1: SETUP
# ===================================================

"""
1. INSTALL REQUIRED DEPENDENCIES
-------------------------------

Ensure you have Python 3.7 or higher installed, then install the required packages:

```bash
pip install aiohttp beautifulsoup4 colorama
```

2. DOWNLOAD THE CODE FILES
-------------------------

Save the following files to your project directory:
- rapidproxyscan.py (Main scanner code)
- config.py (Configuration file)

3. BASIC CONFIGURATION
--------------------

The default settings work great out of the box, but you can customize the scanner 
by editing config.py if needed.
"""

# ===================================================
# STEP 2: RUNNING THE SCANNER
# ===================================================

"""
OPTION 1: QUICK SCAN WITH DEFAULT SETTINGS
----------------------------------------

Run this command to start scanning with default settings (checks every 30 seconds):

```bash
python rapidproxyscan.py
```

OPTION 2: SINGLE SCAN (FOR SCHEDULED TASKS)
-----------------------------------------

To run once and exit (good for cron jobs or scheduled tasks):

```bash
python rapidproxyscan.py --single-run
```

OPTION 3: CUSTOM CONFIGURATION
----------------------------

Adjust parameters directly from command line:

```bash
python rapidproxyscan.py --interval 60 --connections 300 --validation-rounds 2 --timeout 3.0
```

COMMON PARAMETERS:
- --interval: Seconds between scan cycles (default: 30)
- --connections: Maximum concurrent connections (default: 200)
- --validation-rounds: Tests per proxy (default: 3)
- --timeout: Connection timeout in seconds (default: 5.0)
- --verbose: Show detailed logs
"""

# ===================================================
# STEP 3: RESULTS AND OUTPUT
# ===================================================

"""
OUTPUT FILES
-----------

The scanner automatically generates these files:

1. working_proxies_[timestamp].txt
   - Simple list of working proxies in URL format
   - Perfect for direct use in applications

2. working_proxies_[timestamp].csv
   - CSV spreadsheet with full proxy details
   - Great for sorting and filtering in Excel

3. proxies_detailed_[timestamp].json
   - Comprehensive JSON with all proxy information
   - Ideal for programmatic use and API integration

ELITE PROXY SWITCHER INTEGRATION
------------------------------

To use the proxies with Elite Proxy Switcher:

1. Run the scanner: `python rapidproxyscan.py --single-run`
2. Import the generated working_proxies_[timestamp].txt file into Elite Proxy Switcher

Alternatively, enable automatic imports in config.py:
1. Set AUTO_IMPORT_TO_EPS = True
2. Verify EPS_PATH points to your Elite Proxy Switcher executable
3. Run the scanner with: `python rapidproxyscan.py --eps-import`
"""

# ===================================================
# STEP 4: AUTOMATING UPDATES
# ===================================================

"""
SCHEDULE AUTOMATIC UPDATES
------------------------

1. Windows Task Scheduler (Easy setup):
   ```bash
   python rapidproxyscan.py --setup-auto-update
   ```
   This creates a task that runs hourly.

2. Cron job (Linux/Mac):
   Add this line to your crontab to run hourly:
   ```
   0 * * * * cd /path/to/rapidproxyscan && python rapidproxyscan.py --single-run
   ```
"""

# ===================================================
# STEP 5: ADDITIONAL UTILITIES
# ===================================================

"""
ADDITIONAL TOOLS
--------------

RapidProxyScan includes several helpful utilities:

1. Speed Testing:
   Test speed of proxies from a specific file:
   ```bash
   python rapidproxyscan.py --speed-test my_proxies.txt
   ```

2. Clean Old Files:
   Remove proxy files older than 7 days:
   ```bash
   python rapidproxyscan.py --clean-old
   ```

3. Statistics Analysis:
   Analyze proxy performance over time:
   ```bash
   python rapidproxyscan.py --analyze
   ```
"""

# ===================================================
# ADVANCED USAGE EXAMPLE
# ===================================================

"""
ADVANCED: USING IN YOUR OWN SCRIPTS
---------------------------------

Import RapidProxyScan functions in your Python code:

```python
from rapidproxyscan import scan_proxy_list

# Test a specific list of proxies
my_proxies = [
    "http://123.45.67.89:8080",
    "https://98.76.54.32:3128",
    "1.2.3.4:8000"
]

# Run the scan
working_proxies = scan_proxy_list(
    proxy_list=my_proxies,
    validation_rounds=2,
    timeout=3.0
)

# Use the results
for proxy in working_proxies:
    print(f"{proxy['url']} - Response time: {proxy['response_time']:.2f}ms")
```
"""

# ===================================================
# TROUBLESHOOTING
# ===================================================

"""
TROUBLESHOOTING
-------------

1. No proxies found:
   - Check your internet connection
   - Try increasing the timeout (--timeout 10.0)
   - Some proxy sources might be temporarily down, try again later

2. Scanner seems slow:
   - Reduce validation rounds (--validation-rounds 2)
   - Decrease connection timeout (--timeout 3.0)
   - Adjust connection limit based on your system (--connections 100 for slower systems)

3. High CPU/memory usage:
   - Reduce concurrent connections (--connections 100)
   - Ensure your system has enough resources for high-performance networking

4. Import to Elite Proxy Switcher fails:
   - Verify the path to EPS.exe in config.py
   - Try running RapidProxyScan with administrator privileges
   - Manually import the generated working_proxies_[timestamp].txt file
"""

# ===================================================
# CONCLUSION
# ===================================================

"""
That's it! You're now ready to use RapidProxyScan to find and maintain a 
high-quality list of working proxies with minimal effort.

RapidProxyScan's asynchronous design and intelligent validation make it 
significantly faster than Elite Proxy Switcher's built-in scanner while 
ensuring higher reliability through multiple validation rounds.

Happy proxying!
"""

if __name__ == "__main__":
    print("RapidProxyScan - Quick Start Guide")
    print("==================================")
    print("")
    print("For detailed instructions, please see the comments in this file.")
    print("To run the scanner, use: python rapidproxyscan.py")