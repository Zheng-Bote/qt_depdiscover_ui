/**
 * SPDX-FileComment: Main Window implementation for the dependency tracker UI.
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: MIT
 *
 * @file MainWindow.hpp
 * @brief Main GUI component for the application.
 * @version 0.2.0
 * @date 2026-02-22
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2026 ZHENG Robert
 *
 * @license MIT License
 */

#pragma once

#include "../rz_config.hpp"
#include "../utils/CycloneDXParser.hpp"
#include "../utils/DepDiscoverParser.hpp"
#include "../utils/SPDXParser.hpp"
#include "DependencyTableModel.hpp"
#include <QDateTime> // NEU: Für Zeitstempel beim Export
#include <QDesktopServices>
#include <QFileDialog>
#include <QFutureWatcher>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QMainWindow>
#include <QMessageBox>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QStatusBar>
#include <QTabWidget>
#include <QTableView>
#include <QUrl>
#include <QVBoxLayout>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>
#include <QtConcurrent/QtConcurrent>
#include <algorithm>
#include <filesystem>
#include <map>
#include <memory>
#include <qt_gh-update-checker.hpp>

namespace views {

class MainWindow : public QMainWindow {
  Q_OBJECT
public:
  explicit MainWindow(QWidget *parent = nullptr) : QMainWindow(parent) {
    setWindowTitle("C++23 Qt6 Dependency Tracker UI");
    resize(1000, 800);

    auto *central_widget = new QWidget(this);
    auto *layout = new QVBoxLayout(central_widget);
    setCentralWidget(central_widget);

    // --- NEU: Horizontales Layout für die Top-Buttons ---
    auto *top_button_layout = new QHBoxLayout();

    auto *open_button = new QPushButton("Open Dependency File", this);
    connect(open_button, &QPushButton::clicked, this, &MainWindow::onOpenFile);
    top_button_layout->addWidget(open_button);

    m_export_button = new QPushButton("Export CycloneDX SBOM", this);
    m_export_button->setEnabled(false); // Erst aktiv, wenn Daten geladen sind
    connect(m_export_button, &QPushButton::clicked, this,
            &MainWindow::onExportCycloneDX);
    top_button_layout->addWidget(m_export_button);

    top_button_layout->addStretch(); // Drückt die Buttons nach links
    layout->addLayout(top_button_layout);
    // ----------------------------------------------------

    m_tab_widget = new QTabWidget(this);
    layout->addWidget(m_tab_widget);

    // Tab 1: Table
    m_table_view = new QTableView(this);
    m_model = new DependencyTableModel(this);
    m_proxy_model = new QSortFilterProxyModel(this);
    m_proxy_model->setSourceModel(m_model);
    m_proxy_model->setSortRole(Qt::UserRole);

    m_table_view->setModel(m_proxy_model);
    m_table_view->setSortingEnabled(true);
    m_table_view->horizontalHeader()->setSectionResizeMode(
        QHeaderView::Stretch);
    m_table_view->setSelectionBehavior(QAbstractItemView::SelectRows);
    connect(m_table_view, &QTableView::clicked, this,
            &MainWindow::onCellClicked);

    m_tab_widget->addTab(m_table_view, "Dependency Table");

    // Tab 2: Statistics
    auto *stats_widget = new QWidget(this);
    auto *stats_layout = new QVBoxLayout(stats_widget);

    m_chart_view_cve = new QChartView(this);
    m_chart_view_cve->setRenderHint(QPainter::Antialiasing);

    m_chart_view_license = new QChartView(this);
    m_chart_view_license->setRenderHint(QPainter::Antialiasing);

    stats_layout->addWidget(new QLabel("CVE Criticality Distribution", this));
    stats_layout->addWidget(m_chart_view_cve);
    stats_layout->addWidget(new QLabel("License Distribution", this));
    stats_layout->addWidget(m_chart_view_license);

    m_tab_widget->addTab(stats_widget, "Statistics");

    // Footer with Version
    auto *footer_layout = new QHBoxLayout();
    footer_layout->addStretch();
    auto *version_label = new QLabel(this);
    version_label->setText(
        QString("<a href='%1' style='color: blue; text-decoration: "
                "underline;'>Version: %2</a>")
            .arg(rz::config::PROJECT_HOMEPAGE_URL.data())
            .arg(rz::config::VERSION.data()));
    version_label->setOpenExternalLinks(true);
    version_label->setCursor(Qt::PointingHandCursor);
    footer_layout->addWidget(version_label);
    layout->addLayout(footer_layout);

    statusBar()->showMessage("Checking for updates...");
    initUpdateChecker();
  }

private slots:
  void onUpdateCheckFinished() {
    try {
      auto info = m_update_watcher.result();
      if (info.hasUpdate) {
        statusBar()->showMessage(
            QString("Update available: %1").arg(info.latestVersion));
        auto *download_link = new QLabel(this);
        download_link->setText(QString("<a href='%1'>Download %2</a>")
                                   .arg(rz::config::PROJECT_HOMEPAGE_URL.data())
                                   .arg(info.latestVersion));
        download_link->setOpenExternalLinks(true);
        statusBar()->addPermanentWidget(download_link);
      } else {
        statusBar()->showMessage("Version is up to date.", 5000);
      }
    } catch (const std::exception &e) {
      statusBar()->showMessage(QString("Update check failed: %1").arg(e.what()),
                               5000);
    }
  }

