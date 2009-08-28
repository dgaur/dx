#!/bin/sh
#
# Shell script for populating an ext2 filesystem image.  Takes a local
# directory tree and copies/writes into a filesystem image.
#
# The given directory tree is duplicated verbatim in the filesystem image.
# So for example, if the local tree contains:
#	./xxx/
#	./xxx/yyy/
#	./xxx/yyy/zzz/
#	./xxx/yyy/zzz/README.txt
# then this same tree will be created in the filesystem image.
#
# Under Linux/BSD/UN*X, this script is equivalent to:
#	$ mount -o loop <image-file> <mountpoint>
#	$ cp -r <image-contents> <mountpoint>
#
# Under Windows/cygwin, though, the image-file cannot be mounted this way.
# This script is a workaround to accomplish the same task, in any
# environment/OS that supports the e2fsprogs package.
#
# Usage:
#	% sh load-image.sh <image-file> <image-contents>
#		where <image-file> is an existing ext2 filesystem image;
#		and <image-contents> is a directory containing the desired
#		tree/files/layout to be written to the image.
#


if [ $# -ne 2 ]; then
	echo "Usage: $0 <image-file> <image-contents>"
	exit 1
fi


#
# Required ext2 tools, included with the e2fsprogs package.  The exact
# installation path here varies depending on the OS/environment
#
DEBUGFS="/usr/sbin/debugfs"
E2FSCK="/usr/sbin/e2fsck"


#
# Determine the absolute path to the image/output file
#
pushd `dirname $1` > /dev/null
IMAGE_PATH=`pwd`
popd > /dev/null
IMAGE_FILE=${IMAGE_PATH}/`basename $1`
echo "Copying $2 to filesystem image ${IMAGE_FILE} ..."


#
# Temporarily switch to the tree containing the floppy contents
#
pushd $2 > /dev/null
if [ $? -ne 0 ]; then
	echo "No such image directory: $2"
	exit 1
fi


#
# First recreate the directory tree, without any files
#
DIRS=`find . -type d`
for dir in ${DIRS}; do
	# Skip the current/root directory, since it's created automatically when
	# the filesystem is created
	if [ "${dir}" == "." ]; then
		continue
	fi

	# Create this directory in the image
	${DEBUGFS} -R "mkdir ${dir}" -w ${IMAGE_FILE}
done


#
# Now install any files
#
FILES=`find . -type f`
for file in ${FILES}; do
	# Write the file creation commands to a temporary file
	COMMAND_FILE=`mktemp`
	echo "cd `dirname $file`" > ${COMMAND_FILE}
	echo "write $file `basename $file`" >> ${COMMAND_FILE}

	# Write this input file to the image
	${DEBUGFS} -f ${COMMAND_FILE} -w ${IMAGE_FILE}

	# Clean up
	rm -f ${COMMAND_FILE}
done

popd > /dev/null


#
# Examine the resulting image for any errors
#
${E2FSCK} -f -n ${IMAGE_FILE}
