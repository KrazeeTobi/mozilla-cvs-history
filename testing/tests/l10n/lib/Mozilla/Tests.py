# ***** BEGIN LICENSE BLOCK *****
# Version: MPL 1.1/GPL 2.0/LGPL 2.1
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License. You may obtain a copy of the License at
# http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Original Code is l10n test automation.
#
# The Initial Developer of the Original Code is
# Mozilla Foundation
# Portions created by the Initial Developer are Copyright (C) 2006
# the Initial Developer. All Rights Reserved.
#
# Contributor(s):
#	Axel Hecht <l10n@mozilla.com>
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 2 or later (the "GPL"), or
# the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
# in which case the provisions of the GPL or the LGPL are applicable instead
# of those above. If you wish to allow use of your version of this file only
# under the terms of either the GPL or the LGPL, and not to allow others to
# use your version of this file under the terms of the MPL, indicate your
# decision by deleting the provisions above and replace them with the notice
# and other provisions required by the GPL or the LGPL. If you do not delete
# the provisions above, a recipient may use your version of this file under
# the terms of any one of the MPL, the GPL or the LGPL.
#
# ***** END LICENSE BLOCK *****

import logging
import Paths
import Parser

class Base:
  '''Base class for all tests'''
  def __init__(self):
    if not hasattr(self, 'leafName'):
      self.leafName = 'TODO'
      logging.warning(' ' + self + ' does not have leafName set, writing to TODO')
  def run(self):
    '''Run this test for all locales, returns a dictionary with results'''
    pass
  def serialize(self, result, saveHandler):
    '''Serialize the previously generated result, writes the dictionary
    by default'''
    return saveHandler(result, self.leafName)
    pass
  def failureTest(self, myResult, failureResult):
    pass

import CompareLocales
class CompareTest(Base):
  '''Test class to compare locales'''
  def __init__(self):
    '''Initializes the test object'''
    # nothing to be done here
    pass
  def run(self):
    '''Runs CompareLocales.compare()'''
    return CompareLocales.compare()
  def serialize(self, result, saveHandler):
    '''Serialize the CompareLocales result by locale into
      cmp-details-ab-CD
    and a compacted version into
      cmp-data

    '''
    class Separator:
      def __init__(self):
        self.leafBase = 'cmp-details-'
      def getDetails(self, res, locale):
        dic = {}
        res[locale]['tested'].sort()
        self.collectList('missing', res[locale], dic)
        self.collectList('obsolete', res[locale], dic)
        saveHandler(dic, self.leafBase + locale + '.json')
      def collectList(self, name, res, dic):
        dic[name] = {}
        if not res.has_key(name):
          res[name] = []
        counts = dict([(mod,0) for mod in res['tested']])
        counts['total'] = len(res[name])
        for mod, path, key in res[name]:
          counts[Paths.components[mod]] +=1
          if not dic[name].has_key(mod):
            dic[name][mod] = {path:[key]}
            continue
          if not dic[name][mod].has_key(path):
            dic[name][mod][path] = [key]
          else:
            dic[name][mod][path].append(key)
        res[name] = counts
        name += 'Files'
        dic[name] = {}
        if not res.has_key(name):
          res[name] = []
        counts = dict([(mod,0) for mod in res['tested']])
        counts['total'] = len(res[name])
        for mod, path in res[name]:
          counts[Paths.components[mod]] +=1
          if not dic[name].has_key(mod):
            dic[name][mod] = [path]
          else:
            dic[name][mod].append(path)
        res[name] = counts
    
    s = Separator()
    for loc, lResult in result.iteritems():
      s.getDetails(result, loc)
    
    saveHandler(result, 'cmp-data.json')
    
  def failureTest(self, myResult, failureResult):
    '''signal pass/warn/failure for each locale'''
    def sumdata(data, part):
      res = 0
      for mod in [u'browser', u'toolkit']:
        res += data[part][mod] + data[part + u'Files'][mod]
      return res
    def getState(data):
      ret = 0
      if sumdata(data, u'obsolete') > 0:
        ret |= 1
      if sumdata(data, u'missing') > 0:
        ret |= 2
      return ret
    for loc, val in myResult.iteritems():
      if not failureResult.has_key(loc):
        failureResult[loc] = getState(val)
      else:
        failureResult |= getState(val)

from xml import sax
from types import DictType, FunctionType
import md5
from codecs import utf_8_encode
import re
import os

