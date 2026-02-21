/**
 * SPDX-FileComment: Custom Pie Chart Widget.
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: MIT
 *
 * @file PieChartWidget.hpp
 * @brief Simple widget to draw a pie chart using QPainter.
 * @version 0.1.0
 * @date 2026-02-21
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2026 ZHENG Robert
 *
 * @license MIT License
 */

#pragma once

#include <QWidget>
#include <QPainter>
#include <vector>
#include <string>

namespace views {

struct PieSlice {
    std::string label;
    int value;
    QColor color;
};

class PieChartWidget : public QWidget {
    Q_OBJECT
public:
    explicit PieChartWidget(QWidget* parent = nullptr) : QWidget(parent) {}

    void setSlices(std::vector<PieSlice> slices) {
        m_slices = std::move(slices);
        update();
    }

protected:
    void paintEvent(QPaintEvent* event) override {
        Q_UNUSED(event);
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        int total = 0;
        for (const auto& slice : m_slices) total += slice.value;
        if (total == 0) return;

        QRectF rect(10, 10, width() - 200, height() - 20);
        int start_angle = 0;

        for (const auto& slice : m_slices) {
            if (slice.value == 0) continue;
            int span_angle = static_cast<int>(360.0 * slice.value / total * 16.0);
            painter.setBrush(slice.color);
            painter.drawPie(rect, start_angle, span_angle);
            start_angle += span_angle;
        }

        // Legend
        int legend_x = width() - 180;
        int legend_y = 20;
        for (const auto& slice : m_slices) {
            painter.setBrush(slice.color);
            painter.drawRect(legend_x, legend_y, 15, 15);
            painter.setBrush(Qt::NoBrush);
            painter.drawText(legend_x + 25, legend_y + 12, 
                QString("%1: %2").arg(QString::fromStdString(slice.label)).arg(slice.value));
            legend_y += 25;
        }
    }

private:
    std::vector<PieSlice> m_slices;
};

} // namespace views
