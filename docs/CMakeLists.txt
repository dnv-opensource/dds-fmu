find_package(Doxygen REQUIRED) # COMPONENTS dot

message(STATUS "Use custom css themes from doxygen-awesome-css")
set(_doxy_css_version 2.3.2)
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
  "${CMAKE_BINARY_DIR}/_deps/doxygen_awesome_css-src/doxygen-awesome-darkmode-toggle.js"
  "${CMAKE_BINARY_DIR}/_deps/doxygen_awesome_css-src/doxygen-awesome-fragment-copy-button.js"
  "${CMAKE_BINARY_DIR}/_deps/doxygen_awesome_css-src/doxygen-awesome-paragraph-link.js"
  "${CMAKE_SOURCE_DIR}/data/images/favicon.ico"
  )
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

set(_match_string_1 "</head>")
set(_rep_string_1 "\
<link rel=\"icon\" href=\"$relpath^favicon.ico\" type=\"image/x-icon\" />\
<script type=\"text/javascript\" src=\"$relpath^doxygen-awesome-darkmode-toggle.js\"></script>\
<script type=\"text/javascript\" src=\"$relpath^doxygen-awesome-fragment-copy-button.js\"></script>\
<script type=\"text/javascript\" src=\"$relpath^doxygen-awesome-paragraph-link.js\">\
</script><script type=\"text/javascript\">\
DoxygenAwesomeDarkModeToggle.init(); \
DoxygenAwesomeFragmentCopyButton.init(); \
DoxygenAwesomeParagraphLink.init();</script></head>")
set(_match_string_2 "id=\"projectname\">$projectname")
set(_rep_string_2 "id=\"projectname\"><a href=\"$relpath^index.html\">$projectname</a>")

