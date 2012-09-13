import urllib
import sys
import os
import subprocess
import errno
import textwrap
import pwd
import re
from gi.repository import Nautilus, Gtk, GObject, GLib, Gdk

UI_FILE="/usr/share/debathena-nautilus-afs/afs-property-page.ui"
# Paths that are likely in AFS.  This should be an inclusive list, and
# we check for EINVAL when initially fetching the ACL.
AFS_PATHS=("/afs", "/mit")
# Valid rights for site-specific permissions
VALID_SITE_RIGHTS="ABCDEFGH"

class AFSAclException(OSError):
    def __init__(self, errnoOrMsg, message=None):
        if message is None:
            message = errnoOrMsg
            errnoOrMsg = None
        OSError.__init__(self, errnoOrMsg, message)
        self.message = message

class AFSAcl:
    # Escape the ampersand because these are tooltips and go through Pango
    # and maybe everything in Gtk3 does?
    _specialEntities = { "system:anyuser": "Any anonymous user or web browser",
                         "system:authuser": "Any MIT user",
                         "system:expunge": "The IS&amp;T automated expunger",
                         "system:administrators": "An IS&amp;T AFS administrator"}
    
    _englishRights = { "rlidwka": "all permissions",
                       "rlidwk": "write permissions",
                       "rl": "read permissions"}

    def __init__(self, path):
        if not os.path.exists(path):
            raise AFSAclException(errno.ENOENT, "That path does not exist")
        self.path = path
        self._loadAcl()
        
    def _loadAcl(self):
        fsla = subprocess.Popen(["fs", "listacl", "-path", self.path],
                                stdout=subprocess.PIPE, 
                                stderr=subprocess.PIPE)
        (out, err) = fsla.communicate()
        if fsla.returncode != 0:
            if err.startswith("fs: Invalid argument"):
                raise AFSAclException(errno.EINVAL, err.strip())
            elif err.startswith("fs: You don't have the required access rights"):
                raise AFSAclException(errno.EACCES, err.strip())
            else:
                raise AFSAclException(err.strip())
        else:
            self._parseACL(out)
    
    def _parseACL(self, fsla):
        self.pos = {}
        self.neg = {}
        lines = fsla.splitlines()
        # If a directory has no normal rights, we have no idea what's going on
        if "Normal rights:" not in lines:
            raise AFSAclException("No normal rights found while parsing?")
        posidx = lines.index("Normal rights:")
        try:
            negidx = lines.index("Negative rights:")
        except ValueError:
            negidx = None
        if negidx is None:
            self.pos = self._parseEntries(lines[posidx+1:])
        else:
            self.pos = self._parseEntries(lines[posidx+1:negidx])
            self.neg = self._parseEntries(lines[negidx+1:])

    def _parseEntries(self, entList):
        rv = {}
        for i in entList:
            (name, acl) = i.strip().split()
            rv[name] = acl
        return rv

    def _setacl(self, entity, rights, negative=False):
        cmdlist = ["fs", "setacl", "-dir", self.path, "-acl", entity, rights]
        if negative:
            cmdlist.append("-negative")
        fsla = subprocess.Popen(cmdlist,
                                stdout=subprocess.PIPE, 
                                stderr=subprocess.PIPE)
        (out, err) = fsla.communicate()
        if fsla.returncode != 0:
            if err.startswith("fs: Invalid argument"):
                raise AFSAclException(errno.EINVAL, err.strip())
            elif err.startswith("fs: You don't have the required access rights"):
                raise AFSAclException(errno.EACCES, err.strip())
            elif err.startswith("fs: You can not change a backup or readonly volume"):
                raise AFSAclException(errno.EROFS, err.strip())
            else:
                raise AFSAclException(err.strip())
    
    @classmethod
    def isDeactivatedUser(cls, ent):
        if not ent.startswith('-'):
            return False
        try:
            uid = int(ent)
            return (uid < 0)
        except ValueError:
            pass
        return False

    @classmethod
    def entityToEnglish(cls, ent):
        if ent in cls._specialEntities:
            return cls._specialEntities[ent]
        if ent.startswith('system:'):
            return "The Moira group " + ent
        try:
            pwent = pwd.getpwnam(ent)
            return "%s (%s)" % (pwent[4].split(',')[0], ent)
        except KeyError:
            pass
        return "The %suser '%s'" % ("deactivated " if cls.isDeactivatedUser(ent) else "", ent)

    @classmethod
    def rightsToEnglish(cls, rightString):
        # TODO: str.format() or Formatter
        rights=rightString
        site=re.sub(r'[rlidwka]', '', rights)
        if site:
            rights=rightString.replace(site, '')
        english=""
        for right in sorted(cls._englishRights, None, None, True):
            if right in rights:
                english += cls._englishRights[right]
                break
        if re.search(r'[idwka]', rights):
            if english and right == "rl":
                english += " and '%s' permissions" % (rights)
        if not english:
            english += "'%s' permissions" % (rights)
        if rights == "l":
            english = "permission to list, but not read, files and directories"
        if site:
            english += "\nas well as site-specific permission(s) '%s'" % (site)
        return english



