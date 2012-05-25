/*
 * XPCOM component for the MIT Athena Firefox extension.
 */

// XPCOM component scaffolding

const SERVICE_NAME="Athena Service";
const SERVICE_ID="{2276de48-911b-4acd-8b16-ef017b3eecad}";
const SERVICE_CONTRACT_ID = "@mit.edu/athena-service;1";
const SERVICE_CONSTRUCTOR=AthenaService;

const SERVICE_CID = Components.ID(SERVICE_ID);

Components.utils.import("resource://gre/modules/XPCOMUtils.jsm");

// Class constructor
function AthenaService()
{
}

// Class definition
AthenaService.prototype = {

  // classDescription and contractID needed for Gecko 1.9 only
  classDescription: SERVICE_NAME,
  classID: SERVICE_CID,
  contractID: SERVICE_CONTRACT_ID,
  QueryInterface: XPCOMUtils.generateQI([Components.interfaces.nsIObserver]),
  properties: null,

  // Needed for Gecko 1.9 only.  categories are specified in
  // chrome.manifest for Gecko 2.0
  _xpcom_categories: [ { category: "app-startup", service: true } ],

  // nsIObserver implementation 
  observe: function(subject, topic, data)
  {
    const propertiesURI = "chrome://athena/content/athena.properties";
    switch (topic) {
    case "app-startup":
      // Set up to receive notifications when the profile changes
      // (including when it is set at startup) and at XPCOM shutdown.
      // Only used in Gecko 1.9
      var observerService =
        Components.classes['@mozilla.org/observer-service;1']
          .getService(Components.interfaces.nsIObserverService);
      observerService.addObserver(this, "profile-after-change", false);
      observerService.addObserver(this, "xpcom-shutdown", false);

      break;

    case "xpcom-shutdown":
      // The application is exiting.  Remove observers added in
      // "app-startup".
      // Only used in Gecko 1.9
      var observerService =
        Components.classes['@mozilla.org/observer-service;1']
          .getService(Components.interfaces.nsIObserverService);
      observerService.removeObserver(this, "xpcom-shutdown");
      observerService.removeObserver(this, "profile-after-change");
      break;

    case "profile-after-change":
      // This is called when the profile is set at startup, and when
      // a profile change is requested.  Perform our customizations.
      var stringBundleService =
        Components.classes["@mozilla.org/intl/stringbundle;1"]
          .getService(Components.interfaces.nsIStringBundleService);
      properties = stringBundleService.createBundle(propertiesURI);
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

if (XPCOMUtils.generateNSGetFactory) {
  // Gecko 2.0 (FF 4 and higher)
  var NSGetFactory = XPCOMUtils.generateNSGetFactory([AthenaService]);
} else {
  // Gecko 1.9 (FF 3)
  var NSGetModule = XPCOMUtils.generateNSGetModule([AthenaService]);
}

