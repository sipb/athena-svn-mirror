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
 * The Original Code is the layout debugging UI.
 *
 * The Initial Developer of the Original Code is L. David Baron.
 * Portions created by the Initial Developer are Copyright (C) 2002
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   L. David Baron <dbaron@dbaron.org> (original author)
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

var gBrowser;
var gProgressListener;
var gDebugger;
var gRTestIndexList;
var gRTestURLList = null;

const nsILayoutDebuggingTools = Components.interfaces.nsILayoutDebuggingTools;
const nsIDocShell = Components.interfaces.nsIDocShell;
const nsIWebProgressListener = Components.interfaces.nsIWebProgressListener;

const NS_LAYOUT_DEBUGGINGTOOLS_CONTRACTID = "@mozilla.org/layout-debug/layout-debuggingtools;1";

function nsLDBBrowserContentListener()
{
  this.init();
}

nsLDBBrowserContentListener.prototype = {

  init : function()
    {
      this.mStatusText = document.getElementById("status-text");
      this.mURLBar = document.getElementById("urlbar");
      this.mForwardButton = document.getElementById("forward-button");
      this.mBackButton = document.getElementById("back-button");
      this.mStopButton = document.getElementById("stop-button");
    },

  QueryInterface : function(aIID)
    {
      if (aIID.equals(Components.interfaces.nsIWebProgressListener) ||
          aIID.equals(Components.interfaces.nsISupportsWeakReference) ||
          aIID.equals(Components.interfaces.nsISupports))
        return this;
      throw Components.results.NS_NOINTERFACE;
    },

  // nsIWebProgressListener implementation
  onStateChange : function(aWebProgress, aRequest, aStateFlags, aStatus)
    {
      if (aStateFlags & nsIWebProgressListener.STATE_START) {
        this.setButtonEnabled(this.mStopButton, true);
        this.setButtonEnabled(this.mForwardButton, gBrowser.canGoForward);
        this.setButtonEnabled(this.mBackButton, gBrowser.canGoBack);
        
        this.mLoading = true;
      } else if (aStateFlags & nsIWebProgressListener.STATE_STOP) {
        this.setButtonEnabled(this.mStopButton, false);
        this.mStatusText.value = "";

        if (gRTestURLList && this.mLoading) {
          // Let other things happen in the first 20ms, since this
          // doesn't really seem to be when the page is done loading.
          setTimeout("gRTestURLList.doneURL()", 20);
        }
        this.mLoading = false;
      }
    },

  onProgressChange : function(aWebProgress, aRequest,
                              aCurSelfProgress, aMaxSelfProgress,
                              aCurTotalProgress, aMaxTotalProgress)
    {
    },

  onLocationChange : function(aWebProgress, aRequest, aLocation)
    {
      this.mURLBar.value = aLocation.spec;
    },

  onStatusChange : function(aWebProgress, aRequest, aStatus, aMessage)
    {
      this.mStatusText.value = aMessage;
    },

  onSecurityChange : function(aWebProgress, aRequest, aState)
    {
    },

  // non-interface methods
  setButtonEnabled : function(aButtonElement, aEnabled)
    {
      if (aEnabled)
        aButtonElement.removeAttribute("disabled");
      else
        aButtonElement.setAttribute("disabled", "true");
    },

  mStatusText : null,
  mURLBar : null,
  mForwardButton : null,
  mBackButton : null,
  mStopButton : null,

  mLoading : false,

}

function OnLDBLoad()
{
  gBrowser = document.getElementById("browser");

  gProgressListener = new nsLDBBrowserContentListener();
  gBrowser.addProgressListener(gProgressListener);

  gDebugger = Components.classes[NS_LAYOUT_DEBUGGINGTOOLS_CONTRACTID].
                  createInstance(nsILayoutDebuggingTools);

  if (window.arguments && window.arguments[0])
    gBrowser.loadURI(window.arguments[0]);
  else
    gBrowser.goHome();

  // XXX Shouldn't this be gBrowser.contentWindow?
  var win = gBrowser.docShell.getInterface(Components.interfaces.nsIDOMWindow);
  gDebugger.init(win);

  checkPersistentMenus();
  gRTestIndexList = new RTestIndexList();
}

function checkPersistentMenu(item)
{
  var menuitem = document.getElementById("menu_" + item);
  menuitem.setAttribute("checked", gDebugger[item]);
}

