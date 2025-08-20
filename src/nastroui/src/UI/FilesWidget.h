/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef SRC_UI_FILESWIDGET_H
#define SRC_UI_FILESWIDGET_H

#include <NFITS/HDU.h>

#include <QWidget>

#include <vector>
#include <memory>
#include <filesystem>

class QTreeView;

namespace Nastro
{
    class MainWindowVM;
    class FilesModel;
    class FilesModelSortProxy;

    class FilesWidget : public QWidget
    {
        Q_OBJECT

        public:

            explicit FilesWidget(MainWindowVM* pMainWindowVM, QWidget* pParent = nullptr);

        signals:

            void Signal_OnHDUActivated(const std::filesystem::path& filePath, const std::size_t& hduIndex);

        protected:

            void dragEnterEvent(QDragEnterEvent* pEvent) override;
            void dragLeaveEvent(QDragLeaveEvent* pEvent) override;
            void dropEvent(QDropEvent* pEvent) override;

            void paintEvent(QPaintEvent *pEvent) override;

        private slots:

            void Slot_OnTreeView_Activated(const QModelIndex& index);

        private:

            void InitUI();
            void BindVM();
            void InitialState();

        private:

            MainWindowVM* m_pMainWindowVM;

            QTreeView* m_pTreeView{nullptr};
            std::unique_ptr<FilesModel> m_pTreeViewModel;
            std::unique_ptr<FilesModelSortProxy> m_pTreeViewModelSortProxy;

            bool m_inDragDrop{false};
    };
}

#endif //SRC_UI_FILESWIDGET_H
