/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef NASTROUI_SRC_UI_PIXELDETAILSWIDGET_H
#define NASTROUI_SRC_UI_PIXELDETAILSWIDGET_H

#include <QWidget>

#include <utility>
#include <optional>
#include <string>

class QLabel;

namespace Nastro
{
    struct PixelDetails
    {
        std::pair<std::size_t, std::size_t> position;
        double physicalValue{0.0};
        std::optional<std::string> physicalUnit;
    };

    class PixelDetailsWidget : public QWidget
    {
        Q_OBJECT

        public:

            explicit PixelDetailsWidget(QWidget* pParent = nullptr);

            void OnPixelChanged(const std::optional<PixelDetails>& pixelDetails);

        private:

            QLabel* m_pCoordLabel;
            QLabel* m_pPhysicalLabel;
    };
}

#endif //NASTROUI_SRC_UI_PIXELDETAILSWIDGET_H
