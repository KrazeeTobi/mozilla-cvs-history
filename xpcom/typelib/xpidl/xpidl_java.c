/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * The contents of this file are subject to the Mozilla Public License 
 * Version 1.0 (the "License"); you may not use this file except
 * in compliance with the License. You may obtain a copy of the License at 
 * http://www.mozilla.org/MPL/ 
 *
 * Software distributed under the License is distributed on an "AS IS" basis, 
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for 
 * the specific language governing rights and limitations under the License.
 *
 * Contributors:
 *    Michael Allen (michael.allen@sun.com)
 *    Frank Mitchell (frank.mitchell@sun.com)
 */

/*
 * Generate Java interfaces from XPIDL.
 */

#include "xpidl.h"
#include <ctype.h>
#include <glib.h>


struct java_priv_data {
    GHashTable *typedefTable;
};

#define TYPEDEFS(state)     (((struct java_priv_data *)state->priv)->typedefTable)

static gboolean
write_classname_iid_define(FILE *file, const char *className)
{
    const char *iidName;
    if (className[0] == 'n' && className[1] == 's') {
        /* backcompat naming styles */
        fputs("NS_", file);
        iidName = className + 2;
    } else {
        iidName = className;
    }

    while (*iidName) {
        fputc(toupper(*iidName++), file);
    }

    fputs("_IID", file);
    return TRUE;
}

static gboolean 
initial_pass(TreeState *state)
{
    if (state->tree) {
        state->priv = calloc(1, sizeof(struct java_priv_data));
        if (!state->priv)
            return FALSE;
        TYPEDEFS(state) = 0;
        TYPEDEFS(state) = g_hash_table_new(g_str_hash, g_str_equal);
        if (!TYPEDEFS(state)) {
            /* XXX report error */
            free(state->priv);
            return FALSE;
        }

        /*
         * First pass
         */

        fputs("/*\n * ************* DO NOT EDIT THIS FILE ***********\n",
              state->file);

        fprintf(state->file, 
                " *\n * This file was automatically generated from %s.idl.\n", 
                state->basename);

        fputs(" */\n\n", state->file);
	
    } else {

        /* points to other elements of the tree, so just destroy the table */
        g_hash_table_destroy(TYPEDEFS(state));
        free(state->priv);
        state->priv = NULL;

        /*
         * Last pass
         */

        fprintf(state->file, "\n/*\n * end\n */\n");
    }

    return TRUE;
}

static gboolean
forward_declaration(TreeState *state) 
{
    /*
     * Java doesn't need forward declarations unless the declared 
     * class resides in a different package.
     */
#if 0
    IDL_tree iface = state->tree;
    const char *className = IDL_IDENT(IDL_FORWARD_DCL(iface).ident).str;
    const char *pkgName = "org.mozilla.xpcom";
    if (!className)
        return FALSE;
    /* XXX: Get package name and compare */
    fprintf(state->file, "import %s.%s;\n", pkgName, className);
#endif
    return TRUE;
}


static gboolean
interface_declaration(TreeState *state) 
{
    IDL_tree interface = state->tree;
    IDL_tree iterator = NULL;
    char *interface_name = IDL_IDENT(IDL_INTERFACE(interface).ident).str;
    const char *iid = NULL;

    /*
     * Write out JavaDoc comment
     */

    fprintf(state->file, "\n/**\n * Interface %s\n", interface_name);

#ifndef LIBIDL_MAJOR_VERSION
    iid = IDL_tree_property_get(interface, "uuid");
#else
    iid = IDL_tree_property_get(IDL_INTERFACE(interface).ident, "uuid");
#endif

    if (iid != NULL) {
        fprintf(state->file, " *\n * IID: 0x%s\n */\n\n", iid);
    } else {
        fputs(" */\n\n", state->file);
    }


    /*
     * Write "public interface <foo>"
     */

    fprintf(state->file, "public interface %s ", interface_name);

    /*
     * Check for inheritence, and iterator over the inherited names,
     * if any.
     */

    if ((iterator = IDL_INTERFACE(interface).inheritance_spec)) {
        fputs("extends ", state->file);

        do {

            fprintf(state->file, "%s", 
                    IDL_IDENT(IDL_LIST(iterator).data).str);
	    
            if (IDL_LIST(iterator).next) {
                fputs(", ", state->file);
            }
        } while ((iterator = IDL_LIST(iterator).next));

    }

    fputs("\n{\n", state->file);
    
    if (iid) {
        /*
         * Write interface constants for IID
         */

        fputs("    public static final String ", state->file);

        /* XXX s.b just "IID" ? */
        if (!write_classname_iid_define(state->file, interface_name)) {
            return FALSE;
        }

        fprintf(state->file, "_STRING =\n        \"%s\";\n\n", iid);

        fputs("    public static final nsID ", state->file);

        /* XXX s.b just "IID" ? */
        if (!write_classname_iid_define(state->file, interface_name)) {
            return FALSE;
        }

        fprintf(state->file, " =\n        new nsID(\"%s\");\n\n", iid);
    }

    /*
     * Advance the state of the tree, go on to process more
     */
    
    state->tree = IDL_INTERFACE(interface).body;

    if (state->tree && !xpidl_process_node(state)) {
        return FALSE;
    }


    fputs("\n}\n", state->file);

    return TRUE;
}

