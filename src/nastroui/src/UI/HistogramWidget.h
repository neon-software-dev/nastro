/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef NASTROUI_SRC_UI_HISTOGRAMWIDGET_H
#define NASTROUI_SRC_UI_HISTOGRAMWIDGET_H

#include <NFITS/Data/ImageData.h>

#include <QChartView>

#include <vector>
#include <optional>
#include <expected>

class QLineSeries;

namespace Nastro
{
    class HistogramWidget : public QChartView
    {
        Q_OBJECT

        public:

            explicit HistogramWidget(QWidget* pParent = nullptr);

            void DisplayHistogram(const NFITS::PhysicalStats& physicalStats,
                                  const std::optional<double>& minPhysicalValue = std::nullopt,
                                  const std::optional<double>& maxPhysicalValue = std::nullopt);

        signals:

            void Signal_OnMinVertLineChanged(double physicalValue, bool fromDrag);
            void Signal_OnMaxVertLineChanged(double physicalValue, bool fromDrag);

        private:

            struct VertLine
            {
                QLineSeries* pSeries{nullptr};
                bool dragging{false};
            };

        protected:

            void mousePressEvent(QMouseEvent *event) override;
            void mouseMoveEvent(QMouseEvent *event) override;
            void mouseReleaseEvent(QMouseEvent *event) override;

        private:

            void InitUI();

            [[nodiscard]] QLineSeries* CreateVertLine(double xPos, const QString& name);
            [[nodiscard]] bool IsMinVertLine(const VertLine& vertLine) const;
            [[nodiscard]] bool IsMaxVertLine(const VertLine& vertLine) const;

            [[nodiscard]] double GetXPosForPhysicalValue(const double& physicalValue) const;
            [[nodiscard]] double GetPhysicalValueForXPos(const double& xPos) const;

            void HandleVertLineDrag(const VertLine& vertLine, const double& chartXPos);
            void HandleVertLineRelease(VertLine& vertLine);

            [[nodiscard]] static QString GetVertLineName(bool isMaxLine, const double& physicalValue);

        private:

            NFITS::PhysicalStats m_physicalStats;

            std::optional<VertLine> m_minVertLine;
            std::optional<VertLine> m_maxVertLine;

            /*void SetEnableVertLines(bool enable);
            void SetHistogramData(const NFITS::PhysicalStats& physicalStats);

            [[nodiscard]] std::optional<double> GetMinVertLinePhysicalValue();
            [[nodiscard]] std::optional<double> GetMaxVertLinePhysicalValue();

        signals:

            void Signal_OnMinVertLineChanged(double physicalValue);
            void Signal_OnMaxVertLineChanged(double physicalValue);

        protected:

            void mousePressEvent(QMouseEvent *event) override;
            void mouseMoveEvent(QMouseEvent *event) override;
            void mouseReleaseEvent(QMouseEvent *event) override;

        private:

            struct VertLine
            {
                QLineSeries* pSeries{nullptr};
                bool dragging{false};
            };

        private:

            void InitUI();

            [[nodiscard]] std::optional<float> GetVertLinePercent(const std::optional<VertLine>& vertLine);

            bool AddVertLines(float minPercent, float maxPercent);
            [[nodiscard]] std::expected<VertLine, bool> AddVertLine(const qreal& chartX, const QString& label);
            void UpdateVertLinePos(const std::optional<VertLine>& vertLine, const qreal& chartX);

            [[nodiscard]] bool IsMinVertLine(const std::optional<VertLine>& vertLine) const;
            [[nodiscard]] bool IsMaxVertLine(const std::optional<VertLine>& vertLine) const;

            void RespondToMouseRelease(std::optional<VertLine>& vertLine);

        private:

            std::optional<NFITS::PhysicalStats> m_physicalStats;

            std::optional<VertLine> m_minLine;
            std::optional<VertLine> m_maxLine;

            bool m_enableVertLines{false};*/
    };
}

#endif //NASTROUI_SRC_UI_HISTOGRAMWIDGET_H