function checkPersistentMenus()
{
  // Restore the toggles that are stored in prefs.
  checkPersistentMenu("paintFlashing");
  checkPersistentMenu("paintDumping");
  checkPersistentMenu("invalidateDumping");
  checkPersistentMenu("eventDumping");
  checkPersistentMenu("motionEventDumping");
  checkPersistentMenu("crossingEventDumping");
  checkPersistentMenu("reflowCounts");
}


function OnLDBUnload()
{
  gBrowser.removeProgressListener(gProgressListener);
}

function toggle(menuitem)
{
  // trim the initial "menu_"
  var feature = menuitem.id.substring(5);
  gDebugger[feature] = menuitem.getAttribute("checked") == "true";
}

const LDB_RDFNS = "http://mozilla.org/newlayout/LDB-rdf#";
const NC_RDFNS = "http://home.netscape.com/NC-rdf#";

function RTestIndexList() {
  this.init();
}

RTestIndexList.prototype = {

  init : function()
    {
      const nsIPrefService = Components.interfaces.nsIPrefService;
      const PREF_SERVICE_CONTRACTID = "@mozilla.org/preferences-service;1";
      const PREF_BRANCH_NAME = "layout_debugger.rtest_url.";
      const nsIRDFService = Components.interfaces.nsIRDFService;
      const RDF_SERVICE_CONTRACTID = "@mozilla.org/rdf/rdf-service;1";
      const nsIRDFDataSource = Components.interfaces.nsIRDFDataSource;
      const RDF_DATASOURCE_CONTRACTID =
          "@mozilla.org/rdf/datasource;1?name=in-memory-datasource";

      this.mPrefService = Components.classes[PREF_SERVICE_CONTRACTID].
                              getService(nsIPrefService);
      this.mPrefBranch = this.mPrefService.getBranch(PREF_BRANCH_NAME);

      this.mRDFService = Components.classes[RDF_SERVICE_CONTRACTID].
                             getService(nsIRDFService);
      this.mDataSource = Components.classes[RDF_DATASOURCE_CONTRACTID].
                             createInstance(nsIRDFDataSource);

      this.mLDB_Root = this.mRDFService.GetResource(LDB_RDFNS + "Root");
      this.mNC_Name = this.mRDFService.GetResource(NC_RDFNS + "name");
      this.mNC_Child = this.mRDFService.GetResource(NC_RDFNS + "child");

      this.load();

      document.getElementById("menu_RTest_baseline").database.
          AddDataSource(this.mDataSource);
      document.getElementById("menu_RTest_verify").database.
          AddDataSource(this.mDataSource);
      document.getElementById("menu_RTest_remove").database.
          AddDataSource(this.mDataSource);
    },

  save : function()
    {
      this.mPrefBranch.deleteBranch("");

      const nsIRDFLiteral = Components.interfaces.nsIRDFLiteral;
      const nsIRDFResource = Components.interfaces.nsIRDFResource;
      var etor = this.mDataSource.GetTargets(this.mLDB_Root,
                                             this.mNC_Child, true);
      var i = 0;
      while (etor.hasMoreElements()) {
        var resource = etor.getNext().QueryInterface(nsIRDFResource);
        var literal = this.mDataSource.GetTarget(resource, this.mNC_Name, true);
        literal = literal.QueryInterface(nsIRDFLiteral);
        this.mPrefBranch.setCharPref(i.toString(), literal.Value);
        ++i;
      }

      this.mPrefService.savePrefFile(null);
    },

  load : function()
    {
      var count = {value:null};
      var prefList = this.mPrefBranch.getChildList("", count);

      var i = 0;
      for (var pref in prefList) {
        var file = this.mPrefBranch.getCharPref(pref);
        var resource = this.mRDFService.GetResource(file);
        var literal = this.mRDFService.GetLiteral(file);
        this.mDataSource.Assert(this.mLDB_Root, this.mNC_Child, resource, true);
        this.mDataSource.Assert(resource, this.mNC_Name, literal, true);
        ++i;
      }

    },

  /* Add a new list of regression tests to the menus. */
  add : function()
    {
      const nsIFilePicker = Components.interfaces.nsIFilePicker;
      const NS_FILEPICKER_CONTRACTID = "@mozilla.org/filepicker;1";

      var fp = Components.classes[NS_FILEPICKER_CONTRACTID].
                   createInstance(nsIFilePicker);

      // XXX l10n (but this is just for 5 developers, so no problem)
      fp.init(window, "New Regression Test List", nsIFilePicker.modeOpen);
      fp.appendFilters(nsIFilePicker.filterAll);
      fp.defaultString = "rtest.lst";
      if (fp.show() != nsIFilePicker.returnOK)
        return;

      var file = fp.file.persistentDescriptor;
      var resource = this.mRDFService.GetResource(file);
      var literal = this.mRDFService.GetLiteral(file);
      this.mDataSource.Assert(this.mLDB_Root, this.mNC_Child, resource, true);
      this.mDataSource.Assert(resource, this.mNC_Name, literal, true);

      this.save();

    },

  remove : function(file)
    {
      var resource = this.mRDFService.GetResource(file);
      var literal = this.mRDFService.GetLiteral(file);
      this.mDataSource.Unassert(this.mLDB_Root, this.mNC_Child, resource);
      this.mDataSource.Unassert(resource, this.mNC_Name, literal);

      this.save();
    },

  mPrefBranch : null,
  mPrefService : null,
  mRDFService : null,
  mDataSource : null,
  mLDB_Root : null,
  mNC_Child : null,
  mNC_Name : null,
}

