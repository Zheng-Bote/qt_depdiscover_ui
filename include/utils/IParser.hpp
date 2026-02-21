/**
 * SPDX-FileComment: Parser interface for dependency files.
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: MIT
 *
 * @file IParser.hpp
 * @brief Interface for parsing different dependency file formats.
 * @version 0.1.0
 * @date 2026-02-21
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2026 ZHENG Robert
 *
 * @license MIT License
 */

#pragma once

#include <expected>
#include <string>
#include <vector>
#include <filesystem>
#include "../models/Dependency.hpp"

namespace utils {

/**
 * @brief Interface for parsing dependency information from files.
 */
class IParser {
public:
    virtual ~IParser() = default;

    /**
     * @brief Parses a file and returns a list of dependencies.
     * @param path The filesystem path to the file.
     * @return A std::expected containing either a vector of dependencies or an error message.
     */
    [[nodiscard]] virtual std::expected<std::vector<models::Dependency>, std::string> parse(const std::filesystem::path& path) const = 0;
};

} // namespace utils
