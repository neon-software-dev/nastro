/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef SRC_VM_VMUTIL_H
#define SRC_VM_VMUTIL_H

namespace Nastro
{
    template<typename T, typename SignalClass, typename Signal>
    bool UpdateAndEmit(T& target, const T& newValue, SignalClass* sender, Signal signal, bool forceEmit = false)
    {
        if ((target == newValue) && !forceEmit) { return false; }

        target = newValue;
        QMetaObject::invokeMethod(sender, signal, target);

        return true;
    }
}

#endif //SRC_VM_VMUTIL_H
