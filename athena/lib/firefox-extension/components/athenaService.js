/*
 * XPCOM component for the MIT Athena Firefox extension.
 */

// Class constructor
function AthenaService()
{
}

// Class definition
AthenaService.prototype = {

  properties: null,

  // nsISupports implementation
  QueryInterface: function(IID)
  {
    if (!IID.equals(Components.interfaces.nsIObserver) &&
        !IID.equals(Components.interfaces.nsISupports))
      throw Components.results.NS_ERROR_NO_INTERFACE;
    return this;
  },

  // nsIObserver implementation 
  observe: function(subject, topic, data)
  {
    switch (topic) {
    case "app-startup":
      // Set up to receive notifications when the profile changes
      // (including when it is set at startup) and at XPCOM shutdown.
      var observerService =
        Components.classes['@mozilla.org/observer-service;1']
          .getService(Components.interfaces.nsIObserverService);
      observerService.addObserver(this, "profile-after-change", false);
      observerService.addObserver(this, "xpcom-shutdown", false);

      const propertiesURI = "chrome://athena/content/athena.properties";
      var stringBundleService =
        Components.classes["@mozilla.org/intl/stringbundle;1"]
          .getService(Components.interfaces.nsIStringBundleService);
      properties = stringBundleService.createBundle(propertiesURI);
      break;

    case "xpcom-shutdown":
      // The application is exiting.  Remove observers added in
      // "app-startup".
      var observerService =
        Components.classes['@mozilla.org/observer-service;1']
          .getService(Components.interfaces.nsIObserverService);
      observerService.removeObserver(this, "xpcom-shutdown");
      observerService.removeObserver(this, "profile-after-change");
      break;

    case "profile-after-change":
      // This is called when the profile is set at startup, and when
      // a profile change is requested.  Perform our customizations.
      this.customize();
      break;

    default:
      throw Components.Exception("Unknown topic: " + topic);
    }
  },

  customize: function()
  {
    this.setLocalDiskCache();
    this.addCert();
  },

  // If the profile directory is in AFS, set the disk cache to be on
  // local disk (by a default preference).
  setLocalDiskCache: function()
  {
    // We will test if the normalized profile directory path begins with
    // "/afs/".
    const AFSPathRE = /^\/afs\//;
    var prefbranch =
      Components.classes["@mozilla.org/preferences-service;1"]
        .getService(Components.interfaces.nsIPrefBranch);
    
    // Get the profile directory.
    var profileDirectory;
    try {
      profileDirectory =
        Components.classes["@mozilla.org/file/directory_service;1"]
          .getService(Components.interfaces.nsIProperties)
            .get("ProfD", Components.interfaces.nsIFile);
    } catch(e) {
      Components.utils.reportError(e);
      return;
    }

    // Resolve symlinks.
    profileDirectory.normalize();

    if (AFSPathRE.test(profileDirectory.path)) {
      // The profile dir is in AFS.  Set the default cache directory
      // to be under /var/tmp.  Generate a unique directory name based
      // on the user name and profile directory leaf.
      const parentDir = "/var/tmp";
      var parent = Components.classes["@mozilla.org/file/local;1"]
        .createInstance(Components.interfaces.nsILocalFile);
      parent.initWithPath(parentDir);
      if (parent.isDirectory()) {
        var userInfo =
          Components.classes["@mozilla.org/userinfo;1"]
            .getService(Components.interfaces.nsIUserInfo);
        var userName = userInfo.username;
        if (userName && userName.length) {
          var localCacheDir = parentDir + "/Mozilla-Firefox-" + userName
            + "/" + profileDirectory.leafName;
          var pref =
            Components.classes["@mozilla.org/preferences;1"]
              .getService(Components.interfaces.nsIPref);
          pref.SetDefaultCharPref("browser.cache.disk.parent_directory",
                                  localCacheDir);
        }
      }
    }
  },

  addCert: function()
  {
    var certDB =
      Components.classes["@mozilla.org/security/x509certdb;1"]
        .getService(Components.interfaces.nsIX509CertDB2);

    // Loop over all certficates specified in the properties file.
    for (var i = 1; ; i++) {
      try {
        certName = properties.GetStringFromName("Cert" + i);
      } catch(e) {
        // End of list.
        break;
      }
      // Treat any null setting as the end of list.
      if (!(certName && certName.length))
        break;

      // Set up to read the certificate.
      var ioService =
        Components.classes["@mozilla.org/network/io-service;1"]
          .getService(Components.interfaces.nsIIOService);
      var channel = ioService.newChannel("chrome://athena/content/" +
                                         certName,
                                         null, null);
      var input = channel.open();
      var stream =
        Components.classes["@mozilla.org/scriptableinputstream;1"]
          .getService(Components.interfaces.nsIScriptableInputStream);
      stream.init(input);

      // Read the certificate as a Base 64 string; remove header/trailer
      // and newlines.
      var certBase64 = stream.read(input.available());
      stream.close();
      input.close();
      certBase64 = certBase64.replace(/-----BEGIN CERTIFICATE-----/, "");
      certBase64 = certBase64.replace(/-----END CERTIFICATE-----/, "");
      certBase64 = certBase64.replace(/[\r\n]/g, "");

      // Get the trust setting for this cert; default is to trust
      // for all purposes.  Note that there is a bug in firefox 1.5.0.2
      // where the addCertFromBase64() method ignores the trust setting
      // passed to it.  See:
      // https://bugzilla.mozilla.org/show_bug.cgi?id=333767
      try {
        certTrust = properties.GetStringFromName("CertTrust" + i);
      }
      catch(e) {
        certTrust = "C,C,C";
      }

      // Add the cert to the database.  Note that the addCertFromBase64()
      // implementation currently checks whether the cert already exists;
      // there does not seem to be a good way to check for that here.
      certDB.addCertFromBase64(certBase64, certTrust, "");
    }
  },

  // For debugging
  get wrappedJSObject() {
    return this;
  }

}

