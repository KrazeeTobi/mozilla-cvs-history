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
 * The Original Code is Oracle Corporation code.
 *
 * The Initial Developer of the Original Code is Oracle Corporation
 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Stuart Parmenter <stuart.parmenter@oracle.com>
 *   Joey Minta <jminta@gmail.com>
 *   Michael Buettner <michael.buettner@sun.com>
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

var gIsReadOnly = false;
var gStartTime = null;
var gEndTime = null;

function onLoad() {

    var args = window.arguments[0];
    var item = args.calendarEvent;
    var calendar = item.calendar;
    var recinfo = args.recurrenceInfo;

    gStartTime = args.startTime;
    gEndTime = args.endTime;

    // Set period to 'none'.
    document.getElementById("period-list").selectedIndex = 0;

    onChangeCalendar(calendar);

    // Set starting value for 'repeat until' rule.
    setElementValue("repeat-until-date", gStartTime);

    if (item.parentItem != item) {
        item = item.parentItem;
    }

    if (recinfo) {
        // Split out rules and exceptions
        var rrules = splitRecurrenceRules(recinfo);
        var rules = rrules[0];
        var exceptions = rrules[1];

        // Deal with the rules
        if (rules.length > 0) {
            // We only handle 1 rule currently
            var rule = rules[0];
            if (rule instanceof Ci.calIRecurrenceRule) {
                switch (rule.type) {
                    case "DAILY":
                        document.getElementById("period-list").selectedIndex = 0;
                        setElementValue("daily-days", rule.interval);
                        var ruleComp = rule.getComponent("BYDAY", {});
                        if (ruleComp.length > 0) {
                            document.getElementById("daily-group").selectedIndex = 1;
                        } else {
                            document.getElementById("daily-group").selectedIndex = 0;
                        }
                        break;
                    case "WEEKLY":
                        document.getElementById("period-list").selectedIndex = 1;
                        setElementValue("weekly-weeks", rule.interval);
                        var days = rule.getComponent("BYDAY", {});
                        document.getElementById("datepicker-weekday").days = days;
                        break;
                    case "MONTHLY":
                        document.getElementById("period-list").selectedIndex = 2;
                        setElementValue("monthly-interval", rule.interval);
                        var ruleComp = rule.getComponent("BYDAY", {});
                        if (ruleComp.length > 0) {
                            document.getElementById("monthly-group").selectedIndex = 0;
                            var byday = ruleComp[0];
                            var occurrence = (byday - (byday % 8)) / 8;
                            var weekday = byday % 8;
                            setElementValue("monthly-ordinal", occurrence);
                            setElementValue("monthly-weekday", weekday);
                        } else {
                            var ruleComp = rule.getComponent("BYMONTHDAY", {});
                            if (ruleComp.length > 0) {
                                document.getElementById("monthly-group").selectedIndex = 1;
                                document.getElementById("monthly-days").days = ruleComp;
                            }
                        }
                        break;
                    case "YEARLY":
                        document.getElementById("period-list").selectedIndex = 3;
                        setElementValue("yearly-interval", rule.interval);
                        var byMonthRule = rule.getComponent("BYMONTH", {});
                        var byDayRule = rule.getComponent("BYDAY", {});
                        var byMonthDayRule = rule.getComponent("BYMONTHDAY", {});
                        if (byMonthRule.length > 0) {
                            if (byMonthDayRule.length > 0) {
                                document.getElementById("yearly-group").selectedIndex = 0;
                                setElementValue("yearly-month-ordinal", byMonthRule[0]);
                                setElementValue("yearly-days", byMonthDayRule[0]);
                            } else if (byDayRule.length > 0) {
                                document.getElementById("yearly-group").selectedIndex = 1;
                                var byday = byDayRule[0];
                                var occurrence = (byday - (byday % 8)) / 8;
                                var weekday = byday % 8;
                                setElementValue("yearly-ordinal", occurrence);
                                setElementValue("yearly-weekday", weekday);
                                setElementValue("yearly-month-rule", byMonthRule[0]);
                            }
                        }
                        break;
                    default:
                        dump("unable to handle your rule type!\n");
                        break;
                }

                /* load up the duration of the event radiogroup */
                if (rule.isByCount) {
                    if (rule.count == -1) {
                        setElementValue("recurrence-duration", "forever");
                    } else {
                        setElementValue("recurrence-duration", "ntimes");
                        setElementValue("repeat-ntimes-count", rule.count );
                    }
                } else {
                    var endDate = rule.endDate;
                    if (!endDate) {
                        setElementValue("recurrence-duration", "forever");
                    } else {
                        // Convert the datetime from UTC to localtime.
                        endDate = endDate.getInTimezone(calendarDefaultTimezone());
                        setElementValue("recurrence-duration", "until");
                        setElementValue("repeat-until-date", endDate.jsDate);
                    }
                }
            }
        }
    }

    // Update controls
    updateRecurrenceDeck();

    opener.setCursor("auto");
    self.focus();
}

