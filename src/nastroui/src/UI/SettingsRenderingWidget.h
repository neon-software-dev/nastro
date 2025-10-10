/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef NASTROUI_SRC_UI_SETTINGSRENDERINGWIDGET_H
#define NASTROUI_SRC_UI_SETTINGSRENDERINGWIDGET_H

#include <QWidget>
#include <QSettings>

class QFrame;

namespace Nastro
{
    class SettingsRenderingWidget : public QWidget
    {
        Q_OBJECT

        public:

            explicit SettingsRenderingWidget(QWidget* parent = nullptr);

        private slots:

            void Slot_OnBlankPixelColorTriggered();

        private:

            void InitUI();

            void Sync_BlankPixelColor_FromSetting();

        private:

            QSettings m_settings;

            QFrame* m_pColorSwatchFrame{nullptr};
    };
}


#endif //NASTROUI_SRC_UI_SETTINGSRENDERINGWIDGET_H
