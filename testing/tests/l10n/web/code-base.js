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
 * The Original Code is Mozilla.
 *
 * The Initial Developer of the Original Code is
 * Mozilla Foundation.
 * Portions created by the Initial Developer are Copyright (C) 2006
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *  Axel Hecht <axel@pike.org>
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

var controller = {}, view = {};

const kModules = ['browser','mail','toolkit'];
const Tier1 = ['en-GB', 'fr', 'de', 'ja', 'ja-JP-mac', 'pl', 'es-ES'];
const Tier2 = ['zh-CN', 'zh-TW', 'cs', 'da', 'nl', 'fi', 'hu', 'it', 'ko', 'pt-BR', 'pt-PT', 'ru', 'es-AR', 'sv-SE', 'tr'];

tierMap = {};
for each (var loc in Tier1) tierMap[loc] = 'Tier-1';
for each (var loc in Tier2) tierMap[loc] = 'Tier-2';


function keys(obj) {
  var k = [];
  for (var f in obj) k.push(f);
  k.sort();
  return k;
}

function JSON_c() {
  this._requests = [];
  this._getRequest = function() {
    for each (var req in this._requests) {
      if (!req.status)
        return req;
    }
    req = new XMLHttpRequest();
    this._requests.push(req);
    return req;
  };
};
JSON_c.prototype = {
  get : function(aURL, aCallback) {
    var _t = this;
    var req = this._getRequest();
    req.overrideMimeType("text/plain"); // don't parse XML, this is js
    req.onerror = function(event) {
      _t.handleFailure(event.target);
      event.stopPropagation();
    };
    req.onload = function(event) {
      _t.handleSuccess(event.target);
      event.stopPropagation();
    };
    if (aCallback) {
      req.callback = aCallback;
    }
    req.open("GET", aURL, true);
    try {
      req.send(null);
    } catch (e) {
      // work around send sending errors, but not the callback
      controller.handleFailure(req);
    }
    return req;
  },
  // callbacks
  handleSuccess : function(o) {
    if (o.responseText === undefined)
      throw "expected response text in handleSuccess";
    if (o.callback) {
      var obj = eval('('+o.responseText+')');
      o.callback(obj);
      delete o.callback;
    }
  },
  handleFailure : function(o) {
    if (o.callback) {
      o.callback.handleFailure();
      delete o.callback;
      return;
    }
    throw("load failed with " + o.status + " " + o.statusText);
  }
};
var JSON = new JSON_c();


