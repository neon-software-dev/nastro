/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#include "SettingsDialog.h"
#include "SettingsRenderingWidget.h"

#include <QListWidget>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QLabel>

namespace Nastro
{

SettingsDialog::SettingsDialog(QWidget* parent, Qt::WindowFlags f)
    : QDialog(parent, f)
{
    setWindowTitle(tr("Settings"));

    InitUI();
}

void SettingsDialog::InitUI()
{
    m_pSettingsRenderingWidget = new SettingsRenderingWidget();

    auto pCategoriesListWidget = new QListWidget();
    auto pStackedWidget = new QStackedWidget();

    pCategoriesListWidget->addItem(tr("Image Rendering"));
    pStackedWidget->addWidget(m_pSettingsRenderingWidget);

    connect(pCategoriesListWidget, &QListWidget::currentRowChanged, pStackedWidget, &QStackedWidget::setCurrentIndex);

    pCategoriesListWidget->setCurrentRow(0);

    auto pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
    connect(pButtonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);

    auto pRightLayout = new QVBoxLayout();
    pRightLayout->addWidget(pStackedWidget, 1);
    pRightLayout->addWidget(pButtonBox, 0);

    auto pMainLayout = new QHBoxLayout(this);
    pMainLayout->addWidget(pCategoriesListWidget, 0);
    pMainLayout->addLayout(pRightLayout, 1);
}

}
