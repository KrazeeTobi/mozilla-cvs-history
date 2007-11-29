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
 * The Original Code is Calendar views code
 *
 * The Initial Developer of the Original Code is
 *   the Mozilla Calendar Squad
 * Portions created by the Initial Developer are Copyright (C) 2006
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Vladimir Vukicevic <vladimir.vukicevic@oracle.com>
 *   Joey Minta <jminta@gmail.com>
 *   Michael Buettner <michael.buettner@sun.com>
 *   gekacheka@yahoo.com
 *   Matthew Willis <lilmatt@mozilla.com>
 *   Philipp Kewisch <mozilla@kewis.ch>
 *   Martin Schroeder <mschroeder@mozilla.x-home.org>
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

var calendarViewController = {
    QueryInterface: function(aIID) {
        if (!aIID.equals(Components.interfaces.calICalendarViewController) &&
            !aIID.equals(Components.interfaces.nsISupports)) {
            throw Components.results.NS_ERROR_NO_INTERFACE;
        }

        return this;
    },

    createNewEvent: function (aCalendar, aStartTime, aEndTime) {
        aCalendar = aCalendar || getSelectedCalendar();

        // if we're given both times, skip the dialog
        if (aStartTime && aEndTime && !aStartTime.isDate && !aEndTime.isDate) {
            var event = createEvent();
            event.startDate = aStartTime;
            event.endDate = aEndTime;
            var sbs = Components.classes["@mozilla.org/intl/stringbundle;1"]
                                .getService(Components.interfaces.nsIStringBundleService);
            var props = sbs.createBundle("chrome://calendar/locale/calendar.properties");
            event.title = props.GetStringFromName("newEvent");
            setDefaultAlarmValues(event);
            doTransaction('add', event, aCalendar, null, null);
        } else if (aStartTime && aStartTime.isDate) {
            var event = createEvent();
            event.startDate = aStartTime;
            setDefaultAlarmValues(event);
            doTransaction('add', event, aCalendar, null, null);
        } else {
            createEventWithDialog(aCalendar, aStartTime, null);
        }
    },

    pendingJobs: [],

    // in order to initiate a modification for the occurrence passed as argument
    // we create an object that records the necessary details and store it in an
    // internal array ('pendingJobs'). this way we're in a position to terminate
    // any pending modification if need should be.
    createPendingModification: function (aOccurrence) {
        // finalize a (possibly) pending modification. this will notify
        // an open dialog to save any outstanding modifications.
        aOccurrence = this.finalizePendingModification(aOccurrence);

        var pendingModification = {
            controller: this,
            item: aOccurrence,
            finalize: null,
            dispose: function() {
                var array = this.controller.pendingJobs;
                for (var i=0; i<array.length; i++) {
                    if (array[i] == this) {
                        array.splice(i,1);
                        break;
                    }
                }
            }
        }

        this.pendingJobs.push(pendingModification);

        modifyEventWithDialog(aOccurrence,pendingModification);
    },

    // iterate the list of pending modifications and see if the occurrence
    // passed as argument is currently about to be modified (event dialog is
    // open with the item in question). if this should be the case we call
    // finalize() in order to bring the dialog down and avoid dataloss.
    finalizePendingModification: function (aOccurrence) {

      for each (var job in this.pendingJobs) {
          var item = job.item;
          var parent = item.parent;
          if ((item.hashId == aOccurrence.hashId) ||
              (item.parentItem.hashId == aOccurrence.hashId) ||
              (item.hashId == aOccurrence.parentItem.hashId)) {
              // terminate() will most probably create a modified item instance.
              aOccurrence = job.finalize();
              break;
        }
      }

      return aOccurrence;
    },

    modifyOccurrence: function (aOccurrence, aNewStartTime, aNewEndTime, aNewTitle) {

        aOccurrence = this.finalizePendingModification(aOccurrence);

        // if modifying this item directly (e.g. just dragged to new time),
        // then do so; otherwise pop up the dialog
        if (aNewStartTime || aNewEndTime || aNewTitle) {
            var instance = aOccurrence.clone();

            if (aNewTitle) {
                instance.title = aNewTitle;
            }

            // When we made the executive decision (in bug 352862) that
            // dragging an occurrence of a recurring event would _only_ act
            // upon _that_ occurrence, we removed a bunch of code from this
            // function. If we ever revert that decision, check CVS history
            // here to get that code back.

            if (aNewStartTime || aNewEndTime) {
                // Yay for variable names that make this next line look silly
                if (instance instanceof Components.interfaces.calIEvent) {
                    if (aNewStartTime && instance.startDate) {
                        instance.startDate = aNewStartTime;
                    }
                    if (aNewEndTime && instance.endDate) {
                        instance.endDate = aNewEndTime;
                    }
                } else {
                    if (aNewStartTime && instance.entryDate) {
                        instance.entryDate = aNewStartTime;
                    }
                    if (aNewEndTime && instance.dueDate) {
                        instance.dueDate = aNewEndTime;
                    }
                }
            }
            doTransaction('modify', instance, instance.calendar, aOccurrence, null);
        } else {
            // prompt for choice between occurrence and master for recurrent items
            var itemToEdit = getOccurrenceOrParent(aOccurrence);
            if (!itemToEdit) {
                return;  // user cancelled
            }

            this.createPendingModification(itemToEdit);
        }
    },

    deleteOccurrences: function (aCount,
                                 aOccurrences,
                                 aUseParentItems,
                                 aDoNotConfirm) {
        startBatchTransaction();
        var recurringItems = {};

        function getSavedItem(aItemToDelete) {
            // Get the parent item, saving it in our recurringItems object for
            // later use.
            var hashVal = aItemToDelete.parentItem.hashId;
            if (!recurringItems[hashVal]) {
                recurringItems[hashVal] = {
                    oldItem: aItemToDelete.parentItem,
                    newItem: aItemToDelete.parentItem.clone()
                };
            }
            return recurringItems[hashVal];
        }

        // Make sure we are modifying a copy of aOccurrences, otherwise we will
        // run into race conditions when the view's doDeleteItem removes the
        // array elements while we are iterating through them.
        var occurrences = aOccurrences.slice(0);

        for each (var itemToDelete in occurrences) {
            if (aUseParentItems) {
                // Usually happens when ctrl-click is used. In that case we
                // don't need to ask the user if he wants to delete an
                // occurrence or not.
                itemToDelete = itemToDelete.parentItem;
            } else if (!aDoNotConfirm && occurrences.length == 1) {
                // Only give the user the selection if only one occurrence is
                // selected. Otherwise he will get a dialog for each occurrence
                // he deletes.
                itemToDelete = getOccurrenceOrParent(itemToDelete);
            }
            if (!itemToDelete) {
                continue;
            }
            itemToDelete = this.finalizePendingModification(itemToDelete);
            if (itemToDelete.parentItem.hashId != itemToDelete.hashId) {
                var savedItem = getSavedItem(itemToDelete);
                savedItem.newItem.recurrenceInfo
                         .removeOccurrenceAt(itemToDelete.recurrenceId);
                // Dont start the transaction yet. Do so later, in case the
                // parent item gets modified more than once.
            } else {
                doTransaction('delete', itemToDelete, itemToDelete.calendar, null, null);
            }
        }

        // Now handle recurring events. This makes sure that all occurrences
        // that have been passed are deleted.
        for each (var ritem in recurringItems) {
            doTransaction('modify',
                          ritem.newItem,
                          ritem.newItem.calendar,
                          ritem.oldItem,
                          null);
        }
        endBatchTransaction();
    }
};

