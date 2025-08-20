/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef NFITS_INCLUDE_NFITS_SHAREDLIB_H
#define NFITS_INCLUDE_NFITS_SHAREDLIB_H

#ifdef NFITS_STATIC
    #define NFITS_PUBLIC
    #define NFITS_LOCAL
#else
    #if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
        #ifdef NFITS_DO_EXPORT
            #ifdef __GNUC__
                #define NFITS_PUBLIC __attribute__ ((dllexport))
            #else
                #define NFITS_PUBLIC __declspec(dllexport)
            #endif
        #else
            #ifdef __GNUC__
                #define NFITS_PUBLIC __attribute__ ((dllimport))
            #else
                #define NFITS_PUBLIC __declspec(dllimport)
            #endif
        #endif

        #define NFITS_LOCAL
    #else
        #if __GNUC__ >= 4
            #define NFITS_PUBLIC __attribute__ ((visibility ("default")))
            #define NFITS_LOCAL  __attribute__ ((visibility ("hidden")))
        #else
            #define NFITS_PUBLIC
            #define NFITS_LOCAL
        #endif
    #endif
#endif

#endif //NFITS_INCLUDE_NFITS_SHAREDLIB_H
