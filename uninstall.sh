#! /bin/bash

#Get the directory's name so we can delete it
current_dir=`pwd`

#Get out of here
cd ..

#Delete it
#rm -rf $current_dir
echo "The folder "$current_dir" has been removed"

#Tell the user, it's done
echo "Uninstalling process, done..."