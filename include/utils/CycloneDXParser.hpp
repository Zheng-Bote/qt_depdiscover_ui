/**
 * SPDX-FileComment: CycloneDX Parser implementation.
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: MIT
 *
 * @file CycloneDXParser.hpp
 * @brief Parser for CycloneDX JSON files.
 * @version 0.2.0
 */

#pragma once

#include "IParser.hpp"
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <map>

namespace utils {

class CycloneDXParser : public IParser {
public:
  [[nodiscard]] std::expected<std::vector<models::Dependency>, std::string>
  parse(const std::filesystem::path &path) const override {
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

    // --- Parse components ---
    QJsonArray components = root["components"].toArray();
    for (const auto &comp_val : components) {
      QJsonObject comp = comp_val.toObject();
      models::Dependency dep;
      dep.name = comp["name"].toString().toStdString();
      dep.version = comp["version"].toString().toStdString();

      // NEU: Lizenzen auslesen
      std::string lic_str = "";
      QJsonArray licenses = comp["licenses"].toArray();
      for (const auto &lic_val : licenses) {
        QJsonObject lic_obj = lic_val.toObject()["license"].toObject();
        QString id = lic_obj["id"].toString();
        QString name = lic_obj["name"].toString();

        if (!lic_str.empty())
          lic_str += ", ";
        lic_str += (!id.isEmpty() ? id.toStdString() : name.toStdString());
      }
      dep.license = lic_str;

      // Verknüpfungspunkt merken
      std::string bom_ref = comp["bom-ref"].toString().toStdString();
      if (!bom_ref.empty()) {
        bom_ref_map[bom_ref] = dependencies.size();
      }
      dependencies.push_back(std::move(dep));
    }

    // --- Parse vulnerabilities ---
    QJsonArray vulnerabilities = root["vulnerabilities"].toArray();
    for (const auto &vuln_val : vulnerabilities) {
      QJsonObject vuln = vuln_val.toObject();
      models::CVE cve;
      cve.id = vuln["id"].toString().toStdString();
      cve.description = vuln["description"].toString().toStdString();

      // Get score / severity
      QJsonArray ratings = vuln["ratings"].toArray();
      if (!ratings.isEmpty()) {
        QJsonObject rating = ratings[0].toObject();
        if (rating.contains("score")) {
          cve.cvss_score = rating["score"].toDouble();
          cve.criticality = models::CVE::from_score(cve.cvss_score);
        } else if (rating.contains("severity")) {
          QString sev = rating["severity"].toString().toUpper();
          if (sev == "CRITICAL")
            cve.criticality = models::Criticality::CRITICAL;
          else if (sev == "HIGH")
            cve.criticality = models::Criticality::HIGH;
          else if (sev == "MEDIUM")
            cve.criticality = models::Criticality::MEDIUM;
          else if (sev == "LOW")
            cve.criticality = models::Criticality::LOW;
          else
            cve.criticality = models::Criticality::NONE;
        }
      }

      // --- Assign to components ---
      // Methode 1: CycloneDX 1.4 affects Array
      QJsonArray affects = vuln["affects"].toArray();
      for (const auto &affect_val : affects) {
        std::string ref = affect_val.toObject()["ref"].toString().toStdString();
        if (bom_ref_map.contains(ref)) {
          dependencies[bom_ref_map[ref]].cves.push_back(cve);
        }
      }

      // Methode 2: Direktes bom-ref auf der Vulnerability (Wie depdiscover es
      // exportiert)
      if (vuln.contains("bom-ref")) {
        std::string ref = vuln["bom-ref"].toString().toStdString();
        if (bom_ref_map.contains(ref)) {
          // Verhindern, dass wir es doppelt einfügen, falls affects AUCH
          // existiert
          dependencies[bom_ref_map[ref]].cves.push_back(cve);
        }
      }
    }

    for (auto &dep : dependencies) {
      dep.update_max_criticality();
    }

    return dependencies;
  }
};

} // namespace utils