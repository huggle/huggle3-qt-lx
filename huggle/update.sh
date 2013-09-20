c=`git rev-list HEAD --count`
hash=`git describe --always`
echo "build: $c $hash" > version.txt
