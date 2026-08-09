function(add_scenario_test name game)
  set(TEST_DIR ${CMAKE_CURRENT_BINARY_DIR}/scenario/${name})
  file(MAKE_DIRECTORY ${TEST_DIR})
  foreach(data ADISK.DAT ${ARGN})
    configure_file(scenario/${name}/${data} ${TEST_DIR}/${data} COPYONLY)
  endforeach()

  add_test(
    NAME scenario_${name}
    COMMAND $<TARGET_FILE:system3>
      -gamedir ${TEST_DIR}
      -game ${game}
      -encoding utf8
      -fontfile ${PROJECT_SOURCE_DIR}/resources/fonts/MTLc3m.ttf
      -texthook print
    )
  set_tests_properties(scenario_${name} PROPERTIES
    ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy"
    FAIL_REGULAR_EXPRESSION "\\[FAIL\\]"
    PASS_REGULAR_EXPRESSION "(^|[^0-9])0 tests failed"
    TIMEOUT 5
    )
endfunction()

add_scenario_test(system1 system1_generic)
add_scenario_test(system2 system2_generic)
add_scenario_test(system3 system3_generic)
add_scenario_test(gakuen_king gakuen_king AMAP.DAT)
