/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
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
* The Original Code is the JavaScript 2 Prototype.
*
* The Initial Developer of the Original Code is Netscape
* Communications Corporation.   Portions created by Netscape are
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

    // Read a multiname property from a base object, push the value onto the stack
    case eDotRead:
        {
            Multiname *mn = bCon->mMultinameList[BytecodeContainer::getShort(pc)];
            pc += sizeof(short);
            b = pop();
            if (JS2VAL_IS_NULL(b))
                meta->reportError(Exception::referenceError, "Null base", errorPos());            
            if (JS2VAL_IS_OBJECT(b) && (JS2VAL_TO_OBJECT(b)->kind == LimitedInstanceKind)) {
                LimitedInstance *li = checked_cast<LimitedInstance *>(JS2VAL_TO_OBJECT(b));
                b = OBJECT_TO_JS2VAL(li->instance);
                if (!li->limit->read(meta, &b, li->limit, mn, NULL, RunPhase, &a))
                    meta->reportError(Exception::propertyAccessError, "No property named {0}", errorPos(), mn->name);
            }
            else {
                JS2Class *limit = meta->objectType(b);
                if (!limit->read(meta, &b, limit, mn, NULL, RunPhase, &a))
                    meta->reportError(Exception::propertyAccessError, "No property named {0}", errorPos(), mn->name);
            }
            push(a);
        }
        break;

    case eDotDelete:
        {
            Multiname *mn = bCon->mMultinameList[BytecodeContainer::getShort(pc)];
            pc += sizeof(short);
            b = pop();
            bool result;
            JS2Class *limit = meta->objectType(b);
            if (!limit->deleteProperty(meta, b, limit, mn, NULL, &result))
                push(JS2VAL_FALSE);
            else
                push(BOOLEAN_TO_JS2VAL(result));
        }
        break;

    // Write the top value to a multiname property in a base object, leave
    // the value on the stack top
    case eDotWrite:
        {
            a = pop();
            Multiname *mn = bCon->mMultinameList[BytecodeContainer::getShort(pc)];
            pc += sizeof(short);
            b = pop();
            if (JS2VAL_IS_NULL(b))
                meta->reportError(Exception::referenceError, "Null base", errorPos());
            if (JS2VAL_IS_OBJECT(b) && (JS2VAL_TO_OBJECT(b)->kind == LimitedInstanceKind)) {
                LimitedInstance *li = checked_cast<LimitedInstance *>(JS2VAL_TO_OBJECT(b));
                b = OBJECT_TO_JS2VAL(li->instance);
                if (!li->limit->write(meta, b, li->limit, mn, NULL, true, a, false)) {
                    if (!meta->cxt.E3compatibility)
                        meta->reportError(Exception::propertyAccessError, "No property named {0}", errorPos(), mn->name);
                }
            }
            else {
                JS2Class *limit = meta->objectType(b);
                if (!limit->write(meta, b, limit, mn, NULL, true, a, false)) {
                    if (!meta->cxt.E3compatibility)
                        meta->reportError(Exception::propertyAccessError, "No property named {0}", errorPos(), mn->name);
                }
            }
            push(a);
        }
        break;

    // Read the multiname property, but leave the base and the value on the stack
    case eDotRef:
        {
            Multiname *mn = bCon->mMultinameList[BytecodeContainer::getShort(pc)];
            pc += sizeof(short);
            b = pop();
            JS2Class *limit = meta->objectType(b);
            if (!limit->read(meta, &b, limit, mn, NULL, RunPhase, &a))
                meta->reportError(Exception::propertyAccessError, "No property named {0}", errorPos(), mn->name);
            push(b);
            push(a);
        }
        break;

    // Read the multiname from the current environment, push it's value on the stack
    case eLexicalRead: 
        {
            Multiname *mn = bCon->mMultinameList[BytecodeContainer::getShort(pc)];
            pc += sizeof(short);
            meta->env->lexicalRead(meta, mn, phase, &a, NULL);
            push(a);
        }
        break;

    case eLexicalDelete: 
        {
            Multiname *mn = bCon->mMultinameList[BytecodeContainer::getShort(pc)];
            pc += sizeof(short);
            push(BOOLEAN_TO_JS2VAL(meta->env->lexicalDelete(meta, mn, phase)));
        }
        break;

    // Write the top value to the multiname in the environment, leave
    // the value on the stack top.
    case eLexicalWrite: 
        {
            a = top();
            Multiname *mn = bCon->mMultinameList[BytecodeContainer::getShort(pc)];
            pc += sizeof(short);
            // XXX The value of 'createIfMissing' should come from the reference's copy
            // of the !environment.strict at the time the reference was constructed.
            // Have eLexicalWriteCreate ?
            meta->env->lexicalWrite(meta, mn, a, true);
        }
        break;

    // Write the top value to the multiname in the environment, leave
    // the value on the stack top.
    case eLexicalInit: 
        {
            a = pop();
            Multiname *mn = bCon->mMultinameList[BytecodeContainer::getShort(pc)];
            pc += sizeof(short);
            meta->env->lexicalInit(meta, mn, a);
        }
        break;

    // Construct a reference pair consisting of a NULL base and the read value
    case eLexicalRef: 
        {
            Multiname *mn = bCon->mMultinameList[BytecodeContainer::getShort(pc)];
            pc += sizeof(short);
            b = JS2VAL_NULL;
            meta->env->lexicalRead(meta, mn, phase, &a, &b);
            push(b);
            push(a);
        }
        break;

    // Read an index property from a base object, push the value onto the stack
    case eBracketRead:
        {
            indexVal = pop();
            b = pop();
            JS2Class *limit = meta->objectType(b);
            if (!limit->bracketRead(meta, &b, limit, indexVal, RunPhase, &a))
                meta->reportError(Exception::propertyAccessError, "No property named {0}", errorPos(), meta->toString(indexVal));
            push(a);
            indexVal = JS2VAL_VOID;
        }
        break;

    case eBracketDelete:
        {
            indexVal = pop();
            b = pop();
            bool result;
            JS2Class *limit = meta->objectType(b);
            if (!limit->bracketDelete(meta, b, limit, indexVal, &result))
                push(JS2VAL_FALSE);
            else
                push(BOOLEAN_TO_JS2VAL(result));
            indexVal = JS2VAL_VOID;
        }
        break;

    // Write the top value to an index property in a base object, leave
    // the value on the stack top
    case eBracketWrite:
        {
            a = pop();
            indexVal = pop();
            b = pop();
            JS2Class *limit = meta->objectType(b);
            if (!limit->bracketWrite(meta, b, limit, indexVal, a)) {
                if (!meta->cxt.E3compatibility)
                    meta->reportError(Exception::propertyAccessError, "No property named {0}", errorPos(), meta->toString(indexVal));
            }
            push(a);
            indexVal = JS2VAL_VOID;
        }
        break;

    // Leave the base object on the stack and push the property value
    case eBracketRef:
        {
            indexVal = pop();
            b = top();
            JS2Class *limit = meta->objectType(b);
            if (!limit->bracketRead(meta, &b, limit, indexVal, RunPhase, &a))
                meta->reportError(Exception::propertyAccessError, "No property named {0}", errorPos(), meta->toString(indexVal));
            push(a);
            indexVal = JS2VAL_VOID;
        }
        break;
    
    // Leave the base object and index value, push the value
    case eBracketReadForRef:
        {
            indexVal = pop();
            b = top();
            astr = meta->toString(indexVal);
            indexVal = STRING_TO_JS2VAL(astr);
            push(indexVal);
            JS2Class *limit = meta->objectType(b);
            if (!limit->bracketRead(meta, &b, limit, indexVal, RunPhase, &a))
                meta->reportError(Exception::propertyAccessError, "No property named {0}", errorPos(), astr);
            push(a);
            indexVal = JS2VAL_VOID;
            astr = NULL;
        }
        break;

    // Beneath the value is a reference pair (base and index), write to that location but leave just the value
    case eBracketWriteRef:
        {
            a = pop();
            indexVal = pop();
            ASSERT(JS2VAL_IS_STRING(indexVal)); // because the readForRef above will have executed first
            b = pop();
            JS2Class *limit = meta->objectType(b);
            if (!limit->bracketWrite(meta, b, limit, indexVal, a)) {
                if (!meta->cxt.E3compatibility)
                    meta->reportError(Exception::propertyAccessError, "No property named {0}", errorPos(), JS2VAL_TO_STRING(indexVal));
            }
            push(a);
            indexVal = JS2VAL_VOID;
        }
        break;

    case eFrameSlotWrite:
        {
            uint16 slotIndex = BytecodeContainer::getShort(pc);
            pc += sizeof(short);
            ASSERT(slotIndex < localFrame->slots->size());
            a = top();
            (*localFrame->slots)[slotIndex] = a;
        }
        break;

    case eFrameSlotDelete:
        {
            uint16 slotIndex = BytecodeContainer::getShort(pc);
            pc += sizeof(short);
            ASSERT(slotIndex < localFrame->slots->size());
            // XXX some kind of code here?
        }
        break;

    case eFrameSlotRead:
        {
            uint16 slotIndex = BytecodeContainer::getShort(pc);
            pc += sizeof(short);
            ASSERT(slotIndex < localFrame->slots->size());
            push((*localFrame->slots)[slotIndex]);
        }
        break;

    case eFrameSlotRef:
        {
            uint16 slotIndex = BytecodeContainer::getShort(pc);
            pc += sizeof(short);
            push(JS2VAL_NULL);
            ASSERT(slotIndex < localFrame->slots->size());
            push((*localFrame->slots)[slotIndex]);
        }
        break;

    case ePackageSlotWrite:
        {
            uint16 slotIndex = BytecodeContainer::getShort(pc);
            pc += sizeof(short);
            a = top();
            ASSERT(slotIndex < packageFrame->slots->size());
            (*packageFrame->slots)[slotIndex] = a;
        }
        break;

    case ePackageSlotDelete:
        {
            uint16 slotIndex = BytecodeContainer::getShort(pc);
            pc += sizeof(short);
            ASSERT(slotIndex < packageFrame->slots->size());
            // XXX some kind of code here?
        }
        break;

    case ePackageSlotRead:
        {
            uint16 slotIndex = BytecodeContainer::getShort(pc);
            pc += sizeof(short);
            ASSERT(slotIndex < packageFrame->slots->size());
            push((*packageFrame->slots)[slotIndex]);
        }
        break;

    case ePackageSlotRef:
        {
            uint16 slotIndex = BytecodeContainer::getShort(pc);
            pc += sizeof(short);
            push(JS2VAL_NULL);
            ASSERT(slotIndex < packageFrame->slots->size());
            push((*packageFrame->slots)[slotIndex]);
        }
        break;

    case eParameterSlotWrite:
        {
            uint16 slotIndex = BytecodeContainer::getShort(pc);
            pc += sizeof(short);
            a = top();
            ASSERT(slotIndex < parameterFrame->slots->size());
            (*parameterFrame->slots)[slotIndex] = a;
        }
        break;

    case eParameterSlotDelete:
        {
            uint16 slotIndex = BytecodeContainer::getShort(pc);
            pc += sizeof(short);
            ASSERT(slotIndex < parameterFrame->slots->size());
            // XXX some kind of code here?
        }
        break;

    case eParameterSlotRead:
        {
            uint16 slotIndex = BytecodeContainer::getShort(pc);
            pc += sizeof(short);
            ASSERT(slotIndex < parameterFrame->slots->size());
            push((*parameterFrame->slots)[slotIndex]);
        }
        break;

    case eParameterSlotRef:
        {
            uint16 slotIndex = BytecodeContainer::getShort(pc);
            pc += sizeof(short);
            push(JS2VAL_NULL);
            ASSERT(slotIndex < parameterFrame->slots->size());
            push((*parameterFrame->slots)[slotIndex]);
        }
        break;

    case eSlotWrite:
        {
            a = pop();
            uint16 slotIndex = BytecodeContainer::getShort(pc);
            pc += sizeof(short);
            b = pop();
            ASSERT(JS2VAL_IS_OBJECT(b));
            JS2Object *obj = JS2VAL_TO_OBJECT(b);
            ASSERT(obj->kind == SimpleInstanceKind);
            checked_cast<SimpleInstance *>(obj)->slots[slotIndex].value = a;
            push(a);
        }
        break;

    case eSlotRead:
        {
            uint16 slotIndex = BytecodeContainer::getShort(pc);
            pc += sizeof(short);
            b = pop();
            ASSERT(JS2VAL_IS_OBJECT(b));
            JS2Object *obj = JS2VAL_TO_OBJECT(b);
            ASSERT(obj->kind == SimpleInstanceKind);
            push(checked_cast<SimpleInstance *>(obj)->slots[slotIndex].value);
        }
        break;

    case eSlotRef:
        {
            uint16 slotIndex = BytecodeContainer::getShort(pc);
            pc += sizeof(short);
            b = top();
            ASSERT(JS2VAL_IS_OBJECT(b));
            JS2Object *obj = JS2VAL_TO_OBJECT(b);
            ASSERT(obj->kind == SimpleInstanceKind);
            push(checked_cast<SimpleInstance *>(obj)->slots[slotIndex].value);
        }
        break;

