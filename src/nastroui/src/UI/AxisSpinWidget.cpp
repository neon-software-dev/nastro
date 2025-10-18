/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#include "AxisSpinWidget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSpinBox>
#include <QLabel>

namespace Nastro
{

AxisSpinWidget::AxisSpinWidget(unsigned int axis, int64_t axisn, QWidget* pParent)
    : AxisWidget(pParent)
{
    InitUI(axis, axisn);
}

void AxisSpinWidget::InitUI(unsigned int axis, int64_t axisn)
{
    auto pAxisLabel = new QLabel();
    pAxisLabel->setText(QString::fromStdString(std::format("Axis {} Control", axis)));

    auto pSpinBox = new QSpinBox();
    pSpinBox->setMinimum(0);
    pSpinBox->setMaximum(static_cast<int>(axisn - 1));
    connect(pSpinBox, &QSpinBox::valueChanged, [=,this](int val){
        emit Signal_ValueChanged(val);
    });
    auto pSpinLayout = new QHBoxLayout();
    pSpinLayout->addWidget(pSpinBox);

    auto pVertLayout = new QVBoxLayout(this);
    pVertLayout->addWidget(pAxisLabel);
    pVertLayout->addLayout(pSpinLayout);
}

}
