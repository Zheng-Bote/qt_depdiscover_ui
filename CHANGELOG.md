# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - 2026-02-22

### Added
- **Initial Release** of Qt Dependency Tracker UI.
- Support for importing **CycloneDX (JSON)**, **SPDX (Tag-Value)**, and **DepDiscover (JSON)** formats.
- Interactive **Dependency Table** with:
  - Multi-column sorting.
  - Severity-based color coding (Critical, High, Medium, Low, None).
  - Direct links to NVD/OSV for vulnerabilities.
  - Detailed CVE listing per dependency including fixed version detection.
- **Statistics Dashboard** featuring:
  - Vulnerability distribution pie chart using QtCharts.
  - License distribution overview.
- **SBOM Export**: Ability to export enriched dependency data to **CycloneDX 1.4 JSON**.
- **GitHub Update Checker**: Integrated background check for new application versions.
- **Modern C++23 Core**: Leverages C++23 features like `std::expected` and monadic operations.
- **CI/CD Integration**: Automated quality and security scans (CodeQL, Cppcheck, Clang-Tidy) and SBOM generation.
