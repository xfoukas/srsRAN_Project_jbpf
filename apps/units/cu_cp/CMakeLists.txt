#
# Copyright 2021-2024 Software Radio Systems Limited
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

set(SOURCES
        cu_cp_application_unit_impl.cpp
        cu_cp_builder.cpp
        cu_cp_wrapper.cpp
        cu_cp_unit_config_cli11_schema.cpp
        cu_cp_config_translators.cpp
        cu_cp_unit_config_validator.cpp
        cu_cp_unit_config_yaml_writer.cpp
        cu_cp_wrapper.cpp)

add_library(srsran_cu_cp_app_unit STATIC ${SOURCES})
target_include_directories(srsran_cu_cp_app_unit PRIVATE ${CMAKE_SOURCE_DIR})