/**
 * This function provides a neutral way to switch between views.
 * XXX Kind of confusing. This function calls the app specific function, which
 * again calls the common switchToView function. They should be consolidated in
 * a different bug.
 */
function showCalendarView(type) {
    if (isSunbird()) {
        gCalendarWindow.switchToView(type);
    } else {
        ltnShowCalendarView(type);
    }
}

/**
 * This function acts like the above, but does not bring the view to the front
 * if the application is showing other elements (i.e Lightning).
 */
function selectCalendarView(type) {
    if (isSunbird()) {
        gCalendarWindow.switchToView(type);
    } else {
        ltnSelectCalendarView(type);
    }
}

/**
 * This function does the common steps to switch between views. Should be called
 * from app-specific view switching functions
 */
function switchToView(aViewType) {
    var viewDeck = getViewDeck();
    var selectedDay;
    var currentSelection = [];

    // Set up the view commands
    var views = viewDeck.childNodes;
    for (var i = 0; i < views.length; i++) {
        var view = views[i];
        var commandId = "calendar_" + view.id + "_command";
        var command = document.getElementById(commandId);
        if (view.id == aViewType + "-view") {
            command.setAttribute("checked", "true");
        } else {
            command.removeAttribute("checked");
        }
    }

    // Disable the menuitem when not in day or week view.
    var rotated = document.getElementById("calendar_toggle_orientation_command");
    if (aViewType == "day" || aViewType == "week") {
        rotated.removeAttribute("disabled");
    } else {
        rotated.setAttribute("disabled", "true");
    }

    try {
        selectedDay = viewDeck.selectedPanel.selectedDay;
        currentSelection = viewDeck.selectedPanel.getSelectedItems({});
    } catch (ex) {
        // This dies if no view has even been chosen this session, but that's
        // ok because we'll just use now() below.
    }

    if (!selectedDay) {
        selectedDay = now();
    }

    // Anyone wanting to plug in a view needs to follow this naming scheme
    var view = document.getElementById(aViewType + "-view");
    viewDeck.selectedPanel = view;

    var compositeCal = getCompositeCalendar();
    if (view.displayCalendar != compositeCal) {
        view.displayCalendar = compositeCal;
        view.timezone = calendarDefaultTimezone();
        view.controller = calendarViewController;
    }

    view.goToDay(selectedDay);
    view.setSelectedItems(currentSelection.length, currentSelection);
}

