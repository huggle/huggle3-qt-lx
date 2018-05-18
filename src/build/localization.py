#!/usr/bin/python3
import xml;
from pathlib import Path;
import xml.etree.ElementTree;

localization_en = "Localization/en.xml"
if (not Path(localization_en).exists()):
    print("Error: you must run this script from root of source (usually from src folder)")
    exit(1)
print ("Reading the localization file, please wait...")
tree = xml.etree.ElementTree.parse(localization_en)
r = tree.getroot()
print ("Getting list of source files...")
directories = [ "huggle", "huggle_core", "huggle_ui" ]
text = list()
for d in directories:
    directory = Path(d)
    for file_ in directory.iterdir():
        if file_.is_file() and file_.name.endswith(".cpp"):
            with open(str(file_.resolve()), 'r') as cpp:
                text.append(cpp.read())

print ("Looking for strings, this is going to take a while...")
keys = dict()
for child in r:
    name = (child.attrib["name"])
    keys[name]=child.text
    for line in text:
        if name in line:
            del keys[name]
            break

print ("These keys are not used:")
ks = list(keys)
ks.sort();
size = 0
for x in ks:
    size = size + 1
    print (x + ": " + keys[x] + ".")
print ("In total there is " + str(size) + " unused locs!!")
