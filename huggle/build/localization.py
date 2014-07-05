#!/usr/bin/python3
import xml;
from pathlib import Path;
import fileinput;
import xml.etree.ElementTree;

localization_en = "Localization/en.xml"
print ("Reading the localization file, please wait...")
tree = xml.etree.ElementTree.parse(localization_en)
r = tree.getroot()
print ("Getting list of source files...")
directory = Path(".")
text = []
for file_ in directory.iterdir():
    if file_.is_file() and file_.name.endswith(".cpp"):
        for lx in fileinput.input(file_.name):
            text.append(lx)

print ("Looking for strings, this is going to take a while...")
keys = []
for child in r:
    name = (child.attrib["name"])
    keys.append(name)
    for line in text:
        if name in line:
            keys.remove(name);
            break

print ("These keys are not used:")
for x in keys:
    print (x)
     
