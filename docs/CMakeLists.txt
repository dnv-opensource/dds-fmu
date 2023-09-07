find_package(Doxygen REQUIRED) # COMPONENTS dot

message(STATUS "Use custom css themes from doxygen-awesome-css")
set(_doxy_css_version 2.2.1)
include(FetchContent)
set(_css_content doxygen_awesome_css)
if(${CMAKE_VERSION} VERSION_GREATER 3.23)
  set(_download_timestamp DOWNLOAD_EXTRACT_TIMESTAMP YES)
endif()
FetchContent_Declare(
  ${_css_content}
  URL "https://github.com/jothepro/doxygen-awesome-css/archive/refs/tags/v${_doxy_css_version}.tar.gz"
  ${_download_timestamp}
  )

FetchContent_MakeAvailable(${_css_content})

set(DOXYGEN_DOT_IMAGE_FORMAT svg)
set(DOXYGEN_DOT_TRANSPARENT YES)
set(DOXYGEN_GENERATE_TREEVIEW YES)
set(DOXYGEN_INTERACTIVE_SVG YES)
list(APPEND DOXYGEN_HTML_EXTRA_FILES
  "${CMAKE_BINARY_DIR}/_deps/doxygen_awesome_css-src/doxygen-awesome-darkmode-toggle.js")
list(APPEND DOXYGEN_HTML_EXTRA_STYLESHEET
  "${CMAKE_BINARY_DIR}/_deps/doxygen_awesome_css-src/doxygen-awesome.css"
  "${CMAKE_BINARY_DIR}/_deps/doxygen_awesome_css-src/doxygen-awesome-sidebar-only.css"
  "${CMAKE_BINARY_DIR}/_deps/doxygen_awesome_css-src/doxygen-awesome-sidebar-only-darkmode-toggle.css"
  "${CMAKE_BINARY_DIR}/docs/custom.css")
set(DOXYGEN_HTML_COLORSTYLE_HUE 209)
set(DOXYGEN_HTML_COLORSTYLE_SAT 255)
set(DOXYGEN_HTML_COLORSTYLE_GAMMA 113)
set(DOXYGEN_HTML_DYNAMIC_SECTIONS YES)
set(DOXYGEN_TREEVIEW_WIDTH 340) # Must be the same as --side-nav-fixed-width
set(DDSFMU_CUSTOM_TOP_HEIGHT 135)
configure_file(custom.css.in custom.css)

set(DOXYGEN_PROJECT_NAME ${CMAKE_PROJECT_NAME})
#set(DOXYGEN_PROJECT_LOGO "${CMAKE_CURRENT_SOURCE_DIR}/images/ddsfmu_logo.png")

list(APPEND DOXYGEN_EXAMPLE_PATH "${CMAKE_SOURCE_DIR}/examples")
set(DOXYGEN_EXTRACT_ALL YES)
set(DOXYGEN_EXTRACT_PRIVATE NO)
set(DOXYGEN_GENERATE_TAGFILE "${CMAKE_CURRENT_BINARY_DIR}/html/${CMAKE_PROJECT_NAME}.tag")
set(DOXYGEN_HIDE_UNDOC_CLASSES NO)
set(DOXYGEN_HIDE_UNDOC_MEMBERS YES)
set(DOXYGEN_IMAGE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/pdf")
#list(APPEND DOXYGEN_DOTFILE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/dot")
set(DOXYGEN_RECURSIVE YES)
set(DOXYGEN_REPEAT_BRIEF NO)
set(DOXYGEN_SHOW_FILES NO)
set(DOXYGEN_SHOW_USED_FILES NO)
set(DOXYGEN_USE_MATHJAX YES)
set(DOXYGEN_USE_MDFILE_AS_MAINPAGE "${CMAKE_CURRENT_SOURCE_DIR}/index.md")

list(APPEND DOXYGEN_CITE_BIB_FILES "${CMAKE_CURRENT_SOURCE_DIR}/user/bibs.bib")

set(DOXYGEN_WARN_IF_UNDOCUMENTED NO)

set(DOXYGEN_EXCLUDE_PATTERNS "*/temporary/*")

doxygen_add_docs(doc
  "${CMAKE_CURRENT_SOURCE_DIR}/index.md"
  "${CMAKE_CURRENT_SOURCE_DIR}/user"
  "${CMAKE_SOURCE_DIR}/src"
  COMMENT "Doxygen documentation target")

install(
  DIRECTORY "${CMAKE_BINARY_DIR}/docs/html/"
  DESTINATION "${fmuStagingDir}/documentation"
  COMPONENT doc
  EXCLUDE_FROM_ALL
  PATTERN "Makefile" EXCLUDE
  )
