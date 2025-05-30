"""
RapidProxyScan - Integration Guide
=================================

This guide shows how to integrate all the optimization components with the main scanner
and customize the output for viewbot/Supreme integration.
"""

# Import the main scanner and optimization modules
from rapidproxyscan import ProxyScanManager, run_scanner
from proxy_optimizer import OptimizerIntegration, ViewbotFormatter

import asyncio
import argparse
import logging
import os
import time