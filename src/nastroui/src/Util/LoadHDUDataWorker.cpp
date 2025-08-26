/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#include "LoadHDUDataWorker.h"

#include "NFITS/Data/DataUtil.h"

#include <NFITS/DiskFITSByteSource.h>
#include <NFITS/FITSFile.h>

#include <iostream>

namespace Nastro
{

LoadHDUDataWorker::LoadHDUDataWorker(std::filesystem::path filePath, const std::size_t& hduIndex)
    : m_filePath(std::move(filePath))
    , m_hduIndex(hduIndex)
{

}

LoadHDUDataWorker::~LoadHDUDataWorker() = default;

void LoadHDUDataWorker::DoWork()
{
    std::cout << "Loading HDU " << m_hduIndex << " data from file " << m_filePath.filename() << std::endl;

    emit Signal_StatusMsg(QString::fromStdString(std::format("Opening file: {}", m_filePath.filename().string())));

    //
    // Open the file path as a FITS byte source
    //
    auto pByteSource = NFITS::DiskFITSByteSource::Open(m_filePath, false);
    if (!pByteSource)
    {
        std::cout << "LoadHDUDataWorker::DoWork: Failed to open byte source, error: " << pByteSource.error().msg << std::endl;
        emit Signal_WorkCompleteError();
        return;
    }

    if (IsCancelled()) { emit Signal_WorkCancelled(); return; }

    //
    // Open the FITS byte source as a FITS file
    //
    emit Signal_StatusMsg(QString::fromStdString(std::format("Parsing file: {}", m_filePath.filename().string())));

    auto pFITSFile = NFITS::FITSFile::OpenBlocking(std::move(*pByteSource));
    if (!pFITSFile)
    {
        std::cout << "LoadHDUDataWorker::DoWork: Failed to open fits file, error: " << pFITSFile.error().msg << std::endl;
        emit Signal_WorkCompleteError();
        return;
    }

    if (IsCancelled()) { emit Signal_WorkCancelled(); return; }

    //
    // Load the HDU's data
    //
    emit Signal_StatusMsg(QString::fromStdString(std::format("Loading HDU data")));

    const auto pHDU = (*pFITSFile)->GetHDU(m_hduIndex);
    if (!pHDU)
    {
        std::cout << "LoadHDUDataWorker::DoWork: No such HDU index exists in file: " << m_hduIndex << std::endl;
        emit Signal_WorkCompleteError();
        return;
    }

    auto pHDUData = LoadHDUDataBlocking(pFITSFile->get(), *pHDU);
    if (!pHDUData)
    {
        std::cerr << "LoadHDUDataWorker::DoWork: Failed to load HDU data" << std::endl;
        emit Signal_WorkCompleteError();
        return;
    }

    if (IsCancelled()) { emit Signal_WorkCancelled(); return; }

    m_result = std::move(*pHDUData);

    emit Signal_WorkCompleteSuccess();
}

}
