/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#include "MainWindowVM.h"
#include "VMUtil.h"

#include "../Util/ProgressDialogWork.h"
#include "../Util/ImportFilesWorker.h"

#include <QWidget>

#include <iostream>

namespace Nastro
{

MainWindowVM::MainWindowVM(QWidget* pParent)
    : QObject(pParent)
    , m_pParent(pParent)
{

}

std::optional<NFITS::HDU> MainWindowVM::GetImportedFileHDU(const std::filesystem::path& filePath, uintmax_t hduIndex) const
{
    const auto it = m_importedFiles.find(filePath);
    if (it == m_importedFiles.cend())
    {
        std::cerr << "GetImportedFileHDU: File isn't imported: " << filePath << std::endl;
        return std::nullopt;
    }

    if (hduIndex >= it->second.size())
    {
        std::cerr << "GetImportedFileHDU: File doesn't have HDU index: " << hduIndex << std::endl;
        return std::nullopt;
    }

    return it->second.at(hduIndex);
}

void MainWindowVM::OnImportDirectory(const std::filesystem::path& directoryPath)
{
    std::cout << "Importing directory: " << directoryPath << std::endl;

    std::error_code ec{};

    if (!std::filesystem::is_directory(directoryPath, ec))
    {
        std::cerr << "OnImportDirectory: is_directory check failed for: : " << directoryPath << ", error: " << ec.message() << std::endl;
        return;
    }

    std::vector<std::filesystem::path> filePaths;

    for (const auto& dirEntry: std::filesystem::directory_iterator(directoryPath, ec))
    {
        if (ec)
        {
            std::cerr << "DoImportDirectory: directory_iterator failed for: : " << directoryPath << ", error: " << ec.message() << std::endl;
            return;
        }

        if (VALID_FITS_EXTENSIONS.contains(dirEntry.path().extension().string()))
        {
            filePaths.push_back(dirEntry.path());
        }
    }

    return OnImportFiles(filePaths);
}

void MainWindowVM::OnImportFiles(const std::vector<std::filesystem::path>& filePaths)
{
    std::error_code ec{};

    if (filePaths.empty())
    {
        return;
    }

    auto pImportFilesWorker = new ImportFilesWorker(filePaths);

    auto pProgressDialog = new ProgressDialogWork(pImportFilesWorker, ProgressDialogArgs{.isModal = true, .canBeCancelled = true}, m_pParent);
    connect(pProgressDialog, &ProgressDialogWork::Signal_WorkFinished, this, &MainWindowVM::Slot_ImportFiles_WorkFinished);
}

void MainWindowVM::OnHDUActivated(const std::optional<FileHDU>& activatedHDU)
{
    UpdateAndEmit(m_activatedHDU, activatedHDU, this, &MainWindowVM::Signal_OnActivatedHDUChanged);
}

void MainWindowVM::OnPixelHovered(const std::optional<PixelDetails>& pixelDetails)
{
    m_hoveredPixelDetails = pixelDetails;
    emit Signal_OnPixelHoveredChanged(m_hoveredPixelDetails);
}

void MainWindowVM::Slot_ImportFiles_WorkFinished(Worker* pWorker)
{
    const auto& importedFiles = dynamic_cast<ImportFilesWorker*>(pWorker)->GetResult();

    std::unordered_map<std::filesystem::path, std::vector<NFITS::HDU>> newlyImported;

    for (const auto& fileIt : importedFiles)
    {
        if (m_importedFiles.contains(fileIt.first))
        {
            continue;
        }

        m_importedFiles.insert({fileIt.first, fileIt.second});
        newlyImported.insert({fileIt.first, fileIt.second});
    }

    emit Signal_FilesImported(newlyImported);
}

}
