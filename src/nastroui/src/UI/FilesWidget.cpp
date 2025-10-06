/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#include "FilesWidget.h"
#include "FilesModel.h"

#include "../Util/Common.h"

#include "../VM/MainWindowVM.h"

#include <QTreeView>
#include <QVBoxLayout>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QPainter>
#include <QHeaderView>
#include <QToolBar>

#include <iostream>

namespace Nastro
{

FilesWidget::FilesWidget(MainWindowVM* pMainWindowVM, QWidget* pParent)
    : QWidget(pParent)
    , m_pMainWindowVM(pMainWindowVM)
{
    setAcceptDrops(true);

    InitUI();
    BindVM();
    InitialState();
}

void FilesWidget::InitUI()
{
    //
    // Top Toolbar
    //
    auto pTopToolbar = new QToolBar();
    m_pCompareAction = pTopToolbar->addAction(tr("Compare"));
    m_pCompareAction->setEnabled(false);
    connect(m_pCompareAction, &QAction::triggered, this, &FilesWidget::Slot_Compare_ActionTriggered);

    //
    // Files Tree View
    //
    m_pTreeViewModel = std::make_unique<FilesModel>();

    m_pTreeViewModelSortProxy = std::make_unique<FilesModelSortProxy>();
    m_pTreeViewModelSortProxy->setSourceModel(m_pTreeViewModel.get());

    m_pTreeView = new QTreeView();
    m_pTreeView->setModel(m_pTreeViewModelSortProxy.get());
    m_pTreeView->setSortingEnabled(true);
    m_pTreeView->sortByColumn(0, Qt::SortOrder::AscendingOrder);
    m_pTreeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_pTreeView->setSelectionBehavior(QAbstractItemView::SelectItems);
    m_pTreeView->setItemsExpandable(false); // Don't allow expanding/collapsing
    m_pTreeView->setExpandsOnDoubleClick(false); // Don't allow expanding/collapsing
    m_pTreeView->setRootIsDecorated(false); // Hides expand/collapse option
    m_pTreeView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    connect(m_pTreeView, &QAbstractItemView::activated, this, &FilesWidget::Slot_OnTreeView_Activated);
    connect(m_pTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &FilesWidget::Slot_OnTreeViewModel_SelectionChanged);

    //
    // Main Layout
    //
    auto pLayout = new QVBoxLayout(this);
    pLayout->addWidget(pTopToolbar, 0);
    pLayout->addWidget(m_pTreeView, 1);
}

void FilesWidget::BindVM()
{
    // When VM has imported new files, add them to this widget
    connect(m_pMainWindowVM, &MainWindowVM::Signal_FilesImported, this, &FilesWidget::AddFiles);
}

void FilesWidget::InitialState()
{
    // Immediately add all files that the VM already has imported (if any)
    AddFiles(m_pMainWindowVM->GetImportedFiles());
}

void FilesWidget::AddFiles(const std::unordered_map<std::filesystem::path, std::vector<NFITS::HDU>>& importedFiles)
{
    // Add the files to the model
    m_pTreeViewModel->AddFiles(importedFiles);

    // Force the treeview to expand its nodes
    m_pTreeView->expandAll();
}

void FilesWidget::dragEnterEvent(QDragEnterEvent* pEvent)
{
    for (const auto& url : pEvent->mimeData()->urls())
    {
        // Allow dragging local files into the files widget; if anything else is dragged in, reject the drag
        if (!url.isLocalFile())
        {
            pEvent->ignore();
            return;
        }
    }

    pEvent->acceptProposedAction();

    m_inDragDrop = true;
    update();
}

void FilesWidget::dropEvent(QDropEvent* pEvent)
{
    std::vector<std::filesystem::path> filePaths;
    std::vector<std::filesystem::path> directoryPaths;

    for (const auto& url : pEvent->mimeData()->urls())
    {
        const auto path = std::filesystem::path(url.toLocalFile().toStdString());

        std::error_code ec{};
        if (std::filesystem::is_regular_file(path, ec))
        {
            filePaths.emplace_back(path);
        }
        else if (std::filesystem::is_directory(path, ec))
        {
            directoryPaths.emplace_back(path);
        }
    }

    pEvent->acceptProposedAction();

    if (!filePaths.empty())
    {
        m_pMainWindowVM->OnImportFiles(filePaths);
    }
    for (const auto& directoryPath : directoryPaths)
    {
        m_pMainWindowVM->OnImportDirectory(directoryPath);
    }

    m_inDragDrop = false;
    update();
}

void FilesWidget::dragLeaveEvent(QDragLeaveEvent* pEvent)
{
    QWidget::dragLeaveEvent(pEvent);

    m_inDragDrop = false;
    update();
}

void FilesWidget::paintEvent(QPaintEvent* pEvent)
{
    QWidget::paintEvent(pEvent);

    if (m_inDragDrop)
    {
        // Visual state that indicates a drag is active
        QPainter p(this);
        p.setBrush(QColor(0, 0, 0, 50));
        p.setPen(Qt::NoPen);
        p.drawRect(rect());
    }
}

std::vector<const FilesTreeItem*> FilesWidget::GetSelectedTreeItems() const
{
    std::vector<const FilesTreeItem*> selectedTreeItems;

    for (const auto index : m_pTreeView->selectionModel()->selection().indexes())
    {
        const auto sourceIndex = m_pTreeViewModelSortProxy->mapToSource(index);
        if (!sourceIndex.isValid())
        {
            continue;
        }

        selectedTreeItems.push_back(static_cast<const FilesTreeItem*>(sourceIndex.internalPointer()));
    }

    return selectedTreeItems;
}

void FilesWidget::Slot_OnTreeView_Activated(const QModelIndex& index)
{
    if (!index.isValid())
    {
        return;
    }

    const auto sourceIndex = m_pTreeViewModelSortProxy->mapToSource(index);
    if (!sourceIndex.isValid())
    {
        return;
    }

    const auto pTreeItem = static_cast<const FilesTreeItem*>(sourceIndex.internalPointer());
    if (pTreeItem->GetType() != FilesTreeItem::Type::HDU)
    {
        return;
    }

    const auto pHDUTreeItem = dynamic_cast<const HDUFilesTreeItem*>(pTreeItem);

    const auto pParentTreeItem =  static_cast<const FilesTreeItem*>(sourceIndex.parent().internalPointer());
    const auto pFITSTreeItem = dynamic_cast<const FITSFilesTreeItem*>(pParentTreeItem);

    emit Signal_OnHDUActivated(FileHDU{.filePath = pFITSTreeItem->GetFilePath(), .hduIndex = pHDUTreeItem->GetHDUIndex()});
}

void FilesWidget::Slot_OnTreeViewModel_SelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    (void)selected; (void)deselected;

