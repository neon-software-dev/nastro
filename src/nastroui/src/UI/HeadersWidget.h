/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef SRC_UI_HEADERSWIDGET_H
#define SRC_UI_HEADERSWIDGET_H

#include "../Util/Common.h"

#include <NFITS/HDU.h>

#include <QWidget>

class QTableView;
class QStandardItemModel;

namespace Nastro
{
    class MainWindowVM;

    class HeadersWidget : public QWidget
    {
        Q_OBJECT

        public:

            explicit HeadersWidget(MainWindowVM* pMainWindowVM, QWidget* pParent = nullptr);

        private slots:

            void Slot_VM_OnActivatedHDUChanged(const std::optional<FileHDU>& activatedHDU);

        private:

            void InitUI();
            void BindVM();
            void InitialState();

            void DisplayHDU(const NFITS::HDU& hdu);

        private:

            MainWindowVM* m_pMainWindowVM;

            QTableView* m_pTableView{nullptr};
            QStandardItemModel* m_pTableViewModel{nullptr};
    };
}

#endif //SRC_UI_HEADERSWIDGET_H