class AFSPermissionsPane():
    def __init__(self, fp):
        self.filepath = fp
        self.acl = None
        self.rootWidget = None
        self.inAFS = False
        for item in AFS_PATHS:
            if self.filepath.startswith(item):
                self.inAFS=True
        self.builder = Gtk.Builder()
        ui = os.getenv('DA_AFS_PROPERTY_PAGE_UI')
        if not ui:
            ui = UI_FILE
        try:
            self.builder.add_from_file(ui)
        except GLib.GError as e:
            self.rootWidget = self._errorBox("Could not load the AFS user interface.", e.message)
            return

        try:
            self.acl = AFSAcl(self.filepath)
        except OSError as e:
            if e.errno == errno.EACCES:
                self.rootWidget = self.builder.get_object("vboxNoPerms")
                return
            elif e.errno == errno.EINVAL:
                self.inAFS = False
                return
            else:
                self.rootWidget = self._errorBox("Unexpected error!", str(e))
                return

        self.rootWidget = self.builder.get_object("vboxMain")
        handlers = {
            "add_clicked_cb": self.addEntry,
            "edit_clicked_cb": self.editEntry,
            "remove_clicked_cb": self.removeEntry,
            "access_combo_changed_cb": self.dlgAccessChanged,
            "entity_combo_changed_cb": self.dlgEntityChanged,
            "entity_text_changed_cb": self.dlgEntityTextChanged,
            "group_checkbox_toggled_cb": self.groupToggled,
            "aclview_row_activated_cb": self.editEntry,
            "siteperms_entity_insert_text_cb": self.validateSitePerms,
        }
        self.builder.connect_signals(handlers)
        self.addDlg = self.builder.get_object("aclDialog")
        self._refreshAclUI()
        
    # Used to fail gracefully when we can't find the UI.
    def _errorBox(self, msg, longMsg=None):
        vbox = Gtk.VBox(homogeneous=False, spacing=0)
        vbox.show()
        msgTxt = msg
        if longMsg is not None:
            msgTxt += "\n\nError details:\n"
            msgTxt += textwrap.fill(longMsg, 60)
        label = Gtk.Label(msgTxt)
        label.show()
        vbox.pack_start(label, True, True, 0)
        return vbox
    
    # Reload the UI based on the ACL.
    def _refreshAclUI(self):
        self.builder.get_object("lblPath").set_text(self.filepath)
        self.builder.get_object("lblPath").set_tooltip_text(self.filepath)
        self.store = self.builder.get_object("aclListStore")
        self.store.clear()
        if self.acl is not None:
            for i in self.acl.pos.items():
                self.store.append(i + (False, AFSAcl.entityToEnglish(i[0]) + " has " + AFSAcl.rightsToEnglish(i[1]), i[1], 'gtk-yes', Gtk.IconSize.MENU))
            for i in self.acl.neg.items():
                self.store.append(i + (True, AFSAcl.entityToEnglish(i[0]) + " does <b>not</b> have " + AFSAcl.rightsToEnglish(i[1]), i[1] + "  <i>(negative rights)</i>",'gtk-no', Gtk.IconSize.MENU))

    # Reload the ACL.  Note that the initial loading happens in the
    # constructor, because if there are no permissions, we just want 
    # display a vbox, not a dialog (which is obnoxious from a UI
    # perspective
    def _reloadAcl(self):
        self.acl = None
        try:
            self.acl = AFSAcl(self.filepath)
        except OSError as e:
            if e.errno == errno.EACCES:
                self._errDialog("You no longer have permissions to view the ACL for this diectory.")
            else:
                self._errDialog("Unexpected error!", "Full error text:\n" + str(e))
            # Disable the UI, we can't continue at this point
            self.rootWidget.set_sensitive(False)
        # And refresh or clear the UI
        self._refreshAclUI()

    # Get the currently selected entry
    def _getSelectedEntry(self):
        tree = self.builder.get_object("tvACL")
        model, it = tree.get_selection().get_selected()
        if it != None:
            return model[it]
        else:
            return None
    
    # Apply and ACL and deal with the UI accordingly
    def _setacl(self, entity, rights, negative=False):
        try:
            self.acl._setacl(entity, rights, negative)
        except OSError as e:
            if e.errno == errno.EACCES:
                self._errDialog("You don't have permissions to change the ACL on this directory.")
            elif e.errno == errno.EINVAL:
                self._errDialog("Error: %s\n\n(This is typically caused by specifying a user or group that doesn't exist.)" % (e.message))
            elif e.errno == errno.EROFS:
                self._errDialog("This is a read-only filesystem and cannot be changed.","(Hint: If you're trying to change your OldFiles directory,\nyou can't, because it's a nightly snapshot.)")
            else:
                self._errDialog("Unexpected error!", e.message)
        self._reloadAcl()

    # Set the "access" combobox from an ACL
    def _setAccessComboFromACL(self, acl):
        for row in self.builder.get_object("accessCombo").get_model():
            if row[0] == acl:
                self.builder.get_object("accessCombo").set_active_iter(row.iter)
                break

    # Prepare the ACL dialog based on what we want to do
    def _prepareAclDialog(self, editMode=False, entity=None, rights=None, negative=False):
        self.addDlg.set_transient_for(self._getParentWindow())
        self.addDlg.set_title("Change permissions for '%s'" % (entity) if editMode else "Add an entry")
        # Set the "OK" button to unsensitive until something is in the text field
        self.builder.get_object("aclDlgOK").set_sensitive(False)
        # Clear the text field
        self.builder.get_object("entityText").set_text("")
        # Default to "specify manually" for the ACL:
        self._setAccessComboFromACL("-" if editMode else "rl")
        # No need to call dlgAccessChanged here, because set_active_iter will
        # emit the signal.  .set-active() with a row number will not.
        self.builder.get_object("accessNegative").set_active(negative)
        sitePerms = ""
        if rights:
            sitePerms = re.sub(r'[rlidwka]', '', rights)
        self.builder.get_object("sitePerms").set_text(sitePerms)
        self.builder.get_object("sitePermsExpander").set_expanded(sitePerms != "")
        self.builder.get_object("entityCombo").set_sensitive(not editMode)
        self.builder.get_object("entityText").set_sensitive(not editMode)
        self.builder.get_object("entityIsGroup").set_sensitive(not editMode)
        self.builder.get_object("negPermsExpander").set_expanded(negative)
        # We don't support turning negative rights to positive ones
        self.builder.get_object("accessNegative").set_sensitive(not editMode)
        if editMode:
            self.dlgSetRightsFromString(rights, True)
            self.builder.get_object("entityText").set_text(entity)

    # Turn the dialog's state back into somthing that can be applied
    def _applyRightsFromDialog(self):
            entity = self.builder.get_object("entityText").get_text().strip()
            rights=""
            for widget in self.builder.get_object("rightsBox").get_children():
                rights += widget.get_label() if widget.get_active() else ""
            rights += self.builder.get_object("sitePerms").get_text().strip()
            self._setacl(entity, rights, self.builder.get_object("accessNegative").get_active())
        
    # "Add" button callback
    def addEntry(self, widget):
        self._prepareAclDialog()
        if self.addDlg.run() == Gtk.ResponseType.OK:
            self._applyRightsFromDialog()
        self.addDlg.hide()

    # "Edit" button callback
    def editEntry(self, widget, row=None, treeCol=None):
        row = self._getSelectedEntry()
        if row is None:
            return
        self._prepareAclDialog(True, row[0], row[1], row[2])
        if self.addDlg.run() == Gtk.ResponseType.OK:
            self._applyRightsFromDialog()
        self.addDlg.hide()

    # "Remove" button callback
    def removeEntry(self, widget):
        row = self._getSelectedEntry()
        if row is not None:
            if row[0] == os.getenv("USER"):
                if not self._confirmDialog("Are you sure you want to remove yourself from the ACL?"):
                    return
            self._setacl(row[0], "none", row[2])

    # Set the "rights" checkboxes from an ACL string.
    # and whether they should be enabled or disabled
    def dlgSetRightsFromString(self, rightString, enable):
        for widget in self.builder.get_object("rightsBox").get_children():
            widget.set_sensitive(enable)
            if widget.get_label() in rightString:
                widget.set_active(True)
            else:
                widget.set_active(False)

    # callback for "changed" signal on combobox
    # Update the checkboxes to match the combobox
    def dlgAccessChanged(self, widget):
        iter = widget.get_active_iter()
        if iter is not None:
            bits=widget.get_model()[iter][0]
            self.dlgSetRightsFromString(bits, bits == "-")

    # Callback for the "toggled" signal on the "Is a group" checkbox
    def groupToggled(self, widget):
        ent = self.builder.get_object("entityText").get_text()
        if widget.get_active():
            if not ent.startswith("system:"):
                self.builder.get_object("entityText").set_text("system:" + ent)
        else:
            self.builder.get_object("entityText").set_text(re.sub(r'^system:', '', ent))

    # Callback for GtkEditable's "insert-text" signal on the "site permissions entry
    # If it's a zero-length text insertion, then it's "valid"
    # If it's a signal character, ensure it's in the valid set
    # For anything else, display an excalmation point in the box and
    # stop signal emission
    def validateSitePerms(self, widget, text, text_len, ptr):
        if text_len == 0:
            return True
        if text_len == 1:
            if text in VALID_SITE_RIGHTS:
                widget.set_icon_from_stock(Gtk.EntryIconPosition.SECONDARY, None)
                return True
        widget.set_icon_from_stock(Gtk.EntryIconPosition.SECONDARY, 'gtk-dialog-warning')
        widget.set_icon_activatable(Gtk.EntryIconPosition.SECONDARY, False)
        widget.set_icon_tooltip_text(Gtk.EntryIconPosition.SECONDARY, "Only the characters '%s' are allowed" % (VALID_SITE_RIGHTS))
        widget.stop_emission('insert-text')

    # Callback for the "changed" signal on the "entity" Entry.
    def dlgEntityTextChanged(self, widget):
        self.builder.get_object("entityIsGroup").set_active(widget.get_text().startswith("system:"))
        self.builder.get_object("aclDlgOK").set_sensitive(widget.get_text().strip() != "")

    # Callback for the "entity" combobox
    def dlgEntityChanged(self, widget):
        iter = widget.get_active_iter()
        if iter is not None:
            name=widget.get_model()[iter][0]
            if name == "-":
                self.builder.get_object("entityText").set_sensitive(True)
                self.builder.get_object("entityIsGroup").set_sensitive(True)
            else:
                self.builder.get_object("entityText").set_sensitive(False)
                self.builder.get_object("entityText").set_text(name)
                self.builder.get_object("entityIsGroup").set_sensitive(False)

    # Convenience function to get the parent window, since we don't have 
    # access to it directly.
    def _getParentWindow(self):
        # Probably not the best idea
        parent = self.rootWidget.get_parent()
        while parent is not None:
            if isinstance(parent, Gtk.Window):
                break
            parent = parent.get_parent()
        return parent


    # Convenience functions
    def _errDialog(self, message, secondaryMsg=None):
        dlg = Gtk.MessageDialog(self._getParentWindow(),
                                Gtk.DialogFlags.DESTROY_WITH_PARENT,
                                Gtk.MessageType.ERROR,
                                Gtk.ButtonsType.CLOSE,
                                message)
        dlg.set_title("Error")
        if secondaryMsg:
            dlg.format_secondary_text(secondaryMsg)
        dlg.run()
        dlg.destroy()

    def _confirmDialog(self, message, secondaryMsg=None):
        dlg = Gtk.MessageDialog(self._getParentWindow(),
                                Gtk.DialogFlags.DESTROY_WITH_PARENT,
                                Gtk.MessageType.QUESTION,
                                Gtk.ButtonsType.YES_NO,
                                message)
        dlg.set_title("Confirm")
        if secondaryMsg:
            dlg.format_secondary_text(secondaryMsg)
        rval = dlg.run()
        dlg.destroy()
        return (rval == Gtk.ResponseType.YES)


class AFSPropertyPage(GObject.GObject, Nautilus.PropertyPageProvider):
    def __init__(self):
        pass
    
    def get_property_pages(self, files):
        # Not supported for multiple selections
        if len(files) != 1:
            return
        
        file = files[0]
        # Not supported for other URIs
        if file.get_uri_scheme() != 'file':
            return

        # Only works on directories
        # TODO: symlinks?
        if not file.is_directory():
            return

        # Should probably use urlparse, but meh
        filepath = urllib.unquote(file.get_uri()[7:])

        self.property_label = Gtk.Label('AFS Permissions')
        self.property_label.show()

        pane = AFSPermissionsPane(filepath)

        if not pane.inAFS or pane.rootWidget is None:
            return

        return Nautilus.PropertyPage(name="NautilusPython::afs",
                                     label=self.property_label, 
                                     page=pane.rootWidget),
