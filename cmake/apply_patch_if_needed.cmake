cmake_minimum_required(VERSION 3.15)

if (NOT DEFINED SOURCE_DIR)
  message(FATAL_ERROR "SOURCE_DIR is required")
endif()
if (NOT DEFINED PATCH_FILE)
  message(FATAL_ERROR "PATCH_FILE is required")
endif()

if (DEFINED GIT_EXECUTABLE)
  set(_git "${GIT_EXECUTABLE}")
else()
  set(_git "git")
endif()

# 1) If the patch applies cleanly, apply it.
execute_process(
  COMMAND "${_git}" apply --check --whitespace=nowarn --ignore-space-change --ignore-whitespace "${PATCH_FILE}"
  WORKING_DIRECTORY "${SOURCE_DIR}"
  RESULT_VARIABLE _check_result
  OUTPUT_VARIABLE _check_out
  ERROR_VARIABLE _check_err
)

if (_check_result EQUAL 0)
  execute_process(
    COMMAND "${_git}" apply --whitespace=nowarn --ignore-space-change --ignore-whitespace "${PATCH_FILE}"
    WORKING_DIRECTORY "${SOURCE_DIR}"
    RESULT_VARIABLE _apply_result
    OUTPUT_VARIABLE _apply_out
    ERROR_VARIABLE _apply_err
  )
  if (NOT _apply_result EQUAL 0)
    message(FATAL_ERROR "git apply failed: ${_apply_err}")
  endif()
  message(STATUS "Applied patch: ${PATCH_FILE}")
  return()
endif()

# 2) If it does not apply, but would reverse cleanly, it's already applied.
execute_process(
  COMMAND "${_git}" apply --reverse --check --whitespace=nowarn --ignore-space-change --ignore-whitespace "${PATCH_FILE}"
  WORKING_DIRECTORY "${SOURCE_DIR}"
  RESULT_VARIABLE _reverse_check_result
  OUTPUT_VARIABLE _reverse_check_out
  ERROR_VARIABLE _reverse_check_err
)

if (_reverse_check_result EQUAL 0)
  message(STATUS "Patch already applied: ${PATCH_FILE}")
  return()
endif()

message(FATAL_ERROR "Patch neither applies nor reverses cleanly.\n\nApply-check error:\n${_check_err}\n\nReverse-check error:\n${_reverse_check_err}\n")
