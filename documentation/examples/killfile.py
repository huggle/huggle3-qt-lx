#!/bin/echo This is a huggle extension, please use huggle instead
import huggle

def get_author():
    return "Petr Bena";
def get_version():
    return "1.0.0.0";
def get_description():
    return "This is example extension that will prevent pages starting with Wikipedia from being added to queue";
def get_minimal_huggle_version():
    return "3.1.20";

def hook_on_edit_load_to_queue(edit):
    # We need to convert edit from C++ meta object to Python object WikiEdit
    # you can check huggle/Resources/Python/definitions.py for exact structure of all huggle's Python objects
    wiki_edit = huggle.Marshalling.mWikiEdit(edit)

    if (wiki_edit.Page.PageName.startswith("Wikipedia")):
        # Write to debug log
        huggle.debug_log("Skipping page " + wiki_edit.Page.PageName)
        return False

    # We can continue
    return True