  void onCellClicked(const QModelIndex &index) {
    auto source_index = m_proxy_model->mapToSource(index);
    const auto &deps = m_model->dependencies();
    if (source_index.row() < 0 ||
        static_cast<size_t>(source_index.row()) >= deps.size())
      return;

    const auto &dep = deps[static_cast<size_t>(source_index.row())];
    if (dep.cves.empty())
      return;

    if (index.column() == DependencyTableModel::MAX_CRITICALITY) {
      if (dep.max_criticality != models::Criticality::NONE) {
        auto it = std::max_element(dep.cves.begin(), dep.cves.end(),
                                   [](const auto &a, const auto &b) {
                                     return a.criticality < b.criticality;
                                   });
        if (it != dep.cves.end()) {
          openCveUrl(QString::fromStdString(it->id));
        }
      }
    } else if (index.column() == DependencyTableModel::CVE_COUNT) {
      QString message = QString("Vulnerabilities for %1 (%2):\n\n")
                            .arg(QString::fromStdString(dep.name))
                            .arg(QString::fromStdString(dep.version));
      for (const auto &cve : dep.cves) {
        QString crit_str;
        switch (cve.criticality) {
        case models::Criticality::CRITICAL:
          crit_str = "CRITICAL";
          break;
        case models::Criticality::HIGH:
          crit_str = "HIGH";
          break;
        case models::Criticality::MEDIUM:
          crit_str = "MEDIUM";
          break;
        case models::Criticality::LOW:
          crit_str = "LOW";
          break;
        default:
          crit_str = "NONE";
          break;
        }
        message += QString("- %1 (%2): %3\n")
                       .arg(QString::fromStdString(cve.id))
                       .arg(crit_str)
                       .arg(QString::fromStdString(cve.description));
      }
      QMessageBox::information(this, "Vulnerabilities List", message);
    }
  }

  void onOpenFile() {
    QString file_path = QFileDialog::getOpenFileName(
        this, "Open Dependency File", "",
        "All Files (*);;CycloneDX (*.json);;SPDX (*.spdx);;DepDiscover "
        "(depdiscover.json)");

    if (file_path.isEmpty())
      return;

    std::filesystem::path path(file_path.toStdString());
    std::unique_ptr<utils::IParser> parser;

    if (path.extension() == ".spdx") {
      parser = std::make_unique<utils::SPDXParser>();
    } else if (path.filename() == "depdiscover.json") {
      parser = std::make_unique<utils::DepDiscoverParser>();
    } else if (path.extension() == ".json") {
      parser = std::make_unique<utils::CycloneDXParser>();
    } else {
      parser = std::make_unique<utils::CycloneDXParser>();
    }

    auto result = parser->parse(path);
    if (!result) {
      QMessageBox::critical(this, "Parsing Error",
                            QString::fromStdString(result.error()));
      return;
    }

    m_model->setDependencies(std::move(result.value()));
    updateStatistics();

    // NEU: Export Button freischalten, da jetzt Daten vorliegen
    m_export_button->setEnabled(true);
  }

