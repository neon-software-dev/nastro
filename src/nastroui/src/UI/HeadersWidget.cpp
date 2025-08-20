/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#include "HeadersWidget.h"

#include "../VM/MainWindowVM.h"

#include <QVBoxLayout>
#include <QTableView>
#include <QStandardItemModel>
#include <QHeaderView>
#include <QFontDatabase>

namespace Nastro
{

HeadersWidget::HeadersWidget(MainWindowVM* pMainWindowVM, QWidget* pParent)
    : QWidget(pParent)
    , m_pMainWindowVM(pMainWindowVM)
{
    InitUI();
    BindVM();
    InitialState();
}

void HeadersWidget::InitUI()
{
    m_pTableViewModel = new QStandardItemModel();
    m_pTableViewModel->setColumnCount(1);

    m_pTableView = new QTableView();
    m_pTableView->horizontalHeader()->setVisible(false);
    m_pTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_pTableView->setEditTriggers(QAbstractItemView::NoEditTriggers); // No editing of contents
    m_pTableView->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    m_pTableView->setModel(m_pTableViewModel);

    auto pLayout = new QVBoxLayout(this);
    pLayout->addWidget(m_pTableView);
}

void HeadersWidget::BindVM()
{
    connect(m_pMainWindowVM, &MainWindowVM::Signal_OnActivatedHDUChanged, this, &HeadersWidget::Slot_VM_OnActivatedHDUChanged);
}

void HeadersWidget::InitialState()
{
    const auto activatedHDU = m_pMainWindowVM->GetActivatedHDU();
    if (!activatedHDU)
    {
        m_pTableViewModel->clear();
        return;
    }

    if (activatedHDU)
    {
        const auto hdu = m_pMainWindowVM->GetImportedFileHDU(activatedHDU->filePath, activatedHDU->hduIndex);
        if (!hdu) { return; }

        DisplayHDU(*hdu);
    }
}

void HeadersWidget::Slot_VM_OnActivatedHDUChanged(const std::optional<ActivatedHDU>& activatedHDU)
{
    if (!activatedHDU)
    {
        m_pTableViewModel->clear();
        return;
    }

    const auto hdu = m_pMainWindowVM->GetImportedFileHDU(activatedHDU->filePath, activatedHDU->hduIndex);
    if (!hdu) { return; }

    DisplayHDU(*hdu);
}

void HeadersWidget::DisplayHDU(const NFITS::HDU& hdu)
{
    m_pTableViewModel->clear();

    for (const auto& headerBlock : hdu.header.headerBlocks)
    {
        for (const auto& keywordRecord : headerBlock.keywordRecords)
        {
            const auto keywordRecordRaw = keywordRecord.GetKeywordRecordRaw();

            // Ignore empty keyword records
            if (std::ranges::all_of(keywordRecordRaw, [](const char c){ return c == ' '; }))
            {
                continue;
            }

            auto *item = new QStandardItem(QString::fromStdString(keywordRecord.GetKeywordRecordRaw()));
            m_pTableViewModel->appendRow(item);
        }
    }
}

}