set(NFR_GRANT "309660")
set(_match_string_3 "\$navpath")
set(_rep_string_3 "\$navpath\
    <li class=\"footer\"><a href=\"https://github.com/dnv-opensource/${CMAKE_PROJECT_NAME}/releases\"><img alt=\"fmu-artifacts\" style=\"vertical-align:middle\" src=\"https://img.shields.io/badge/Artifacts-FMU-red.svg?style=flat-square&color=c8102e&labelColor=ffffff&logo=data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACgAAAAgCAYAAABgrToAAAAACXBIWXMAABcRAAAXEQHKJvM/AAAA6UlEQVRYw2NkIACM7T3+w9hnD+5gZKAzYCLWcQMFmAaz4xgYGBhYcDhuAxLX8OzBHRcGWwj6I6W7C4MyigcLIORAx8HuwAEHjKTmVuSyEJde9PKSWHXY1DNR23HocsSqw1fMNGIRr4fSCxkYGB4QaXAjkj6i1MDU4auhCAWx49mDOw6QWv2hO46QGnxJgoWSBIzH5xMZGBjyCeklJm2SnIuJSTdnD+4oIKWBgc9MlsFYtJCdi0cL6lEHjjpw1IHDsblFTl08GoKjDhx14FAAg2Xog/GEgB6GQyw+XGJkYGBgwCY3KBw4mAAAR4FoEwTL8p4AAAAASUVORK5CYII=\"/></a></li>\
    <li class=\"footer\"><a href=\"https://github.com/dnv-opensource/${CMAKE_PROJECT_NAME}\"><img alt=\"source-code\" style=\"vertical-align:middle\" src=\"https://img.shields.io/badge/Github-Code-gray.svg?style=flat-square&logo=github&color=red\"/></a></li>\
    <li class=\"footer\"><img alt=\"funding\" style=\"vertical-align:middle\" src=\"https://img.shields.io/badge/Research%20Council%20of%20Norway-Grant:%20${NFR_GRANT}-gray.svg?style=flat-square&labelColor=ffffff&logo=data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABwAAAAcCAAAAABXZoBIAAAAnElEQVR4AczJERDEMBSE4V8Ctc5Uz+XczjFerUNd4j5xPKpGQnWt2/nUYnuTCbV5c3wLb9/Oh37kXzET2zO7aRqHfEEHZ+1EywVH2GovzeIdfW0AvxXdkSLt+FR0hE+HWVrrfMOzQ68CRQF4dEg9OIBXhy0DwGxjUQQWE1dJBwQT9zrOJBPVYuJiomuYvsMeZZwCwsKskYM/geEBAKDGccZO9+2TAAAAAElFTkSuQmCC\"/></li>\
    <li class=\"footer\"><a href=\"https://www.sintef.no/ocean\"><img alt=\"sintef\" style=\"vertical-align:middle\" src=\"https://img.shields.io/badge/SINTEF-Ocean-white.svg?style=flat-square&labelColor=023366&color=023366&logo=data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAMAAABEpIrGAAACc1BMVEUAAAAiIiJNTU1OTk5oaGh4eHiEhISGhoaHh4eIiIiJiYmLi4uTk5OdnZ2qqqqvr6+1tbW2tra3t7e4uLi5ubm6urr////6+vr7+/v+/v78/Pz9/f36+vr7+/v7+/v8/Pz7+/v8/Pz7+/v7+/v8/Pz8/Pz7+/v8/Pz7+/v8/Pz7+/v8/Pz7+/v8/Pz7+/v8/Pz8/Pz7+/v8/Pz9/f37+/v7+/v8/Pz8/Pz7+/v8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Pz8/Py89OkCAAAA0HRSTlMAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAEBAQICAwMEBAUFBgcHCAkJCgoLDA0NDw8QEREREhMUFRYWFxkdHyAiJCUnKissLS4vMDEyMzQ2Nzk6PD9CQ0RFRkpLTE5PUFFUVVdZWltdXl9gYmRlZ2prbG1ub3Fyc3R3eHx9foCBgoeIio+RkpOVmJ2en6ChoqOlpqeoq7GytLa4ubq7vL2+v8DBwsPExcbLzs/Q1NXW19jZ293e3+Hi4+Tl5ufo7O3v8PHy8/T19vf4+fr7/P3+kKTZQQAAAqJJREFUeNps02OjM1cUBeBVZpK6jV7btm3bNq5t27Zt2/bNzfpJPZOZus/nw73Xxt8Ylh06d/PWlaNr9Pgf2gtBZSMUevKb6qOfrMI//fKgkioPGPa9yep2X4u/2ZZMoTnG+/3zp7sh/Hi2oPGiBlAdrCNHQk4ZzAeu3n1w69gqCcAiv/EHEhQbqsm0A8b78T1WMiO2pDfXfiMgeU5egY0ugvTWn1UeMbsLWPmoaOCdDl+ntm6C7NICgwyPJsiF4oC3t3QQTHZz4WYcGAmUL/k+neVLrk3TGn9MgrTn8Gb9N4D02uqv0YWMbAVwaM56ZXkD59/Kq3dM09IedVoDTZzlMk7RDsBn5v38kXSUIHyibM5FwjFL5k+GhmwJSOT7X8tYYYagzaPizReaQssRRI+tgr6Bh7bP8xlkm2ep6F8LX36AG09j6VSH/g7HV0N2jCqx4RXD8ZKPsXshBx9ZYoBsZTdVwbjDTDzgZxxhHFwZA8X5MioScJtluEdnsSBSLAiDSnc8SV3wRCx4SmdxRZS4Ihx/MlUpV7iKK+z5WTwyWVyX/CX+FGZ7pDaF4QgVj9Q3lH21fb7W8Fe00m3f3DTCD9pSnhaFGjL+WmbZBxXWDMuF+s6DliM750Sh8JlnxCPcoMIzW6lPjTLzJweKUotm+WB5Q+9WKDTJolnfnmyj5fKKZtrZ2t2yGNemUxerC9brtZvcR0j/3zw4slUJzFvoHk2kbIHsxAv3lBGS4eYb0wyUlMj17YfmbGWX014J2mzKBt79cH2QInJqaIvWASK0nTWpEfPkmAjtcrdpKqFVYl8of1MnYn9Pjr1+h30jOfFA+tvgdL+yPVL/8Olr57DyOZJ/DY46evVepw04ZqWiV4zef4e3vbKawszvQ868BLM/AHN6VvaH1lCDAAAAAElFTkSuQmCC\"/></a></li>\
    <li class=\"footer\"><a href=\"https://www.dnv.com/research\"><img alt=\"sintef\" style=\"vertical-align:middle\" src=\"https://img.shields.io/badge/DNV-GRD-99D9F0.svg?style=flat-square&labelColor=white&logo=data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHZpZXdCb3g9IjAgMCA2MTIuMjcgMjYxLjQ5Ij48ZGVmcz48c3R5bGU+LmNscy0xe2ZpbGw6Izk4ZDlmMDt9LmNscy0ye2ZpbGw6IzNmOWMzNTt9LmNscy0ze2ZpbGw6IzAwMzU5MTt9LmNscy00e2ZpbGw6IzBmMjA0Yjt9PC9zdHlsZT48L2RlZnM+PGcgaWQ9IkxheWVyXzIiIGRhdGEtbmFtZT0iTGF5ZXIgMiI+PGcgaWQ9IkxheWVyXzEtMiIgZGF0YS1uYW1lPSJMYXllciAxIj48cmVjdCBjbGFzcz0iY2xzLTEiIHdpZHRoPSI2MTIuMjciIGhlaWdodD0iNzMuMSIvPjxyZWN0IGNsYXNzPSJjbHMtMiIgeT0iOTcuNDUiIHdpZHRoPSI2MTIuMjciIGhlaWdodD0iMTIuMTkiLz48cmVjdCBjbGFzcz0iY2xzLTMiIHk9IjEyMS43OSIgd2lkdGg9IjYxMi4yNyIgaGVpZ2h0PSIyNC4zNSIvPjxwYXRoIGNsYXNzPSJjbHMtNCIgZD0iTTM3MC4yOCwxODYuOTFxLTktNC0yMS40OS00SDMwNi4yM3Y3OC42Mmg0Mi41NnExMi40OCwwLDIxLjQ5LTRhMjkuNzcsMjkuNzcsMCwwLDAsMTMuODQtMTIuMTVxNC44MS04LjEzLDQuODItMjAuMzl2LTUuNDZxMC0xMi4yNS00LjgyLTIwLjM4QTI5Ljc5LDI5Ljc5LDAsMCwwLDM3MC4yOCwxODYuOTFabTUuNzcsMzcuNThxMCwxMi43OC02LjgyLDE5dC0yMC4zMyw2LjE5SDMxOS4zNFYxOTQuNzJIMzQ4LjlxMTMuNTIsMCwyMC4zMyw2dDYuODIsMTkuMTNaIi8+PHBvbHlnb24gY2xhc3M9ImNscy00IiBwb2ludHM9IjQ4Ny4zNyAyNDQuMzEgNDI2LjU3IDE4Mi44NyA0MTkuMzQgMTgyLjg3IDQxMy43OSAxODIuODcgNDEzLjc5IDI2MS40OSA0MjYuNTcgMjYxLjQ5IDQyNi41NyAyMDAuMTMgNDg3LjM3IDI2MS40OSA1MDAuMjcgMjYxLjQ5IDUwMC4yNyAxODIuODcgNDg3LjM3IDE4Mi44NyA0ODcuMzcgMjQ0LjMxIi8+PHBvbHlnb24gY2xhc3M9ImNscy00IiBwb2ludHM9IjU5OC4yOCAxODIuODcgNTY4LjIgMjQ0LjMxIDUzOC4xMSAxODIuODcgNTIzLjMzIDE4Mi44NyA1NjIuNzUgMjYxLjQ5IDU3Mi45MSAyNjEuNDkgNjEyLjIyIDE4Mi44NyA1OTguMjggMTgyLjg3Ii8+PC9nPjwvZz48L3N2Zz4="/></a></li>\
    ")

# Write to files for platform-safe string handling
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/replace1 "${_match_string_1}" "\n" "${_rep_string_1}")
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/replace2 "${_match_string_2}" "\n" "${_rep_string_2}")
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/replace3 "${_match_string_3}" "\n" "${_rep_string_3}")