    const auto selectedTreeItems = GetSelectedTreeItems();

    //
    // Enable the compare action in the case where the selection is 2 or more (non-empty) images
    //
    const bool allSelectionImageHDUS = std::ranges::all_of(selectedTreeItems, [](const FilesTreeItem* pTreeItem){
        if (pTreeItem->GetType() != FilesTreeItem::Type::HDU) { return false; }

        const auto pHDUTreeItem = dynamic_cast<const HDUFilesTreeItem*>(pTreeItem);
        if (pHDUTreeItem->GetHDU().type != NFITS::HDU::Type::Image) { return false; }

        if (pHDUTreeItem->GetHDU().dataByteSize == 0) { return false; }

        return true;
    });

    m_pCompareAction->setEnabled(allSelectionImageHDUS && selectedTreeItems.size() >= 2);
}

void FilesWidget::Slot_Compare_ActionTriggered(bool)
{
    std::vector<FileHDU> compares;

    const auto selectedTreeItems = GetSelectedTreeItems();

    for (const auto& pTreeItem : selectedTreeItems)
    {
        if (pTreeItem->GetType() != FilesTreeItem::Type::HDU) { continue; }

        const auto pHDUTreeItem = dynamic_cast<const HDUFilesTreeItem*>(pTreeItem);

        const bool emptyData = pHDUTreeItem->GetHDU().GetDataByteSize() == 0U;
        if (emptyData) { continue; }

        const auto pParentTreeItem =  pHDUTreeItem->GetParent();
        if (!pParentTreeItem) { continue; }

        const auto pFITSTreeItem = dynamic_cast<const FITSFilesTreeItem*>(*pParentTreeItem);

        compares.push_back(FileHDU{.filePath = pFITSTreeItem->GetFilePath(), .hduIndex = pHDUTreeItem->GetHDUIndex()});
    }

    if (!compares.empty())
    {
        emit Signal_OnCompareImageHDUs(compares);
    }
}

}
