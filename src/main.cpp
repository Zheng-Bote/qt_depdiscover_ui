/**
 * SPDX-FileComment: Main entry point for the dependency tracker UI.
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: MIT
 *
 * @file main.cpp
 * @brief Entry point for the application.
 * @version 0.1.0
 * @date 2026-02-21
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2026 ZHENG Robert
 *
 * @license MIT License
 */

#include <QApplication>
#include "views/MainWindow.hpp"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    views::MainWindow window;
    window.show();
    
    return app.exec();
}