function onSave(item) {
    // Always return 'null' if this item is an occurrence.
    if (item.parentItem != item) {
        return null;
    }

    // This works, but if we ever support more complex recurrence,
    // e.g. recurrence for Martians, then we're going to want to
    // not clone and just recreate the recurrenceInfo each time.
    // The reason is that the order of items (rules/dates/datesets)
    // matters, so we can't always just append at the end.  This
    // code here always inserts a rule first, because all our
    // exceptions should come afterward.
    var deckNumber = Number(getElementValue("period-list"));

    var args = window.arguments[0];
    var recurrenceInfo = args.recurrenceInfo;
    if (recurrenceInfo) {
        recurrenceInfo = recurrenceInfo.clone();
        var rrules = splitRecurrenceRules(recurrenceInfo);
        if (rrules[0].length > 0) {
            recurrenceInfo.deleteRecurrenceItem(rrules[0][0]);
        }
    } else {
        recurrenceInfo = createRecurrenceInfo();
        recurrenceInfo.item = item;
    }

    var recRule = createRecurrenceRule();
    switch (deckNumber) {
    case 0:
        recRule.type = "DAILY";
        var dailyGroup = document.getElementById("daily-group");
        if (dailyGroup.selectedIndex == 0) {
            var ndays = Math.max(1, Number(getElementValue("daily-days")));
            recRule.interval = ndays;
        } else {
            recRule.interval = 1;
            var onDays = [2, 3, 4, 5, 6];
            recRule.setComponent("BYDAY", onDays.length, onDays);
        }
        break;
    case 1:
        recRule.type = "WEEKLY";
        var ndays = Number(getElementValue("weekly-weeks"));
        recRule.interval = ndays;
        var onDays = document.getElementById("datepicker-weekday").days;
        if (onDays.length > 0) {
            recRule.setComponent("BYDAY", onDays.length, onDays);
        }
        break;
    case 2:
        recRule.type = "MONTHLY";
        var monthInterval = Number(getElementValue("monthly-interval"));
        recRule.interval = monthInterval;
        var monthlyGroup = document.getElementById("monthly-group");
        if (monthlyGroup.selectedIndex==0) {
            var ordinal = Number(getElementValue("monthly-ordinal"));
            var day_of_week = Number(getElementValue("monthly-weekday"));
            var sign = ordinal < 0 ? -1 : 1;
            var onDays = [ (Math.abs(ordinal) * 8 + day_of_week) * sign ];
            recRule.setComponent("BYDAY", onDays.length, onDays);
        } else {
            var monthlyDays = document.getElementById("monthly-days").days;
            if (monthlyDays.length > 0) {
                recRule.setComponent("BYMONTHDAY", monthlyDays.length, monthlyDays);
            }
        }
        break;
    case 3:
        recRule.type = "YEARLY";
        var yearInterval = Number(getElementValue("yearly-interval"));
        recRule.interval = yearInterval;
        var yearlyGroup = document.getElementById("yearly-group");
        if (yearlyGroup.selectedIndex == 0) {
            var yearlyByMonth = [ Number(getElementValue("yearly-month-ordinal")) ];
            recRule.setComponent("BYMONTH", yearlyByMonth.length, yearlyByMonth);
            var yearlyByDay = [ Number(getElementValue("yearly-days")) ];
            recRule.setComponent("BYMONTHDAY", yearlyByDay.length, yearlyByDay);
        } else {
            var yearlyByMonth = [ Number(getElementValue("yearly-month-rule")) ];
            recRule.setComponent("BYMONTH", yearlyByMonth.length, yearlyByMonth);
            var ordinal = Number(getElementValue("yearly-ordinal"));
            var day_of_week = Number(getElementValue("yearly-weekday"));
            var sign = ordinal < 0 ? -1 : 1;
            var onDays = [ (Math.abs(ordinal) * 8 + day_of_week) * sign ];
            recRule.setComponent("BYDAY", onDays.length, onDays);
        }
        break;
    }

    // Figure out how long this event is supposed to last
    switch (document.getElementById("recurrence-duration").selectedItem.value) {
        case "forever":
            recRule.count = -1;
            break;
        case "ntimes":
            recRule.count = Math.max(1, getElementValue("repeat-ntimes-count"));
            break;
        case "until":
            // Get the datetime from the control (which is in localtime),
            // set the time to 23:59:99 and convert that to UTC time.
            var endDate = getElementValue("repeat-until-date")
            endDate.setHours(23);
            endDate.setMinutes(59);
            endDate.setSeconds(59);
            endDate.setMilliseconds(999);
            endDate = jsDateToDateTime(endDate);
            recRule.endDate = endDate;
            break;
    }

    if (recRule.interval < 1) {
        return null;
    }

    recurrenceInfo.insertRecurrenceItemAt(recRule, 0);
    return recurrenceInfo;
}