// XPCOM component scaffolding

const SERVICE_NAME="Athena Service";
const SERVICE_ID="{2276de48-911b-4acd-8b16-ef017b3eecad}";
const SERVICE_CONTRACT_ID = "@mit.edu/athena-service;1";
const SERVICE_CONSTRUCTOR=AthenaService;

const SERVICE_CID = Components.ID(SERVICE_ID);

var AthenaServiceFactory = {
  createInstance: function(outer, IID)
  {
    if (outer != null)
      throw Components.results.NS_ERROR_NO_AGGREGATION;

    return (new AthenaService()).QueryInterface(IID);
  }
};

// Module definition, XPCOM registration
var AthenaServiceModule = {
  _firstTime: true,

  registerSelf: function(compMgr, fileSpec, location, type)
  {
    if (this._firstTime) {
      // Register the component.
      var compReg =
        compMgr.QueryInterface(Components.interfaces.nsIComponentRegistrar);
      compReg.registerFactoryLocation(SERVICE_CID, SERVICE_NAME,
                                      SERVICE_CONTRACT_ID, fileSpec,
                                      location, type);

      // Set the component to receive app-startup notification.
      var catMgr =
        Components.classes["@mozilla.org/categorymanager;1"]
          .getService(Components.interfaces.nsICategoryManager);
      catMgr.addCategoryEntry("app-startup", SERVICE_NAME,
                              "service," + SERVICE_CONTRACT_ID,
                              true, true, null);

      this._firstTime = false;
    }
  },

  unregisterSelf: function(compMgr, location, type)
  {
    // Unregister.
    var compReg =
      compMgr.QueryInterface(Components.interfaces.nsIComponentRegistrar);
    compReg.unregisterFactoryLocation(SERVICE_CID, location);

    // Remove app-startup notification.
    var catMgr =
      Components.classes["@mozilla.org/categorymanager;1"]
        .getService(Components.interfaces.nsICategoryManager);
    catMgr.deleteCategoryEntry("app-startup", SERVICE_NAME, true);
  },

  getClassObject: function(compMgr, CID, IID)
  {
    if (!IID.equals(Components.interfaces.nsIFactory))
      throw Components.results.NS_ERROR_NOT_IMPLEMENTED;

    if (CID.equals(SERVICE_CID))
      return AthenaServiceFactory;

    throw Components.results.NS_ERROR_NO_INTERFACE;
  },

  canUnload: function(compMgr)
  {
    return true;
  }
};

// Module initialization
// This function is called when the application registers the component.
function NSGetModule(compMgr, fileSpec)
{
  return AthenaServiceModule;
}
