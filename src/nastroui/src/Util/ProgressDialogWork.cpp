/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#include "ProgressDialogWork.h"

namespace Nastro
{

ProgressDialogWork::ProgressDialogWork(Worker* pWorker, const ProgressDialogArgs& args, QWidget* pParent)
    : m_pWorker(pWorker)
    , m_progressDialog(pParent, (Qt::WindowFlags() & (~Qt::WindowCloseButtonHint & ~Qt::WindowTitleHint)) | Qt::CustomizeWindowHint)
{
    const auto workerMaxMin = m_pWorker->GetMaxMin();
    const auto minValue = workerMaxMin ? static_cast<int>(workerMaxMin->first) : 0;
    const auto maxValue = workerMaxMin ? static_cast<int>(workerMaxMin->second) : 0;

    m_progressDialog.setWindowModality(args.isModal ? Qt::WindowModal : Qt::NonModal);
    m_progressDialog.setLabelText(tr("Working..."));
    m_progressDialog.setMinimumDuration(static_cast<int>(args.displayDelayMs));
    m_progressDialog.setMinimum(minValue);
    m_progressDialog.setMaximum(maxValue);
    m_progressDialog.setValue(0);
    if (args.canBeCancelled)
    {
        connect(&m_progressDialog, &QProgressDialog::canceled, [this]() { m_pWorker->Cancel(); });
    }
    else
    {
        m_progressDialog.setCancelButton(nullptr);
    }

    m_pWorker->moveToThread(&m_thread);
    connect(&m_thread, &QThread::started, [this](){ m_pWorker->DoWork(); });
    if (args.autoDestroyWorkerWhenFinished)
    {
        connect(&m_thread, &QThread::finished, m_pWorker, &QObject::deleteLater);
    }
    connect(m_pWorker, &Worker::Signal_ChangeMax, this, &ProgressDialogWork::Slot_Worker_ChangeMax);
    connect(m_pWorker, &Worker::Signal_Progress, this, &ProgressDialogWork::Slot_Worker_Progress);
    connect(m_pWorker, &Worker::Signal_StatusMsg, this, &ProgressDialogWork::Slot_Worker_StatusMsg);
    connect(m_pWorker, &Worker::Signal_WorkCancelled, this, &ProgressDialogWork::Slot_Worker_WorkCancelled);
    connect(m_pWorker, &Worker::Signal_WorkCompleteError, this, &ProgressDialogWork::Slot_Worker_WorkCompleteError);
    connect(m_pWorker, &Worker::Signal_WorkCompleteSuccess, this, &ProgressDialogWork::Slot_Worker_WorkCompleteSuccess);
    m_thread.start();
}

ProgressDialogWork::~ProgressDialogWork()
{
    m_thread.quit();
    m_thread.wait();
}

void ProgressDialogWork::Slot_Worker_ChangeMax(unsigned int value)
{
    m_progressDialog.setMaximum(static_cast<int>(value));
}

void ProgressDialogWork::Slot_Worker_Progress(unsigned int value)
{
    m_progressDialog.setValue(static_cast<int>(value));
}

void ProgressDialogWork::Slot_Worker_StatusMsg(QString msg)
{
    m_progressDialog.setLabelText(msg);
}

void ProgressDialogWork::Slot_Worker_WorkCancelled()
{
    emit Signal_WorkCancelled(m_pWorker);
    emit Signal_WorkFinished(m_pWorker);

    deleteLater();
}

void ProgressDialogWork::Slot_Worker_WorkCompleteError()
{
    if (m_pWorker->IsCancelled())
    {
        emit Signal_WorkCancelled(m_pWorker);
    }
    else
    {
        emit Signal_WorkCompleteError(m_pWorker);
    }

    emit Signal_WorkFinished(m_pWorker);

    deleteLater();
}

void ProgressDialogWork::Slot_Worker_WorkCompleteSuccess()
{
    if (m_pWorker->IsCancelled())
    {
        emit Signal_WorkCancelled(m_pWorker);
    }
    else
    {
        emit Signal_WorkCompleteSuccess(m_pWorker);
    }

    emit Signal_WorkFinished(m_pWorker);

    deleteLater();
}

}
