/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#include "WCSWidget.h"

#include "../VM/MainWindowVM.h"

#include <QStandardItemModel>
#include <QTableView>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QFontDatabase>
#include <QTimer>

namespace Nastro
{

WCSWidget::WCSWidget(MainWindowVM* pMainWindowVM, QWidget* pParent)
    : QWidget(pParent)
    , m_pMainWindowVM(pMainWindowVM)
{
    InitUI();
    BindVM();
}

void WCSWidget::InitUI()
{
    m_pTableViewModel = new QStandardItemModel();
    m_pTableViewModel->setColumnCount(2);
    m_pTableViewModel->setHorizontalHeaderLabels({"Type", "Value"});

    m_pTableView = new QTableView();
    m_pTableView->horizontalHeader()->setVisible(true);
    m_pTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    m_pTableView->horizontalHeader()->setStretchLastSection(true);
    m_pTableView->verticalHeader()->setVisible(false);
    m_pTableView->setEditTriggers(QAbstractItemView::NoEditTriggers); // No editing of contents
    m_pTableView->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    m_pTableView->setModel(m_pTableViewModel);

    auto pLayout = new QVBoxLayout(this);
    pLayout->addWidget(m_pTableView);
}

void WCSWidget::BindVM()
{
    connect(m_pMainWindowVM, &MainWindowVM::Signal_OnPixelHoveredChanged, this, &WCSWidget::Slot_VM_OnPixelHoveredChanged);
}

void WCSWidget::Slot_VM_OnPixelHoveredChanged(const std::optional<PixelDetails>& pixelDetails)
{
    m_pTableViewModel->removeRows(0, m_pTableViewModel->rowCount());

    if (!pixelDetails)
    {
        return;
    }

    for (const auto& wcsCoord : pixelDetails->wcsCoords)
    {
        auto item = new QStandardItem(QString::fromStdString(wcsCoord.coordinateType));
        auto item2 = new QStandardItem(QString::fromStdString(std::format("{:.6f}", wcsCoord.worldCoord)));
        m_pTableViewModel->appendRow({item, item2});
    }
}

}
