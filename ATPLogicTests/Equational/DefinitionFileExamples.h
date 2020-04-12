#pragma once


// json string representing a full group theory context
static const char* group_theory_definition_file = "{\n"
"	name : \"Group Theory\",\n"
"definitions : [\n"
"{ name: \"e\", arity : 0 },\n"
"{ name: \"i\", arity : 1 },\n"
"{ name: \"*\", arity : 2 },\n"
"], \n"
"axioms : [ \n"
"\"*(*(x, y), z) = *(x, *(y, z))\",\n"
"\"*(x, e) = x\",\n"
"\"*(e, x) = x\",\n"
"\"*(x, i(x)) = e\",\n"
"\"*(i(x), x) = e\",\n"
"] }"
;


