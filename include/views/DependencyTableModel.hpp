/**
 * SPDX-FileComment: Dependency Table Model for Qt GUI.
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: MIT
 *
 * @file DependencyTableModel.hpp
 * @brief Custom model for displaying dependencies in a QTableView.
 * @version 0.1.0
 * @date 2026-02-21
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2026 ZHENG Robert
 *
 * @license MIT License
 */

#pragma once

#include <QAbstractTableModel>
#include <QBrush>
#include <QColor>
#include <vector>
#include "../models/Dependency.hpp"

namespace views {

class DependencyTableModel : public QAbstractTableModel {
    Q_OBJECT
public:
        enum Column {
            NAME,
            VERSION,
            FIXED_VERSION,
            LICENSE,
            MAX_CRITICALITY,
            CVE_COUNT,
            COLUMN_COUNT
        };
    
        explicit DependencyTableModel(QObject* parent = nullptr) : QAbstractTableModel(parent) {}
    
        void setDependencies(std::vector<models::Dependency> deps) {
            beginResetModel();
            m_dependencies = std::move(deps);
            endResetModel();
        }
    
        int rowCount(const QModelIndex& parent = QModelIndex()) const override {
            return parent.isValid() ? 0 : static_cast<int>(m_dependencies.size());
        }
    
        int columnCount(const QModelIndex& /*parent*/ = QModelIndex()) const override {
            return COLUMN_COUNT;
        }
    
        QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override {
            if (!index.isValid()) return {};
    
            const auto& dep = m_dependencies[static_cast<size_t>(index.row())];
    
            if (role == Qt::DisplayRole) {
                switch (index.column()) {
                    case NAME: return QString::fromStdString(dep.name);
                    case VERSION: return QString::fromStdString(dep.version);
                    case FIXED_VERSION: return QString::fromStdString(dep.fixed_version.empty() ? "-" : dep.fixed_version);
                    case LICENSE: return QString::fromStdString(dep.license.empty() ? "Unknown" : dep.license);
                    case MAX_CRITICALITY: return criticalityToString(dep.max_criticality);
                    case CVE_COUNT: return static_cast<int>(dep.cves.size());
                    default: return {};
                }
            } else if (role == Qt::EditRole || role == Qt::UserRole) {
                // Used for sorting
                switch (index.column()) {
                    case NAME: return QString::fromStdString(dep.name);
                    case VERSION: return QString::fromStdString(dep.version);
                    case FIXED_VERSION: return QString::fromStdString(dep.fixed_version);
                    case LICENSE: return QString::fromStdString(dep.license);
                    case MAX_CRITICALITY: return static_cast<int>(dep.max_criticality);
                    case CVE_COUNT: return static_cast<int>(dep.cves.size());
                    default: return {};
                }
            } else if (role == Qt::BackgroundRole) {
                if (dep.max_criticality == models::Criticality::NONE && !dep.cves.empty()) {
                    // Check if it's really "SAFE" or just unknown
                    bool all_safe = std::all_of(dep.cves.begin(), dep.cves.end(), [](const auto& c) {
                        return c.id == "SAFE" || c.id == "NOT-CHECKED";
                    });
                    if (!all_safe) return QBrush(QColor(200, 200, 200, 100)); // Gray for unknown/moderate but present CVEs
                }
                return QBrush(criticalityToColor(dep.max_criticality));
            }
    
            return {};
        }
    
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override {
            if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
                switch (section) {
                    case NAME: return "Name";
                    case VERSION: return "Version";
                    case FIXED_VERSION: return "Fixed Version";
                    case LICENSE: return "License";
                    case MAX_CRITICALITY: return "Criticality";
                    case CVE_COUNT: return "CVE Count";
                    default: return {};
                }
            }
            return {};
        }
    

    const std::vector<models::Dependency>& dependencies() const { return m_dependencies; }

private:
    static QString criticalityToString(models::Criticality c) {
        switch (c) {
            case models::Criticality::CRITICAL: return "CRITICAL";
            case models::Criticality::HIGH: return "HIGH";
            case models::Criticality::MEDIUM: return "MEDIUM";
            case models::Criticality::LOW: return "LOW";
            default: return "NONE";
        }
    }

    static QColor criticalityToColor(models::Criticality c) {
        switch (c) {
            case models::Criticality::CRITICAL: return QColor(255, 0, 0, 100);
            case models::Criticality::HIGH: return QColor(255, 128, 0, 100);
            case models::Criticality::MEDIUM: return QColor(255, 255, 0, 100);
            case models::Criticality::LOW: return QColor(0, 255, 0, 100);
            default: return Qt::transparent;
        }
    }

    std::vector<models::Dependency> m_dependencies;
};

} // namespace views
