/**
 * SPDX-FileComment: SPDX Parser implementation.
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: MIT
 *
 * @file SPDXParser.hpp
 * @brief Parser for SPDX tag-value files.
 * @version 0.2.0
 */

#pragma once

#include "IParser.hpp"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>

namespace utils {

class SPDXParser : public IParser {
private:
  // Helper function to remove trailing \r and whitespaces
  static std::string trim(const std::string &s) {
    auto start = s.begin();
    while (start != s.end() &&
           std::isspace(static_cast<unsigned char>(*start))) {
      start++;
    }
    auto end = s.end();
    do {
      end--;
    } while (std::distance(start, end) >= 0 &&
             std::isspace(static_cast<unsigned char>(*end)));
    return std::string(start, end + 1);
  }

public:
  [[nodiscard]] std::expected<std::vector<models::Dependency>, std::string>
  parse(const std::filesystem::path &path) const override {
    std::ifstream file(path);
    if (!file.is_open()) {
      return std::unexpected("Could not open file: " + path.string());
    }

    std::vector<models::Dependency> dependencies;
    std::string line;
    models::Dependency current_dep;
    bool in_package = false;

    while (std::getline(file, line)) {
      line = trim(line); // Remove whitespace & carriage returns
      if (line.empty())
        continue;

      if (line.starts_with("PackageName: ")) {
        if (in_package) {
          dependencies.push_back(std::move(current_dep));
          current_dep = {};
        }
        current_dep.name = trim(line.substr(13));
        in_package = true;
      } else if (line.starts_with("PackageVersion: ")) {
        current_dep.version = trim(line.substr(16));
      } else if (line.starts_with("PackageLicenseConcluded: ")) {
        current_dep.license = trim(line.substr(25));
        if (current_dep.license == "NOASSERTION") {
          current_dep.license = "Unknown";
        }
      }
    }

    if (in_package) {
      dependencies.push_back(std::move(current_dep));
    }

    // If no packages found, try to look at files as dependencies
    if (dependencies.empty()) {
      file.clear();
      file.seekg(0);
      while (std::getline(file, line)) {
        line = trim(line);
        if (line.starts_with("FileName: ")) {
          models::Dependency dep;
          dep.name = trim(line.substr(10));
          dependencies.push_back(std::move(dep));
        }
      }
    }

    return dependencies;
  }
};

} // namespace utils