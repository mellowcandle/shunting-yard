// Copyright 2011 - 2014 Brian Marshall. All rights reserved.
//
// Use of this source code is governed by the BSD 2-Clause License that can be
// found in the LICENSE file.

// Convenience functions
#define is_numeric(c) ((c >= '0' && c <= '9') || c == '.')
#define is_alpha(c) ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'))
#define is_operand(c) (is_numeric(c) || is_alpha(c))
#define is_operator(c) (c != '\0' && strchr("+-*/%=^!", c) != NULL)
