/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef SRC_UI_AXISSLIDERWIDGET_H
#define SRC_UI_AXISSLIDERWIDGET_H

#include "AxisWidget.h"

namespace Nastro
{
    /**
     * AxisWidget which provides a QSlider for choosing axis value
     */
    class AxisSliderWidget : public AxisWidget
    {
        Q_OBJECT

        public:

            AxisSliderWidget(unsigned int axis, int64_t axisn, QWidget* pParent = nullptr);

        private:

            void InitUI(unsigned int axis, int64_t axisn);
    };
}

#endif //SRC_UI_AXISSLIDERWIDGET_H
