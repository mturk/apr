/* ====================================================================
 * The Apache Software License, Version 1.1
 *
 * Copyright (c) 2000 The Apache Software Foundation.  All rights
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. The end-user documentation included with the redistribution,
 *    if any, must include the following acknowledgment:
 *       "This product includes software developed by the
 *        Apache Software Foundation (http://www.apache.org/)."
 *    Alternately, this acknowledgment may appear in the software itself,
 *    if and wherever such third-party acknowledgments normally appear.
 *
 * 4. The names "Apache" and "Apache Software Foundation" must
 *    not be used to endorse or promote products derived from this
 *    software without prior written permission. For written
 *    permission, please contact apache@apache.org.
 *
 * 5. Products derived from this software may not be called "Apache",
 *    nor may "Apache" appear in their name, without prior written
 *    permission of the Apache Software Foundation.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE APACHE SOFTWARE FOUNDATION OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * ====================================================================
 *
 * This software consists of voluntary contributions made by many
 * individuals on behalf of the Apache Software Foundation.  For more
 * information on the Apache Software Foundation, please see
 * <http://www.apache.org/>.
 */

#ifndef MISC_H
#define MISC_H

#include "apr.h"
#include "apr_private.h"
#include "apr_general.h"
#include "apr_pools.h"
#include "apr_getopt.h"
#include "apr_thread_proc.h"
#include "apr_file_io.h"
#include "apr_errno.h"
#include "apr_getopt.h"
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#if APR_HAVE_STDIO_H
#include <stdio.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#if APR_HAVE_SIGNAL_H
#include <signal.h>
#endif
#if APR_HAVE_PTHREAD_H
#include <pthread.h>
#endif
#ifdef BEOS
#include <kernel/OS.h>
#endif
 
typedef struct datastruct {
    const void *data;
    const char *key;
    struct datastruct *next;
    struct datastruct *prev;
} datastruct;

struct apr_other_child_rec_t {
    struct apr_other_child_rec_t *next;
    int id;  /* This is either a pid or tid depending on the platform */
    void (*maintenance) (int, void *, int);
    void *data;
    int write_fd;
};

#ifdef WIN32
#define WSAHighByte 2
#define WSALowByte 0
/* Platform specific designation of run time os version.
 * Gaps allow for specific service pack levels that
 * export new kernel or winsock functions or behavior.
 */
typedef enum {
        APR_WIN_95 = 0,
        APR_WIN_98 = 4,
        APR_WIN_NT = 8,
        APR_WIN_NT_4 = 12,
        APR_WIN_NT_4_SP2 = 14,
        APR_WIN_NT_4_SP3 = 15,
        APR_WIN_NT_4_SP4 = 16,
        APR_WIN_NT_4_SP6 = 18,
        APR_WIN_2000 = 24
} apr_oslevel_e;


typedef enum {
    DLL_WINBASEAPI = 0,    // kernel32 From WinBase.h
    DLL_WINADVAPI = 1,     // advapi32 From WinBase.h
    DLL_WINSOCKAPI = 2,    // mswsock  From WinSock.h
    DLL_WINSOCK2API = 3,   // ws2_32   From WinSock2.h
    DLL_defined = 4        // must define as last idx_ + 1
} apr_dlltoken_e;

FARPROC apr_load_dll_func(apr_dlltoken_e fnLib, char *fnName, int ordinal);

/* The apr_load_dll_func call WILL fault if the function cannot be loaded */

#define APR_DECLARE_LATE_DLL_FUNC(lib, rettype, calltype, fn, ord, args, names) \
    typedef rettype (calltype *apr_winapi_fpt_##fn) args; \
    static apr_winapi_fpt_##fn apr_winapi_pfn_##fn = NULL; \
    __inline rettype apr_winapi_##fn args \
    {   if (!apr_winapi_pfn_##fn) \
            apr_winapi_pfn_##fn = (apr_winapi_fpt_##fn) \
                                      apr_load_dll_func(lib, #fn, ord); \
        return (*(apr_winapi_pfn_##fn)) names; }; \

/* Provide late bound declarations of every API function missing from
 * one or more supported releases of the Win32 API
 *
 * lib is the enumerated token from apr_dlltoken_e, and must correspond
 * to the string table entry in start.c used by the apr_load_dll_func().
 * Token names (attempt to) follow Windows.h declarations prefixed by DLL_
 * in order to facilitate comparison.  Use the exact declaration syntax
 * and names from Windows.h to prevent ambigutity and bugs.
 *
 * rettype and calltype follow the original declaration in Windows.h
 * fn is the true function name - beware Ansi/Unicode #defined macros
 * ord is the ordinal within the library, use 0 if it varies between versions
 * args is the parameter list following the original declaration, in parens
 * names is the parameter list sans data types, enclosed in parens
 *
 * #undef/re#define the Ansi/Unicode generic name to abate confusion
 * In the case of non-text functions, simply #define the original name
 */

APR_DECLARE_LATE_DLL_FUNC(DLL_WINBASEAPI, BOOL, WINAPI, GetFileAttributesExA, 0, (
    IN LPCSTR lpFileName,
    IN GET_FILEEX_INFO_LEVELS fInfoLevelId,
    OUT LPVOID lpFileInformation),
    (lpFileName, fInfoLevelId, lpFileInformation));
#undef GetFileAttributesEx
#define GetFileAttributesEx apr_winapi_GetFileAttributesExA

APR_DECLARE_LATE_DLL_FUNC(DLL_WINBASEAPI, BOOL, WINAPI, CancelIo, 0, (
    IN HANDLE hFile),
    (hFile));
#define CancelIo apr_winapi_CancelIo

apr_status_t apr_get_oslevel(struct apr_pool_t *, apr_oslevel_e *);
#endif /* WIN32 */

#endif  /* ! MISC_H */

