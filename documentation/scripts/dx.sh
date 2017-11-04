#!/bin/dash
# This script is used to autogenerate documentation stored at http://tools.wmflabs.org/huggle/docs

echo "Opening huggle directory"
cd $HOME/repo/huggle3-qt-lx || exit 1
cd huggle || exit 1

echo "Checking version"
if [ -f version.txt ];then
  cp version.txt version.old
fi
git pull >> /dev/null 2>&1 || exit 1

echo "Running update"
sh update.sh

if [ ! -f version.old ] || [ "`diff version.old version.txt`" != "" ];then
  echo "`date` Rebuilding repository"
  this=`cat version.txt | sed 's/ /^/g'`
  cd $HOME/public_html/docs || exit 1
  if [ -d temp ];then
    echo "`date` Old removing temp"
    rm -fr temp || exit 1
  fi
  echo "`date` Making temp"
  mkdir temp || exit 1
  cd temp || exit 1
  cat $HOME/services/huggle | sed "s/&REVISION/$this/" > huggle
  /usr/bin/doxygen huggle || exit 1
  echo "`date` Finished running doxy copying over"
  cp -vr $HOME/repo/huggle3-qt-lx/documentation html/documentation
  if [ -d ../old ];then
    rm -fr ../old
  fi
  cd .. || exit 1
  echo "`date` Moving docs"
  mv head old || exit 1
  mv temp/html head || exit 1
  rm -rf temp || exit 1
  echo "`date` Finished"
else
  echo "Doxygen is up to date"
fi