baseView = {
  init: function() {
    this.content = document.getElementById('content');
    var checks = document.getElementById('checks');
    checks.addEventListener("click", view.onClickDisplay, true);
    for (var i = 0; i < checks.childNodes.length; i++) {
      if (checks.childNodes[i] instanceof HTMLInputElement) {
        this.setTierDisplay(checks.childNodes[i]);
      }
    }
  },
  showHeader: function() {
    document.getElementById('head').innerHTML='<td class="locale">Locale</td>';
  },
  updateView: function(aLocales, aClosure) {
    YAHOO.widget.Logger.log('updateView called');
    this.showHeader();
    var oldIndex = 0;
    var selection = null;
    for each (loc in aLocales) {
      var id = "row-" + loc;
      var oldChild = this.content.childNodes[oldIndex++];
      var t;
      if (oldChild) {
        t = oldChild;
        t.innerHTML = '';
        if (t.id != id) {
          // we don't have the right row, make sure to remove the ID
          // from the pre-existing one, just in case
          var obsRow = document.getElementById(id);
          if (obsRow) {
            obsRow.id = '';
          }
        }
      }
      else {
        t = document.createElement("tr");
        this.content.appendChild(t);
      }
      t.id = id;
      t.className = '';
      if (tierMap[loc])
        t.className += ' ' + tierMap[loc];
      else
        t.className += ' Tier-3';
      t.innerHTML = '<td class="locale">' + loc + '</td>';
      t.appendChild(controller.getContent(loc));
    };
    while ((oldChild = this.content.childNodes[oldIndex])) {
      this.content.removeChild(oldChild);
    }
  },
  addTab: function(aName, aKey) {
    var tab = document.createElement('li');
    tab.textContent = aName;
    tab.id = '_tab_' + aKey;
    document.getElementById('tabs').appendChild(tab);
    tab.onclick = function(event){return controller.select(aKey);};
  },
  selectTab: function(aKey) {
    var tabs = document.getElementById('tabs');
    var id = '_tab_' + aKey;
    for (var i=0; i < tabs.childNodes.length; i++) {
      var tab = tabs.childNodes[i];
      var toBeSelected = tab.id == id;
      if (toBeSelected) {
        tab.setAttribute('selected','selected');
      }
      else {
        tab.removeAttribute('selected');
      }
    }
    return true;
  },
  onClickDisplay: function(event) {
    if (event.target.localName != 'INPUT') {
      return false;
    }
    var handler = function() {return function(){view.setTierDisplay(event.target)}};
    window.setTimeout(handler(),10);
    return true;
  },
  setTierDisplay: function(aTier, aTruth) {
    if (aTruth === undefined && aTier instanceof HTMLInputElement) {
      aTruth = aTier.checked;
    }
    var disable = aTruth;
    var name = aTier;
    if (aTier instanceof HTMLInputElement) {
      name = aTier.name;
    }
    var id = 'style-' + name;
    document.getElementById(id).disabled = disable;
    return false;
  },
  setTag: function(aTag) {
    document.getElementById('tag-view').value = aTag;
  },
  getCell: function() {
    return document.createElement('td');
  },
  getClass: function(aLoc) {
    if (tierMap[aLoc]) {
      return tierMap[aLoc];
    }
    return 'Tier-3';
  }
};

baseController = {
  _l: [],
  _c: null,
  _pending: [],
  _target: null,
  _tag: '.',
  beforeUnSelect: function(){},
  beforeSelect: function(){},
  get locales() {
    return this._l;
  },
  get tag() {
    return baseController._tag;
  },
  set tag(aTag) {
    baseController._tag = aTag;
    view.setTag(aTag);
  },
  addLocales: function(lst) {
    var nl = lst.filter(function(el,i,a) {return this._l.indexOf(el) < 0;}, this);
    this._l = this._l.concat(nl);
    this._l.sort();
  },
  select: function(aKey) {
    if (this._target == aKey) {
      return;
    }
    if (this._target)
      this._c[this._target].controller.beforeUnSelect();
    this._target = aKey;
    var nCV = this._c[this._target];
    view = nCV.view;
    controller = nCV.controller;
    controller.beforeSelect();
    controller._target = aKey;
    view.selectTab(aKey);
    this.delay(controller, controller.showView, []);
  },
  showView: function(aClosure) {
    view.updateView(controller.locales, aClosure);
  },
  getContent: function(aLoc) {
    if (! this._target) return;
    return this._c[this._target].getContent(aLoc);
  },
  addPane: function(aName, aKey, aController, aView) {
    if (! this._c) {
      this._pending.push([aName, aKey, aController, aView]);
      return;
    }
    view.addTab(aName, aKey);
    this._c[aKey] = {controller: aController, view: aView};
  },
  loaded: function() {
    this._c = {};
    for each (var c in this._pending) {
      this.addPane.apply(this, c);
    }
    this.select(this._pending[0][1]);
    delete this._pending;
  },
  delay: function(context, f, args) {
    window.setTimeout(function(){f.apply(context,args);}, 10);
  }
};

var outlineController = {
  __proto__: baseController,
  getHeader: function() {
    var h = document.createElement('th');
    h.textContent = 'Details';
    return h;
  },
  getContent: function(aLoc) {
    var cell = view.getCell();
    cell.innerHTML = 'Details for ' + aLoc;
    return cell;
  }
};
var outlineView = {
  __proto__: baseView
};
//baseController.addPane('Outline', 'outline', outlineController, outlineView);

view = baseView;
controller = baseController;
