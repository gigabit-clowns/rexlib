find_package(Doxygen REQUIRED)

# Set color theme
set(DOXYGEN_HTML_COLORSTYLE_HUE 220)
set(DOXYGEN_HTML_COLORSTYLE_SAT 100)
set(DOXYGEN_HTML_COLORSTYLE_GAMMA 80)

# An attribute macro between a keyword and the name it declares hides the
# declaration from Doxygen, so such macros are expanded to nothing.
set(DOXYGEN_MACRO_EXPANSION YES)
set(DOXYGEN_EXPAND_ONLY_PREDEF YES)
set(DOXYGEN_PREDEFINED "REXLIB_MAY_ALIAS=")

doxygen_add_docs(
    doxygen
    ${PROJECT_SOURCE_DIR}/include
)
