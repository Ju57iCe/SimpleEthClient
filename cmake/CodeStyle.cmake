find_program(
	CLANG_TIDY_EXE
    NAMES "clang-tidy"
	DOC "Path to clang-tidy executable"
)

set(CLANG_TIDY_LOWER_CASE_CONF "\
-config={ \
Checks: '-*,readability-identifier-naming', \
CheckOptions: \
  - { key: readability-identifier-naming.ClassCase, value: lower_case } \
  - { key: readability-identifier-naming.StructCase, value: lower_case } \
  - { key: readability-identifier-naming.PublicMemberCase, value: lower_case } \
  - { key: readability-identifier-naming.ProtectedMemberCase, value: lower_case } \
  - { key: readability-identifier-naming.ProtectedMemberSuffix, value: '_'} \
  - { key: readability-identifier-naming.PrivateMemberCase, value: lower_case } \
  - { key: readability-identifier-naming.PrivateMemberSuffix, value: '_'} \
  - { key: readability-identifier-naming.FunctionCase, value: lower_case } \
  - { key: readability-identifier-naming.ParameterCase, value: lower_case } \
  - { key: readability-identifier-naming.EnumCase, value: aNy_CasE} \
  - { key: readability-identifier-naming.EnumConstantCase, value: aNy_CasE }\
  - { key: readability-identifier-naming.NamespaceCase, value: lower_case}\
  - { key: readability-identifier-naming.TypedefCase, value: lower_case}\
  - { key: readability-identifier-naming.TypeAliasCase, value: lower_case}\
  - { key: readability-identifier-naming.UsingCase, value: lower_case} \
  - { key: readability-identifier-naming.TypeTemplateParameterCase, value: CamelCase} \
  - { key: readability-identifier-naming.LocalConstantCase, value: 'lower_case'} \
  - { key: readability-identifier-naming.LocalVariableCase, value: 'lower_case'} \
  - { key: readability-identifier-naming.GlobalConstantCase, value: 'aNy_CasE'} \
  - { key: readability-identifier-naming.GlobalVariableCase, value: 'aNy_CasE'} \
  - { key: readability-identifier-naming.StaticVariableCase, value: lower_case} \
  - { key: readability-identifier-naming.ConstexprFunctionCase, value: lower_case} \
  - { key: readability-identifier-naming.ConstexprMethodCase, value: lower_case} \
  - { key: readability-identifier-naming.ConstexprVariableCase, value: aNy_CasE}\
} -p ${CMAKE_CURRENT_BINARY_DIR}")

message (STATUS "${${CMAKE_CURRENT_SOURCE_DIR}/build}")

set(CLANG_TIDY_CAMEL_CASE_FSM_CONF "\
-config={ \
Checks: '-*,readability-identifier-naming', \
CheckOptions: \
  - { key: readability-identifier-naming.ClassCase, value: 'CamelCase' } \
  - { key: readability-identifier-naming.StructCase, value: 'CamelCase' } \
  - { key: readability-identifier-naming.PublicMemberCase, value: 'camelBack' } \
  - { key: readability-identifier-naming.ProtectedMemberCase, value: 'camelBack' } \
  - { key: readability-identifier-naming.ProtectedMemberPrefix, value: 'm_'} \
  - { key: readability-identifier-naming.PrivateMemberCase, value: 'camelBack' } \
  - { key: readability-identifier-naming.PrivateMemberPrefix, value: 'm_'} \
  - { key: readability-identifier-naming.FunctionCase, value: 'CamelCase' } \
  - { key: readability-identifier-naming.ParameterCase, value: 'camelBack' } \
  - { key: readability-identifier-naming.EnumCase, value: 'aNy_CasE'} \
  - { key: readability-identifier-naming.EnumConstantCase, value: 'aNy_CasE' }\
  - { key: readability-identifier-naming.NamespaceCase, value: 'CamelCase'}\
  - { key: readability-identifier-naming.TypedefCase, value: 'CamelCase'}\
  - { key: readability-identifier-naming.TypeAliasCase, value: 'CamelCase'}\
  - { key: readability-identifier-naming.UsingCase, value: 'CamelCase'} \
  - { key: readability-identifier-naming.TypeTemplateParameterCase, value: 'CamelCase'} \
  - { key: readability-identifier-naming.LocalConstantCase, value: 'camelBack'} \
  - { key: readability-identifier-naming.LocalVariableCase, value: 'camelBack'} \
  - { key: readability-identifier-naming.GlobalConstantCase, value: 'aNy_CasE'} \
  - { key: readability-identifier-naming.GlobalVariableCase, value: 'aNy_CasE'} \
  - { key: readability-identifier-naming.StaticVariableCase, value: 'camelBack'} \
  - { key: readability-identifier-naming.ConstexprFunctionCase, value: 'CamelCase'} \
  - { key: readability-identifier-naming.ConstexprMethodCase, value: 'CamelCase'} \
  - { key: readability-identifier-naming.ConstexprVariableCase, value: 'aNy_CasE'}\
  - { key: readability-identifier-naming.PrivateMethodCase, value: 'camelBack'}\
  - { key: readability-identifier-naming.PrivateMethodPrefix, value: '_'}\
  - { key: readability-identifier-naming.PublicMethodCase, value: 'aNy_CasE'}\
}")

function(set_target_clang_tidy_args target ARGS_LIST)
	if(CLANG_TIDY_EXE)
		#message(STATUS "==========================")
		#message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
		#message(STATUS "clang-tidy command line: ${ARGS_LIST}")
		#message(STATUS "==========================")

		set(DO_CLANG_TIDY "${CLANG_TIDY_EXE}"
				"${ARGS_LIST}"
			)
		set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

        #message(STATUS "${DO_CLANG_TIDY}")
		set_target_properties(${target} PROPERTIES
			CXX_CLANG_TIDY "${DO_CLANG_TIDY}"
		)
	endif()
endfunction()

function(make_headers_regular_expression)
    file(GLOB_RECURSE headers "*.h")
    set(name_string "")
    foreach(file ${headers})
        get_filename_component(STRIPPED ${file} NAME_WE)

        if("${name_string}" STREQUAL "")
            set(name_string "${STRIPPED}")
        else()
            set(name_string "${name_string}|${STRIPPED}")
        endif()
    endforeach(file)

    set(headers_to_check ${name_string} PARENT_SCOPE)
endfunction()

function(set_code_style target preset enable_headers extra)

if(preset STREQUAL "lower_case")
    set(tidy_check_config ${CLANG_TIDY_LOWER_CASE_CONF})
endif()

if(preset STREQUAL "CamelCase")
    set(tidy_check_config ${CLANG_TIDY_CAMEL_CASE_CONF})
endif()

if(NOT IGNORE_CODE_STYLE_CHECKS)
    if(enable_headers MATCHES "check_headers")
        make_headers_regular_expression(headers_to_check)
        set(extra_flags "-header-filter=(${headers_to_check})")
        set(extra ${extra} ${extra_flags})
    endif()

	message(STATUS "========================")
	message(STATUS "target : ${target}")
	message(STATUS "requested style preset : ${preset}")
	message(STATUS "========================")

	set(args ${extra} ${tidy_check_config})
	set_target_clang_tidy_args(${target} "${args}")
endif()
endfunction()