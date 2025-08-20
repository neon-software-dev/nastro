/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef SRC_UTIL_PROGRESSDIALOGWORK_H
#define SRC_UTIL_PROGRESSDIALOGWORK_H

#include "Worker.h"

#include <QThread>
#include <QProgressDialog>

namespace Nastro
{
    struct ProgressDialogArgs
    {
        bool isModal{true};
        bool canBeCancelled{true};
        unsigned int displayDelayMs{500};
        bool autoDestroyWorkerWhenFinished{true};
    };

    class ProgressDialogWork : public QObject
    {
        Q_OBJECT

        public:

            ProgressDialogWork(Worker* pWorker, const ProgressDialogArgs& args, QWidget* pParent = nullptr);
            ~ProgressDialogWork() override;

        signals:

            /** Emitted when the work has finished running, whether it was successful or not, cancelled or not; always called at end */
            void Signal_WorkFinished(Worker* pWorker);

            /** Emitted when the work has completed with an error, followed by a Signal_WorkFinished */
            void Signal_WorkCompleteError(Worker* pWorker);

            /** Emitted when the work has completed with no error, followed by a Signal_WorkFinished */
            void Signal_WorkCompleteSuccess(Worker* pWorker);

            /** Emitted when the work has stopped due to cancellation, followed by a Signal_WorkFinished */
            void Signal_WorkCancelled(Worker* pWorker);

        private slots:

            void Slot_Worker_ChangeMax(unsigned int value);
            void Slot_Worker_Progress(unsigned int value);
            void Slot_Worker_StatusMsg(QString msg);
            void Slot_Worker_WorkCancelled();
            void Slot_Worker_WorkCompleteError();
            void Slot_Worker_WorkCompleteSuccess();

        private:

            Worker* m_pWorker;

            QThread m_thread;
            QProgressDialog m_progressDialog;
    };
}

#endif //SRC_UTIL_PROGRESSDIALOGWORK_H
