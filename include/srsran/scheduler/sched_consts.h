/*
 *
 * Copyright 2021-2025 Software Radio Systems Limited
 *
 * This file is part of srsRAN.
 *
 * srsRAN is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * srsRAN is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * A copy of the GNU Affero General Public License can be found in
 * the LICENSE file in the top-level directory of this distribution
 * and at http://www.gnu.org/licenses/.
 *
 */

#pragma once

namespace srsran {

/// SSB constants.
/// FR1 = [ 410 MHz – 7125 MHz] (TS 38.101, Section 5.1) and ARFCN corresponding to 7.125GHz is 875000.
const unsigned FR1_MAX_FREQUENCY_ARFCN = 875000;
/// The cutoff frequency for case A, B and C paired is 3GHz, corresponding to 600000 ARFCN (TS 38.213, Section 4.1).
const unsigned CUTOFF_FREQ_ARFCN_CASE_A_B_C = 600000;
/// The cutoff frequency for case C unpaired is 1.88GHz, corresponding to 376000 ARFCN (TS 38.213, Section 4.1).
const unsigned CUTOFF_FREQ_ARFCN_CASE_C_UNPAIRED = 376000;
const unsigned NOF_SSB_OFDM_SYMBOLS              = 4;

/// SIB1 constants.
/// SIB1 periodicity, see TS 38.331, Section 5.2.1.
const unsigned SIB1_PERIODICITY = 160;
/// [Implementation defined] Max numbers of beams, to be used for SIB1 scheduler.
/// NOTE: This is temporary, and valid only for FR1.
const unsigned MAX_NUM_BEAMS = 8;

/// [Implementation defined] Maximum allowed slot offset between DCI and its scheduled PDSCH. Values {0,..,32}.
const unsigned SCHEDULER_MAX_K0 = 15;

/// [Implementation defined] Maximum allowed slot offset between PDSCH to the DL ACK/NACK. Values {0,..,15}.
const unsigned SCHEDULER_MAX_K1 = 15;

/// [Implementation defined] Maximum allowed slot offset between DCI and its scheduled first PUSCH. Values {0,..,32}.
/// \remark As per TS 38.306, \c ul-SchedulingOffset field in \c ueCapabilityInformation indicates whether the UE
/// supports UL scheduling slot offset (K2) greater than 12.
/// \remark [Implementation defined] While testing with COTS UE, the CRC is KO whenever k2=12 is used to schedule PUSCH.
/// And, when maximum k2 is restricted to 11 the BLER is not seen. Hence, in order to support UEs not supporting k2 > 12
/// and to reduce BLER, we restrict maximum value of k2 to 11.
const unsigned SCHEDULER_MAX_K2 = 11;

/// Maximum value of NTN cell specific Koffset. See TS 38.331
const unsigned NTN_CELL_SPECIFIC_KOFFSET_MAX = 512;

/// Maximum value of Msg delta. See table 6.1.2.1.1-5, in TS 38.214.
const unsigned MAX_MSG3_DELTA = 6;

/// Maximum number of PDSCH time domain resource allocations. See TS 38.331, \c maxNrofDL-Allocations.
const unsigned MAX_NOF_PDSCH_TD_RESOURCE_ALLOCATIONS = 16;

} // namespace srsran
