include_guard()

# Solution taken from https://www.linkedin.com/pulse/simple-elegant-wrong-how-integrate-clang-format-friends-brendan-drew/
find_program(CLANG_FORMAT_EXECUTABLE clang-format)

function(add_clangformat _targetname)
    if (CLANG_FORMAT_EXECUTABLE)
        if (NOT TARGET ${_targetname})
            message(FATAL_ERROR "add_clangformat should only be called on targets(got " ${_targetname} ")")
        endif ()

        # figure out which sources this should be applied to
        get_target_property(_clang_sources ${_targetname} SOURCES)
        get_target_property(_builddir ${_targetname} BINARY_DIR)


        set(_sources "")
        foreach (_source ${_clang_sources})
            # remove cmake generator expressions if exists
            string(REGEX REPLACE "([^:]+:)" "" _source "${_source}")
            string(REGEX REPLACE ">" "" _source "${_source}")

            if (NOT TARGET ${_source})
                get_filename_component(_source_file ${_source} NAME)
                get_source_file_property(_clang_loc "${_source}" LOCATION)

                set(_format_file ${_targetname}_${_source_file}.format)

                add_custom_command(OUTPUT ${_format_file}
                        DEPENDS ${_source}
                        COMMENT "Clang-Format ${_source}"
                        COMMAND ${CLANG_FORMAT_EXECUTABLE} -style=file -fallback-style=WebKit -i ${_clang_loc}
                        COMMAND ${CMAKE_COMMAND} -E touch ${_format_file})

                list(APPEND _sources ${_format_file})
            endif ()
        endforeach ()

        if (_sources)
            add_custom_target(${_targetname}_clangformat
                    SOURCES ${_sources}
                    COMMENT "Clang-Format for target ${_target}")

            add_dependencies(${_targetname} ${_targetname}_clangformat)
        endif ()

    endif ()
endfunction()

function(clangformat_folder)
    if (CLANG_FORMAT_EXECUTABLE)

        foreach (_folder ${ARGN})
            # figure out which sources this should be applied to
            file(GLOB_RECURSE _clang_sources
                    ${_folder}/*.c
                    ${_folder}/*.cxx
                    ${_folder}/*.cpp
                    ${_folder}/*.h
                    )
            set(_sources "")

            file(RELATIVE_PATH _rel_path ${CMAKE_CURRENT_SOURCE_DIR} ${_folder})
            string(REPLACE "/" "_" _dir_name ${_rel_path})

            foreach (_source ${_clang_sources})

                # remove cmake generator expressions if exists
                string(REGEX REPLACE "([^:]+:)" "" _source "${_source}")
                string(REGEX REPLACE ">" "" _source "${_source}")

                if (NOT TARGET ${_source})
                    string(REPLACE "/" "_" _source_file ${_source})
                    get_source_file_property(_clang_loc "${_source}" LOCATION)

                    set(_format_file ${CMAKE_CURRENT_BINARY_DIR}/${_dir_name}_${_source_file}.format)

                    add_custom_command(OUTPUT ${_format_file}
                            DEPENDS ${_source}
                            COMMENT "Clang-Format ${_source}"
                            COMMAND ${CLANG_FORMAT_EXECUTABLE} -style=file -fallback-style=WebKit -i ${_clang_loc}
                            COMMAND ${CMAKE_COMMAND} -E touch ${_format_file})

                    list(APPEND _sources ${_format_file})
                endif ()
            endforeach ()

            if (NOT TARGET clang-format)
                add_custom_target(clang-format)
            endif ()

            if (_sources)
                add_custom_target(clangformat_${_dir_name}
                        SOURCES ${_sources}
                        COMMENT "Clang-Format for folder ${_dir_name}")

                add_dependencies(clang-format clangformat_${_dir_name})
            endif ()
        endforeach ()

    endif ()
endfunction()