class SearchTest(Base):
  """Test class to collect information from search plugins and to
  verify that they're doing something good.

  """
  def __init__(self):
    '''Set up the test class with a good leaf name'''
    self.leafName = 'search-results.json'
    pass
  def run(self):
    '''Collect all data from the MozSearch plugins in both the /cvsroot
    repository for en-US and the /l10n repository for locales

    '''
    class DummyHandler(sax.handler.ContentHandler):
      def startDocument(self):
        self.md5 = md5.new()
        self.indent = ''
        self.engine = {'urls':[]}
        self.lNames = []
        self.textField = None
        return
      def endDocument(self):
        self.engine['md5'] = self.md5.hexdigest()
        return
      def startElementNS(self, (ns, local), qname, attrs):
        self.indent += '  '
        if ns != u'http://www.mozilla.org/2006/browser/search/':
          raise UserWarning, ('bad namespace: ' + ns)
        self.lNames.append(local)
        handler = self.getOpenHandler()
        if handler:
          handler(self, attrs)
        self.update(ns+local)
        for qna in attrs.getQNames():
          self.update(qna[0] + qna[1] + attrs.getValueByQName(qna))
        return
      def endElementNS(self, (ns, local), qname):
        self.lNames.pop()
        self.indent = self.indent[0:-2]
      def characters(self, content):
        self.update(content)
        if not self.textField:
          return
        self.engine[self.textField] = content
        self.textField = None
      def update(self, content):
        self.md5.update(utf_8_encode(content)[0])
      def openURL(self, attrs):
        entry = {'params':{},
                 'type': attrs.getValueByQName(u'type'),
                 'template': attrs.getValueByQName(u'template')}
        self.engine['urls'].append(entry)
      def handleParam(self, attrs):
        try:
          self.engine['urls'][-1]['params'][attrs.getValueByQName(u'name')] = attrs.getValueByQName(u'value')
        except KeyError:
          raise UserWarning, 'bad param'
        return
      def handleMozParam(self, attrs):
        try:
          self.engine['urls'][-1]['MozParams'] = {
            'name': attrs.getValueByQName(u'name'),
            'condition': attrs.getValueByQName(u'condition'),
            'trueValue': attrs.getValueByQName(u'trueValue'),
            'falseValue': attrs.getValueByQName(u'falseValue')}
        except KeyError:
          raise UserWarning, 'bad mozParam'
        return
      def handleShortName(self, attrs):
        self.textField = 'ShortName'
        return
      def handleImage(self, attrs):
        self.textField = 'Image'
        return
      def getOpenHandler(self):
        return self.getHandler(DummyHandler.openHandlers)
      def getHandler(self, handlers):
        for local in self.lNames:
          if type(handlers) != DictType or not handlers.has_key(local):
            return
          handlers = handlers[local]
        if handlers.has_key('_handler'):
          return handlers['_handler']
        return
      openHandlers = {'SearchPlugin':
                      {'ShortName': {'_handler': handleShortName},
                       'Image': {'_handler': handleImage},
                       'Url':{'_handler': openURL,
                              'Param': {'_handler':handleParam},
                              'MozParam': {'_handler':handleMozParam}
                              }
                       }
                      }
    
    handler = DummyHandler()
    parser = sax.make_parser()
    parser.setContentHandler(handler)
    parser.setFeature(sax.handler.feature_namespaces, True)
    
    locales = [loc.strip() for loc in open('mozilla/browser/locales/all-locales')]
    locales.insert(0, 'en-US')
    sets = {}
    details = {}
    
    for loc in locales:
      try:
        lst = open(Paths.get_path('browser',loc,'searchplugins/list.txt'),'r')
      except IOError:
        logging.error("Locale " + loc + " doesn't have search plugins")
        details[Paths.get_path('browser',loc,'searchplugins/list.txt')] = {
          'error': 'not found'
          }
        continue
      sets[loc] = {'list': []}
      regprop = Paths.get_path('browser', loc, 'chrome/browser-region/region.properties')
      p = Parser.getParser(regprop)
      p.read(regprop)
      orders = {}
      for key, val in p:
        m = re.match('browser.search.order.([1-9])', key)
        if m:
          orders[val.strip()] = int(m.group(1))
      sets[loc]['orders'] = orders
      for fn in lst:
        name = fn.strip()
        leaf = 'searchplugins/' + name + '.xml'
        _path = Paths.get_path('browser','en-US', leaf)
        if not os.access(_path, os.R_OK):
          _path = Paths.get_path('browser', loc, leaf)
        logging.info('testing ' + _path)
        sets[loc]['list'].append(_path)
        try:
          parser.parse(_path)
        except IOError:
          logging.error("can't open " + _path)
          details[_path] = {'_name': name, 'error': 'not found'}
          continue
        except UserWarning, ex:
          logging.error("error in searchplugin " + _path)
          details[_path] = {'_name': name, 'error': ex.args[0]}
          continue
        except sax._exceptions.SAXParseException, ex:
          logging.error("error in searchplugin " + _path)
          details[_path] = {'_name': name, 'error': ex.args[0]}
          continue
        details[_path] = handler.engine
        details[_path]['_name'] = name
    
    engines = {'locales': sets,
      'details': details}
    return engines
  
