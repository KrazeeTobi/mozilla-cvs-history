#!/usr/bin/perl
#
# The contents of this file are subject to the Netscape Public
# License Version 1.1 (the "License"); you may not use this file
# except in compliance with the License. You may obtain a copy of
# the License at http://www.mozilla.org/NPL/
#
# Software distributed under the License is distributed on an "AS
# IS" basis, WITHOUT WARRANTY OF ANY KIND, either express oqr
# implied. See the License for the specific language governing
# rights and limitations under the License.
#
# The Original Code is JavaScript 2 Prototype.
#
# The Initial Developer of the Original Code is Netscape
# Communications Corporation.  Portions created by Netscape are
# Copyright (C) 1997-1999 Netscape Communications Corporation. All
# Rights Reserved.
#
# Alternatively, the contents of this file may be used under the
# terms of the GNU Public License (the "GPL"), in which case the
# provisions of the GPL are applicable instead of those above.
# If you wish to allow use of your version of this file only
# under the terms of the GPL and not to allow others to use your
# version of this file under the NPL, indicate your decision by
# deleting the provisions above and replace them with the notice
# and other provisions required by the GPL.  If you do not delete
# the provisions above, a recipient may use your version of this
# file under either the NPL or the GPL.
#
# Contributers:
#

use strict;
use jsicodes;

my $tab = "    ";

my @map = &get_map();
my $map_body = join ("\n", @map);
my $map_size = $#map + 1;
my $gen_body = &get_generator();

print <<MAP_END;
/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * The contents of this file are subject to the Netscape Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/NPL/
 *
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express oqr
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 *
 * The Original Code is the JavaScript 2 Prototype.
 *
 * The Initial Developer of the Original Code is Netscape
 * Communications Corporation.  Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation. All
 * Rights Reserved.
 *
 * Contributor(s): 
 *
 * Alternatively, the contents of this file may be used under the
 * terms of the GNU Public License (the "GPL"), in which case the
 * provisions of the GPL are applicable instead of those above.
 * If you wish to allow use of your version of this file only
 * under the terms of the GPL and not to allow others to use your
 * version of this file under the NPL, indicate your decision by
 * deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL.  If you do not delete
 * the provisions above, a recipient may use your version of this
 * file under either the NPL or the GPL.
 */

/***** THIS FILE IS AUTO GENERATED!  DO NOT EDIT THIS FILE! *****
 * make changes to js2/tools/genmap.pl and regenerate this file *
 ***** THIS FILE IS AUTO GENERATED!  DO NOT EDIT THIS FILE! *****/

#ifndef __icodemap_h

#define __icodemap_h

#include "systemtypes.h"
#include "icodeasm.h"

namespace JavaScript {
namespace ICodeASM {

    static uint icodemap_size = $map_size;

    static struct {
        char *name;
        OperandType otype[4];
    } icodemap [] = {
$map_body
    };

    VM::Instruction *InstructionFromNode (StatementNode *node)
    {
        using namespace VM;
        using namespace JSTypes;
        Instruction *i;
        
        switch (node->icodeID)
        {
$gen_body
            default:
                NOT_REACHED("Unknown icodeID");
        }

        return i;
        
    }
                
 
}
}

#endif /* #ifdef __icodemap_h */
MAP_END

sub get_map {
    my $k;
    my @map;
    for $k (sort(keys(%jsicodes::ops))) {
        my $map_entry .= $tab . $tab . "{\"$k\", {";
        my $c = $jsicodes::ops{$k};
        if ($c->{"params"}) {
            my @ot;
            my @params = @{$c->{"params"}};
            my $p;
            @ot = ();
            for $p (@params) {
                if ($p eq "ArgumentList") {
                    push (@ot, "otArgumentList");
                } elsif ($p eq "BinaryOperator::BinaryOp") {
                    push (@ot, "otBinaryOp");
                } elsif ($p eq "ICodeModule*") {
                    push (@ot, "otICodeModule");
                } elsif ($p eq "JSClass*") {
                    push (@ot, "otJSClass");
                } elsif ($p eq "JSFunction*") {
                    push (@ot, "otJSFunction");
                } elsif ($p eq "JSString*") {
                    push (@ot, "otJSString");
                } elsif ($p eq "JSType*") {
                    push (@ot, "otJSType");
                } elsif ($p eq "Label*") {
                    push (@ot, "otLabel");
                } elsif ($p eq "TypedRegister") {
                    push (@ot, "otRegister");
                } elsif ($p eq "bool") {
                    push (@ot, "otBool");
                } elsif ($p eq "const StringAtom*") {
                    push (@ot, "otStringAtom");
                } elsif ($p eq "double") {
                    push (@ot, "otDouble");
                } elsif ($p eq "uint32") {
                    push (@ot, "otUInt32");
                } else {
                    die "unknown parameter type '$p' for icode '$k'.\n";
                }
            }
            $map_entry .= join (", ", @ot);
        } else {
            $map_entry .= "otNone";
        }
        $map_entry .= "}},";
        push (@map, $map_entry);
    }

    return @map;
}

sub get_generator {
    my $k;
    my $rval = "";
    my $icode_id = 0;
    for $k (sort(keys(%jsicodes::ops))) {
        my $c = $jsicodes::ops{$k};
        my @args = ();
        if ($c->{"params"}) {
            my @params = @{$c->{"params"}};
            my $arg_num = 0;
            my $p;
            for $p (@params) {
                if ($p eq "TypedRegister") {
                    push (@args, "TypedRegister(static_cast<Register>(node->operand[$arg_num].data), 0)");
                } elsif ($p eq "ArgumentList") {
                    push (@args, "*(reinterpret_cast<ArgumentList *>(node->operand[$arg_num].data))");
                } elsif ($p =~ /\*$/) {
                    push (@args, "reinterpret_cast<$p>(node->operand[$arg_num].data)");
                } else {
                    push (@args, "static_cast<$p>(node->operand[$arg_num].data)");
                }
                $arg_num++;
            }
        }

        $rval .= $tab . $tab . $tab . "case $icode_id:\n";
        $rval .= $tab . $tab . $tab . $tab . 
          "i = new " . &jsicodes::get_classname($k) . " (" . join (", ", @args) .
            ");\n";
        $rval .= $tab . $tab . $tab . $tab . "break;\n";
        $icode_id++;
    }

    return $rval;
}