static gboolean
process_list(TreeState *state)
{
    IDL_tree iter;
    for (iter = state->tree; iter; iter = IDL_LIST(iter).next) {
        state->tree = IDL_LIST(iter).data;
        if (!xpidl_process_node(state))
            return FALSE;
    }
    return TRUE;
}

static gboolean 
xpcom_to_java_type (TreeState *state) 
{
    if (!state->tree) {
        fputs("Object", state->file);
        return TRUE;
    }

    switch(IDL_NODE_TYPE(state->tree)) {

    case IDLN_TYPE_INTEGER: {

        switch(IDL_TYPE_INTEGER(state->tree).f_type) {

        case IDL_INTEGER_TYPE_SHORT:
            fputs("short", state->file);
            break;

        case IDL_INTEGER_TYPE_LONG:
            fputs("int", state->file);
            break;

        case IDL_INTEGER_TYPE_LONGLONG:
            fputs("long", state->file);
            break;
	    
        default:
            g_error("   Unknown integer type: %d\n",
                    IDL_TYPE_INTEGER(state->tree).f_type);
            return FALSE;

        }

        break;
    }

    case IDLN_TYPE_CHAR:
    case IDLN_TYPE_WIDE_CHAR:
        fputs("char", state->file);
        break;

    case IDLN_TYPE_WIDE_STRING:
    case IDLN_TYPE_STRING:
        fputs("String", state->file);
        break;

    case IDLN_TYPE_BOOLEAN:
        fputs("boolean", state->file);
        break;

    case IDLN_TYPE_OCTET:
        fputs("byte", state->file);
        break;

    case IDLN_TYPE_FLOAT:
        switch(IDL_TYPE_FLOAT(state->tree).f_type) {

        case IDL_FLOAT_TYPE_FLOAT:
            fputs("float", state->file);
            break;

        case IDL_FLOAT_TYPE_DOUBLE:
            fputs("double", state->file);
            break;
	    
        default:
            g_error("    Unknown floating point typ: %d\n",
                    IDL_NODE_TYPE(state->tree));
            break;
        }
        break;


    case IDLN_IDENT:
        if (IDL_NODE_UP(state->tree) &&
            IDL_NODE_TYPE(IDL_NODE_UP(state->tree)) == IDLN_NATIVE) {
            const char *user_type = IDL_NATIVE(IDL_NODE_UP(state->tree)).user_type;
            if (strcmp(user_type, "void") == 0) {
                fputs("Object", state->file);
            }
            else if (strcmp(user_type, "nsID") == 0 ||
                     strcmp(user_type, "nsIID") == 0 ||
                     strcmp(user_type, "nsCID") == 0) {
                /* XXX: s.b test for "iid" attribute */
                /* XXX: special class for nsIDs */
                fputs("nsID", state->file);
            }
            else {
                /* XXX: special class for opaque types */
                fputs("OpaqueValue", state->file);
            }
        } else {
            const char *ident_str = IDL_IDENT(state->tree).str;

            /* XXX: big kludge; s.b. way to match to typedefs */
            if (strcmp(ident_str, "PRInt8") == 0 ||
                strcmp(ident_str, "PRUint8") == 0) {
                fputs("byte", state->file);
            }
            else if (strcmp(ident_str, "PRInt16") == 0 ||
                     strcmp(ident_str, "PRUint16") == 0) {
                fputs("short", state->file);
            }
            else if (strcmp(ident_str, "PRInt32") == 0 ||
                     strcmp(ident_str, "PRUint32") == 0) {
                fputs("int", state->file);
            }
            else if (strcmp(ident_str, "PRInt64") == 0 ||
                     strcmp(ident_str, "PRUint64") == 0) {
                fputs("long", state->file);
            }
            else if (strcmp(ident_str, "nsrefcnt") == 0) {
                fputs("int", state->file);
            }
            else {
                IDL_tree real_type = 
                    g_hash_table_lookup(TYPEDEFS(state), ident_str);

                if (real_type) {
                    IDL_tree orig_tree = state->tree;

                    state->tree = real_type;
                    xpcom_to_java_type(state);

                    state->tree = orig_tree;
                }
                else {
                    fputs(ident_str, state->file);
                }
            }
        }

        break;

    case IDLN_TYPE_ENUM:
    case IDLN_TYPE_OBJECT:
    default:
        g_error("    Unknown type: %d\n",
                IDL_TYPE_FLOAT(state->tree).f_type);
        break;
    }

    return TRUE;

}

