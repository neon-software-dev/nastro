/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#include "UI/MainWindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    using namespace Nastro;

    QApplication app(argc, argv);

    QIcon appIcon(":/images/nastro_icon_transparent_1024.png");

    auto pMainWindow = new MainWindow();
    pMainWindow->setWindowTitle("nastroui");
    pMainWindow->setWindowIcon(appIcon);
    pMainWindow->show();

    return QApplication::exec();
}
