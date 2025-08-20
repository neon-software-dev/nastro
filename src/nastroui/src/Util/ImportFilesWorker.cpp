/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#include "ImportFilesWorker.h"

#include <NFITS/DiskFITSByteSource.h>
#include <NFITS/FITSFile.h>

#include <iostream>

namespace Nastro
{

ImportFilesWorker::ImportFilesWorker(std::vector<std::filesystem::path> filePaths)
    : m_filePaths(std::move(filePaths))
{

}

void ImportFilesWorker::DoWork()
{
    for (const auto& filePath : m_filePaths)
    {
        ImportFile(filePath);

        if (IsCancelled())
        {
            emit Signal_WorkCancelled();
            return;
        }
    }

    emit Signal_WorkCompleteSuccess();
}

void ImportFilesWorker::ImportFile(const std::filesystem::path& filePath)
{
    std::cout << "Importing file: " << filePath << std::endl;
    emit Signal_StatusMsg(QString::fromStdString(std::format("{}: {}", tr("Importing file").toStdString(), filePath.filename().string())));

    std::error_code ec{};

    if (!std::filesystem::is_regular_file(filePath, ec))
    {
        std::cerr << "ImportFilesWorker: File is not a regular filesystem file: " << filePath << std::endl;
        return;
    }

    auto byteSource = NFITS::DiskFITSByteSource::Open(filePath, false);
    if (!byteSource)
    {
        std::cerr << "ImportFilesWorker: Failed to open file as byte source, error: " << byteSource.error().msg << std::endl;
        return;
    }

    auto result = NFITS::FITSFile::OpenBlocking(std::move(*byteSource));
    if (!result)
    {
        std::cerr << "ImportFilesWorker: Failed to open file as fits file, error: " << result.error().msg << std::endl;
        return;
    }

    std::vector<NFITS::HDU> hdus;
    for (std::size_t x = 0; x < (*result)->GetNumHDUs(); ++x)
    {
        hdus.push_back(*(*(*result)->GetHDU(x)));
    }

    m_importedHDUs.insert({filePath, hdus});
}

}
