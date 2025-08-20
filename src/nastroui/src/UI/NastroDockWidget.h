/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef SRC_UI_NASTRODOCKWIDGET_H
#define SRC_UI_NASTRODOCKWIDGET_H

#include <QDockWidget>

namespace Nastro
{
    /**
     * A QDockWidget which emits a signal when it's closed
     */
    class NastroDockWidget : public QDockWidget
    {
        Q_OBJECT

        public:

            explicit NastroDockWidget(const QString &title,
                                      QWidget *parent = nullptr,
                                      Qt::WindowFlags flags = Qt::WindowFlags())
                : QDockWidget(title, parent, flags)
            { }

            explicit NastroDockWidget(QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags())
                : QDockWidget(parent, flags)
            {}

        signals:

            void Signal_Closed();

        protected:

            void closeEvent(QCloseEvent* pEvent) override
            {
                emit Signal_Closed();

                QDockWidget::closeEvent(pEvent);
            }
    };
}

#endif //SRC_UI_NASTRODOCKWIDGET_H
