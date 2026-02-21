/**
 * SPDX-FileComment: CycloneDX Parser implementation.
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: MIT
 *
 * @file CycloneDXParser.hpp
 * @brief Parser for CycloneDX JSON files.
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
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <map>

namespace utils {

class CycloneDXParser : public IParser {
public:
    [[nodiscard]] std::expected<std::vector<models::Dependency>, std::string> parse(const std::filesystem::path& path) const override {
        QFile file(QString::fromStdString(path.string()));
        if (!file.open(QIODevice::ReadOnly)) {
            return std::unexpected("Could not open file: " + path.string());
        }

        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        if (doc.isNull()) {
            return std::unexpected("Invalid JSON in CycloneDX file");
        }

        QJsonObject root = doc.object();
        std::vector<models::Dependency> dependencies;
        std::map<std::string, size_t> bom_ref_map;

        // Parse components
        QJsonArray components = root["components"].toArray();
        for (const auto& comp_val : components) {
            QJsonObject comp = comp_val.toObject();
            models::Dependency dep;
            dep.name = comp["name"].toString().toStdString();
            dep.version = comp["version"].toString().toStdString();
            dep.license = ""; // Simplified for now
            
            std::string bom_ref = comp["bom-ref"].toString().toStdString();
            if (!bom_ref.empty()) {
                bom_ref_map[bom_ref] = dependencies.size();
            }
            dependencies.push_back(std::move(dep));
        }

        // Parse vulnerabilities
        QJsonArray vulnerabilities = root["vulnerabilities"].toArray();
        for (const auto& vuln_val : vulnerabilities) {
            QJsonObject vuln = vuln_val.toObject();
            models::CVE cve;
            cve.id = vuln["id"].toString().toStdString();
            cve.description = vuln["description"].toString().toStdString();
            
            // Get score
            QJsonArray ratings = vuln["ratings"].toArray();
            if (!ratings.isEmpty()) {
                cve.cvss_score = ratings[0].toObject()["score"].toDouble();
            }
            cve.criticality = models::CVE::from_score(cve.cvss_score);

            // Assign to components
            QJsonArray affects = vuln["affects"].toArray();
            for (const auto& affect_val : affects) {
                std::string ref = affect_val.toObject()["ref"].toString().toStdString();
                if (bom_ref_map.contains(ref)) {
                    dependencies[bom_ref_map[ref]].cves.push_back(cve);
                }
            }
        }

        for (auto& dep : dependencies) {
            dep.update_max_criticality();
        }

        return dependencies;
    }
};

} // namespace utils
