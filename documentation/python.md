Python guide
=============

This document explains how to write an extension for huggle in python

Hooks
======

You can run a code when some event is triggered in huggle using hooks which are 
basically just functions, which specific names, that huggle expects you to have
in your script, bellow is a list of all hooks you can use:

##hook_shutdown
No parameters, called when huggle is shut down

##hook_speedy_finished
3 parameters: string page_name, string user_name, string result

This is called when you finish tagging of a page for speedy deletion

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


