/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#include "LoadHDUDataWorker.h"

#include <NFITS/DiskFITSByteSource.h>
#include <NFITS/FITSFile.h>
#include <NFITS/Data/DataUtil.h>

#include <iostream>

namespace Nastro
{

LoadHDUDataWorker::LoadHDUDataWorker(std::vector<FileHDU> hdus)
    : m_hdus(std::move(hdus))
{

}

LoadHDUDataWorker::~LoadHDUDataWorker() = default;

void LoadHDUDataWorker::DoWork()
{
    std::vector<std::unique_ptr<NFITS::Data>> result;

    for (const auto& hdu : m_hdus)
    {
        auto data = LoadHDU(hdu);
        if (!data)
        {
            return;
        }

        result.push_back(std::move(*data));
    }

    m_result = std::move(result);
    emit Signal_WorkCompleteSuccess();
}

std::expected<std::unique_ptr<NFITS::Data>, bool> LoadHDUDataWorker::LoadHDU(const FileHDU& hdu)
{
    std::cout << "Loading HDU " << hdu.hduIndex << " data from file " << hdu.filePath.filename() << std::endl;
    emit Signal_StatusMsg(QString::fromStdString(std::format("Opening file: {}", hdu.filePath.filename().string())));

    //
    // Open the file path as a FITS byte source
    //
    auto pByteSource = NFITS::DiskFITSByteSource::Open(hdu.filePath, false);
    if (!pByteSource)
    {
        std::cout << "LoadHDUDataWorker::DoWork: Failed to open byte source, error: " << pByteSource.error().msg << std::endl;
        emit Signal_WorkCompleteError();
        return std::unexpected(false);
    }
    if (IsCancelled())
    {
        emit Signal_WorkCancelled();
        return std::unexpected(false);
    }

    //
    // Open the FITS byte source as a FITS file
    //
    emit Signal_StatusMsg(QString::fromStdString(std::format("Parsing file: {}", hdu.filePath.filename().string())));

    auto pFITSFile = NFITS::FITSFile::OpenBlocking(std::move(*pByteSource));
    if (!pFITSFile)
    {
        std::cout << "LoadHDUDataWorker::DoWork: Failed to open fits file, error: " << pFITSFile.error().msg << std::endl;
        emit Signal_WorkCompleteError();
        return std::unexpected(false);
    }
    if (IsCancelled())
    {
        emit Signal_WorkCancelled();
        return std::unexpected(false);
    }

    //
    // Load the HDU's data
    //
    emit Signal_StatusMsg(QString::fromStdString(std::format("Loading HDU data")));

    const auto pHDU = (*pFITSFile)->GetHDU(hdu.hduIndex);
    if (!pHDU)
    {
        std::cout << "LoadHDUDataWorker::DoWork: No such HDU index exists in file: " << hdu.hduIndex << std::endl;
        emit Signal_WorkCompleteError();
        std::unexpected(false);
    }

    auto pHDUData = LoadHDUDataBlocking(pFITSFile->get(), *pHDU);
    if (!pHDUData)
    {
        std::cerr << "LoadHDUDataWorker::DoWork: Failed to load HDU data: " << pHDUData.error().msg << std::endl;
        emit Signal_WorkCompleteError();
        return std::unexpected(false);
    }
    if (IsCancelled())
    {
        emit Signal_WorkCancelled();
        return std::unexpected(false);
    }

    return std::move(*pHDUData);
}

}
