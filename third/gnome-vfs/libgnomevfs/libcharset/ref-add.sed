/^# Packages using this file: / {
  s/# Packages using this file://
  ta
  :a
  s/ gnome-vfs / gnome-vfs /
  tb
  s/ $/ gnome-vfs /
  :b
  s/^/# Packages using this file:/
}