function moveView(aNumber) {
    currentView().moveView(aNumber);
}

/**
 * Returns the calendar view deck.
 */
function getViewDeck() {
    return document.getElementById("view-deck");
}

/**
 * Returns the currently visible calendar view.
 */
function currentView() {
    return getViewDeck().selectedPanel;
}

/**
 * Returns the selected day in the views in a app (Sunbird vs. Lightning)
 * neutral way
 */
function getSelectedDay() {
    return currentView().selectedDay;
}

var gMidnightTimer;

/** Creates a timer that will fire after midnight.  Pass in a function as
 * aRefreshCallback that should be called at that time.
 */
function scheduleMidnightUpdate(aRefreshCallback) {
    var jsNow = new Date();
    var tomorrow = new Date(jsNow.getFullYear(), jsNow.getMonth(), jsNow.getDate() + 1);
    var msUntilTomorrow = tomorrow.getTime() - jsNow.getTime();

    // Is an nsITimer/callback extreme overkill here? Yes, but it's necessary to
    // workaround bug 291386.  If we don't, we stand a decent chance of getting
    // stuck in an infinite loop.
    var udCallback = {
        notify: function(timer) {
            aRefreshCallback();
        }
    };

    if (!gMidnightTimer) {
        // Observer for wake after sleep/hibernate/standby to create new timers and refresh UI
        var wakeObserver = {
           observe: function(aSubject, aTopic, aData) {
               if (aTopic == "wake_notification") {
                   aRefreshCallback();
               }
           }
        };

        // Add observer
        var observerService = Components.classes["@mozilla.org/observer-service;1"]
                                        .getService(Components.interfaces.nsIObserverService);
        observerService.addObserver(wakeObserver, "wake_notification", false);

        // Remove observer on unload
        window.addEventListener("unload",
                                function() {
                                    observerService.removeObserver(wakeObserver, "wake_notification");
                                }, false);

        gMidnightTimer = Components.classes["@mozilla.org/timer;1"]
                                   .createInstance(Components.interfaces.nsITimer);
    } else {
        gMidnightTimer.cancel();
    }
    gMidnightTimer.initWithCallback(udCallback, msUntilTomorrow, gMidnightTimer.TYPE_ONE_SHOT);
}

