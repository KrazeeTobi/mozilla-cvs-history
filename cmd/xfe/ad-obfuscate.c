/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * The contents of this file are subject to the Netscape Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/NPL/
 *
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 *
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is Netscape
 * Communications Corporation.  Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation. All
 * Rights Reserved.
 *
 * Contributor(s): 
 */

/**********************************************************************
 ad-obfuscate.c
 By Daniel Malmer
 16 Jan 1996

 Used to generate the Enterprise Kit default resources.
 Reads from the standard input.
 Prepends each line with quotes, prints out the obfuscated line, and
 adds a quote at the end.
 The effect is to create one long character string with the octal
 version of the input, with 42 added to each character.
**********************************************************************/


#include <stdio.h>
#include <string.h>

int
main()
{
    char buf[1024];
    char* ptr;

    while ( fgets(buf, sizeof(buf), stdin) != NULL ) {
        printf("\"");
        for ( ptr = &(buf[0]); *ptr != '\0'; ptr++ ) {
            printf("\\%o", (*ptr)+42);
        }
        printf("\"\n");
    }
}


