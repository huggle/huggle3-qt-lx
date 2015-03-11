# This file is just an interface to describe the huggle module, it exist just for doxygen


"""@package huggle
This module is an interface to use huggle. 

In case you want to create an extension for huggle in python, you should import it.
When loaded in huggle, you can access all of following functions.
"""
def huggle_version():
    """Returns a version of a huggle in form of string

    Version numbers are separated with dot symbol and build id is appended to the end
    if the version (this works only if huggle was built from within a git repository)
    """
    return "unknown";

def configuration_get_project_wiki_url():
    """Returns a project /wiki url"""

    return "http://en.wikipedia.org/wiki/";

def configuration_get_project_script_url():
    return "http://en.wikipedia.org/w/";

def configuration_get_user():
    """Returns a username as stored in huggle configuration"""
    return "JimmyWales";

def wikipage_append(wikipage, summary, text):
    """Appends a text to a page, this is a background async task so you will not know if
    it was successfull or not.

    This function return true unless it's not possible to edit"""
    return true;

def log(str_message):
    """Logs a message to huggle system log
    Calls Huggle::Python::Log @pythonengine.cpp

    Return false on error
    """
    return true;

def error_log(str_message):
    """Logs a message to huggle error log
    Calls Huggle::Python::ErrorLog @pythonengine.cpp

    Return false on error
    """
    return true;

def debug_log(str_message, int_verbosity):
    """Logs a message to huggle debug log
    Calls Huggle::Python::DebugLog @pythonengine.cpp

    Return false on error
    """
    return true;

def warning_log(str_message):
    """Logs a message to huggle warning log
    Calls Huggle::Python::WarningLog @pythonengine.cpp

    Return false on error
    """
    return true;

class Marshalling:
    @staticmethod
    def mWikiEdit(edit):
        return WikiEdit(edit);

    @staticmethod
    def mWikiPage(page):
        return WikiPage(page);

    @staticmethod
    def mWikiSite(site):
        return WikiSite(site);

    @staticmethod
    def mWikiUser(user):
        return WikiUser(user);