// Returns the actual style sheet object with the specified path.  Callers are
// responsible for any caching they may want to do.
function getStyleSheet(aStyleSheetPath) {
    for each (var sheet in document.styleSheets) {
        if (sheet.href == aStyleSheetPath) {
            return sheet;
        }
    }
    // Avoid the js strict "function does not always return a value" warning.
    return null;
}

// Updates the style rules for a particular object.  If the object is a
// category (and hence doesn't have a uri), we set the border color.  If
// it's a calendar, we set the background color
function updateStyleSheetForObject(aObject, aSheet) {
    var selectorPrefix, name, ruleUpdaterFunc;
    if (aObject.uri) {
        // This is a calendar, so we're going to set the background color
        name = aObject.uri.spec;
        selectorPrefix = "item-calendar=";
        ruleUpdaterFunc = function calendarRuleFunc(aRule, aIndex) {
            var color = aObject.getProperty('color');
            if (!color) {
                color = "#A8C2E1";
            }
            aRule.style.backgroundColor = color;
            aRule.style.color = getContrastingTextColor(color);
        };
    } else {
        // This is a category, where we set the border.  Also note that we 
        // use the ~= selector, since there could be multiple categories
        name = aObject.replace(' ','_');
        selectorPrefix = "item-category~=";
        ruleUpdaterFunc = function categoryRuleFunc(aRule, aIndex) {
            var color = getPrefSafe("calendar.category.color."+aObject, null);
            if (color) {
                aRule.style.backgroundColor = color;
            } else {
                aSheet.deleteRule(aIndex);
            }
        };
    }

    var selector = '.calendar-item[' + selectorPrefix + '"' + name + '"]';

    // Now go find our rule
    var rule, ruleIndex;
    for (var i = 0; i < aSheet.cssRules.length; i++) {
        var maybeRule = aSheet.cssRules[i];
        if (maybeRule.selectorText && (maybeRule.selectorText == selector)) {
            rule = maybeRule;
            ruleIndex = i;
            break;
        }
    }

    if (!rule) {
        aSheet.insertRule(selector + ' { }', aSheet.cssRules.length);
        rule = aSheet.cssRules[aSheet.cssRules.length-1];
    }

    ruleUpdaterFunc(rule, ruleIndex);
}

/** 
 *  Sets the selected day in the minimonth to the currently selected day
 *  in the embedded view.
 */
function observeViewDaySelect(event) {
    var date = event.detail;
    var jsDate = new Date(date.year, date.month, date.day);

    // for the month and multiweek view find the main month,
    // which is the month with the most visible days in the view;
    // note, that the main date is the first day of the main month
    var jsMainDate;
    if (!event.originalTarget.supportsDisjointDates) {
        var mainDate = null;
        var maxVisibleDays = 0;
        var startDay = currentView().startDay;
        var endDay = currentView().endDay;
        var firstMonth = startDay.startOfMonth;
        var lastMonth = endDay.startOfMonth;
        for (var month = firstMonth.clone(); month.compare(lastMonth) <= 0; month.month += 1) {
            var visibleDays = 0;
            if (month.compare(firstMonth) == 0) {
                visibleDays = startDay.endOfMonth.day - startDay.day + 1;
            } else if (month.compare(lastMonth) == 0) {
                visibleDays = endDay.day;
            } else {
                visibleDays = month.endOfMonth.day;
            }
            if (visibleDays > maxVisibleDays) {
                mainDate = month.clone();
                maxVisibleDays = visibleDays;
            }
        }
        jsMainDate = new Date(mainDate.year, mainDate.month, mainDate.day);
    }

    getMinimonth().selectDate(jsDate, jsMainDate);
    currentView().focus();
}

