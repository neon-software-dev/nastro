/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef SRC_UI_FILESWIDGET_H
#define SRC_UI_FILESWIDGET_H

#include "../Util/Common.h"

#include <NFITS/HDU.h>

#include <QWidget>
#include <QItemSelection>

#include <vector>
#include <memory>
#include <filesystem>

class QTreeView;
class QAction;

namespace Nastro
{
    class MainWindowVM;
    class FilesModel;
    class FilesModelSortProxy;
    class FilesTreeItem;

    class FilesWidget : public QWidget
    {
        Q_OBJECT

        public:

            explicit FilesWidget(MainWindowVM* pMainWindowVM, QWidget* pParent = nullptr);

        signals:

            void Signal_OnHDUActivated(const FileHDU& activatedHDU);
            void Signal_OnCompareImageHDUs(const std::vector<FileHDU>& compares);

        protected:

            void dragEnterEvent(QDragEnterEvent* pEvent) override;
            void dragLeaveEvent(QDragLeaveEvent* pEvent) override;
            void dropEvent(QDropEvent* pEvent) override;

            void paintEvent(QPaintEvent *pEvent) override;

        private slots:

            void Slot_OnTreeView_Activated(const QModelIndex& index);
            void Slot_OnTreeViewModel_SelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
            void Slot_Compare_ActionTriggered(bool checked);

        private:

            void InitUI();
            void BindVM();
            void InitialState();

            [[nodiscard]] std::vector<const FilesTreeItem*> GetSelectedTreeItems() const;

        private:

            MainWindowVM* m_pMainWindowVM;

            QTreeView* m_pTreeView{nullptr};
            std::unique_ptr<FilesModel> m_pTreeViewModel;
            std::unique_ptr<FilesModelSortProxy> m_pTreeViewModelSortProxy;

            QAction* m_pCompareAction{nullptr};

            bool m_inDragDrop{false};
    };
}

#endif //SRC_UI_FILESWIDGET_H
