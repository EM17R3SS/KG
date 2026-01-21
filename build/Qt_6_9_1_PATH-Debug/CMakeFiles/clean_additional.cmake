# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "BezierCurve3D_autogen"
  "CMakeFiles/BezierCurve3D_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/BezierCurve3D_autogen.dir/ParseCache.txt"
  )
endif()
