/**
 * SPDX-FileComment: SPDX Parser implementation.
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: MIT
 *
 * @file SPDXParser.hpp
 * @brief Parser for SPDX tag-value files.
 * @version 0.1.0
 * @date 2026-02-21
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2026 ZHENG Robert
 *
 * @license MIT License
 */

#pragma once

#include "IParser.hpp"
#include <fstream>
#include <sstream>

namespace utils {

class SPDXParser : public IParser {
public:
    [[nodiscard]] std::expected<std::vector<models::Dependency>, std::string> parse(const std::filesystem::path& path) const override {
        std::ifstream file(path);
        if (!file.is_open()) {
            return std::unexpected("Could not open file: " + path.string());
        }

        std::vector<models::Dependency> dependencies;
        std::string line;
        models::Dependency current_dep;
        bool in_package = false;

        while (std::getline(file, line)) {
            if (line.starts_with("PackageName: ")) {
                if (in_package) {
                    dependencies.push_back(std::move(current_dep));
                    current_dep = {};
                }
                current_dep.name = line.substr(13);
                in_package = true;
            } else if (line.starts_with("PackageVersion: ")) {
                current_dep.version = line.substr(16);
            } else if (line.starts_with("PackageLicenseConcluded: ")) {
                current_dep.license = line.substr(25);
            }
        }

        if (in_package) {
            dependencies.push_back(std::move(current_dep));
        }

        // If no packages found, try to look at files as dependencies (simplified)
        if (dependencies.empty()) {
            file.clear();
            file.seekg(0);
            while (std::getline(file, line)) {
                if (line.starts_with("FileName: ")) {
                    models::Dependency dep;
                    dep.name = line.substr(10);
                    dependencies.push_back(std::move(dep));
                }
            }
        }

        return dependencies;
    }
};

} // namespace utils
