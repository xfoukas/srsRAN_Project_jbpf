#
# Copyright 2021-2025 Software Radio Systems Limited
#
# This file is part of srsRAN
#
# srsRAN is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as
# published by the Free Software Foundation, either version 3 of
# the License, or (at your option) any later version.
#
# srsRAN is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU Affero General Public License for more details.
#
# A copy of the GNU Affero General Public License can be found in
# the LICENSE file in the top-level directory of this distribution
# and at http://www.gnu.org/licenses/.
#

"""
Pytest configuration
"""
import pytest
from pytest_html.html_report import HTMLReport


def pytest_html_report_title(report: HTMLReport):
    """
    Change title
    """
    report.title = "E2E Suite"


def pytest_addoption(parser: pytest.Parser):
    """
    Add Viavi options to pytest
    """
    parser.addoption(
        "--viavi-manual-campaign-filename", action="store", default="default_filename", help="Viavi campaign filename"
    )
    parser.addoption("--viavi-manual-test-name", action="store", default="default_test", help="Viavi test name")
    parser.addoption("--viavi-manual-test-timeout", action="store", type=int, default=1800, help="Viavi test timeout")
    parser.addoption(
        "--viavi-manual-gnb-arguments", action="store", type=str, default="", help="Viavi gnb arguments"
    )
