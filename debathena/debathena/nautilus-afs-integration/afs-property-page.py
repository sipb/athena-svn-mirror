import urllib
import os
import gtk
import gtk.glade
import re
import nautilus
import subprocess

gladeFile = "/usr/share/debathena-nautilus-afs-integration/afs-property-page.glade"

import sys
try:
    import afs.acl
except ImportError:
    pass

class AFSPermissionsPane():
    def __init__(self, fp):
        self.filepath = fp
        self.acl = None
        self.noPermissions = False
        self.inAFS = True
        if 'afs.acl' not in sys.modules.keys():
            if (not self.filepath.startswith("/afs")) and (not self.filepath.startswith("/mit")):
                self.inAFS = False
            b = gtk.VBox()
            l = gtk.Label("Sorry, this feature is not available on this platform.\n(afs.acl module could not be imported.)\n")
            l.set_single_line_mode(False)
            l.show()
            b.pack_start(l)
            b.show()
            self.rootWidget = b
            return
        if not os.path.exists(gladeFile):
            b = gtk.VBox()
            l = gtk.Label("Cannot find Glade file:\n%s" % gladeFile)
            l.set_single_line_mode(False)
            l.show()
            b.pack_start(l)
            b.show()
            self.rootWidget = b
            return
        self.widgets = gtk.glade.XML(gladeFile, "vboxMain")
        self.rootWidget = self.widgets.get_widget("vboxMain")
        try:
            self.acl = afs.acl.ACL.retrieve(self.filepath)
        except OSError as (errno, errstr):
            if errno == 13:
                self.noPermissions = True
                self.widgets = gtk.glade.XML(gladeFile, "vboxNoPerms")
                self.rootWidget = self.widgets.get_widget("vboxNoPerms")
            elif errno == 22:
                self.inAFS = False
        
                
        if self.acl != None:
            self._initWidgets()
            self.refreshACL()

    def _initWidgets(self):
        self.listStore = gtk.ListStore(str, int, bool, str)
        self.buttonActions = {"btnAdd": self.addItem,
                              "btnEdit": self.editItem,
                              "btnRemove": self.removeItem}
        for btn in self.buttonActions:
            self.widgets.get_widget(btn).connect("clicked", self.buttonHandler)
        self.treeView = self.widgets.get_widget("tvACL")
        self.treeView.set_model(self.listStore)
        colUser = gtk.TreeViewColumn('User')
        colBits = gtk.TreeViewColumn('Permissions')
        self.treeView.append_column(colUser)
        self.treeView.append_column(colBits)
        cellRenderer = gtk.CellRendererText()
        colUser.pack_start(cellRenderer)
        colBits.pack_start(cellRenderer)
        colUser.set_cell_data_func(cellRenderer, self.renderUser)
        colBits.set_cell_data_func(cellRenderer, self.renderBitmask)
        self.treeView.set_tooltip_column(3)

    def getWidget(self):
        if self.noPermissions:
            return NoPermissionsPane()
        else:
            return self.widgets.get_widget("vboxMain")

    def renderBitmask(self, col, cell, model, iter, user_data=None):
        rights = afs.acl.showRights(model.get_value(iter, 1))
        if afs.acl.rightsToEnglish(rights):
            rights += '  (' + afs.acl.rightsToEnglish(rights) + ')'
        cell.set_property('text', rights)

    def renderUser(self, col, cell, model, iter, user_data=None):
        cell.set_property('strikethrough-set', True)
        isNeg = model.get_value(iter, 2)
        if isNeg:
            cell.set_property('strikethrough',True)
        else:
            cell.set_property('strikethrough', False)
        cell.set_property('text', model.get_value(iter, 0))

    def refreshACL(self):
        if self.acl != None:
            self.widgets.get_widget("lblPath").set_text(self.filepath)
            self.listStore.clear()
            try:
                self.acl = afs.acl.ACL.retrieve(self.filepath)
            except OSError as (errno, errstr):
                if errno == 13:
                    msg = gtk.MessageDialog(None, gtk.DIALOG_MODAL,
                                            gtk.MESSAGE_QUESTION, 
                                            gtk.BUTTONS_OK,
                                            "Cannot reload ACL for directory.  Perhaps your AFS tokens expired or someone else changed permissions?")
                    msg.run()
                    msg.destroy()
            for i in self.acl.pos.items():
                self.listStore.append(i + (False,i[0]))
            for i in self.acl.neg.items():
                self.listStore.append(i + (True,i[0] + ' (Negative Rights)'))

    def removeItem(self):
        selectedRowIter = self.treeView.get_selection().get_selected()[1]
        if selectedRowIter == None:
            return
        entity, isNeg = self.listStore.get(selectedRowIter, 0, 2)
        if isNeg:
            msg = gtk.MessageDialog(None,gtk.DIALOG_MODAL,
                                    gtk.MESSAGE_INFO, gtk.BUTTONS_OK,
                                    "Removing negative ACLS is not yet supported")
            msg.run()
            msg.destroy()
            return
        msg = 'Are you sure you want to remove %s from the AFS ACL for this directory?'
        if (entity == os.getenv('ATHENA_USER')) or (entity == os.getenv('USER')):
            msg += "\n\nWARNING: You are about to remove yourself from this ACL!"
        question = gtk.MessageDialog(None, gtk.DIALOG_MODAL,
                                     gtk.MESSAGE_QUESTION,
                                     gtk.BUTTONS_YES_NO,
                                     msg % entity)
        response = question.run()
        question.destroy()
        if response == gtk.RESPONSE_YES:
            self.acl.remove(entity)
            self.applyChanges()

    def applyChanges(self):
        try:
            self.acl.apply(self.filepath)
        except OSError as (errno, errstr):
            msg = gtk.MessageDialog(None, gtk.DIALOG_MODAL,
                                    gtk.MESSAGE_ERROR, gtk.BUTTONS_OK,
                                    "Error: %s" % errstr)
            msg.run()
            msg.destroy()
        self.refreshACL()

    def editItem(self):
        selectedRowIter = self.treeView.get_selection().get_selected()[1]
        if selectedRowIter == None:
            return
        entity, acl, isNeg = self.listStore.get(selectedRowIter, 0, 1, 2)
        if isNeg:
            msg = gtk.MessageDialog(None,gtk.DIALOG_MODAL,
                                    gtk.MESSAGE_INFO, gtk.BUTTONS_OK,
                                    "Editing negative ACLS is not yet supported")
            msg.run()
            msg.destroy()
            return
        add = AddEditACLDialog()
        add.editMode(entity, afs.acl.showRights(acl))
        response = add.run()
        while (response == gtk.RESPONSE_OK) and not add.validate():
            response=add.run()
        if (response == gtk.RESPONSE_OK):
            self.acl.set(add.getEntity(), afs.acl.readRights(add.getAcl()))
            self.applyChanges()
        add.destroy()

    def addItem(self):
        add = AddEditACLDialog()
        response = add.run()
        while (response == gtk.RESPONSE_OK) and not add.validate():
            response=add.run()
        if (response == gtk.RESPONSE_OK):
            self.acl.set(add.getEntity(), afs.acl.readRights(add.getAcl()))
            self.applyChanges()
        add.destroy()

    def buttonHandler(self, btn):
        self.buttonActions[btn.name]()

