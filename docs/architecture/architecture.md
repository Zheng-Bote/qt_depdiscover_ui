<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**

- [Architecture: Qt Dependency Tracker UI](#architecture-qt-dependency-tracker-ui)
  - [Component Overview](#component-overview)
  - [Class Diagram](#class-diagram)
  - [Component Diagram](#component-diagram)
  - [Sequence Diagram: Program Start & Update Check](#sequence-diagram-program-start--update-check)
  - [Sequence Diagram: Data Interaction](#sequence-diagram-data-interaction)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

# Architecture: Qt Dependency Tracker UI

This project follows a Model-View-Controller (MVC) like architecture to ensure separation of concerns, scalability, and maintainability.

## Component Overview

- **Models (`models::`)**: Define data structures for `Dependency` and `CVE`.
    - `Dependency`: Contains name, version, fixed version, licenses, and a list of CVEs.
    - `CVE`: Contains ID, description, fixed version, and criticality level (mapped from scores or vectors).
- **Utils (Parsers) (`utils::`)**: Logic for handling different file formats.
    - `IParser`: Interface for all parsers using `std::expected`.
    - `CycloneDXParser`: Standard JSON SBOM support.
    - `SPDXParser`: Support for Tag-Value SPDX files.
    - `DepDiscoverParser`: Custom JSON format support with CVSS vector heuristic.
- **Views (`views::`)**: Qt GUI components.
    - `MainWindow`: Orchestrates the UI and connects logic.
    - `DependencyTableModel`: Custom sortable model based on `QAbstractTableModel`.
    - `QtCharts integration`: Native visualization for statistics.
- **External Integration**:
    - `qt_gh_update_checker`: Background update checks via GitHub API.

## Class Diagram

```mermaid
classDiagram
    class IParser {
        <<interface>>
        +parse(path) expected
    }
    class CycloneDXParser {
        +parse(path) expected
    }
    class SPDXParser {
        +parse(path) expected
    }
    class DepDiscoverParser {
        +parse(path) expected
    }
    IParser <|-- CycloneDXParser
    IParser <|-- SPDXParser
    IParser <|-- DepDiscoverParser

    class Dependency {
        +string name
        +string version
        +string fixed_version
        +string license
        +vector<CVE> cves
        +Criticality max_criticality
        +update_max_criticality()
    }
    class CVE {
        +string id
        +string description
        +string fixed_version
        +double cvss_score
        +Criticality criticality
    }
    Dependency "1" *-- "many" CVE

    class MainWindow {
        +onOpenFile()
        +onCellClicked(index)
        +updateStatistics()
        -initUpdateChecker()
    }
    class DependencyTableModel {
        +setDependencies(deps)
        +data(index, role)
    }
    
    class GitHubUpdateChecker {
        +checkForUpdates()
        +updateAvailable(available, version) signal
    }

    MainWindow *-- DependencyTableModel
    MainWindow *-- GitHubUpdateChecker
    MainWindow ..> IParser
```

## Component Diagram

```mermaid
graph TD
    subgraph UI
        MW[MainWindow]
        TM[DependencyTableModel]
        Chart[QtCharts]
    end

    subgraph Business Logic
        P[Parsers: CycloneDX, SPDX, DepDiscover]
        UC[GitHubUpdateChecker]
    end

    subgraph Data Models
        D[Dependency Model]
        C[CVE Model]
    end

    MW --> TM
    MW --> Chart
    MW --> P
    MW --> UC
    P --> D
    D --> C
    TM --> D
```

## Sequence Diagram: Program Start & Update Check

```mermaid
sequenceDiagram
    participant App as MainWindow
    participant Thread as QtConcurrent
    participant GH as GitHub API
    participant UI as Status Bar

    App->>App: initUpdateChecker()
    App->>Thread: Run check_github_update()
    App->>UI: Show "Checking for updates..."
    Thread->>GH: GET /releases/latest
    GH-->>Thread: return JSON
    Thread-->>App: signal finished()
    alt Update Available
        App->>UI: Show version & Download Link
    else Up to date
        App->>UI: Show "Up to date" (5s)
    end
```

## Sequence Diagram: Data Interaction

```mermaid
sequenceDiagram
    participant User
    participant View as TableView
    participant Model as DependencyTableModel
    participant OS as Web Browser / MsgBox

    User->>View: Click "Criticality" cell
    View->>Model: get index
    View-->>User: Open Browser (NVD/OSV)

    User->>View: Click "CVE Count" cell
    View->>Model: get index
    View-->>User: Show MessageBox with CVE list
```
