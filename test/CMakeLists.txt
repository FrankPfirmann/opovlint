include_directories(${PROJECT_SOURCE_DIR}/test/include)
include_directories(${PROJECT_SOURCE_DIR}/src)

set(unit_test_names implicitcond_matcher union_matcher explicitcast conditionalassgn implicitconversion system explicitconstructor allimplicitconversion globalscope ifelse)

foreach(name ${unit_test_names})
  add_executable(test_${name}${EXE_SUFFIX} src/test_${name}.cpp)
  target_link_libraries(test_${name}${EXE_SUFFIX}  stdimpl )
endforeach()
