# Options:
#
# declare_internal_datasource (FILE <datasource name> URL <url> URL_HASH <algo>=<hashValue> CONFIGURE <true|false>)
#
# FILE <datasource name> (required):
#    The name of the downloaded file (must be unique across all declared datasources).
#    This will put the file into the ``<build>/data/<datasource name>`` folder.
#
# URL <url> (required):
#    The path of the internal datasource.
#
# URL_HASH <algo>=<hashValue> (required):
#    Hash of the archive file to be downloaded. The argument should be of the form <algo>=<hashValue> where algo can be
#    any of the hashing algorithms supported by the file() command. Specifying this option is strongly recommended for
#    URL downloads, as it ensures the integrity of the downloaded content. It is also used as a check for a previously
#    downloaded file, allowing connection to the remote location to be avoided altogether if the local directory already
#    has a file from an earlier download that matches the specified hash.
#
# CONFIGURE <true|false> (required):
#    If true, replace "@data_dir@" inside files with the actual data directory.
function (declare_internal_datasource)
    set (options "")
    set (one_value_args FILE URL_HASH URL CONFIGURE)
    set (multi_value_args "")
    set (data_dir "${CMAKE_CURRENT_BINARY_DIR}/data")

    cmake_parse_arguments (ARG "${options}" "${one_value_args}" "${multi_value_args}" ${ARGN})

    string (REPLACE "=" ";" url_hash_args ${ARG_URL_HASH})
    list (GET url_hash_args 0 hash_algorithm)
    list (GET url_hash_args 1 expected_hash_value)
    file ("${hash_algorithm}" "${ARG_URL}" actual_hash_value)

    if (NOT "${expected_hash_value}" STREQUAL "${actual_hash_value}")
        message (SEND_ERROR "DOWNLOAD HASH mismatch\n"
                            "  for file: [${ARG_FILE}]\n"
                            "    expected hash: [${expected_hash_value}]\n"
                            "      actual hash: [${actual_hash_value}]\n")
    endif ()

    file (MAKE_DIRECTORY ${data_dir})

    if (ARG_CONFIGURE)
        configure_file (${ARG_URL} "${data_dir}/${ARG_FILE}" @ONLY)
    else ()
        configure_file (${ARG_URL} "${data_dir}/${ARG_FILE}" COPYONLY)
    endif ()
endfunction ()
