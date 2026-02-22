<!--
SPDX-FileComment: Architecture documentation for the dependency tracker UI.
SPDX-FileType: DOCUMENTATION
SPDX-FileContributor: ZHENG Robert
SPDX-FileCopyrightText: 2026 ZHENG Robert
SPDX-License-Identifier: MIT

@file architecture.md
@brief Detailed architecture description and diagrams.
@version 0.2.0
@date 2026-02-22

@author ZHENG Robert (robert@hase-zheng.net)
@copyright Copyright (c) 2026 ZHENG Robert

@license MIT License
-->

# Architecture: Qt Dependency Tracker UI

This project follows a Model-View-Controller (MVC) like architecture to ensure separation of concerns, scalability, and maintainability.

---

<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->

**Table of Contents**

- [Architecture: Qt Dependency Tracker UI](#architecture-qt-dependency-tracker-ui)
  - [Component Overview](#component-overview)
  - [Class Diagram](#class-diagram)
  - [Component Diagram](#component-diagram)
  - [Sequence Diagram: Program Start \& Update Check](#sequence-diagram-program-start--update-check)
  - [Sequence Diagram: Data Interaction](#sequence-diagram-data-interaction)
  - [Use Case Diagram](#use-case-diagram)
  - [Activity Diagram: User Workflow](#activity-diagram-user-workflow)
  - [State Machine Diagram](#state-machine-diagram)
  - [Deployment Diagram](#deployment-diagram)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

---

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
  - `MainWindow`: Orchestrates the UI and connects logic (acts as a Controller).
  - `DependencyTableModel`: Custom sortable model based on `QAbstractTableModel`.
  - `QtCharts integration`: Native visualization for statistics.
- **External Integration**:
  - `qt_gh_update_checker`: Background update checks via GitHub API.

## Class Diagram

```mermaid
classDiagram
    class IParser {
        <<interface>>
        +parse(path) std::expected
    }
    class CycloneDXParser {
        +parse(path) std::expected
    }
    class SPDXParser {
        +parse(path) std::expected
    }
    class DepDiscoverParser {
        +parse(path) std::expected
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
        +onExportCycloneDX()
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

## Use Case Diagram

```mermaid
graph TD
    User((Security Engineer))
    subgraph App [Dependency Tracker UI]
        UC1(Load SBOM file)
        UC2(View dependency table)
        UC3(Inspect CVE details)
        UC4(View distribution statistics)
        UC5(Export to CycloneDX JSON)
        UC6(Check for application updates)
    end
    User --- UC1
    User --- UC2
    User --- UC3
    User --- UC4
    User --- UC5
    UC6 --- User
```

## Activity Diagram: User Workflow

```mermaid
flowchart TD
    Start([Start]) --> Init[Start Application]
    Init --> Fork1{ }
    Fork1 --> Update[Check GitHub for updates]
    Fork1 --> Display[Display Main Window]
    Update --> Join1{ }
    Display --> Join1
    Join1 --> Open[Open Dependency File]
    Open --> Success{Parsing successful?}
    Success -- Yes --> ShowTable[Display Dependency Table]
    ShowTable --> UpdateStats[Update Statistics Charts]
    UpdateStats --> EnableExport[Enable Export Button]
    EnableExport --> Fork2{ }
    Fork2 --> Inspect[Inspect CVEs in Browser]
    Fork2 --> ViewList[View Vulnerability List]
    Fork2 --> Export[Export to CycloneDX]
    Inspect --> End([End])
    ViewList --> End
    Export --> End
    Success -- No --> Error[Show Error Message]
    Error --> End
```

## State Machine Diagram

```mermaid
stateDiagram-v2
    [*] --> Initialized
    Initialized --> UpdateChecking: Startup
    UpdateChecking --> Initialized: Update info received
    Initialized --> ParsingData: File opened
    ParsingData --> DataLoaded: Success
    ParsingData --> Initialized: Failure (Error dialog)
    DataLoaded --> DataLoaded: Refresh (Open new file)
    DataLoaded --> Exporting: Export button clicked
    Exporting --> DataLoaded: Export finished
    DataLoaded --> [*]: Close
```

## Deployment Diagram

```mermaid
graph TD
    subgraph UserPC [User PC]
        subgraph OS [OS: Linux / Windows / macOS]
            App[QtDependencyTrackerUI]
            Qt[Qt6 Libraries]
        end
        SBOM[(SBOM File)]
    end
    subgraph External [External APIs]
        GHAPI[GitHub API]
        VulnInfo[NVD/OSV.dev Web]
    end
    App <--> SBOM
    App --- Qt
    App --> GHAPI
    App -.-> VulnInfo
```

## Object Diagram

Example of objects after loading a simple JSON:

```mermaid
graph TD
    dep1[Dependency: zlib]
    dep1_v[Version: 1.3.1]
    dep1_l[License: Zlib]
    cve1[CVE: CVE-2023-45853]
    cve1_s[Score: 9.8]
    cve1_c[Criticality: CRITICAL]

    dep1 --- dep1_v
    dep1 --- dep1_l
    dep1 --- cve1
    cve1 --- cve1_s
    cve1 --- cve1_c
```

## Package Diagram

```mermaid
graph TD
    subgraph root
        main[main.cpp]
        config[rz_config.hpp]
    end

    subgraph models
        D[Dependency.hpp]
        C[CVE.hpp]
    end

    subgraph utils
        IP[IParser.hpp]
        P1[CycloneDXParser.hpp]
        P2[SPDXParser.hpp]
        P3[DepDiscoverParser.hpp]
    end

    subgraph views
        MW[MainWindow.hpp]
        TM[DependencyTableModel.hpp]
        PW[PieChartWidget.hpp]
    end

    views --> models
    views --> utils
    utils --> models
    root --> views
```

## Timing Diagram: Update Check

```mermaid
sequenceDiagram
    participant App as MainWindow
    participant Thread as QtConcurrent
    participant Network as GitHub API

    Note over App: t=0: App Start
    App->>Thread: Start background check
    Note over Thread: t=1: Send HTTPS Request
    Thread->>Network: GET latest release
    Note over Network: t=2: Network latency...
    Network-->>Thread: 200 OK (JSON)
    Note over Thread: t=3: Parse JSON
    Thread-->>App: Emit Finished signal
    Note over App: t=4: Update Status Bar
```

## Communication Diagram: Open File

```mermaid
graph LR
    User --1: Click Open--> MW[MainWindow]
    MW --2: Show Dialog--> FD[QFileDialog]
    FD --3: File Path--> MW
    MW --4: Parse(Path)--> P[Parser]
    P --5: std::expected--> MW
    MW --6: setDependencies--> TM[DependencyTableModel]
    TM --7: dataChanged--> TV[QTableView]
    MW --8: updateCharts--> CV[QChartView]
```

## Interaction Overview Diagram

```mermaid
stateDiagram-v2
    state "App Start" as AS
    state "Update Logic" as UL {
        [*] --> CheckGitHub
        CheckGitHub --> FinishUpdate: Result
    }
    state "Interaction" as I {
        [*] --> Idle
        Idle --> LoadFile: onOpenFile
        LoadFile --> UpdateUI: Parse Success
        UpdateUI --> Idle
        Idle --> Export: onExport
        Export --> Idle
    }
    AS --> UL
    AS --> I
```

## Profile Diagram (UML Extension)

This diagram shows the stereotypes and extensions used in the project, such as Qt-specific signals/slots and C++23 features.

```mermaid
classDiagram
    class CPlusPlus23 {
        <<profile>>
    }
    class QtFramework {
        <<profile>>
    }
    class Signal {
        <<stereotype>>
    }
    class Slot {
        <<stereotype>>
    }
    class Expected {
        <<stereotype>>
    }
    
    CPlusPlus23 ..> Expected: defines
    QtFramework ..> Signal: defines
    QtFramework ..> Slot: defines
```