static gboolean
xpcom_to_java_param(TreeState *state) 
{
    IDL_tree param = state->tree;
    state->tree = IDL_PARAM_DCL(param).param_type_spec;

    /*
     * Put in type of parameter
     */

    if (!xpcom_to_java_type(state)) {
        return FALSE;
    }

    /*
     * If the parameter is out or inout, make it a Java array of the
     * appropriate type
     */

    if (IDL_PARAM_DCL(param).attr != IDL_PARAM_IN) {
        fputs("[]", state->file);
    }

    /*
     * Put in name of parameter 
     */

    fputc(' ', state->file);

    fputs(IDL_IDENT(IDL_PARAM_DCL(param).simple_declarator).str, state->file);

    return TRUE;
}


static gboolean
type_declaration(TreeState *state) 
{
    /*
     * Unlike C, Java has no type declaration directive.
     * Instead, we record the mapping, and look up the actual type
     * when needed.
     */
    IDL_tree type = IDL_TYPE_DCL(state->tree).type_spec;
    IDL_tree dcls = IDL_TYPE_DCL(state->tree).dcls;

    /* XXX: check for illegal types */

    g_hash_table_insert(TYPEDEFS(state),
                        IDL_IDENT(IDL_LIST(dcls).data).str,
                        type);

    return TRUE;
}

static gboolean
method_declaration(TreeState *state) 
{
    /* IDL_tree method_tree = state->tree; */
    struct _IDL_OP_DCL *method = &IDL_OP_DCL(state->tree);
    gboolean method_notxpcom = 
        (IDL_tree_property_get(method->ident, "notxpcom") != NULL);
    gboolean method_noscript = 
        (IDL_tree_property_get(method->ident, "noscript") != NULL);
    IDL_tree iterator = NULL;
    IDL_tree retval_param = NULL;

    if (!verify_method_declaration(state))
        return FALSE;

    xpidl_write_comment(state, 4);

    /*
     * Write beginning of method declaration
     */
    fputs("    ", state->file);
    if (!method_noscript) {
        /* Nonscriptable methods become package-protected */
        fputs("public ", state->file);
    }

    /*
     * Write return type
     * Unlike C++ headers, Java interfaces return the declared 
     * return value; an exception indicates XPCOM method failure.
     */
    if (method_notxpcom || method->op_type_spec) {
        state->tree = method->op_type_spec;
        if (!xpcom_to_java_type(state)) {
            return FALSE;
        }
    } else {
        /* Check for retval attribute */
        for (iterator = method->parameter_dcls; iterator != NULL; 
             iterator = IDL_LIST(iterator).next) {

            IDL_tree original_tree = state->tree;

            state->tree = IDL_LIST(iterator).data;

            if (IDL_tree_property_get(IDL_PARAM_DCL(state->tree).simple_declarator, 
                                      "retval")) {
                retval_param = iterator;

                state->tree = IDL_PARAM_DCL(state->tree).param_type_spec;

                /*
                 * Put in type of parameter
                 */

                if (!xpcom_to_java_type(state)) {
                    return FALSE;
                }
            }

            state->tree = original_tree;
        }

        if (retval_param == NULL) {
            fputs("void", state->file);
        }
    }
 
    /*
     * Write method name
     */

    fprintf(state->file, " %s(", IDL_IDENT(method->ident).str);

    /*
     * Write parameters
     */
    for (iterator = method->parameter_dcls; iterator != NULL; 
         iterator = IDL_LIST(iterator).next) {

        /* Skip "retval" */
        if (iterator == retval_param) {
            continue;
        }

        if (iterator != method->parameter_dcls) {
            fputs(", ", state->file);
        }
        
        state->tree = IDL_LIST(iterator).data;

        if (!xpcom_to_java_param(state)) {
            return FALSE;
        }
    }

    fputs(")", state->file);

    if (method->raises_expr) {
        IDL_tree iter = method->raises_expr;
        IDL_tree dataNode = IDL_LIST(iter).data;

        fputs(" throws ", state->file);
        fputs(IDL_IDENT(dataNode).str, state->file);
        iter = IDL_LIST(iter).next;

        while (iter) {
            dataNode = IDL_LIST(iter).data;
            fprintf(state->file, ", %s", IDL_IDENT(dataNode).str);
            iter = IDL_LIST(iter).next;
        }
    }

    fputs(";\n", state->file);

    return TRUE;
    
}


