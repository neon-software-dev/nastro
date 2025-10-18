/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef SRC_UI_MAINWINDOW_H
#define SRC_UI_MAINWINDOW_H

#include "ImageWidget.h"
#include "PixelDetailsWidget.h"

#include "../Util/Common.h"

#include <NFITS/HDU.h>

#include <QMainWindow>

#include <filesystem>
#include <optional>
#include <vector>
#include <unordered_map>

class QDockWidget;
class QVBoxLayout;
class QMdiArea;
class QMdiSubWindow;

namespace Nastro
{
    class MainWindowVM;
    class FilesWidget;
    class HeadersWidget;
    class WCSWidget;
    class Worker;
    class NastroDockWidget;

    class MainWindow : public QMainWindow
    {
        Q_OBJECT

        public:

            explicit MainWindow(std::optional<std::filesystem::path> initialLaunchPath = std::nullopt);

        private slots:

            void Slot_VM_FilesImported(const std::unordered_map<std::filesystem::path, std::vector<NFITS::HDU>>& files);

            void Slot_File_ImportFiles_ActionTriggered();
            void Slot_File_ImportDirectory_ActionTriggered();
            void Slot_File_Settings_ActionTriggered();
            void Slot_File_Exit_ActionTriggered();

            void Slot_FilesWidget_OnHDUActivated(const Nastro::FileHDU& activatedHDU);
            void Slot_OnCompareImageHDUs(const std::vector<Nastro::FileHDU>& compares);

            void Slot_OpenHDU_LoadHDUData_Complete(Nastro::Worker* pWorker);
            void Slot_CompareHDUs_LoadHDUData_Complete(Nastro::Worker* pWorker);

            void Slot_UI_MdiArea_SubWindowActivated(QMdiSubWindow* pMdiSubWindow);

        private:

            void InitUI();
            void InitMenuBar();
            void InitWidgets();

            void BindVM();

            void OnViewFiles();
            void OnViewHeaders();
            void OnViewWCS();

            void LoadAndDisplayHDU(const FileHDU& fileHDU);

        private:

            std::optional<std::filesystem::path> m_initialLaunchPath;
            bool m_initialWindowOpened{false};

            std::unique_ptr<MainWindowVM> m_pVM;

            QAction* m_pViewFilesAction{nullptr};
            QAction* m_pViewHeadersAction{nullptr};
            QAction* m_pViewWCSAction{nullptr};

            QMdiArea* m_pMdiArea{nullptr};

            std::optional<FilesWidget*> m_pFilesWidget{nullptr};
            std::optional<NastroDockWidget*> m_pFilesDockWidget;

            std::optional<HeadersWidget*> m_pHeadersWidget;
            std::optional<NastroDockWidget*> m_pHeadersDockWidget;

            std::optional<WCSWidget*> m_pWCSWidget;
            std::optional<NastroDockWidget*> m_pWCSDockWidget;
    };
}

#endif //SRC_UI_MAINWINDOW_H
