## This file contains the list of changes in this fork

## Format of changes in this file
```
### [id] Title
#### Description
#### Files
```
Example
### [000] Title
#### Description
The Description.
#### Files
[File](CHANGES.md)


## Format of changes in source files
```
// [id] Title
```

### [001] Kiosk Mode Window Gets Normalized on ALT+TAB
#### Description
Windows only. When pressing ALT+TAB in Kiosk Mode (ie real fullscreen) the window gets normalized.
#### Files
[modules/juce_gui_basics/native/juce_win32_Windowing.cpp](modules/juce_gui_basics/native/juce_win32_Windowing.cpp)

### [002] Fix FullScreen Mode
#### Description
Windows and MacOs. FullScreen mode (ie Kiosk Mode) is not working as expected.
Workaround methods are DocumentWindow::toggleRealFullScreen() and DocumentWindow::isRealFullScreen()
#### Files
[modules/juce_gui_basics/windows/juce_DocumentWindow.h](modules/juce_gui_basics/windows/juce_DocumentWindow.h)<br/>
[modules/juce_gui_basics/windows/juce_DocumentWindow.cpp](modules/juce_gui_basics/windows/juce_DocumentWindow.cpp)

### [003] MenuBar Disappears in Real Full Screen Mode
#### Description
Windows only.
#### Files
[modules/juce_gui_basics/windows/juce_DocumentWindow.cpp](modules/juce_gui_basics/windows/juce_DocumentWindow.cpp)

### [004] MenuBar Height Update Method
#### Description
Method in DocumentWindow to update the height of the menu bar to either the LF provided value or to a custom value.
#### Files
[modules/juce_gui_basics/windows/juce_DocumentWindow.h](modules/juce_gui_basics/windows/juce_DocumentWindow.h)<br/>
[modules/juce_gui_basics/windows/juce_DocumentWindow.cpp](modules/juce_gui_basics/windows/juce_DocumentWindow.cpp)

### [005] Window Looses Focus When a SubMenu Gets Hidden
#### Description
Windows only. When a SubMenu gets hidden, the menu-owning window looses the focus.
Fixed by moving the setVisible() call before the exitModalState() call in PopupMenu::hide().
This also seems to fix the problem where the window-close button does not work while a Menu is open.
#### Files
[modules/juce_gui_basics/menus/juce_PopupMenu.cpp](modules/juce_gui_basics/menus/juce_PopupMenu.cpp)

### [006] KeyPress Modifier Strings Upper Case
#### Description
The KeyPress Modifier Strings are lowercase by default. For example "ctrl + X" is displayed in menus.
Changed to Ctrl, Shift, ... in KeyPress::getTextDescription().
#### Files
[modules/juce_gui_basics/keyboard/juce_KeyPress.cpp](modules/juce_gui_basics/keyboard/juce_KeyPress.cpp)
