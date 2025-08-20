/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef SRC_UI_AXISWIDGET_H
#define SRC_UI_AXISWIDGET_H

#include <QWidget>

namespace Nastro
{
    /**
     * Base class for widgets which let the user choose an image axis value
     */
    class AxisWidget : public QWidget
    {
        Q_OBJECT

        public:

            explicit AxisWidget(QWidget* pParent = nullptr)
                : QWidget(pParent)
            { }

        signals:

            void Signal_ValueChanged(int value);

    };
}

#endif //SRC_UI_AXISWIDGET_H