function onAccept() {
    var args = window.arguments[0];
    var item = args.calendarEvent;
    args.onOk(onSave(item));
    return true;
}

function onChangeCalendar(calendar) {
    var args = window.arguments[0];
    var item = args.calendarEvent;

    // Set 'gIsReadOnly' if the calendar is read-only
    gIsReadOnly = false;
    if (calendar && calendar.readOnly) {
        gIsReadOnly = true;
    }

    // Disable or enable controls based on a set or rules
    // - whether this item is a stand-alone item or an occurrence
    // - whether or not this item is read-only
    // - whether or not the state of the item allows recurrence rules
    //     - tasks without an entrydate are invalid
    disableOrEnable(item);

    updateRecurrenceControls();
}

function disableOrEnable(item) {
    if (item.parentItem != item) {
       disableRecurrenceFields("disable-on-occurrence");
    } else if (gIsReadOnly) {
        disableRecurrenceFields("disable-on-readonly");
    } else if (isToDo(item) && !gStartTime) {
        disableRecurrenceFields("disable-on-readonly");
    } else {
        enableRecurrenceFields("disable-on-readonly");
    }
}

function disableRecurrenceFields(aAttributeName) {
    var disableElements = document.getElementsByAttribute(aAttributeName, "true");
    for (var i = 0; i < disableElements.length; i++) {
        disableElements[i].setAttribute('disabled', 'true');
    }
}

function enableRecurrenceFields(aAttributeName) {
    var enableElements = document.getElementsByAttribute(aAttributeName, "true");
    for (var i = 0; i < enableElements.length; i++) {
        enableElements[i].removeAttribute('disabled');
    }
}

function splitRecurrenceRules(recurrenceInfo) {
    var ritems = recurrenceInfo.getRecurrenceItems({});
    var rules = [];
    var exceptions = [];
    for each (var r in ritems) {
        if (r.isNegative) {
            exceptions.push(r);
        } else {
            rules.push(r);
        }
    }
    return [rules, exceptions];
}

function updateRecurrenceDeck() {
    document.getElementById("period-deck")
            .selectedIndex = Number(getElementValue("period-list"));
    updateRecurrenceControls();
}

function updateRecurrenceRange() {
    var args = window.arguments[0];
    var item = args.calendarEvent;
    if (item.parentItem != item || gIsReadOnly) {
        return;
    }

    var radioRangeForever =
        document.getElementById("recurrence-range-forever");
    var radioRangeFor =
        document.getElementById("recurrence-range-for");
    var radioRangeUntil =
        document.getElementById("recurrence-range-until");
    var rangeTimesCount =
        document.getElementById("repeat-ntimes-count");
    var rangeUntilDate =
        document.getElementById("repeat-until-date");
    var rangeAppointmentsLabel =
        document.getElementById("repeat-appointments-label");

    var deckNumber = Number(getElementValue("period-list"));

    radioRangeForever.removeAttribute("disabled");
    radioRangeFor.removeAttribute("disabled");
    radioRangeUntil.removeAttribute("disabled");
    rangeAppointmentsLabel.removeAttribute("disabled");

    var durationSelection = document.getElementById("recurrence-duration")
                                    .selectedItem.value;
    if (durationSelection == "forever") {
    }

    if (durationSelection == "ntimes") {
        rangeTimesCount.removeAttribute("disabled");
    } else {
        rangeTimesCount.setAttribute("disabled", "true");
    }

    if (durationSelection == "until") {
        rangeUntilDate.removeAttribute("disabled");
    } else {
        rangeUntilDate.setAttribute("disabled", "true");
    }
}

