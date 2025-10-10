/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef NASTROUI_SRC_UI_SETTINGSDIALOG_H
#define NASTROUI_SRC_UI_SETTINGSDIALOG_H

#include <QDialog>

namespace Nastro
{
    class SettingsDialog : public QDialog
    {
        Q_OBJECT

        public:

            explicit SettingsDialog(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

        private:

            void InitUI();

        private:

            QWidget* m_pSettingsRenderingWidget{nullptr};
    };
}

#endif //NASTROUI_SRC_UI_SETTINGSDIALOG_H
