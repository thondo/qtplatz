#!/bin/bash

function find_qmake() {
    local __arch=`uname`
    local __result=$1

    case "${__arch}" in
		Linux*)
			local hints=( 	  "/Qt/5.15.1" "/Qt/5.15.0" \
							  "/Qt/5.14.2" "/Qt/5.14.1" \
							  "/Qt/5.12.7" \
							  "/Qt/5.11.2" )
			;;
		*)
			local hints=( "/Qt/5.15.2" "/Qt/5.15.1" "/Qt/5.15.0" \
							  "/Qt/5.14.2" "/Qt/5.14.1" \
							  "/Qt/5.12.7" \
							  "/Qt/5.11.2" )
			;;
	esac

    case "${__arch}" in
	Linux*)
	    local __dirs=()
	    for hint in "${hints[@]}"; do
			__dirs+=("/opt$hint/gcc_64")
	    done
	    ;;
	Darwin*)
	    local home=~
	    local __dirs=()
	    for hint in "${hints[@]}"; do
			__dirs+=("$home$hint/clang_64")
			__dirs+=("/opt$hint/clang_64")
	    done
	    ;;
	*)
	    echo "######## unknown arch: " $__arch
	    ;;
    esac

    for dir in "${__dirs[@]}"; do
		echo "-----------" $dir
		if [ -f $dir/bin/qmake ]; then
			if $dir/bin/qmake --version &> /dev/null ; then
				eval $__result="'$dir/bin/qmake'"
				return 0; #true
			fi
		fi
    done
    return 1; #false
}
