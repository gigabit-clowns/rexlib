# SPDX-License-Identifier: GPL-3.0-only

find_package(Doxygen REQUIRED)

# Set color theme
set(DOXYGEN_HTML_COLORSTYLE_HUE 220)
set(DOXYGEN_HTML_COLORSTYLE_SAT 100)
set(DOXYGEN_HTML_COLORSTYLE_GAMMA 80)

# An attribute macro between a keyword and the name it declares hides the
# declaration from Doxygen, so such macros are expanded to nothing. The
# macros that declare an operation are expanded to the class they declare,
# so that the comment above each declaration documents that class.
set(DOXYGEN_MACRO_EXPANSION YES)
set(DOXYGEN_EXPAND_ONLY_PREDEF YES)
set(
	DOXYGEN_PREDEFINED
	"REXLIB_API="
	"REXLIB_MAY_ALIAS="
	"REXLIB_DECLARE_OPERATION(op, component_traits, outputs, inputs, shape, ...)=\
class op##_operation : public ::rexlib::trivial_operation<op##_operation, \
op##_operation_traits> {}"
	"REXLIB_DECLARE_PARAMETRIC_OPERATION(op, component_traits, outputs, inputs, \
shape, parameters, ...)=class op##_operation : public \
::rexlib::parametric_operation<op##_operation, op##_operation_traits> {}"
)

# Every operation derives from rexlib::operation, so its inheritance graph
# holds them all.
set(DOXYGEN_DOT_GRAPH_MAX_NODES 200)

# A warning fails the build, so that a comment naming a parameter that is not
# there does not go unnoticed. A member left undocumented is not one.
set(DOXYGEN_WARN_IF_UNDOCUMENTED NO)
set(DOXYGEN_WARN_AS_ERROR FAIL_ON_WARNINGS)

doxygen_add_docs(
	rexlib-docs
	${PROJECT_SOURCE_DIR}/include
	${CMAKE_CURRENT_SOURCE_DIR}/namespaces.dox
)
