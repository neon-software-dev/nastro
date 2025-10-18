/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef NASTROUI_SRC_UI_PIXELDETAILSWIDGET_H
#define NASTROUI_SRC_UI_PIXELDETAILSWIDGET_H

#include "../Util/Common.h"

#include <NFITS/WCS/WCSParams.h>

#include <QWidget>

#include <utility>
#include <optional>
#include <string>

class QLabel;
class QHBoxLayout;

namespace Nastro
{
    class PixelDetailsWidget : public QWidget
    {
        Q_OBJECT

        public:

            explicit PixelDetailsWidget(QWidget* pParent = nullptr);

            void DisplayPixelDetails(const std::optional<PixelDetails>& pixelDetails);

        private:

            void InitUI();

        private:

            QHBoxLayout* m_pMainLayout{nullptr};

            QLabel* m_pCoordLabel{nullptr};
            QLabel* m_pPhysicalLabel{nullptr};
    };
}

#endif //NASTROUI_SRC_UI_PIXELDETAILSWIDGET_H
