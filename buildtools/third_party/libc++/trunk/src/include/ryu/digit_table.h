//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// Copyright (c) Microsoft Corporation.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// Copyright 2018 Ulf Adams
// Copyright (c) Microsoft Corporation. All rights reserved.

// Boost Software License - Version 1.0 - August 17th, 2003

// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:

// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

#ifndef _LIBCPP_SRC_INCLUDE_RYU_DIGIT_TABLE_H
#define _LIBCPP_SRC_INCLUDE_RYU_DIGIT_TABLE_H

#include <__charconv/tables.h>
#include <__config>

_LIBCPP_BEGIN_NAMESPACE_STD

// A table of all two-digit numbers. This is used to speed up decimal digit
// generation by copying pairs of digits into the final output.
//
// In order to minimize the diff in the Ryu code between MSVC STL and libc++
// the code uses the name __DIGIT_TABLE. In order to avoid code duplication it
// reuses the table already available in libc++.
inline constexpr auto& __DIGIT_TABLE = __itoa::__digits_base_10;

_LIBCPP_END_NAMESPACE_STD

#endif // _LIBCPP_SRC_INCLUDE_RYU_DIGIT_TABLE_H
