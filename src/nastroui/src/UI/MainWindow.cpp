/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#include "MainWindow.h"
#include "FilesWidget.h"
#include "HeadersWidget.h"
#include "ImageWidget.h"
#include "NastroDockWidget.h"

#include "../VM/MainWindowVM.h"

#include "../Data/ImageData.h"

#include "../Util/LoadHDUDataWorker.h"
#include "../Util/ProgressDialogWork.h"

#include <NFITS/IFITSByteSource.h>

#include <QDockWidget>
#include <QLabel>
#include <QMenuBar>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QMdiArea>
#include <QMdiSubWindow>

namespace Nastro
{

MainWindow::MainWindow()
    : QMainWindow(nullptr, Qt::WindowFlags())
    , m_pVM(std::make_unique<MainWindowVM>(this))
{
    InitUI();
    BindVM();
}

void MainWindow::InitUI()
{
    InitMenuBar();
    InitWidgets();
}

void MainWindow::InitMenuBar()
{
    // File Menu
    {
        auto pFileMenu = menuBar()->addMenu(tr("&File"));

        auto pImportFilesAction = pFileMenu->addAction(tr("Import &Files"));
        connect(pImportFilesAction, &QAction::triggered, this, &MainWindow::Slot_File_ImportFiles_ActionTriggered);

        auto pImportDirectoryAction = pFileMenu->addAction(tr("Import &Directory"));
        connect(pImportDirectoryAction, &QAction::triggered, this, &MainWindow::Slot_File_ImportDirectory_ActionTriggered);

        auto pExitAction = pFileMenu->addAction(tr("&Exit"));
        connect(pExitAction, &QAction::triggered, this, &MainWindow::Slot_File_Exit_ActionTriggered);
    }

    // View Menu
    {
        auto pViewMenu = menuBar()->addMenu(tr("&View"));

        m_pViewFilesAction = pViewMenu->addAction(tr("&Files"));
        connect(m_pViewFilesAction, &QAction::triggered, [this](bool){ OnViewFiles(); });

        m_pViewHeadersAction = pViewMenu->addAction(tr("&Headers"));
        connect(m_pViewHeadersAction, &QAction::triggered, [this](bool){ OnViewHeaders(); });
    }
}

void MainWindow::InitWidgets()
{
    //
    // Main Widgets
    //
    m_pMdiArea = new QMdiArea(this);
    m_pMdiArea->setViewMode(QMdiArea::SubWindowView);
    connect(m_pMdiArea, &QMdiArea::subWindowActivated, this, &MainWindow::Slot_UI_MdiArea_SubWindowActivated);

    setCentralWidget(m_pMdiArea);

    //
    // Open default widgets
    //
    OnViewFiles();
}

void MainWindow::BindVM()
{

}

void MainWindow::Slot_File_ImportFiles_ActionTriggered()
{
    // Produce a string that looks like "*.fits *.fit *.fts"
    std::string extensionsStr;

    std::size_t pos = 0;
    for (const auto& extension : VALID_FITS_EXTENSIONS)
    {
        extensionsStr += std::format("*{}", extension);

        if (pos++ != VALID_FITS_EXTENSIONS.size() - 1)
        {
            extensionsStr += " ";
        }
    }

    const auto filterStr = std::format("FITS files ({});;All files (*)", extensionsStr);

    const auto qFileNameList = QFileDialog::getOpenFileNames(this, tr("Select Files"), QString(), QString::fromStdString(filterStr));

    std::vector<std::filesystem::path> filePaths;
    std::ranges::transform(qFileNameList, std::back_inserter(filePaths), [](const auto& qFileName){
        return std::filesystem::path(qFileName.toStdString());
    });

    if (!filePaths.empty())
    {
        m_pVM->OnImportFiles(filePaths);
    }
}

void MainWindow::Slot_File_ImportDirectory_ActionTriggered()
{
    const auto qDirectoryName = QFileDialog::getExistingDirectory(this, tr("Select Directory"));

    if (!qDirectoryName.isEmpty())
    {
        m_pVM->OnImportDirectory(std::filesystem::path(qDirectoryName.toStdString()));
    }
}

void MainWindow::Slot_File_Exit_ActionTriggered()
{
   close();
}

void MainWindow::Slot_FilesWidget_OnHDUActivated(const std::filesystem::path& filePath, const size_t& hduIndex)
{
    //
    // Load the specified HDU data from the file
    //
    auto pWorker = new LoadHDUDataWorker(filePath, hduIndex);

    auto pProgressDialog = new ProgressDialogWork(pWorker, ProgressDialogArgs{.isModal = true, .canBeCancelled = true}, this);
    connect(pProgressDialog, &ProgressDialogWork::Signal_WorkFinished, this, &MainWindow::Slot_LoadHDUData_Complete);
}

void MainWindow::Slot_LoadHDUData_Complete(Nastro::Worker* pWorker)
{
    if (pWorker->IsCancelled()) { return; }

    auto pLoadHDUDataWorker = dynamic_cast<LoadHDUDataWorker*>(pWorker);
    const auto filePath = pLoadHDUDataWorker->GetFilePath();
    const auto hduIndex = pLoadHDUDataWorker->GetHDUIndex();

    auto& pData = pLoadHDUDataWorker->GetResult();
    if (!pData)
    {
        return;
    }

    switch ((*pData)->GetType())
    {
        case Data::Type::Image:
        {
            auto pImageWidget = new ImageWidget(filePath, hduIndex, std::move(*pData), this);

            auto pSubWindow = m_pMdiArea->addSubWindow(pImageWidget);
            pSubWindow->setAttribute(Qt::WA_DeleteOnClose);
            pSubWindow->setWindowTitle(QString::fromStdString(std::format("{} - HDU {}", filePath.filename().string(), hduIndex)));
            pSubWindow->show();
        }
        break;
    }
}

void MainWindow::Slot_UI_MdiArea_SubWindowActivated(QMdiSubWindow* pMdiSubWindow)
{
    if (pMdiSubWindow == nullptr)
    {
        m_pVM->OnHDUActivated(std::nullopt);
        return;
    }

    const auto pMdiHDUWidget = dynamic_cast<const MdiHDUWidget*>(pMdiSubWindow->widget());
    m_pVM->OnHDUActivated(
        ActivatedHDU{
            .filePath = pMdiHDUWidget->GetFilePath(),
            .hduIndex = pMdiHDUWidget->GetHDUIndex()
        }
    );
}

void MainWindow::OnViewFiles()
{
    // Mark the view action disabled since the view is being opened now
    m_pViewFilesAction->setEnabled(false);

    // If the widget already exists (was previously closed), just restore/reshow it
    if (m_pFilesDockWidget)
    {
        (*m_pFilesDockWidget)->show();
        return;
    }

    // Otherwise, create the widget and its dock widget
    m_pFilesWidget = new FilesWidget(m_pVM.get(), this);
    connect(*m_pFilesWidget, &FilesWidget::Signal_OnHDUActivated, this, &MainWindow::Slot_FilesWidget_OnHDUActivated);

    auto dockWidget = new NastroDockWidget(tr("Imported Files"), this);
    dockWidget->setAllowedAreas(Qt::AllDockWidgetAreas);
    dockWidget->setWidget(*m_pFilesWidget);
    dockWidget->setMinimumWidth(200);
    connect(dockWidget, &NastroDockWidget::Signal_Closed, [this](){ m_pViewFilesAction->setEnabled(true); });

    addDockWidget(Qt::LeftDockWidgetArea, dockWidget);

    m_pFilesDockWidget = dockWidget;
}

void MainWindow::OnViewHeaders()
{
    // Mark the view action disabled since the view is being opened now
    m_pViewHeadersAction->setEnabled(false);

    // If the widget already exists (was previously closed), just restore/reshow it
    if (m_pHeadersDockWidget)
    {
        (*m_pHeadersDockWidget)->show();
        return;
    }

    // Otherwise, create the widget and its dock widget
    m_pHeadersWidget = new HeadersWidget(m_pVM.get(), this);

    auto dockWidget = new NastroDockWidget(tr("Headers"), this);
    dockWidget->setAllowedAreas(Qt::AllDockWidgetAreas);
    dockWidget->setWidget(*m_pHeadersWidget);
    dockWidget->setMinimumWidth(200);
    connect(dockWidget, &NastroDockWidget::Signal_Closed, [this](){ m_pViewHeadersAction->setEnabled(true); });

    addDockWidget(Qt::RightDockWidgetArea, dockWidget);

    m_pHeadersDockWidget = dockWidget;
}

}