  // --- NEU: Export Logik ---
  void onExportCycloneDX() {
    QString file_path = QFileDialog::getSaveFileName(
        this, "Export CycloneDX SBOM", "cyclonedx.sbom.json",
        "CycloneDX JSON (*.json);;All Files (*)");

    if (file_path.isEmpty())
      return;

    QJsonObject root;
    root["bomFormat"] = "CycloneDX";
    root["specVersion"] = "1.4";
    root["version"] = 1;

    // Metadata
    QJsonObject metadata;
    metadata["timestamp"] =
        QDateTime::currentDateTimeUtc().toString(Qt::ISODate);

    QJsonObject tool;
    tool["vendor"] = QString::fromStdString(rz::config::AUTHOR.data());
    tool["name"] = QString::fromStdString(rz::config::PROJECT_NAME.data());
    tool["version"] = QString::fromStdString(rz::config::VERSION.data());

    QJsonArray tools;
    tools.append(tool);
    metadata["tools"] = tools;
    root["metadata"] = metadata;

    QJsonArray components;
    QJsonArray vulnerabilities;

    // Iterate über die geladenen Dependencies im Model
    for (const auto &dep : m_model->dependencies()) {
      QJsonObject comp;
      QString purl = QString("pkg:generic/%1@%2")
                         .arg(QString::fromStdString(dep.name))
                         .arg(QString::fromStdString(
                             dep.version.empty() ? "unknown" : dep.version));

      comp["type"] = "library";
      comp["name"] = QString::fromStdString(dep.name);
      comp["version"] =
          QString::fromStdString(dep.version.empty() ? "unknown" : dep.version);
      comp["bom-ref"] = purl;
      comp["purl"] = purl;

      // Lizenz einfügen
      if (!dep.license.empty() && dep.license != "Unknown") {
        QJsonArray licenses;
        QJsonObject licObj;
        QJsonObject licDetail;
        licDetail["id"] = QString::fromStdString(dep.license);
        licObj["license"] = licDetail;
        licenses.append(licObj);
        comp["licenses"] = licenses;
      }

      components.append(comp);

      // CVEs verarbeiten
      for (const auto &cve : dep.cves) {
        if (cve.id == "SAFE" || cve.id == "NOT-CHECKED" ||
            cve.id == "CHECK-ERROR")
          continue;

        QJsonObject vuln;
        vuln["bom-ref"] = purl; // Verknüpft CVE mit der Komponente
        vuln["id"] = QString::fromStdString(cve.id);

        QJsonObject source;
        source["name"] = "OSV.dev";
        vuln["source"] = source;

        if (!cve.description.empty()) {
          vuln["description"] = QString::fromStdString(cve.description);
        }

        // Score und Rating
        QJsonArray ratings;
        QJsonObject rating;
        rating["score"] = cve.cvss_score;

        QString sev_str = "unknown";
        switch (cve.criticality) {
        case models::Criticality::CRITICAL:
          sev_str = "critical";
          break;
        case models::Criticality::HIGH:
          sev_str = "high";
          break;
        case models::Criticality::MEDIUM:
          sev_str = "medium";
          break;
        case models::Criticality::LOW:
          sev_str = "low";
          break;
        default:
          sev_str = "unknown";
          break;
        }

        rating["severity"] = sev_str;
        ratings.append(rating);
        vuln["ratings"] = ratings;

        vulnerabilities.append(vuln);
      }
    }

    root["components"] = components;
    if (!vulnerabilities.isEmpty()) {
      root["vulnerabilities"] = vulnerabilities;
    }

    // JSON schreiben
    QJsonDocument doc(root);
    QFile file(file_path);
    if (file.open(QIODevice::WriteOnly)) {
      file.write(doc.toJson(QJsonDocument::Indented));
      file.close();
      QMessageBox::information(this, "Export Successful",
                               "CycloneDX SBOM exported successfully to:\n" +
                                   file_path);
    } else {
      QMessageBox::critical(this, "Export Failed",
                            "Could not write to file:\n" + file_path);
    }
  }
  // ------------------------------