const nsIFileInputStream = Components.interfaces.nsIFileInputStream;
const nsILineInputStream = Components.interfaces.nsILineInputStream;
const nsILocalFile = Components.interfaces.nsILocalFile;
const nsIFileURL = Components.interfaces.nsIFileURL;

const NS_LOCAL_FILE_CONTRACTID = "@mozilla.org/file/local;1";
const NS_STANDARDURL_CONTRACTID = "@mozilla.org/network/standard-url;1";
const NS_LOCALFILEINPUTSTREAM_CONTRACTID =
          "@mozilla.org/network/file-input-stream;1";


function RunRTest(aFilename, aIsBaseline)
{
  if (gRTestURLList) {
    // XXX Does alert work?
    alert("Already running regression test.\n");
    return;
  }

  dump("Running " + (aIsBaseline?"baseline":"verify") + " test for " + aFilename + ".\n");

  var listFile = Components.classes[NS_LOCAL_FILE_CONTRACTID].
                    createInstance(nsILocalFile);
  listFile.persistentDescriptor = aFilename;
  gRTestURLList = new RTestURLList(listFile, aIsBaseline);
  gRTestURLList.startURL();
}

function RTestURLList(aLocalFile, aIsBaseline) {
  this.init(aLocalFile, aIsBaseline);
}

RTestURLList.prototype = {
  init : function(aLocalFile, aIsBaseline)
    {
      this.mIsBaseline = aIsBaseline;
      this.mURLs = new Array();
      this.readFileList(aLocalFile);
    },

  readFileList : function(aLocalFile)
    {
      var dirURL = Components.classes[NS_STANDARDURL_CONTRACTID].
                       createInstance(nsIFileURL);
      dirURL.file = aLocalFile.parent;

      var fis = Components.classes[NS_LOCALFILEINPUTSTREAM_CONTRACTID].
                    createInstance(nsIFileInputStream);
      fis.init(aLocalFile, -1, -1, false);
      var lis = fis.QueryInterface(nsILineInputStream);

      var line = {value:null};
      while (lis.readLine(line)) {
        var str = line.value;
        str = /^[^#]*/.exec(str); // strip everything after "#"
        str = /\S*/.exec(str); // take the first chunk of non-whitespace
        if (!str || str == "")
          continue;

        var item = dirURL.resolve(str);
        if (item.match(/\/rtest.lst$/)) {
          var itemurl = Components.classes[NS_STANDARDURL_CONTRACTID].
                            createInstance(nsIFileURL);
          itemurl.spec = item;
          this.readFileList(itemurl.file);
        } else {
          this.mURLs.push(item);
        }
      }
    },

  doneURL : function() {
    // XXX Dump or compare regression test data here!

    this.mCurrentURL = null;

    this.startURL();
  },

  startURL : function() {
    this.mCurrentURL = this.mURLs.shift();
    if (!this.mCurrentURL) {
      gRTestURLList = null;
      return;
    }

    gBrowser.loadURI(this.mCurrentURL);
  },

  mURLs : null,
  mCurrentURL : null,
  mIsBaseline : null,
}