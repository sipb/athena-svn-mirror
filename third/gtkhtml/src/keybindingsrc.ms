#
# MS like keybindings for HTML Editor
#

binding "gtkhtml-bindings-ms"
{
  bind "Home"                 { "cursor_move" (left,   all) }
  bind "End"                  { "cursor_move" (right,  all) }
  bind "<Ctrl>Home"           { "cursor_move" (up,   all) }
  bind "<Ctrl>End"            { "cursor_move" (down, all) }

  bind "<Shift>Up"            { "command" (selection-move-up) }
  bind "<Shift>Down"          { "command" (selection-move-down) }
  bind "<Shift>Left"          { "command" (selection-move-left) }
  bind "<Shift>Right"         { "command" (selection-move-right) }
  bind "<Ctrl><Shift>Left"    { "command" (selection-move-prev-word) }
  bind "<Ctrl><Shift>Right"   { "command" (selection-move-next-word) }
  bind "<Alt><Shift>Left"     { "command" (selection-move-prev-word) }
  bind "<Alt><Shift>Right"    { "command" (selection-move-next-word) }

  bind "<Shift>Home"          { "command" (selection-move-bol) }
  bind "<Shift>End"           { "command" (selection-move-eol) }

  bind "<Shift>Page_Up"       { "command" (selection-move-pageup) }
  bind "<Shift>KP_Page_Up"    { "command" (selection-move-pageup) }
  bind "<Shift>Page_Down"     { "command" (selection-move-pagedown) }
  bind "<Shift>KP_Page_Down"  { "command" (selection-move-pagedown) }

  bind "<Ctrl><Shift>Home"    { "command" (selection-move-bod) }
  bind "<Ctrl><Shift>End"     { "command" (selection-move-eod) }

  bind "<Ctrl>a"              { "command" (select-all) }
  bind "<Ctrl>p"              { "command" (select-paragraph) }

  bind "<Ctrl>c"              { "command" (copy) }
  bind "<Ctrl>Insert"         { "command" (copy) }
  bind "<Ctrl>KP_Insert"      { "command" (copy) }

  bind "<Ctrl>x"              { "command" (cut) }
  bind "<Shift>Delete"        { "command" (cut) }
  bind "<Shift>KP_Delete"     { "command" (cut) }

  bind "<Ctrl>v"              { "command" (paste) }
  bind "<Shift>Insert"        { "command" (paste) }
  bind "<Shift>KP_Insert"     { "command" (paste) }

  bind "<Ctrl>b"              { "command" (bold-toggle) }
  bind "<Ctrl>i"              { "command" (italic-toggle) }
  bind "<Ctrl>u"              { "command" (underline-toggle) }
  bind "<Ctrl>s"              { "command" (strikeout-toggle) }

  bind "<Ctrl>z"              { "command" (undo) }
  bind "<Ctrl>r"              { "command" (redo) }

  bind "<Ctrl><Alt>l"         { "command" (align-left) }
  bind "<Ctrl><Alt>c"         { "command" (align-center) }
  bind "<Ctrl><Alt>r"         { "command" (align-right) }

  bind "Tab"                  { "command" (insert-tab-or-next-cell) }
  bind "ISO_Left_Tab"         { "command" (insert-tab-or-next-cell) }
  bind "<Ctrl>Tab"            { "command" (indent-more) }
  bind "<Ctrl>ISO_Left_Tab"   { "command" (indent-more) }
  bind "<Ctrl><Shift>Tab"            { "command" (indent-less) }
  bind "<Ctrl><Shift>ISO_Left_Tab"   { "command" (indent-less) }
  bind "<Shift>Tab"           { "command" (prev-cell) }
  bind "<Shift>ISO_Left_Tab"  { "command" (prev-cell) }

  bind "<Ctrl>0"              { "command" (style-normal) }
  bind "<Ctrl>1"              { "command" (style-header1) }
  bind "<Ctrl>2"              { "command" (style-header2) }
  bind "<Ctrl>3"              { "command" (style-header3) }
  bind "<Ctrl>4"              { "command" (style-header4) }
  bind "<Ctrl>5"              { "command" (style-header5) }
  bind "<Ctrl>6"              { "command" (style-header6) }
  bind "<Ctrl>7"              { "command" (style-pre) }
  bind "<Ctrl>8"              { "command" (style-address) }
  bind "<Ctrl><Alt>1"         { "command" (style-itemdot) }
  bind "<Ctrl><Alt>2"         { "command" (style-itemroman) }
  bind "<Ctrl><Alt>3"         { "command" (style-itemdigit) }

  bind "<Alt>1"               { "command" (size-minus-2) }
  bind "<Alt>2"               { "command" (size-minus-1) }
  bind "<Alt>3"               { "command" (size-plus-0) }
  bind "<Alt>4"               { "command" (size-plus-1) }
  bind "<Alt>5"               { "command" (size-plus-2) }
  bind "<Alt>6"               { "command" (size-plus-3) }
  bind "<Alt>7"               { "command" (size-plus-4) }

  bind "<Alt>8"	              { "command" (size-inc) }
  bind "<Alt>9"               { "command" (size-dec) }

  bind "<Alt>c"               { "command" (capitalize-word) }
  bind "<Alt>l"               { "command" (downcase-word) }
  bind "<Alt>u"               { "command" (upcase-word) }

  bind "<Ctrl><Alt><Shift>s"       { "command" (spell-suggest) }
  bind "<Ctrl><Alt><Shift>p"       { "command" (spell-personal-add) }
  bind "<Ctrl><Alt><Shift>n"       { "command" (spell-session-add) }

  bind "<Ctrl><Shift>r"       { "command" (insert-rule) }

  bind "<Alt>space"           { "command" (popup-menu) }
  bind "<Alt>Return"          { "command" (property-dialog) }
  bind "<Alt>KP_Enter"        { "command" (property-dialog) }

  bind "<Alt>q"               { "command" (indent-paragraph) }

  bind "<Ctrl><Shift>t"       { "command" (insert-table-1-1) }

  bind "<Ctrl><Shift>c"       { "command" (insert-col-after) }
  bind "<Ctrl><Alt>c"         { "command" (insert-col-before) }
  bind "<Alt><Shift>c"        { "command" (delete-col) }

  bind "<Ctrl><Shift>r"       { "command" (insert-row-after) }
  bind "<Ctrl><Alt>r"         { "command" (insert-row-before) }
  bind "<Alt><Shift>r"        { "command" (delete-row) }

  bind "<Ctrl><Shift>b"       { "command" (inc-border) }
  bind "<Ctrl><Alt>b"         { "command" (dec-border) }
  bind "<Alt><Shift>b"        { "command" (zero-border) }

  bind "<Ctrl><Shift>s"       { "command" (inc-spacing) }
  bind "<Ctrl><Alt>s"         { "command" (dec-spacing) }
  bind "<Alt><Shift>s"        { "command" (zero-spacing) }

  bind "<Ctrl><Shift>p"       { "command" (inc-padding) }
  bind "<Ctrl><Alt>p"         { "command" (dec-padding) }
  bind "<Alt><Shift>p"        { "command" (zero-padding) }
}