  void updateStatistics() {
    auto *cve_series = new QPieSeries();
    int counts[5] = {0, 0, 0, 0, 0};
    std::map<std::string, int> licenses;

    for (const auto &dep : m_model->dependencies()) {
      counts[static_cast<int>(dep.max_criticality)]++;
      std::string lic = dep.license.empty() ? "Unknown" : dep.license;
      licenses[lic]++;
    }

    auto addCveSlice = [&](const char *label, int count, QColor color) {
      if (count > 0) {
        auto *slice =
            cve_series->append(QString("%1 (%2)").arg(label).arg(count), count);
        slice->setBrush(color);
      }
    };

    addCveSlice("None", counts[0], Qt::lightGray);
    addCveSlice("Low", counts[1], Qt::green);
    addCveSlice("Medium", counts[2], Qt::yellow);
    addCveSlice("High", counts[3], QColor(255, 128, 0));
    addCveSlice("Critical", counts[4], Qt::red);

    auto *cve_chart = new QChart();
    cve_chart->addSeries(cve_series);
    cve_chart->setTitle("Vulnerability Distribution");
    cve_chart->legend()->setAlignment(Qt::AlignRight);
    m_chart_view_cve->setChart(cve_chart);

    auto *lic_series = new QPieSeries();
    for (const auto &[name, count] : licenses) {
      lic_series->append(
          QString("%1 (%2)").arg(QString::fromStdString(name)).arg(count),
          count);
    }

    auto *lic_chart = new QChart();
    lic_chart->addSeries(lic_series);
    lic_chart->setTitle("License Distribution");
    lic_chart->legend()->setAlignment(Qt::AlignRight);
    m_chart_view_license->setChart(lic_chart);
  }

private:
  void openCveUrl(const QString &cve_id) {
    if (!cve_id.isEmpty() && cve_id != "SAFE" && cve_id != "NOT-CHECKED") {
      QString url;
      if (cve_id.startsWith("CVE-")) {
        url = QString("https://nvd.nist.gov/vuln/detail/%1").arg(cve_id);
      } else {
        url = QString("https://osv.dev/vulnerability/%1").arg(cve_id);
      }
      QDesktopServices::openUrl(QUrl(url));
    }
  }

  void initUpdateChecker() {
    QString repoUrl =
        QString::fromStdString(rz::config::PROJECT_HOMEPAGE_URL.data());
    QString currentVersion = QString::fromStdString(rz::config::VERSION.data());

    connect(&m_update_watcher, &QFutureWatcher<qtgh::UpdateInfo>::finished,
            this, &MainWindow::onUpdateCheckFinished);

    auto future = QtConcurrent::run([repoUrl, currentVersion]() {
      return qtgh::check_github_update(repoUrl, currentVersion);
    });
    m_update_watcher.setFuture(future);
  }

  QPushButton *m_export_button; // NEU: Member für den Export-Button, um Status
                                // zu steuern
  QTabWidget *m_tab_widget;
  QTableView *m_table_view;
  DependencyTableModel *m_model;
  QSortFilterProxyModel *m_proxy_model;
  QChartView *m_chart_view_cve;
  QChartView *m_chart_view_license;
  QFutureWatcher<qtgh::UpdateInfo> m_update_watcher;
};

} // namespace views