/* -*- Mode: C; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:expandtab:shiftwidth=2:tabstop=2:
 */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is
 * Sun Microsystems, Inc.
 * Portions created by the Initial Developer are Copyright (C) 2002
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Ginn Chen (ginn.chen@sun.com)
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef __MAI_REDUNDANT_OBJECT_FACTORY_H__
#define __MAI_REDUNDANT_OBJECT_FACTORY_H__

/*  atk 1.11.0 or later */
#if ATK_MAJOR_VERSION >=2 || \
    (ATK_MAJOR_VERSION == 1 && ATK_MINOR_VERSION >= 11)
#define USE_ATK_ROLE_REDUNDANT_OBJECT
#endif

#ifdef USE_ATK_ROLE_REDUNDANT_OBJECT
G_BEGIN_DECLS

typedef struct _maiRedundantObjectFactory       maiRedundantObjectFactory;
typedef struct _maiRedundantObjectFactoryClass  maiRedundantObjectFactoryClass;

struct _maiRedundantObjectFactory
{
  AtkObjectFactory parent;
};

struct _maiRedundantObjectFactoryClass
{
  AtkObjectFactoryClass parent_class;
};

GType mai_redundant_object_factory_get_type();

AtkObjectFactory *mai_redundant_object_factory_new();

G_END_DECLS

#endif /* USE_ATK_ROLE_REDUNDANT_OBJECT */

#endif /* __NS_MAI_REDUNDANT_OBJECT_FACTORY_H__ */
