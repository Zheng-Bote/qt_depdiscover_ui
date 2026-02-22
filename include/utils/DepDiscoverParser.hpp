/**
 * SPDX-FileComment: DepDiscover Parser implementation.
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: MIT
 *
 * @file DepDiscoverParser.hpp
 * @brief Parser for DepDiscover custom JSON files.
 * @version 0.2.0
 * @date 2026-02-22
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2026 ZHENG Robert
 *
 * @license MIT License
 */

#pragma once

#include "IParser.hpp"
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <string>

namespace utils {

class DepDiscoverParser : public IParser {
public:
  [[nodiscard]] std::expected<std::vector<models::Dependency>, std::string>
  parse(const std::filesystem::path &path) const override {
    QFile file(QString::fromStdString(path.string()));
    if (!file.open(QIODevice::ReadOnly)) {
      return std::unexpected("Could not open file: " + path.string());
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (doc.isNull()) {
      return std::unexpected("Invalid JSON in DepDiscover file");
    }

    QJsonObject root = doc.object();
    std::vector<models::Dependency> dependencies;

    QJsonArray deps_arr = root["dependencies"].toArray();
    for (const auto &dep_val : deps_arr) {
      QJsonObject dep_obj = dep_val.toObject();
      models::Dependency dep;
      dep.name = dep_obj["name"].toString().toStdString();
      dep.version = dep_obj["version"].toString().toStdString();

      QJsonArray licenses = dep_obj["licenses"].toArray();
      if (!licenses.isEmpty()) {
        dep.license = licenses[0].toString().toStdString();
      }

      QJsonArray cves_arr = dep_obj["cves"].toArray();
      for (const auto &cve_val : cves_arr) {
        QJsonObject cve_obj = cve_val.toObject();
        models::CVE cve;
        cve.id = cve_obj["id"].toString().toStdString();
        cve.description = cve_obj["summary"].toString().toStdString();
        cve.fixed_version = cve_obj["fixed_version"].toString().toStdString();

        // --- FIX: Intelligentes Parsing von Severity/Score ---
        QJsonValue sev_val = cve_obj["severity"];

        // Ist es nativ als Zahl im JSON gespeichert?
        if (sev_val.isDouble()) {
          cve.cvss_score = sev_val.toDouble();
          cve.criticality = models::CVE::from_score(cve.cvss_score);
        } else {
          std::string sev = sev_val.toString().toUpper().toStdString();

          // Versuch, den String als Zahl zu parsen (z.B. "7.5")
          try {
            cve.cvss_score = std::stod(sev);
            cve.criticality = models::CVE::from_score(cve.cvss_score);
          } catch (...) {
            // Fallback für reine Text-Werte
            if (sev == "CRITICAL")
              cve.criticality = models::Criticality::CRITICAL;
            else if (sev == "HIGH")
              cve.criticality = models::Criticality::HIGH;
            else if (sev == "MEDIUM" || sev == "MODERATE")
              cve.criticality = models::Criticality::MEDIUM;
            else if (sev == "LOW")
              cve.criticality = models::Criticality::LOW;
            else if (sev.starts_with("CVSS:")) {
              // Heuristik für CVSS Vektoren
              if (sev.find("/C:H") != std::string::npos ||
                  sev.find("/I:H") != std::string::npos ||
                  sev.find("/A:H") != std::string::npos) {
                cve.criticality = models::Criticality::HIGH;
              } else {
                cve.criticality = models::Criticality::MEDIUM;
              }
            } else {
              cve.criticality = models::Criticality::NONE;
            }
          }
        }
        // -----------------------------------------------------

        dep.cves.push_back(std::move(cve));
      }
      dep.update_max_criticality();
      dependencies.push_back(std::move(dep));
    }

    return dependencies;
  }
};

} // namespace utils