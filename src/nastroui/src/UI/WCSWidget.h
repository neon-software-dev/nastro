/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef NASTROUI_SRC_UI_WCSWIDGET_H
#define NASTROUI_SRC_UI_WCSWIDGET_H

#include "../Util/Common.h"

#include <QWidget>

#include <vector>
#include <optional>

class QTableView;
class QStandardItemModel;

namespace Nastro
{
    class MainWindowVM;

    class WCSWidget : public QWidget
    {
        Q_OBJECT

        public:

            explicit WCSWidget(MainWindowVM* pMainWindowVM, QWidget* pParent = nullptr);

        private slots:

            void Slot_VM_OnPixelHoveredChanged(const std::optional<PixelDetails>& pixelDetails);

        private:

            void InitUI();
            void BindVM();

        private:

            MainWindowVM* m_pMainWindowVM{nullptr};

            QTableView* m_pTableView{nullptr};
            QStandardItemModel* m_pTableViewModel{nullptr};
    };
}

#endif //NASTROUI_SRC_UI_WCSWIDGET_H