class AddEditACLDialog():
    def __init__(self, editMode=False):
        self.widgets = gtk.glade.XML(gladeFile, "dlgAdd")
        self.window = self.widgets.get_widget("dlgAdd")
        self.entry = self.widgets.get_widget("entryName")
        for rb in self.widgets.get_widget_prefix("rb"):
            rb.connect("toggled", self.toggleHandler)

    def editMode(self, user, acl):
        self.window.set_title('Edit ACL Entry')
        self.entry.set_text(user)
        self.entry.set_editable(False)
        self.widgets.get_widget('lblType').set_property('visible', False)
        for rb in self.widgets.get_widget_prefix("rb"):
            rb.set_property('visible', False)
        self.widgets.get_widget("cbAcl").get_child().set_text(acl)

    def run(self):
        return self.window.run()

    def destroy(self):
        self.window.destroy()

    def toggleHandler(self, radioBtn, data=None):
        if not radioBtn.get_active():
            return
        if radioBtn.name == "rbUser":
            self.entry.set_text("")
            self.entry.set_editable(True)
        if radioBtn.name == "rbGroup":
            # Do this in the GUI, so that people can override it if they
            # really know what they're doing, or are using user groups or
            # something.  
            if re.search(":", self.entry.get_text()) == None:
                self.entry.set_text("system:")
            self.entry.set_editable(True)
        if radioBtn.name == "rbAuthuser":
            self.entry.set_text("system:authuser")
            self.entry.set_editable(False)
        if radioBtn.name == "rbAnyuser":
            self.entry.set_text("system:anyuser")
            self.entry.set_editable(False)

    def getEntity(self):
        return self.entry.get_text().strip()

    def getAcl(self):
        combobox = self.widgets.get_widget("cbAcl")
        model = combobox.get_model()
        active = combobox.get_active()
        if active < 0:
            # in case someone typos whitespace in the box, which would parse
            # to "none".   
            acl = combobox.get_child().get_text().strip()
        else:
            acl = model[active][0]
        return acl

    def validate(self):
        if not self.getEntity():
            msg = gtk.MessageDialog(self.window,gtk.DIALOG_MODAL,
                                    gtk.MESSAGE_INFO, gtk.BUTTONS_OK,
                                    "You did not specify the user or group.")
            msg.run()
            msg.destroy()
            return False

        acl = self.getAcl()
        if not acl:
            msg = gtk.MessageDialog(self.window,gtk.DIALOG_MODAL,
                                    gtk.MESSAGE_INFO, gtk.BUTTONS_OK,
                                    "You did not specify the permissions.")
            msg.run()
            msg.destroy()
            return False
        try:
            r = afs.acl.readRights(acl)
        except ValueError:
            msg = gtk.MessageDialog(self.window,gtk.DIALOG_MODAL,
                                    gtk.MESSAGE_ERROR, gtk.BUTTONS_OK,
                                    "Invalid permissions '%s'.  Select one from the drop down list or type a valid AFS permission string into the box provided." % acl)
            msg.run()
            msg.destroy()
            return False
        if re.search('[widka]', afs.acl.showRights(r)) and self.widgets.get_widget("rbAnyuser").get_active():
            msg = gtk.MessageDialog(self.window,gtk.DIALOG_MODAL,
                                    gtk.MESSAGE_WARNING, gtk.BUTTONS_YES_NO,
                                    "WARNING:  You are attempting to assign '%s' permissions to system:anyuser (i.e. any user, anywhere on the Internet).  This is EXTREMELY DANGEROUS.  You may experience data loss and your directory may be used by spammers to create malicious websites.  MIT IS&T reserves the right to disable access to any AFS directories with these permissions.  Consider selecting 'All MIT Users' instead.\n\nAre you absolutely sure you want to continue?" % acl)

            response = msg.run()
            msg.destroy()
            if response != gtk.RESPONSE_YES:
                return False
        return True


class AFSPropertyPage(nautilus.PropertyPageProvider):
    def __init__(self):
        self.property_label = gtk.Label('AFS')

    def get_property_pages(self, files):
        # Does not work for multiple selections
        if len(files) != 1:
            return
        
        file = files[0]
        # Does not work on non-file:// URIs
        if file.get_uri_scheme() != 'file':
            return

        # Only works on directories
        if not file.is_directory():
            return

#        if 'afs.acl' not in sys.modules.keys():
#            return

        filepath = urllib.unquote(file.get_uri()[7:])
        
        pane = AFSPermissionsPane(filepath)

        # If the file does not appear to be in AFS, don't show the pane
        if not pane.inAFS:
            return
        
        self.property_label.show()

        return nautilus.PropertyPage("NautilusPython::afs",
                                     self.property_label, 
                                     pane.rootWidget),

