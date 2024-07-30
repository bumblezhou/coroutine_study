macro(add_custom_exe TARGET_NAME)
    # message("+------------------------------------------------------------+")
    # message("| --> ARGV: <<< ${ARGV} >>>")
	message("+------------------------------------------------------------+")
    message("| --> TARGET_NAME: <<< ${TARGET_NAME} >>>")
	message("+------------------------------------------------------------+")
    add_executable(${ARGV}
    )
    target_include_directories(${TARGET_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/include
    )
    target_link_libraries(${TARGET_NAME} PRIVATE
        pthread
    )
    target_compile_features(${TARGET_NAME} PRIVATE
        cxx_std_20
    )
endmacro()

add_custom_exe(sample_01
  sample_01.cpp
)

add_custom_exe(sample_02
  sample_02.cpp
)

add_custom_exe(sample_03
  sample_03.cpp
)