set(DOXYGEN_HTML_HEADER ${CMAKE_CURRENT_BINARY_DIR}/custom_header.html)
set(DOXYGEN_HTML_FOOTER ${CMAKE_CURRENT_BINARY_DIR}/custom_footer.html)

add_custom_command(OUTPUT ${DOXYGEN_HTML_HEADER} ${DOXYGEN_HTML_FOOTER}
  COMMAND Doxygen::doxygen -w html custom_header.html custom_footer.html drop_style.css
  COMMAND ${CMAKE_COMMAND} -DIN_FILE=${DOXYGEN_HTML_HEADER} -DOUT_FILE=${DOXYGEN_HTML_HEADER} -DDATA_FILE="${CMAKE_CURRENT_BINARY_DIR}/replace1" -P ${CMAKE_SOURCE_DIR}/cmake/replace-in-file.cmake
  COMMAND ${CMAKE_COMMAND} -DIN_FILE=${DOXYGEN_HTML_HEADER} -DOUT_FILE=${DOXYGEN_HTML_HEADER} -DDATA_FILE="${CMAKE_CURRENT_BINARY_DIR}/replace2" -P ${CMAKE_SOURCE_DIR}/cmake/replace-in-file.cmake
  COMMAND ${CMAKE_COMMAND} -DIN_FILE=${DOXYGEN_HTML_FOOTER} -DOUT_FILE=${DOXYGEN_HTML_FOOTER} -DDATA_FILE="${CMAKE_CURRENT_BINARY_DIR}/replace3" -P ${CMAKE_SOURCE_DIR}/cmake/replace-in-file.cmake
  COMMENT "Create custom doxygen header and footer")

add_custom_target(doc-header DEPENDS ${DOXYGEN_HTML_HEADER} ${DOXYGEN_HTML_FOOTER})

doxygen_add_docs(doc
  "${CMAKE_CURRENT_SOURCE_DIR}/index.md"
  "${CMAKE_CURRENT_SOURCE_DIR}/user"
  "${CMAKE_BINARY_DIR}/gen_md"
  "${CMAKE_SOURCE_DIR}/src"
  COMMENT "Doxygen documentation target")

add_dependencies(doc doc-header)

install(
  DIRECTORY "${CMAKE_BINARY_DIR}/docs/html/"
  DESTINATION "${fmuStagingDir}/documentation"
  COMPONENT doc
  EXCLUDE_FROM_ALL
  PATTERN "Makefile" EXCLUDE
  )
