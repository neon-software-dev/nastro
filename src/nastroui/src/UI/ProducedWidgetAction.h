/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef NASTROUI_SRC_UI_PRODUCEDWIDGETACTION_H
#define NASTROUI_SRC_UI_PRODUCEDWIDGETACTION_H

#include <QWidgetAction>

#include <functional>

namespace Nastro
{
    /**
     * @param pOwner The QObject which created/owns the FactoryWidgetAction
     * @param pParent The QWidget which the QWidgetAction is inserting a QWidget into
     */
    using WidgetProducer = std::function<QWidget*(QObject* pOwner,QWidget* pParent)>;

    /**
     * Custom QWidgetAction which creates QWidgets by invoking a provided producer function
     */
    class ProducedWidgetAction : public QWidgetAction
    {
        Q_OBJECT

        public:

            ProducedWidgetAction(QObject* pOwner, WidgetProducer producer);

        protected:

            [[nodiscard]] QWidget* createWidget(QWidget *parent) override;

        private:

            QObject* m_pOwner;
            WidgetProducer m_producer;
    };
}

#endif //NASTROUI_SRC_UI_PRODUCEDWIDGETACTION_H
