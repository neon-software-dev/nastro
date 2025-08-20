/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#include "AxisSliderWidget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSlider>
#include <QLabel>

namespace Nastro
{

AxisSliderWidget::AxisSliderWidget(unsigned int axis, int64_t axisn, QWidget* pParent)
    : AxisWidget(pParent)
{
    InitUI(axis, axisn);
}

void AxisSliderWidget::InitUI(unsigned int axis, int64_t axisn)
{
    assert(axisn >= 1);

    auto pAxisLabel = new QLabel();
    pAxisLabel->setText(QString::fromStdString(std::format("Axis {} Control", axis)));

    auto pValueLabel = new QLabel();
    pValueLabel->setText(QString::fromStdString(std::format("{}", 0)));

    auto pSlider = new QSlider();
    pSlider->setOrientation(Qt::Orientation::Horizontal);
    pSlider->setMinimum(0);
    pSlider->setMaximum(static_cast<int>(axisn - 1));
    pSlider->setTickPosition(QSlider::TickPosition::TicksAbove);
    connect(pSlider, &QSlider::valueChanged, [=,this](int val){
        pValueLabel->setText(QString::fromStdString(std::format("{}", val)));
        emit Signal_ValueChanged(val);
    });

    auto pSliderLayout = new QHBoxLayout();
    pSliderLayout->addWidget(pSlider);
    pSliderLayout->addWidget(pValueLabel);

    auto pVertLayout = new QVBoxLayout(this);
    pVertLayout->addWidget(pAxisLabel);
    pVertLayout->addLayout(pSliderLayout);
}

}
