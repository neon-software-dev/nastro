/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#include "PixelDetailsWidget.h"

#include <NFITS/WCS/WCS.h>

#include <QLabel>
#include <QHBoxLayout>

namespace Nastro
{

PixelDetailsWidget::PixelDetailsWidget(QWidget* pParent)
    : QWidget(pParent)
{
    InitUI();
    DisplayPixelDetails(std::nullopt);
}

void PixelDetailsWidget::InitUI()
{
    m_pCoordLabel = new QLabel();
    m_pCoordLabel->setStyleSheet("border: 1px solid gray; padding: 4px;");
    m_pCoordLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

    m_pPhysicalLabel = new QLabel();
    m_pPhysicalLabel->setStyleSheet("border: 1px solid gray; padding: 4px;");
    m_pPhysicalLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

    m_pMainLayout = new QHBoxLayout(this);
    m_pMainLayout->setAlignment(Qt::AlignLeft);
    m_pMainLayout->addWidget(m_pCoordLabel);
    m_pMainLayout->addWidget(m_pPhysicalLabel);
}

void PixelDetailsWidget::DisplayPixelDetails(const std::optional<PixelDetails>& pixelDetails)
{
    if (!pixelDetails)
    {
        m_pCoordLabel->setText("Coord: (None)");
        m_pPhysicalLabel->setText("Physical: None");
        return;
    }

    if (pixelDetails->pixelCoordinate.size() >= 2)
    {
        m_pCoordLabel->setText(QString::fromStdString(
            std::format("Pixel: ({:.2f}, {:.2f})", pixelDetails->pixelCoordinate.at(0), pixelDetails->pixelCoordinate.at(1)))
        );
    }

    const auto unitsStr = pixelDetails->physicalUnit ? *pixelDetails->physicalUnit : "Units";
    const auto physicalText = std::format("Physical: {:.4f} ({})", pixelDetails->physicalValue, unitsStr);
    m_pPhysicalLabel->setText(QString::fromStdString(physicalText));
}

}