static gboolean
constant_declaration(TreeState *state)
{
    /*
     * The C++ header XPIDL module only allows for shorts and longs (ints)
     * to be constants, so we will follow the same convention
     */

    struct _IDL_CONST_DCL *declaration = &IDL_CONST_DCL(state->tree);
    const char *name = IDL_IDENT(declaration->ident).str;

    gboolean success;
    gboolean isshort = FALSE;

    /*
     * Consts must be in an interface
     */

    if (!IDL_NODE_UP(IDL_NODE_UP(state->tree)) ||
        IDL_NODE_TYPE(IDL_NODE_UP(IDL_NODE_UP(state->tree))) != 
        IDLN_INTERFACE) {

        IDL_tree_warning(state->tree, IDL_WARNING1,
                         "A constant \"%s\" was declared outside an interface."
                         "  It was ignored.", name);

        return TRUE;
    }

    /*
     * Make sure this is a numeric short or long constant.
     */

    success = (IDLN_TYPE_INTEGER == IDL_NODE_TYPE(declaration->const_type));

    if (success) {
        /*
         * We aren't successful yet, we know it's an integer, but what *kind*
         * of integer?
         */

        switch(IDL_TYPE_INTEGER(declaration->const_type).f_type) {

        case IDL_INTEGER_TYPE_SHORT:
            /*
             * We're OK
             */
            isshort = TRUE;
            break;

        case IDL_INTEGER_TYPE_LONG:
            /*
             * We're OK
             */            
            break;
            
        default:
            /*
             * Whoops, it's some other kind of number
             */
            
            success = FALSE;
        }	
    }

    if (success) {
	xpidl_write_comment(state, 4);

        fprintf(state->file, "    public static final %s %s = %d;\n",
		(isshort ? "short" : "int"),
                name, (int) IDL_INTEGER(declaration->const_exp).value);
    } else {
        IDL_tree_warning(state->tree, IDL_WARNING1,
                         "A constant \"%s\" was not of type short or long."
                         "  It was ignored.", name);	
    }

    return TRUE;

}

#define ATTR_IDENT(tree) (IDL_IDENT(IDL_LIST(IDL_ATTR_DCL((tree)).simple_declarations).data))
#define ATTR_PROPS(tree) (IDL_LIST(IDL_ATTR_DCL((tree)).simple_declarations).data)
#define ATTR_TYPE_DECL(tree) (IDL_ATTR_DCL((tree)).param_type_spec)


static gboolean
attribute_declaration(TreeState *state)
{
    gboolean read_only = IDL_ATTR_DCL(state->tree).f_readonly;
    char *attribute_name = ATTR_IDENT(state->tree).str;

    gboolean method_noscript = 
        (IDL_tree_property_get(ATTR_PROPS(state->tree), "noscript") != NULL);


    /* Comment */
    xpidl_write_comment(state, 4);

    state->tree = ATTR_TYPE_DECL(state->tree);

    /*
     * Write access permission ("public" unless nonscriptable)
     */
    fputs("    ", state->file);
    if (!method_noscript) {
        fputs("public ", state->file);
    }

    /*
     * Write the proper Java return value for the get operation
     */
    if (!xpcom_to_java_type(state)) {
        return FALSE;
    }
    
    /*
     * Write the name of the accessor ("get") method.
     */
    fprintf(state->file, " get%c%s();\n",
            toupper(attribute_name[0]), attribute_name + 1);


    if (!read_only) {
        /* Nonscriptable methods become package-protected */
        fputs("    ", state->file);
        if (!method_noscript) {
            fputs("public ", state->file);
        }

        /*
         * Write attribute access method name and return type
         */
        fprintf(state->file, "void set%c%s(",
                toupper(attribute_name[0]), 
                attribute_name+1);
        
        /*
         * Write the proper Java type for the set operation
         */
        if (!xpcom_to_java_type(state)) {
            return FALSE;
        }

        /*
         * Write the name of the formal parameter.
         */
        fputs(" value);\n", state->file);
    }

    return TRUE;
}


static gboolean
enum_declaration(TreeState *state)
{
    IDL_tree_warning(state->tree, IDL_WARNING1,
                     "enums not supported, enum \'%s\' ignored",
                     IDL_IDENT(IDL_TYPE_ENUM(state->tree).ident).str);
    return TRUE;
}

nodeHandler *
xpidl_java_dispatch(void)
{
    static nodeHandler table[IDLN_LAST];

    if (!table[IDLN_NONE]) {
        table[IDLN_NONE] = initial_pass;
        table[IDLN_INTERFACE] = interface_declaration;
        table[IDLN_LIST] = process_list;

        table[IDLN_OP_DCL] = method_declaration;
        table[IDLN_ATTR_DCL] = attribute_declaration;
        table[IDLN_CONST_DCL] = constant_declaration;

        table[IDLN_TYPE_DCL] = type_declaration;
        table[IDLN_FORWARD_DCL] = forward_declaration;

        table[IDLN_TYPE_ENUM] = enum_declaration;
    }

    return table;
}
