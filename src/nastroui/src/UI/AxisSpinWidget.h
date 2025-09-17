/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef SRC_UI_AXISSPINWIDGET_H
#define SRC_UI_AXISSPINWIDGET_H

#include "AxisWidget.h"

namespace Nastro
{
    /**
     * AxisWidget which provides a QSpinBox for choosing axis value
     */
    class AxisSpinWidget : public AxisWidget
    {
        Q_OBJECT

        public:

            AxisSpinWidget(unsigned int axis, uint64_t axisn, QWidget* pParent = nullptr);

        private:

            void InitUI(unsigned int axis, uint64_t axisn);
    };
}

#endif //SRC_UI_AXISSPINWIDGET_H
