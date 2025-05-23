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

add_executable(gnb
        gnb.cpp
        gnb_appconfig_cli11_schema.cpp
        gnb_appconfig_validators.cpp
        gnb_appconfig_translators.cpp
        gnb_appconfig_yaml_writer.cpp
        )

install(TARGETS gnb
        RUNTIME)

target_link_libraries(gnb
        srsran_app_services
        srsgnb_app_f1u_connector
        srsran_cu_cp
        srsran_network
        -Wl,--whole-archive srsran_ngap -Wl,--no-whole-archive
        ngap_asn1
        srsran_e2
        e2ap_asn1
        srsran_gateway
        srsran_pcap
        srsran_support
        srsran_versioning
        srsran_flexible_du
        srsran_f1c_gateway
        srsran_e1_gateway
        srsran_cu_cp_app_unit
        srsran_cu_up_app_unit
        )

if (DPDK_FOUND)
    add_definitions(-DDPDK_FOUND)
    target_link_libraries(gnb hal_dpdk)
    if (ENABLE_PUSCH_HWACC)
      add_definitions(-DENABLE_PUSCH_HWACC)
      target_link_libraries(gnb hal_hwacc_pusch hal_bbdev_factory)
    endif (ENABLE_PUSCH_HWACC)
    if (ENABLE_PDSCH_HWACC)
      add_definitions(-DENABLE_PDSCH_HWACC)
      target_link_libraries(gnb hal_hwacc_pdsch hal_bbdev_factory)
    endif (ENABLE_PDSCH_HWACC)
endif (DPDK_FOUND)

if (ENABLE_JBPF)
        add_definitions(-DJBPF_ENABLED)
        target_link_libraries(gnb jbpf)
endif (ENABLE_JBPF)

add_backward(gnb)

target_include_directories(gnb PRIVATE ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/external)