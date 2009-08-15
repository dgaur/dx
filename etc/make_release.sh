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
# Build the actual media images
#
function build_image
	{
	output_dir=releases/$1/$2	# releases/<version>/<retail-or-debug>

	# Clean/create the output directory
	rm -rf ${output_dir}
	mkdir -p ${output_dir}

	# Clean the entire tree, even the debug files
	make clean DEBUG=1

	# Build everything
	make media INCLUDE_ARCHIVE=1 $3
	if [ $? -ne 0 ]; then
		echo "Build failed (options $3)"
		exit 1
	fi

	# Save the media images
	floppy_image="${DX_ROOT_DIR}/media/floppy/dx.vfd"
	iso_image="${DX_ROOT_DIR}/media/iso/dx.iso"
	for image in ${floppy_image} ${iso_image}; do
		cp ${image} ${output_dir}
		if [ $? -ne 0 ]; then
			echo "Missing ${image}"
			exit 1
		fi
	done
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
	echo
	echo "Build and package a new OS release"
	echo
	echo "Usage: $0 [-c] [-d] <version>"
	echo "    -c        Commit version changes back to local repository"
	echo "    -d        Enable debug output"
	echo "    version   New version number (e.g., x.y.z), must be single token"
	echo
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

# Makefiles all depend on DX_ROOT_DIR
if [ -z "${DX_ROOT_DIR}" ]; then
	echo "Cannot build when DX_ROOT_DIR is not set"
	exit 1
fi


RELEASE_DATE=`date +%Y-%m-%d`


# Build from the root of the tree
cd ${DX_ROOT_DIR}


# Insert the new version information into the appropriate places
update_readme			${NEW_VERSION} ${RELEASE_DATE}
update_release_notes	${NEW_VERSION} ${RELEASE_DATE}
update_version_h		${NEW_VERSION} ${RELEASE_DATE}


# Commit these changes so that they are properly reflected/reported in the
# release build
if [ -n "${ENABLE_COMMITS}" ]; then
	echo
	#@commit -m "Automatic update for version ${NEW_VERSION}"
	#@tag ...
fi


# Build the actual release images
build_image ${NEW_VERSION} "production" ""
build_image ${NEW_VERSION} "debug" "DEBUG=1"


# To avoid confusion in subsequent builds, update the version header with a
# placeholder value
if [ -n "${ENABLE_COMMITS}" ]; then
	# Avoid overwriting the prior (uncommitted) changes
	update_version "post-${NEW_VERSION}" "unreleased"
	#@commit
fi


# Brief summary
echo
echo "Built new release, version ${NEW_VERSION}, release date ${RELEASE_DATE}"
if [ -n "${ENABLE_COMMITS}" ]; then
	echo "All changes committed back to local repository"
else
	echo "No changes committed; all changes still pending in local sandbox"
fi
echo


# Done
cd -



