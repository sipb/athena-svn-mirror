import os

# This module should be kept compatible with Python 1.5.2.

__revision__ = "$Id: debug.py,v 1.1.1.1 2004-04-28 21:37:18 zacheiss Exp $"

# If DISTUTILS_DEBUG is anything other than the empty string, we run in
# debug mode.
DEBUG = os.environ.get('DISTUTILS_DEBUG')

