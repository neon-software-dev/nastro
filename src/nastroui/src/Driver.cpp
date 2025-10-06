/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#include "UI/MainWindow.h"

#include <QApplication>

#include <filesystem>
#include <iostream>
#include <expected>

struct ProgramArgs
{
    std::optional<std::filesystem::path> initialLaunchFile;
};

std::expected<ProgramArgs, std::string> ParseArgs(int argc, char *argv[])
{
    ProgramArgs args{};

    if (argc >= 2)
    {
        const auto initialLaunchFile = std::filesystem::path(std::string(argv[1]));
        if (!std::filesystem::is_regular_file(initialLaunchFile))
        {
            return std::unexpected("Argument must be a path to a file");
        }
        args.initialLaunchFile = initialLaunchFile;
    }

    return args;
}

int main(int argc, char *argv[])
{
    using namespace Nastro;

    QApplication app(argc, argv);

    QIcon appIcon(":/images/nastro_icon_transparent_1024.png");

    const auto args = ParseArgs(argc, argv);
    if (!args)
    {
        std::cerr << "Failed to parse program arguments: " << args.error() << std::endl;
        return 1;
    }

    auto pMainWindow = new MainWindow(args->initialLaunchFile);
    pMainWindow->setWindowTitle("nastroui");
    pMainWindow->setWindowIcon(appIcon);
    pMainWindow->show();

    return QApplication::exec();
}
