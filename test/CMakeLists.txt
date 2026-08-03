function(add_scenario_test system)
  set(TEST_DIR ${CMAKE_CURRENT_BINARY_DIR}/scenario/${system})
  file(MAKE_DIRECTORY ${TEST_DIR})
  configure_file(scenario/${system}/ADISK.DAT ${TEST_DIR}/ADISK.DAT COPYONLY)

  add_test(
    NAME scenario_${system}
    COMMAND $<TARGET_FILE:system3>
      -gamedir ${TEST_DIR}
      -game ${system}_generic
      -encoding utf8
      -fontfile ${PROJECT_SOURCE_DIR}/resources/fonts/MTLc3m.ttf
      -texthook print
    )
  set_tests_properties(scenario_${system} PROPERTIES
    ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy"
    FAIL_REGULAR_EXPRESSION "\\[FAIL\\]"
    PASS_REGULAR_EXPRESSION "(^|[^0-9])0 tests failed"
    TIMEOUT 5
    )
endfunction()

add_scenario_test(system1)
add_scenario_test(system2)
add_scenario_test(system3)
