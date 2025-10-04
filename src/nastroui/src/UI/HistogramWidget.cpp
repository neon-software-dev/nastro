/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#include "HistogramWidget.h"

#include <QAreaSeries>
#include <QLineSeries>
#include <QChart>
#include <QChartView>
#include <QValueAxis>

#include <cmath>

namespace Nastro
{

HistogramWidget::HistogramWidget(QWidget* pParent)
    : QChartView(pParent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    InitUI();
}

void HistogramWidget::InitUI()
{
    auto pChart = new QChart();
    pChart->setTitle(tr("Histogram"));

    setChart(pChart);
}

void HistogramWidget::DisplayHistogram(const NFITS::PhysicalStats& physicalStats,
                                       const std::optional<double>& minPhysicalValue,
                                       const std::optional<double>& maxPhysicalValue)
{
    //
    // Clear existing chart content
    //
    chart()->removeAllSeries();
    m_minVertLine = std::nullopt;
    m_maxVertLine = std::nullopt;

    m_physicalStats = physicalStats;

    //
    // Build histogram series
    //
    auto pLineSeries = new QLineSeries();
    for (std::size_t x = 0; x < physicalStats.histogram.size(); ++x)
    {
        pLineSeries->append((int)x, (int)physicalStats.histogram.at(x));
    }

    auto pAreaSeries = new QAreaSeries(pLineSeries);
    pAreaSeries->setName(tr("Physical Value"));

    //
    // Add series to the chart
    //
    chart()->addSeries(pAreaSeries);
    chart()->createDefaultAxes();

    const auto pXAxis = dynamic_cast<QValueAxis*>(chart()->axes(Qt::Orientation::Horizontal).front());
    pXAxis->setTitleText(tr("Bin"));

    const auto pYAxis = dynamic_cast<QValueAxis*>(chart()->axes(Qt::Orientation::Vertical).front());
    pYAxis->setTitleText(tr("Count"));

    //
    // Build vert line series
    //
    if (minPhysicalValue)
    {
        const auto pSeries = CreateVertLine(GetXPosForPhysicalValue(*minPhysicalValue), GetVertLineName(false, *minPhysicalValue));
        m_minVertLine = VertLine{.pSeries = pSeries, .dragging = false};
    }
    if (maxPhysicalValue)
    {
        const auto pSeries = CreateVertLine(GetXPosForPhysicalValue(*maxPhysicalValue), GetVertLineName(true, *maxPhysicalValue));
        m_maxVertLine = VertLine{.pSeries = pSeries, .dragging = false};
    }
}

QLineSeries* HistogramWidget::CreateVertLine(double xPos, const QString& name)
{
    const auto pXAxis = dynamic_cast<QValueAxis*>(chart()->axes(Qt::Orientation::Horizontal).front());
    const auto pYAxis = dynamic_cast<QValueAxis*>(chart()->axes(Qt::Orientation::Vertical).front());

    auto pSeries = new QLineSeries();
    pSeries->setName(name);
    pSeries->append(xPos, pYAxis->min());
    pSeries->append(xPos, pYAxis->max());

    QPen pen(0x059605);
    pen.setWidth(5);
    pSeries->setPen(pen);

    chart()->addSeries(pSeries);

    pSeries->attachAxis(pXAxis);
    pSeries->attachAxis(pYAxis);

    return pSeries;
}

bool HistogramWidget::IsMinVertLine(const VertLine& vertLine) const
{
    return m_minVertLine && (m_minVertLine->pSeries == vertLine.pSeries);
}

bool HistogramWidget::IsMaxVertLine(const VertLine& vertLine) const
{
    return m_maxVertLine && (m_maxVertLine->pSeries == vertLine.pSeries);
}

double HistogramWidget::GetXPosForPhysicalValue(const double& physicalValue) const
{
    const auto pXAxis = dynamic_cast<QValueAxis*>(chart()->axes(Qt::Orientation::Horizontal).front());

    const auto physicalRange = m_physicalStats.minMax.second - m_physicalStats.minMax.first;
    auto physicalPercentage = (physicalValue - m_physicalStats.minMax.first) / physicalRange;
    physicalPercentage = std::clamp(physicalPercentage, 0.0, 1.0);

    const auto xAxisRange = pXAxis->max() - pXAxis->min();

    return (physicalPercentage * xAxisRange) + pXAxis->min();
}

double HistogramWidget::GetPhysicalValueForXPos(const double& xPos) const
{
    const auto pXAxis = dynamic_cast<QValueAxis*>(chart()->axes(Qt::Orientation::Horizontal).front());

    const auto xRange = pXAxis->max() - pXAxis->min();
    auto xPercentage = (xPos - pXAxis->min()) / xRange;
    xPercentage = std::clamp(xPercentage, 0.0, 1.0);

    const auto physicalValueRange = m_physicalStats.minMax.second - m_physicalStats.minMax.first;

    return (xPercentage * physicalValueRange) + m_physicalStats.minMax.first;
}

void HistogramWidget::mousePressEvent(QMouseEvent* event)
{
    QChartView::mousePressEvent(event);

    const auto chartPos = chart()->mapToValue(event->pos());
    if (chartPos.isNull()) { return; }
    const auto chartX = chartPos.x();

    // How far away from a vert line counts as clicking the line (in x-axis units)
    static constexpr float LINE_CLICK_DIST = 1.0f;

    // If we have both min/max lines, determine which line the click is closest to,
    // and if it's close enough to it, mark that line as being dragged
    if (m_minVertLine && m_maxVertLine)
    {
        const auto distMin = std::abs(m_minVertLine->pSeries->points().at(0).x() - chartX);
        const auto distMax = std::abs(m_maxVertLine->pSeries->points().at(0).x() - chartX);

        if ((distMin < distMax) && (distMin <= LINE_CLICK_DIST))
        {
            m_minVertLine->dragging = true;
        }
        else if (distMax <= LINE_CLICK_DIST)
        {
            m_maxVertLine->dragging = true;
        }
    }
    // Otherwise, if we only have a min line, and the click is close enough to it, mark it as being dragged
    else if (m_minVertLine)
    {
        const auto distMin = std::abs(m_minVertLine->pSeries->points().at(0).x() - chartX);
        if (distMin <= LINE_CLICK_DIST)
        {
            m_minVertLine->dragging = true;
        }
    }
    // Likewise for the max line
    else if (m_maxVertLine)
    {
        const auto distMax = std::abs(m_maxVertLine->pSeries->points().at(0).x() - chartX);
        if (distMax <= LINE_CLICK_DIST)
        {
            m_maxVertLine->dragging = true;
        }
    }
}

void HistogramWidget::mouseMoveEvent(QMouseEvent* event)
{
    QChartView::mouseMoveEvent(event);

    const auto chartPos = chart()->mapToValue(event->pos());
    if (chartPos.isNull()) { return; }
    const auto chartX = chartPos.x();

    if (m_minVertLine && m_minVertLine->dragging)
    {
        HandleVertLineDrag(*m_minVertLine, chartX);
    }

    if (m_maxVertLine && m_maxVertLine->dragging)
    {
        HandleVertLineDrag(*m_maxVertLine, chartX);
    }
}

void HistogramWidget::mouseReleaseEvent(QMouseEvent* event)
{
    QChartView::mouseReleaseEvent(event);

    if (m_minVertLine && m_minVertLine->dragging) { HandleVertLineRelease(*m_minVertLine); }
    if (m_maxVertLine && m_maxVertLine->dragging) { HandleVertLineRelease(*m_maxVertLine); }
}

void HistogramWidget::HandleVertLineDrag(const VertLine& vertLine, const double& chartXPos)
{
    const auto pYAxis = dynamic_cast<QValueAxis*>(chart()->axes(Qt::Orientation::Vertical).front());

    vertLine.pSeries->removePoints(0, 2);
    vertLine.pSeries->append(chartXPos, pYAxis->min());
    vertLine.pSeries->append(chartXPos, pYAxis->max());

    vertLine.pSeries->setName(GetVertLineName(IsMaxVertLine(vertLine), GetPhysicalValueForXPos(chartXPos)));
}

void HistogramWidget::HandleVertLineRelease(VertLine& vertLine)
{
    const auto lineX = vertLine.pSeries->points().at(0).x();
    const auto physicalValue = GetPhysicalValueForXPos(lineX);

    if (IsMinVertLine(vertLine))
    {
        emit Signal_OnMinVertLineChanged(physicalValue, true);
    }
    else
    {
        emit Signal_OnMaxVertLineChanged(physicalValue, true);
    }

    vertLine.dragging = false;
}

QString HistogramWidget::GetVertLineName(bool isMaxLine, const double& physicalValue)
{
    const auto baseName = isMaxLine ? tr("Max") : tr("Min");
    const auto lineName = std::format("{} (Physical: {:.4f})", baseName.toStdString(), physicalValue);
    return QString::fromStdString(lineName);
}

/*
    QPen pen(0x059605);
    pen.setWidth(3);
    pHistogramSeries->setPen(pen);

    QLinearGradient gradient(QPointF(0, 0), QPointF(0, 1));
    gradient.setColorAt(0.0, 0x3cc63c);
    gradient.setColorAt(1.0, 0x26f626);
    gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
    pHistogramSeries->setBrush(gradient);
 */

}
