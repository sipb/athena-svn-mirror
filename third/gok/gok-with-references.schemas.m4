dnl If you modify this file and commit the new version to CVS please
dnl generate gok-with-references.schemas.in and also
dnl and commit it to CVS. Build it with this command ('make' should do this):
dnl     m4 gok-with-references.schemas.m4 > gok-with-references.schemas.in
dnl
dnl gok-with-references.schemas.in is needed by intltool-update
dnl
dnl Here is how gok.schemas is generated:
dnl     gok-with-references.schemas.m4
dnl         m4
dnl     gok-with-references.schemas.in
dnl         @INTLTOOL_SCHEMAS_RULE@
dnl     gok-with-references.schemas
dnl         sed
dnl     gok.schemas

dnl Define a macro that expands into a schema
dnl $1 = key
dnl $2 = type
dnl $3 = default
dnl $4 = short description
dnl $5 = long description
define(`mkschema', `<schema>
      <key>/schemas/apps/gok/$1</key>
      <applyto>/apps/gok/$1</applyto>
      <owner>gok</owner>
      <type>$2</type>
      <default>$3</default>
      <locale name="C">
        <short>$4</short>
        <long>$5</long>
      </locale>
    </schema>')

define(`mkschema_i18n_default', `<schema>
      <key>/schemas/apps/gok/$1</key>
      <applyto>/apps/gok/$1</applyto>
      <owner>gok</owner>
      <type>$2</type>
      <locale name="C">
        <default>$3</default>
        <short>$4</short>
        <long>$5</long>
      </locale>
    </schema>')

dnl Define a macro that expands into keys for an action
dnl $1 = name
dnl $2 = displayname
dnl $3 = type
dnl $4 = state
dnl $5 = number
dnl $6 = rate
dnl $7 = permanent
dnl $8 = key_averaging
define(`mkaction', mkschema_i18n_default(`actions/$1/displayname', `string', $2,
                       `The name to display to the user for this action',
                       `')
    mkschema(`actions/$1/type', `string', $3,
             `The type of this action',
             `')
    mkschema(`actions/$1/state', `string', $4,
             `The state in which this action is activated',
             `')
    mkschema(`actions/$1/number', `int', $5,
             `The number if applicable',
             `')
    mkschema(`actions/$1/rate', `int', $6,
             `The rate for this action',
             `')
    mkschema(`actions/$1/permanent', `bool', $7,
             `Is this action permanent',
             `')
    mkschema(`actions/$1/key_averaging', `bool', $8,
             `Does this action use key averaging',
             `'))

define(`switch_action', mkaction($1, $2, switch, press, $3, 0, true, false))
define(`mousebutton_action', mkaction($1, $2, mousebutton, press, $3, 0, true, false))
define(`mousepointer_action', mkaction($1, $2, mousepointer, press, 0, 0, true, false))
define(`dwell_action', mkaction($1, $2, dwell, press, 0, 100, true, false))

dnl Define a macro that expands into keys for a feedback
dnl $1 = name
dnl $2 = displayname
dnl $3 = flash
dnl $4 = number of flashes
dnl $5 = sound
dnl $6 = soundname
dnl $7 = permanent
define(`mkfeedback', mkschema_i18n_default(`feedbacks/$1/displayname', `string', $2,
                         `The name to display to the user for this feedback',
                         `')
    mkschema(`feedbacks/$1/flash', `bool', $3,
             `Does this feedback flash',
             `')
    mkschema(`feedbacks/$1/number_flashes', `int', $4,
             `The number of times this feedback will flash',
             `')
    mkschema(`feedbacks/$1/sound', `bool', $5,
             `Does this feedback play a sound',
             `')
    mkschema_i18n_default(`feedbacks/$1/soundname', `string', $6,
             `The name of the sound that this feedback will play',
             `')
    mkschema(`feedbacks/$1/speech', `bool', $5,
             `Does this feedback speak the label of a GOK key',
             `')
    mkschema(`feedbacks/$1/permanent', `bool', $7,
             `Is this feedback permanent',
             `'))

<!-- Please do not modify this file directly but rather modify
     gok-with-references.schemas.m4, this file is generated from it.
     If you modify gok-with-references.schemas.m4 and commit it to CVS
     then please also commit the new gok-with-references.schemas.in
     that is generated from it to CVS.

     Here is how gok.schemas is generated:
         gok-with-references.schemas.m4
             m4
         gok-with-references.schemas.in
             @INTLTOOL_SCHEMAS_RULE@
         gok-with-references.schemas
             sed
         gok.schemas
-->

<gconfschemafile>
  <schemalist>
    mkschema(`layout/key_spacing', `int', `3',
             `The space between keys',
             `')
    mkschema(`layout/key_width', `int', `60',
             `The key width',
             `')
    mkschema(`layout/key_height', `int', `40',
             `The key height',
              `')
    mkschema(`layout/keyboard_x', `int', `100',
             `The keyboards X coordinate',
             `')
    mkschema(`layout/keyboard_y', `int', `200',
             `The keyboards Y coordinate',
             `')
    mkschema(`access_method', `string', `directselection',
             `The access method to use',
             `')
    mkschema(`input_device', `string',`',
	     `The name of the xinput device to use',
	     `')
    mkschema(`valuator_sensitivity', `float',`0.25',
	     `A multiplier to be applied to input device valuator events before processing',
	     `')
    mkschema(`use_aux_dicts', `bool',`false',
	     `Whether to use additional word lists when searching for GOK word-completion candidates',
	     `')
    mkschema(`aux_dictionaries', `string',`',
	     `A semicolon-delimited list of fully specified paths to additional word-completion dictionaries',
	     `')
    mkschema(`prefs_locked', `bool', `false',
             `Are the preferences to be restricted?',
             `')
    mkschema(`spy/gui_search_depth', `int',`200',
	     `How many levels down the GUI tree to search for accessible objects',
	     `')
    mkschema(`spy/gui_search_breadth', `int',`200',
	     `How many children of each node in the GUI tree to search for accessible objects',
	     `')
    mkschema(`word_complete', `bool', `true',
             `Is word completion on',
             `')
    mkschema(`number_predictions', `int', `5',
             `The number of predictions for word completion',
             `')
    mkschema(`keyboard_directory', `string', `$pkgdatadir',
             `The primary or system directory to load keyboard files from.',
             `')
    mkschema(`aux_keyboard_directory', `string', `$pkgdatadir',
             `The directory to load user-specific or custom keyboard files from.',
             `')
    mkschema(`access_method_directory', `string', `$pkgdatadir',
             `The directory to load access method files from.',
             `')
    mkschema(`dictionary_directory', `string', `$pkgdatadir',
             `The directory to load dictionary files from.',
             `')
    mkschema(`per_user_dictionary', `bool', `true',
             `Does each GOK user have a private copy of the word prediction dictionary?',
             `')
    mkschema(`resource_directory', `string', `$pkgdatadir',
             `The directory to load the gok resource file from.',
             `')
    mkschema(`drive_corepointer',`bool', `false',
	     `Does the core pointer follow the GOK keyboard pointer?',
	     `')
    mkschema(`expand',`bool', `False',
	     `Does the GOK window expand to fill the full screen width?',
	     `')
    mkschema(`use_xkb_geom',`bool', `true',
	     `Does GOK generate its compose keyboard dynamically based on information from the X Server?',
	     `')
    mkschema(`compose_kbd_type',`string', `xkb',
	     `Determines the keyboard type used for the default compose keyboard',
	     `Should be one of the following: xkb, default, alpha, alpha-freq, custom; if custom, custom_compose_keyboard should be defined.')
    mkschema(`custom_compose_kbd',`string', `',
	     `The full pathname to a file which defines a custom GOK compose keyboard',
	     `')
    mkschema(`dock_type',`string',`',
	     `GOK main window anchor location (if a dock), or empty string if not',`')
    switch_action(`switch1', `Switch 1', 1)
    switch_action(`switch2', `Switch 2', 2)
    switch_action(`switch3', `Switch 3', 3)
    switch_action(`switch4', `Switch 4', 4)
    switch_action(`switch5', `Switch 5', 5)
    mousebutton_action(`mousebutton1', `Left Mouse Button', 1)
    mousebutton_action(`mousebutton2', `Middle Mouse Button', 2)
    mousebutton_action(`mousebutton3', `Right Mouse Button', 3)
    mousebutton_action(`mousebutton4', `Mouse Button 4', 4)
    mousebutton_action(`mousebutton5', `Mouse Button 5', 5)
    mousepointer_action(`mousepointer', `Mouse Pointer')
    dwell_action(`dwell', `Dwell')
    mkfeedback(`none', `None', `false', `0', `false', `none', `false', `true')
    mkfeedback(`key_flashing', `Key flashing', `true', `4', `false', `none',
               `false', `true')
    mkfeedback(`goksound1', `Sound one', `false', `0', `true',
               `$pkgdatadir/goksound1.wav', `false', `true')
    mkfeedback(`goksound2', `Sound two', `false', `0', `true',
               `$pkgdatadir/goksound2.wav', `false', `true')
    mkfeedback(`gokspeech', `Speech', `false', `0', `false',
               `none', `true', `true')
  </schemalist>
</gconfschemafile>
