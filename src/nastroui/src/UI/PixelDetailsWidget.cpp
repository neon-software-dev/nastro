/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#include "PixelDetailsWidget.h"

#include <QLabel>
#include <QHBoxLayout>

namespace Nastro
{

PixelDetailsWidget::PixelDetailsWidget(QWidget* pParent)
    : QWidget(pParent)
{
    m_pCoordLabel = new QLabel();
    m_pCoordLabel->setStyleSheet("border: 1px solid gray; padding: 4px;");

    m_pPhysicalLabel = new QLabel();
    m_pPhysicalLabel->setStyleSheet("border: 1px solid gray; padding: 4px;");

    auto pLayout = new QHBoxLayout(this);
    pLayout->addWidget(m_pCoordLabel);
    pLayout->addWidget(m_pPhysicalLabel);
    pLayout->addStretch();

    OnPixelChanged(std::nullopt);
}

void PixelDetailsWidget::OnPixelChanged(const std::optional<PixelDetails>& pixelDetails)
{
    if (pixelDetails)
    {
        m_pCoordLabel->setText(QString::fromStdString(std::format("Coord: ({}, {})",
                                                                  pixelDetails->position.first,
                                                                  pixelDetails->position.second)));

        const auto unitsStr = pixelDetails->physicalUnit ? *pixelDetails->physicalUnit : "Units";
        const auto physicalText = std::format("Physical ({}): {:.4f}", unitsStr, pixelDetails->physicalValue);
        m_pPhysicalLabel->setText(QString::fromStdString(physicalText));
    }
    else
    {
        m_pCoordLabel->setText("Coord: (None)");
        m_pPhysicalLabel->setText("Physical: None");
    }
}

}
