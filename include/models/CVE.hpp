/**
 * SPDX-FileComment: CVE Model definition for the dependency tracker.
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: MIT
 *
 * @file CVE.hpp
 * @brief Defines the CVE (Common Vulnerabilities and Exposures) structure.
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
#include <string_view>

namespace models {

/**
 * @brief Enum for CVE criticality levels.
 */
enum class Criticality {
    NONE,
    LOW,
    MEDIUM,
    HIGH,
    CRITICAL
};

/**
 * @brief Represents a CVE entry.
 */
struct CVE {
    std::string id;
    std::string description;
    std::string fixed_version;
    double cvss_score{0.0};
    Criticality criticality{Criticality::NONE};

    /**
     * @brief Determines the criticality level based on the CVSS score.
     * @param score The CVSS score.
     * @return The determined Criticality level.
     */
    [[nodiscard]] constexpr static Criticality from_score(double score) noexcept {
        if (score >= 9.0) return Criticality::CRITICAL;
        if (score >= 7.0) return Criticality::HIGH;
        if (score >= 4.0) return Criticality::MEDIUM;
        if (score >= 0.1) return Criticality::LOW;
        return Criticality::NONE;
    }
};

} // namespace models
