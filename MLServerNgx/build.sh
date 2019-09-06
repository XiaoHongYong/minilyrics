#!/bin/bash

CUR_DIR="${BASH_SOURCE-$0}"
CUR_DIR=`dirname ${CUR_DIR}`
cd ${CUR_DIR}


function exit_if_err() {
    rc=$?
    if [ $rc -ne 0 ]; then
        echo $*
        exit $rc
    fi
}

function create_lib() {
    echo "void f(){}" > tmp.c
    gcc -c tmp.c -o tmp.o
    ar rcs $1 tmp.o
    rm tmp.c
    rm tmp.o
}

required_libs=(mlserver sdk_mlserver mllib_mlserver sdk_mlserver mlprotocol_mlserver utils_mlserver base_mlserver)
created_fake_libs=()

function generate_fake_lib_for_nginx_cfg() {
    #
    # Generate fake libxx.a for txsafe-configure.sh
    #
    cd lib

    for item in ${required_libs[*]}
    do
        name="lib${item}.a"
        if [ ! -f $name ]; then
            created_fake_libs+=($name)
            create_lib $name
            printf "Created fake lib: %s\n" $item
        fi
    done

    cd ..
}

function remove_fake_lib_for_nginx_cfg() {
    cd lib

    for item in ${created_fake_libs[*]}
    do
        rm $item
        printf "Removed fake lib: %s\n" $item
    done

    cd ..
}

function clean() {
    rm lib/*
    rm product/nginx

    cd ../base
    make -f MakefileMLServer clean

    cd ../Utils
    make -f MakefileMLServer clean

    cd ../MLLib
    make -f MakefileMLServer clean

    cd ../MLProtocol
    make -f MakefileMLServer clean

    cd ../SDK
    make -f MakefileMLServer clean

    cd ../MLServerNgx
    make clean

    cd third-parties/openresty/build/nginx
    make  clean
    cd -
    
    generate_fake_lib_for_nginx_cfg
    
    cd third-parties/openresty/
    ./config-for-ubuntu.sh
    cd -

    remove_fake_lib_for_nginx_cfg
}

target=release
is_clean=false

while (($# > 0)); do
    case "$1" in
        "debug")
            target="debug"
        ;;

        "release")
            target="release"
        ;;

        "clean")
            is_clean=true
        ;;

        *)
            echo "Invalid parameters. ($1)"
            exit 1
        ;;
    esac
    shift
done

if [ "$is_clean" == "true" ]; then
    clean
fi

cd ../MLServerNgx
make -f Makefile ${target}
exit_if_err "Failed to build MLServerNgx"

cd ../base
make -f MakefileMLServer ${target}
exit_if_err "Failed to build base"

cd ../Utils
make -f MakefileMLServer ${target}
exit_if_err "Failed to build Utils"

cd ../MLLib
make -f MakefileMLServer ${target}
exit_if_err "Failed to build MLLib"

cd ../MLProtocol
make -f MakefileMLServer ${target}
exit_if_err "Failed to build MLProtocol"

cd ../SDK
make -f MakefileMLServer ${target}
exit_if_err "Failed to build SDK"
cd ../MLServerNgx

cd third-parties/openresty/build/nginx
make
exit_if_err "Failed to build nginx"
cd -

cp third-parties/openresty/build/nginx/objs/nginx product/
exit_if_err "Failed to copy nginx to product folder."

