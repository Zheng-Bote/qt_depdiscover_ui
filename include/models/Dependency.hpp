/**
 * SPDX-FileComment: Dependency Model definition for the tracker.
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: MIT
 *
 * @file Dependency.hpp
 * @brief Defines the Dependency structure for tracking project components.
 * @version 0.1.0
 * @date 2026-02-21
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2026 ZHENG Robert
 *
 * @license MIT License
 */

#pragma once

#include <string>
#include <vector>
#include <optional>
#include "CVE.hpp"

namespace models {

/**
 * @brief Represents a software dependency.
 */
struct Dependency {
    std::string name;
    std::string version;
    std::string fixed_version;
    std::string license;
    std::vector<CVE> cves;
    Criticality max_criticality{Criticality::NONE};

    /**
     * @brief Updates the max criticality and fixed version based on associated CVEs.
     */
    void update_max_criticality() noexcept {
        max_criticality = Criticality::NONE;
        fixed_version = "";
        for (const auto& cve : cves) {
            if (cve.criticality > max_criticality) {
                max_criticality = cve.criticality;
                // Favor the fixed version from the most critical CVE
                if (!cve.fixed_version.empty()) {
                    fixed_version = cve.fixed_version;
                }
            }
        }
        
        // If max_criticality is still NONE but we have a fixed version somewhere, use the first one
        if (fixed_version.empty()) {
            for (const auto& cve : cves) {
                if (!cve.fixed_version.empty()) {
                    fixed_version = cve.fixed_version;
                    break;
                }
            }
        }
    }
};

} // namespace models
