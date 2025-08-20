/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef SRC_UTIL_WORKER_H
#define SRC_UTIL_WORKER_H

#include <QObject>

#include <utility>
#include <optional>

namespace Nastro
{
    class Worker : public QObject
    {
        Q_OBJECT

        signals:

            void Signal_ChangeMax(unsigned int value);
            void Signal_Progress(unsigned int value);
            void Signal_StatusMsg(QString msg);
            void Signal_WorkCancelled();
            void Signal_WorkCompleteError();
            void Signal_WorkCompleteSuccess();

        public:

            ~Worker() override = default;

            void Cancel();
            [[nodiscard]] bool IsCancelled() const noexcept { return m_isCancelled; }

            virtual void DoWork() = 0;

            [[nodiscard]] virtual std::optional<std::pair<unsigned int, unsigned int>> GetMaxMin() const { return std::nullopt; };

        protected:

            bool m_isCancelled{false};
    };
}

#endif //SRC_UTIL_WORKER_H
