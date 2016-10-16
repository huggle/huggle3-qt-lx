#!/bin/echo This is a huggle extension, please use huggle instead
import huggle
import os.path

ignored = None
killfile = "ignored_pages"

def get_author():
    return "Petr Bena";
def get_version():
    return "1.0.0.0";
def get_description():
    return "This extension will prevent all pages located in file " + killfile + " from being added to queue"
def get_minimal_huggle_version():
    return "3.1.20";

# we purposefuly load the database of pages here, once the main window is loaded, so that initial startup of huggle (login form) doesn't get slowed down
def hook_main_window_is_loaded():
    if (not os.path.isfile(killfile)):
        huggle.warning_log("There is no killfile located in " + killfile + " the extension will not be working")
        return
    with open(killfile) as input_file:
        ignored = input_file.readlines()
    return

def hook_on_edit_load_to_queue(edit):
    # In case there is no list of ignored pages, do nothing
    if (ignored is None):
        return True

    # We need to convert edit from C++ meta object to Python object WikiEdit
    # you can check huggle/Resources/Python/definitions.py for exact structure of all huggle's Python objects
    wiki_edit = huggle.Marshalling.mWikiEdit(edit)

    if (wiki_edit.Page.PageName in ignored):
        # Write to debug log
        huggle.debug_log("killfile: " + wiki_edit.Page.PageName)
        return False

    # We can continue
    return True
