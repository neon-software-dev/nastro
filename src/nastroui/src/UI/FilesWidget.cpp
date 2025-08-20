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
    m_pTreeViewModel = std::make_unique<FilesModel>();

    m_pTreeViewModelSortProxy = std::make_unique<FilesModelSortProxy>();
    m_pTreeViewModelSortProxy->setSourceModel(m_pTreeViewModel.get());

    m_pTreeView = new QTreeView();
    m_pTreeView->setModel(m_pTreeViewModelSortProxy.get());
    m_pTreeView->setSortingEnabled(true);
    m_pTreeView->sortByColumn(0, Qt::SortOrder::AscendingOrder);
    m_pTreeView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    connect(m_pTreeView, &QAbstractItemView::activated, this, &FilesWidget::Slot_OnTreeView_Activated);

    auto pLayout = new QVBoxLayout(this);
    pLayout->addWidget(m_pTreeView);
}

void FilesWidget::BindVM()
{
    connect(m_pMainWindowVM, &MainWindowVM::Signal_FilesImported, this, [this](const auto& addedFiles){
        m_pTreeViewModel->AddFiles(addedFiles);
    });
}

void FilesWidget::InitialState()
{
    m_pTreeViewModel->AddFiles(m_pMainWindowVM->GetImportedFiles());
}

void FilesWidget::Slot_OnTreeView_Activated(const QModelIndex& index)
{
    if (!index.isValid())
    {
        return;
    }

    //const auto sourceIndex = index;
    const auto sourceIndex = m_pTreeViewModelSortProxy->mapToSource(index);
    if (!sourceIndex.isValid())
    {
        return;
    }

    auto pTreeItem = static_cast<const FilesTreeItem*>(sourceIndex.internalPointer());
    if (pTreeItem->GetType() != FilesTreeItem::Type::HDU)
    {
        return;
    }

    auto pHDUTreeItem = dynamic_cast<const HDUFilesTreeItem*>(pTreeItem);

    auto pParentTreeItem =  static_cast<const FilesTreeItem*>(sourceIndex.parent().internalPointer());
    auto pFITSTreeItem = dynamic_cast<const FITSFilesTreeItem*>(pParentTreeItem);

    emit Signal_OnHDUActivated(pFITSTreeItem->GetFilePath(), pHDUTreeItem->GetHDUIndex());
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

}
