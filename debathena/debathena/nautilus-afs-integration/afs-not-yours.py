import urllib
import gtk
import nautilus

import sys
try:
    import afs.acl
except ImportError:
    pass

class AFSPermissionsPane():
    def __init__(self, fp):
        self.filepath = fp
        self.inAFS = False
        if self.filepath.startswith("/afs") or self.filepath.startswith("/mit"):
            self.inAFS = True
        b = gtk.VBox()
        l = gtk.Label("Sorry, this feature won't work until you upgrade to a version of python-nautilus that knows to look in /usr/lib/nautilus/extensions-2.0/pythyon")
        l.set_single_line_mode(False)
        l.show()
        b.pack_start(l)
        b.show()
        self.rootWidget = b
        return

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

        filepath = urllib.unquote(file.get_uri()[7:])
        
        pane = AFSPermissionsPane(filepath)

        # If the file does not appear to be in AFS, don't show the pane
        if not pane.inAFS:
            return
        
        self.property_label.show()

        return nautilus.PropertyPage("NautilusPython::afs",
                                     self.property_label, 
                                     pane.rootWidget),