function updatePreview() {
    var args = window.arguments[0];
    var item = args.calendarEvent;
    if (item.parentItem != item) {
        item = item.parentItem;
    }

    // TODO: We should better start the whole dialog with a newly cloned item
    // and always pump changes immediately into it. this would eliminate the
    // need to break the encapsulation, as we do it here. but we need the item
    // to contain the startdate in order to calculate the recurrence preview.
    item = item.clone();
    var kDefaultTimezone = calendarDefaultTimezone();
    if (isEvent(item)) {
        var startDate = gStartTime.getInTimezone(kDefaultTimezone);
        var endDate = gEndTime.getInTimezone(kDefaultTimezone);
        if (startDate.isDate) {
            endDate.day--;
        }

        item.startDate = startDate;
        item.endDate = endDate;
    }
    if (isToDo(item)) {
        var entryDate = gStartTime;
        if (entryDate) {
            entryDate = entryDate.getInTimezone(kDefaultTimezone);
        } else {
            item.recurrenceInfo = null;
        }
        item.entryDate = entryDate;
        var dueDate = gEndTime;
        if (dueDate) {
            dueDate = dueDate.getInTimezone(kDefaultTimezone);
        }
        item.dueDate = dueDate;
    }

    var recInfo = onSave(item);
    var preview = document.getElementById("recurrence-preview");
    preview.updatePreview(recInfo);
}

function updateRecurrenceControls() {
    updateRecurrencePattern();
    updateRecurrenceRange();
    updatePreview();
}

/**
 * Disables/enables controls related to the recurrence pattern.
 * the status of the controls depends on which period entry is selected
 * and which form of pattern rule is selected.
 */
function updateRecurrencePattern() {
    var args = window.arguments[0];
    var item = args.calendarEvent;
    if (item.parentItem != item || gIsReadOnly) {
        return;
    }

    switch (Number(getElementValue("period-list"))) {
        // daily
        case 0:
            var dailyGroup = document.getElementById("daily-group");
            var dailyDays = document.getElementById("daily-days");
            dailyDays.removeAttribute("disabled", "true");
            if (dailyGroup.selectedIndex == 1) {
                dailyDays.setAttribute("disabled", "true");
            }
            break;
        // weekly
        case 1:
            break;
        // monthly
        case 2:
            var monthlyGroup = document.getElementById("monthly-group");
            var monthlyOrdinal = document.getElementById("monthly-ordinal");
            var monthlyWeekday = document.getElementById("monthly-weekday");
            var monthlyDays = document.getElementById("monthly-days");
            monthlyOrdinal.removeAttribute("disabled", "true");
            monthlyWeekday.removeAttribute("disabled", "true");
            monthlyDays.removeAttribute("disabled", "true");
            if (monthlyGroup.selectedIndex == 0) {
                monthlyDays.setAttribute("disabled", "true");
            } else {
                monthlyOrdinal.setAttribute("disabled", "true");
                monthlyWeekday.setAttribute("disabled", "true");
            }
            break;
        // yearly
        case 3:
            var yearlyGroup = document.getElementById("yearly-group");
            var yearlyDays = document.getElementById("yearly-days");
            var yearlyMonthOrdinal = document.getElementById("yearly-month-ordinal");
            var yearlyOrdinal = document.getElementById("yearly-ordinal");
            var yearlyWeekday = document.getElementById("yearly-weekday");
            var yearlyMonthRule = document.getElementById("yearly-month-rule");
            yearlyDays.removeAttribute("disabled", "true");
            yearlyMonthOrdinal.removeAttribute("disabled", "true");
            yearlyOrdinal.removeAttribute("disabled", "true");
            yearlyWeekday.removeAttribute("disabled", "true");
            yearlyMonthRule.removeAttribute("disabled", "true");
            if (yearlyGroup.selectedIndex == 0) {
                yearlyOrdinal.setAttribute("disabled", "true");
                yearlyWeekday.setAttribute("disabled", "true");
                yearlyMonthRule.setAttribute("disabled", "true");
            } else {
                yearlyDays.setAttribute("disabled", "true");
                yearlyMonthOrdinal.setAttribute("disabled", "true");
            }
            break;
    }
}
