
    --------------------
    QSwitch DESIGN GOALS
    --------------------

    Locate and switch to any open window as fast as possible.

    - Open windows are sorted alphabetically by their process names.

    - On a second level, open windows are sorted alphabetically by their titles.

    - On successive levels, the backslash "\" within Window titles is used to group windows hierarchically.
      This way, especially windows which belong to Explorer can be located very fast.


    Filtering
    ---------
    Using a special key (which can be user-defined), only windows of the current foreground process
    are displayed.
    The filtered view is also shown, if the user-defined mouse-button is double clicked.


    System Requirements
    -------------------
    - Windows Vista ore higher
    - Graphics Card with OpenGL driver


    User Interface
    --------------
    Keyboard
    - up / down arrow keys: move up or down in list
    - left / right arrow keys: jump to first entry of a group within the hierarchy
    - home / end: jump to first / last entry
    - Enter: bring selected window to front
    - ESC: hide QSwitch
    
    Mouse
    - left-click onto line: select the line
    - left-double-click onto line: bring selected window to front
    - left-click into free space to hide QSwitch
    
    Scrolling
    - use the scroll-wheel of the mouse

    
    Settings Dialog
    ---------------
    - activate / hide with a definable special key, or key combination, eg F12
    - activate / hide filtered view with a definable special key, or key combination, eg F12
    - activate / hide with a definable special mouse-button, if you have more than 2 mouse-buttons
      hint: a double-click onto this mouse button activates the filtered view (see "Filtering").
    - activation can be suspended, when a full-screen app is running in foreground, e.g. a game
    - activate by moving the mouse into upper-right corner [a definable corner] of the screen
    - background color and transparency


Thumbnail Preview
-----------------
The thumbnail preview is only shown, if the Aero-style is activated.
It won't work with older graphics cards.


SFML
----
Places uses SFML, which is a very good multi-media library for C++.


Backup
------
If you wish to backup the configuration of QSwitch, you need to backup the folder "%appdata%\QSwitch"

Note: %appdata% usually resolves to C:\Users\<user name>\AppData\Roaming, but you can
      also enter "%appdata%" in Explorer.


Enjoy,
Thorsten Radde
