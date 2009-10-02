#!/bin/sh
#
# make_release.sh
#
# Build and archive a full release of the OS.  Updates version number and
# release documentation automatically.
#
# Usage:
#	% sh make_release.sh [-c] [-d] <NEW-VERSION-STRING>
#		-c			Enables commits to the local repository
#		-d			Enables debug output
#		<version>	The new version string, must be a single token
#



#
# Build the actual media images from source
#
function build_image
	{
	local saved_image=releases/dx-$1-$2		# dx-<version>-<retail-or-debug>

	# Create the output directory
	mkdir -p `dirname ${saved_image}`

	# Clean the entire tree, even the debug files
	make clean DEBUG=1

	# Build everything.  Include a clean copy (archive) of the current project
	# tree + documentation, so that these may be included in the release
	# package  @@doxygen is reading the hg tree here, not the archive tree
	make archive media $3
	if [ $? -ne 0 ]; then
		echo "Build failed (options $3)"
		exit 1
	fi

	# Save the floppy image
	built_floppy_image="${DX_ROOT_DIR}/media/floppy/dx.vfd"
	saved_floppy_image="${saved_image}.vfd"
	save_image ${built_floppy_image} ${saved_floppy_image}

	# Save the .iso image
	built_iso_image="${DX_ROOT_DIR}/media/iso/dx.iso"
	saved_iso_image="${saved_image}.iso"
	save_image ${built_iso_image} ${saved_iso_image}

	return
	}


#
# Save a copy of a release image.  This is mainly useful for archiving old
# releases; and for posting release images online
#
function save_image
	{
	local built_image=$1
	local saved_image=$2

	# Save the image to the release directory
	cp -f ${built_image} ${saved_image}
	if [ $? -ne 0 ]; then
		echo "Missing ${built_image}"
		exit 1
	fi

	# Generate an MD5 signature for the image
	md5sum ${saved_image} > ${saved_image}.md5

	return
	}


#
# Patch the README with the new version information
#
function update_readme
	{
	sed -i "s/\(This is the dx operating system\).*/\1, version $1, released $2/" README
	}


#
# Patch the release notes with the new version information
#
function update_release_notes
	{
	sed -i "s/\(.*PENDING-RELEASE.*\)/\1\n\n\* Version $1, released $2:/" \
		doc/release_notes.txt
	}


#
# Patch the version.h file with the latest version information
#
function update_version_h
	{
	local header="src/inc/dx/version.h"

	sed -i "s/.*DX_VERSION.*/#define DX_VERSION         \"$1\"/" ${header}
	sed -i "s/.*DX_RELEASE_DATE.*/#define DX_RELEASE_DATE    \"$2\"/" ${header}
	}


#
# Show a brief help/usage message and then exit
#
function usage
	{
	cat <<-END_OF_USAGE

Build and package a new OS release

Usage: $0 [-c] [-d] <version>
    -c        Commit version changes back to local repository
    -d        Enable debug output
    version   New version number (e.g., x.y.z), must be single token

END_OF_USAGE

	exit 1
	}




#
# main()
#

# Parse the command line arguments, if any
for argument in "$@"; do
	case ${argument} in
		-c|--commit)
			# Enable commits to the local source tree
			ENABLE_COMMITS="yes"
			;;

		-d|--debug)
			# Debug
			set -x
			;;

		-h|-?|--help)
			# Help
			usage
			;;

		*)
			# Assume this is the desired version string
			if [ -n "$NEW_VERSION" ]; then
				usage
			fi

			NEW_VERSION=${argument}
			;;
	esac
done


# Must provide a version number
if [ -z "${NEW_VERSION}" ]; then
	usage
fi


# The various Makefiles all depend on DX_ROOT_DIR
if [ -z "${DX_ROOT_DIR}" ]; then
	echo "Cannot build when DX_ROOT_DIR is not set"
	exit 1
fi


# Build from the root of the current tree
local_tree=`dirname $0`
pushd ${local_tree}/..


# This should now be the DX_ROOT_DIR.  If not, then the current source tree
# is not the DX_ROOT_DIR tree; and thus the links will be incorrect, the
# include files will be incorrect; etc.
tempfile="tmp.$$"
touch ${tempfile}
if [ ! -e ${DX_ROOT_DIR}/${tempfile} ]; then
	echo "DX_ROOT_DIR mismatch"
	exit 1
fi
rm ${tempfile}


# Automatically infer the release date
RELEASE_DATE=`date +%Y-%m-%d`


# Insert the new version information into the appropriate places
update_readme			${NEW_VERSION} ${RELEASE_DATE}
update_release_notes	${NEW_VERSION} ${RELEASE_DATE}
update_version_h		${NEW_VERSION} ${RELEASE_DATE}


# Commit these changes so that they are properly reflected/reported in the
# release build
if [ -n "${ENABLE_COMMITS}" ]; then
	hg commit -m "Automatic update for version ${NEW_VERSION}"
	hg tag -m "Automatic tag for version ${NEW_VERSION}" \
		"version-${NEW_VERSION}"
fi


# Build the actual release images
build_image ${NEW_VERSION} "production" ""
build_image ${NEW_VERSION} "debug" "DEBUG=1"


# To avoid confusion in subsequent builds, update the version header with a
# placeholder value
if [ -n "${ENABLE_COMMITS}" ]; then
	# Avoid overwriting the prior (uncommitted) changes
	update_version_h "post-${NEW_VERSION}" "unreleased"
	hg commit -m "Automatic update after committing version ${NEW_VERSION}"
fi


# Brief summary of results
echo
echo "Built new release, version ${NEW_VERSION}, release date ${RELEASE_DATE}"
if [ -n "${ENABLE_COMMITS}" ]; then
	echo "All changes committed back to local repository"
else
	echo "No changes committed; all changes still pending in local sandbox"
fi
echo


# Done
popd



