Python guide
=============

This document explains how to write an extension for huggle in python

Running huggle with Python support
===================================

Huggle need to be compiled with Python support for it to execute python extensions, you can do that by configuring
it with option --python

By default linux packages as well as Windows packages should contain python version of huggle, it however may
need extra python libraries to be installed for it to work.

How to create a python extension
================================

Python extensions are stored in extension folder which is visible in system logs when Huggle starts, for example:
Mon Oct 10 00:18:03 2016   Loading plugins in /usr/share/huggle/extensions and /home/petanb/.local/share/data/Wikimedia/Huggle/extensions/

In this case, placing any file suffixed with .py in any of these folders will work as an extension for huggle.

Minimal Python extension may look like this:

```
#!/bin/echo This is a huggle extension, please use huggle instead
import huggle

def get_author():
    return "Petr Bena";
def get_version():
    return "1.0.0.0";
def get_description():
    return "This is an example extension"
def get_minimal_huggle_version():
    return "3.1.10";
```

Writing this code into any file suffixed with .py will make a working extension that does nothing :)

You will however see it when you open huggle's preferences under tab extensions

Marshalling
============

Huggle is written in C++ and uses mostly C++ classes as object types, since Python doesn't friend with C++ so much
we implemented a Marshalling subsystem. How it works?

Huggle converts the C++ object using serialization into "C++ meta object" a Python dict that contains various data
of C++ object serialized into dictionary. Then huggle.Marshalling class provides functions that will turn these
into Python objects.

You should never directly work with meta objects as their format may change in future with no warning.

Hooks
======

You can run a code when some event is triggered in huggle using hooks which are 
basically just functions, which specific names, that huggle expects you to have
in your script, bellow is a list of all hooks you can use:

##hook_shutdown()
No parameters, called when huggle is shut down

##hook_speedy_finished(string page_name, string user_name, string result)
3 parameters: string page_name, string user_name, string result

This is called when you finish tagging of a page for speedy deletion

##hook_main_window_is_loaded
0 parameters, called when main window of huggle is loaded

##hook_good_edit(C++ edit)
1 parameter: use huggle.Marshalling.mWikiEdit(edit) to retrieve the WikiEdit instance

##hook_on_edit_post_process(C++ edit)
1 parameter: use huggle.Marshalling.mWikiEdit(edit) to retrieve the WikiEdit instance

##hook_on_edit_pre_process(C++ edit)
1 parameter: use huggle.Marshalling.mWikiEdit(edit) to retrieve the WikiEdit instance

##hook_on_edit_load_to_queue(C++ edit)
1 parameter: use huggle.Marshalling.mWikiEdit(edit) to retrieve the WikiEdit instance
returns: bool

This hook is called before edit is added to queue, if False is returned, the edit is not inserted to queue

Internal functions
===================

Huggle provides a set of function hidden within special "huggle" module, they are not real python functions,
but C++ functions that can be called from your Python extension. See huggle.py in this folder for more.


Debugging
===========

It's useful to start huggle with verbose level higher than 2 in order to see 
how it loads and parse your script on start, for example:
```
Mon Jun 23 12:26:37 2014   DEBUG[1]: Loading hook symbols for python 
/usr/share/huggle/extensions/huggle_logs.py
Mon Jun 23 12:26:37 2014   DEBUG[2]: Loading hook symbols of 
hook_speedy_finished /usr/share/huggle/extensions/huggle_logs.py
```