/** Provides a neutral way to get the minimonth, regardless of whether we're in
 * Sunbird or Lightning.
 */
function getMinimonth() {
    var sbMinimonth = document.getElementById("lefthandcalendar");
    return sbMinimonth || document.getElementById("ltnMinimonth");
}

/**
 * Update the view orientation based on the checked state of the command
 */
function toggleOrientation() {
    var cmd = document.getElementById("calendar_toggle_orientation_command");
    var newValue = (cmd.getAttribute("checked") == "true" ? "false" : "true");
    cmd.setAttribute("checked", newValue);

    var deck = getViewDeck();
    for each (var view in deck.childNodes) {
        view.rotated = !view.rotated;
    }

    // orientation refreshes automatically
}

/**
 * Toggle the workdays only checkbox and refresh the current view
 */
function toggleWorkdaysOnly() {
    var cmd = document.getElementById("calendar_toggle_workdays_only_command");
    var newValue = (cmd.getAttribute("checked") == "true" ? "false" : "true");
    cmd.setAttribute("checked", newValue);

    var deck = getViewDeck();
    for each (var view in deck.childNodes) {
        view.workdaysOnly = !view.workdaysOnly;
    }

    // Refresh the current view
    currentView().goToDay(currentView().selectedDay);
}

/**
 * Toggle the tasks in view checkbox and refresh the current view
 */
function toggleTasksInView() {
    var cmd = document.getElementById("calendar_toggle_tasks_in_view_command");
    var newValue = (cmd.getAttribute("checked") == "true" ? "false" : "true");
    cmd.setAttribute("checked", newValue);

    var deck = getViewDeck();
    for each (var view in deck.childNodes) {
        view.tasksInView = !view.tasksInView;
    }

    // Refresh the current view
    currentView().goToDay(currentView().selectedDay);
}

/**
 * Provides a neutral way to go to the current day
 */
function goToDate(aDate) {
    getMinimonth().value = aDate.jsDate;
    currentView().goToDay(aDate);
}

/**
 * Sets up menuitems for the views
 */
function initializeViews() {
    // Set up the checkbox variables. Do not use the typical toggle*() functions
    // because those will actually toggle the current value.
    const kWorkdaysCommand = "calendar_toggle_workdays_only_command";
    const kTasksInViewCommand = "calendar_toggle_tasks_in_view_command";
    const kOrientation = "calendar_toggle_orientation_command";
    const kHideCompleted = "hide-completed-checkbox";
    var workdaysOnly = (document.getElementById(kWorkdaysCommand)
                                .getAttribute("checked") == "true");

    var tasksInView = (document.getElementById(kTasksInViewCommand)
                               .getAttribute("checked") == "true");
    var rotated = (document.getElementById(kOrientation)
                           .getAttribute("checked") == "true");
    var showCompleted = !document.getElementById(kHideCompleted).checked;

    var deck = getViewDeck();
    for each (var view in deck.childNodes) {
        view.workdaysOnly = workdaysOnly;
        view.tasksInView = tasksInView;
        view.showCompleted = showCompleted;
        view.rotated = rotated;
    }

    // Restore the last shown view
    var deck = getViewDeck();
    if (deck.selectedIndex < 0) {
        // No deck item was selected beforehand, default to week view.
        selectCalendarView("week");
    } else {
        var viewNode = deck.childNodes[deck.selectedIndex];
        var viewName = viewNode.id.replace(/-view/, "");
        selectCalendarView(viewName);
    }
}

/**
 *  Deletes items currently selected in the view.
 */
function deleteSelectedEvents() {
    var selectedItems = currentView().getSelectedItems({});
    calendarViewController.deleteOccurrences(selectedItems.length,
                                             selectedItems,
                                             false,
                                             false);
}

/**
 *  Edit the items currently selected in the view.
 */
function editSelectedEvents() {
    var selectedItems = currentView().getSelectedItems({});
    if (selectedItems && selectedItems.length >= 1) {
        modifyEventWithDialog(getOccurrenceOrParent(selectedItems[0]));
    }
}
