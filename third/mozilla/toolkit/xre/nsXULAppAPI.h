/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
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
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2002
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *  Brian Ryner <bryner@brianryner.com>
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

#ifndef _nsXULAppAPI_h__
#define _nsXULAppAPI_h__

#include "prtypes.h"
#include "nsCRT.h"
#include "nsString.h"

// This class holds application-specific information used to
// initialize the XUL environment.

class nsXREAppData {
public:
  nsXREAppData()
    : mUseSplash(PR_FALSE), mUseStartupPrefs(PR_FALSE), mProductName(nsnull) { }

  // Set whether the application should use a splash screen.
  // If set to true, the splash screen must be linked to the application as follows:
  //   Windows: via an IDB_SPLASH bitmap resource
  //   Unix: via an xpm named splash_xpm
  void SetSplashEnabled(PRBool aEnabled) { mUseSplash = aEnabled; }
  PRBool GetSplashEnabled() const { return mUseSplash; }

  // Set the product name for this application.
  // The product name is used for determining the profile location.
  // On Windows, profiles will be in Documents and Settings\<user>\<ProductName>
  // On Unix, profiles will be in ~/.<ProductName>
  void SetProductName(const nsACString& aName) { mProductName.Assign(aName); }
  const nsACString& GetProductName() const { return mProductName; }

  // Set whether the "general.startup.*" prefs are processed if no
  // command line arguments are given.
  void SetUseStartupPrefs(PRBool aUsePrefs) { mUseStartupPrefs = aUsePrefs; }
  PRBool GetUseStartupPrefs() const { return mUseStartupPrefs; }

private:
  PRPackedBool mUseSplash;
  PRPackedBool mUseStartupPrefs;
  nsCString mProductName;
};

// Call this function to begin execution of the XUL application.
// This function does not return until the user exits the application.
// The return code is a native result code suitable for returning from
// your main() function.

int xre_main(int argc, char* argv[], const nsXREAppData& aAppData);

#endif // _nsXULAppAPI_h__