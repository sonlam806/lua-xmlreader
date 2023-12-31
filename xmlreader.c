#include "lua.h"
#include "lauxlib.h"

#include "libxml/xmlreader.h"

#include "error.h"

typedef xmlTextReaderPtr xmlreader;

static const char *const xmlreader_types[] = {
  "none",
  "element",
  "attribute",
  "text",
  "cdata",
  "entity reference",
  "entity",
  "processing instruction",
  "comment",
  "document",
  "doctype",
  "document fragment",
  "notation",
  "whitespace",
  "significant whitespace",
  "end element",
  "end entity",
  "xml declaration",
  NULL
};

static const char *const xmlreader_states[] = {
  "initial",
  "interactive",
  "error",
  "eof",
  "closed",
  "reading",
  NULL
};

static const char *const xmlreader_properties[] = {
  "load_dtd",
  "default_attrs",
  "validate",
  "subst_entities",
  NULL
};

#define LXMLREADER "xmlreader"

/*** forward definitions ***/
static void xmlreader_pusherror(lua_State *L);

/*** helpers ***/

static xmlreader push_xmlreader(lua_State *L, xmlreader xr) {
  xmlreader *xrp = (xmlreader*)lua_newuserdata(L, sizeof(xmlreader));
  *xrp = xr;
  luaL_getmetatable(L, LXMLREADER);
  lua_setmetatable(L, -2);
  return xr;
}

static xmlreader check_xmlreader(lua_State *L, int n) {
  xmlreader *xrp, xr;
  xrp = (xmlreader*)luaL_checkudata(L, n, LXMLREADER);
  xr = *xrp;
  if (xr == NULL)
    luaL_error(L, LXMLREADER" has been freed");
  return xr;
}

#define BOOL_OR_ERROR(L, var)\
  if (var != -1) {\
    lua_pushboolean(L, var);\
    return 1; }\
  else {\
    lua_pushnil(L);\
    xmlreader_pusherror(L);\
    return 2; }

static int xmlreader_gc(lua_State *L) {
  xmlreader *xrp = (xmlreader*)luaL_checkudata(L, 1, LXMLREADER);

  if (*xrp) {
    xmlFreeTextReader(*xrp);
    *xrp = NULL;
  }
  return 0;
}

static int xmlreader_tostring(lua_State *L) {
  xmlreader *xrp = (xmlreader*)luaL_checkudata(L, 1, LXMLREADER);
  if (*xrp)
    lua_pushfstring(L, "<"LXMLREADER"> (%p)", *xrp);
  else
    lua_pushfstring(L, "<"LXMLREADER"> (closed)");
  return 1;
}

/*** iterators ***/

/* reader:read() */
static int xmlreader_read(lua_State *L) {
  xmlreader xr = check_xmlreader(L, 1);

  int ret = xmlTextReaderRead(xr);
  if (ret != -1) {
    lua_pushboolean(L, ret);
    return 1;
  } else {
    lua_pushnil(L);
    xmlreader_pusherror(L);
    return 2;
  }
}

#define ERR_MSG "current node is neither an element nor attribute, or has no child nodes"
/* reader:read_inner_xml() */
static int xmlreader_read_inner_xml(lua_State *L) {
  xmlreader xr = check_xmlreader(L, 1);
  char *str = (char*)xmlTextReaderReadInnerXml(xr);
  if (str) {
    lua_pushstring(L, str);
    xmlFree(str);
    return 1;
  } else {
    lua_pushnil(L);
    lua_pushstring(L, ERR_MSG);
    return 2;
  }
}

/* reader:read_outer_xml() */
static int xmlreader_read_outer_xml(lua_State *L)
{
  xmlreader xr = check_xmlreader(L, 1);
  char *str = (char*)xmlTextReaderReadOuterXml(xr);
  if (str) {
    lua_pushstring(L, str);
    xmlFree(str);
    return 1;
  } else {
    lua_pushnil(L);
    lua_pushstring(L, ERR_MSG);
    return 2;
  }
}
#undef ERR_MSG

/* reader:read_string() */
static int xmlreader_read_string(lua_State *L) {
  xmlreader xr = check_xmlreader(L, 1);
  char *str = (char*)xmlTextReaderReadString(xr);
  if (str) {
    lua_pushstring(L, str);
    xmlFree(str);
    return 1;
  } else {
    lua_pushnil(L);
    lua_pushstring(L, "not positioned on element or text node");
    return 2;
  }
}

/* reader:read_attribute_value() */
static int xmlreader_read_attribute_value(lua_State *L)
{
  xmlreader xr = check_xmlreader(L, 1);

  int ret = xmlTextReaderReadAttributeValue(xr);
  BOOL_OR_ERROR(L, ret);
}

/*** attributes of the node ***/

/* reader:attribute_count() */
static int xmlreader_attribute_count(lua_State *L) {
  xmlreader xr = check_xmlreader(L, 1);
  int ret = xmlTextReaderAttributeCount(xr);
  if (ret != -1) {
    lua_pushinteger(L, ret);
    return 1;
  } else {
    lua_pushnil(L);
    xmlreader_pusherror(L);
    return 2;
  }
}

/* reader:depth() */
static int xmlreader_depth(lua_State *L) {
  xmlreader xr = check_xmlreader(L, 1);
  int ret = xmlTextReaderDepth(xr);
  if (ret != -1) {
    lua_pushinteger(L, ret);
    return 1;
  } else {
    lua_pushnil(L);
    xmlreader_pusherror(L);
    return 2;
  }
}

/* reader:has_attributes() */
static int xmlreader_has_attributes(lua_State *L) {
  xmlreader xr = check_xmlreader(L, 1);

  int ret = xmlTextReaderHasAttributes(xr);
  BOOL_OR_ERROR(L, ret);
}

/* reader:has_value() */
static int xmlreader_has_value(lua_State *L) {
  xmlreader xr = check_xmlreader(L, 1);

  int ret = xmlTextReaderHasValue(xr);
  BOOL_OR_ERROR(L, ret);
}

/* reader:is_default() */
static int xmlreader_is_default(lua_State *L) {
  xmlreader xr = check_xmlreader(L, 1);

  int ret = xmlTextReaderIsDefault(xr);
  BOOL_OR_ERROR(L, ret);
}

/* reader:is_empty_element() */
static int xmlreader_is_empty_element(lua_State *L) {
  xmlreader xr = check_xmlreader(L, 1);

  int ret = xmlTextReaderIsEmptyElement(xr);
  BOOL_OR_ERROR(L, ret);
}

/* reader:node_type() */
static int xmlreader_node_type(lua_State *L) {
  xmlreader xr = check_xmlreader(L, 1);

  int type = xmlTextReaderNodeType(xr);
  if (type == -1) {
    lua_pushnil(L);
    xmlreader_pusherror(L);
    return 2;
  } else {
    lua_pushstring(L, xmlreader_types[type]);
    return 1;
  }
}

/* reader:quote_char() */
static int xmlreader_quote_char(lua_State *L) {
  xmlreader xr = check_xmlreader(L, 1);

  int ret = xmlTextReaderQuoteChar(xr);
  if (ret != -1) {
    lua_pushfstring(L, "%c", ret);
    return 1;
  } else {
    lua_pushnil(L);
    xmlreader_pusherror(L);
    return 2;
  }
}

/* reader:read_state() */
static int xmlreader_read_state(lua_State *L) {
  xmlreader xr = check_xmlreader(L, 1);

  int state = xmlTextReaderReadState(xr);
  if (state != -1) {
    lua_pushstring(L, xmlreader_states[state]);
    return 1;
  }
  else {
    lua_pushnil(L);
    xmlreader_pusherror(L);
    return 2;
  }
}

/* reader:is_namespace_declaration() */
static int xmlreader_is_namespace_declaration(lua_State *L) {
  xmlreader xr = check_xmlreader(L, 1);

  int ret = xmlTextReaderIsNamespaceDecl(xr);
  BOOL_OR_ERROR(L, ret);
}

/* reader:base_uri() */
static int xmlreader_base_uri(lua_State *L) {
  xmlreader xr = check_xmlreader(L, 1);
  char *uri = (char*)xmlTextReaderBaseUri(xr);
  if (uri) {
    lua_pushstring(L, uri);
    xmlFree(uri);
    return 1;
  } else {
    lua_pushnil(L);
    lua_pushstring(L, "not available");
    return 2;
  }
}

/* reader:local_name() */
static int xmlreader_local_name(lua_State *L) {
  xmlreader xr = check_xmlreader(L, 1);
  char *ln = (char*)xmlTextReaderLocalName(xr);
  if (ln) {
    lua_pushstring(L, ln);
    xmlFree(ln);
    return 1;
  } else {
    lua_pushnil(L);
    lua_pushstring(L, "not available");
    return 2;
  }
}

/* reader:name() */
static int xmlreader_name(lua_State *L) {
  xmlreader xr = check_xmlreader(L, 1);
  char *name = (char*)xmlTextReaderName(xr);
  if (name) {
    lua_pushstring(L, name);
    xmlFree(name);
    return 1;
  } else {
    lua_pushnil(L);
    lua_pushstring(L, "not available");
    return 2;
  }
}

/* reader:namespace_uri() */
static int xmlreader_namespace_uri(lua_State *L) {
  xmlreader xr = check_xmlreader(L, 1);
  char *nsuri = (char*)xmlTextReaderNamespaceUri(xr);
  if (nsuri) {
    lua_pushstring(L, nsuri);
    xmlFree(nsuri);
    return 1;
  } else {
    lua_pushnil(L);
    lua_pushstring(L, "not available");
    return 2;
  }
}

/* reader:prefix() */
static int xmlreader_prefix(lua_State *L) {
  xmlreader xr = check_xmlreader(L, 1);
  char *prefix = (char*)xmlTextReaderPrefix(xr);
  if (prefix) {
    lua_pushstring(L, prefix);
    xmlFree(prefix);
    return 1;
  } else {
    lua_pushnil(L);
    lua_pushstring(L, "not available");
    return 2;
  }
}

/* reader:xml_lang() */
static int xmlreader_xml_lang(lua_State *L) {
  xmlreader xr = check_xmlreader(L, 1);
  char *xml_lang = (char*)xmlTextReaderXmlLang(xr);
  if (xml_lang) {
    lua_pushstring(L, xml_lang);
    xmlFree(xml_lang);
    return 1;
  } else {
    lua_pushnil(L);
    lua_pushstring(L, "no xml:lang value exists");
    return 2;
  }
}

/* ignore xmlTextReaderConstString */

/* reader:value() */
static int xmlreader_value(lua_State *L) {
  xmlreader xr = check_xmlreader(L, 1);
  char *value = (char*)xmlTextReaderValue(xr);
  if (value) {
    lua_pushstring(L, value);
    xmlFree(value);
    return 1;
  } else {
    lua_pushnil(L);
    lua_pushstring(L, "not available");
    return 2;
  }
}

/*** methods of XmlTextReader ***/

/* reader:close() */
static int xmlreader_close(lua_State *L) {
  xmlreader xr = check_xmlreader(L, 1);

  int ret = xmlTextReaderClose(xr);
  if (ret == 0) {
    lua_pushboolean(L, 1);
    return 1;
  } else {
    lua_pushnil(L);
    xmlreader_pusherror(L);
    return 2;
  }
}

/* reader:get_attribute(name|idx) */
static int xmlreader_get_attribute(lua_State *L) {
  xmlreader xr = check_xmlreader(L, 1);
  char *attr;

  if (lua_type(L, 2) == LUA_TNUMBER) {
		luaL_checkany(L, 2);
		int n = lua_tointeger(L, 2);
    attr = (char*)xmlTextReaderGetAttributeNo(xr, n);
  } else {
    const xmlChar *name = (xmlChar*)luaL_checkstring(L, 2);
    attr = (char*)xmlTextReaderGetAttribute(xr, name);
  }

  if (attr) {
    lua_pushstring(L, attr);
    xmlFree(attr);
    return 1;
  } else {
    lua_pushnil(L);
    xmlreader_pusherror(L);
    return 2;
  }
}

/* reader:get_attribute_ns(nsuri) */
static int xmlreader_get_attribute_ns(lua_State *L)
{
  xmlreader xr = check_xmlreader(L, 1);
  const xmlChar *localname = (xmlChar*)luaL_checkstring(L, 2);
  const xmlChar *namespaceuri = (xmlChar*)luaL_checkstring(L, 3);

  char *attr = (char*)xmlTextReaderGetAttributeNs(xr, localname, namespaceuri);
  if (attr) {
    lua_pushstring(L, attr);
    xmlFree(attr);
    return 1;
  } else {
    lua_pushnil(L);
    xmlreader_pusherror(L);
    return 2;
  }
}

/* reader:lookup_namespace() */
static int xmlreader_lookup_namespace(lua_State *L) {
  xmlreader xr = check_xmlreader(L, 1);

  char *ret = (char*)xmlTextReaderLookupNamespace(xr, (xmlChar*)luaL_checkstring(L, 2));
  if (ret) {
    lua_pushstring(L, ret);
    xmlFree(ret);
    return 1;
  } else {
    lua_pushnil(L);
    xmlreader_pusherror(L);
    return 2;
  }
}

/* reader:move_to_attribute(name|idx) */
static int xmlreader_move_to_attribute(lua_State *L)
{
  xmlreader xr = check_xmlreader(L, 1);
  int ret;

  if (lua_type(L, 2) == LUA_TNUMBER) {
		luaL_checkany(L, 2);
		int n = lua_tointeger(L, 2);
    ret = xmlTextReaderMoveToAttributeNo(xr, n);
  } else {
    const xmlChar *name = (xmlChar*)luaL_checkstring(L, 2);
    ret = xmlTextReaderMoveToAttribute(xr, name);
  }

  BOOL_OR_ERROR(L, ret);
}

/* reader:move_to_attribute_ns(localname, nsuri) */
static int xmlreader_move_to_attribute_ns(lua_State *L) {
  xmlreader xr = check_xmlreader(L, 1);
  const xmlChar *localname = (xmlChar*)luaL_checkstring(L, 2);
  const xmlChar *namespaceuri = (xmlChar*)luaL_checkstring(L, 3);

  int ret = xmlTextReaderMoveToAttributeNs(xr, localname, namespaceuri);
  BOOL_OR_ERROR(L, ret);
}

/* reader:move_to_first_attribute() */
static int xmlreader_move_to_first_attribute(lua_State *L) {
  xmlreader xr = check_xmlreader(L, 1);

  int ret = xmlTextReaderMoveToFirstAttribute(xr);
  BOOL_OR_ERROR(L, ret);
}

/* reader:move_to_next_attribute() */
static int xmlreader_move_to_next_attribute(lua_State *L) {
  xmlreader xr = check_xmlreader(L, 1);

  int ret = xmlTextReaderMoveToNextAttribute(xr);
  BOOL_OR_ERROR(L, ret);
}

/* reader:move_to_element() */
static int xmlreader_move_to_element(lua_State *L) {
  xmlreader xr = check_xmlreader(L, 1);

  int ret = xmlTextReaderMoveToElement(xr);
  BOOL_OR_ERROR(L, ret);
}

/* reader:encoding() */
static int xmlreader_encoding(lua_State *L) {
  xmlreader xr = check_xmlreader(L, 1);
  const char *enc = (char*)xmlTextReaderConstEncoding(xr);
  if (enc) {
    lua_pushstring(L, enc);
    return 1;
  } else {
    lua_pushnil(L);
    xmlreader_pusherror(L);
    return 2;
  }
}

/*** extensions ***/

/* for now, don't implement xmlTextReaderSetParserProp, it's better done in
 * the constructor */

static int xmlreader_get_parser_property(lua_State *L) {
  xmlreader xr = check_xmlreader(L, 1);
  int prop = luaL_checkoption(L, 2, NULL, xmlreader_properties);

  int ret = xmlTextReaderGetParserProp(xr, prop);
  if (ret != -1) {
    lua_pushinteger(L, ret);
    return 1;
  } else {
    lua_pushnil(L);
    xmlreader_pusherror(L);
    return 2;
  }
}

/* reader:line_number() */
static int xmlreader_line_number(lua_State *L) {
  xmlreader xr = check_xmlreader(L, 1);

  int ret = xmlTextReaderGetParserLineNumber(xr);

  if (ret != 0) {
    lua_pushinteger(L, ret);
  } else {
    lua_pushnil(L); /* Should we just return 0? */
  }
  return 1;
}

/* reader:column_number() */
static int xmlreader_column_number(lua_State *L) {
  xmlreader xr = check_xmlreader(L, 1);

  int ret = xmlTextReaderGetParserColumnNumber(xr);

  if (ret != 0)
    lua_pushinteger(L, ret);
  else
    lua_pushnil(L); /* Should we just return 0? */
  return 1;
}

/* reader:next_node() */
static int xmlreader_next_node(lua_State *L) {
  xmlreader xr = check_xmlreader(L, 1);

  int ret = xmlTextReaderNext(xr);
  BOOL_OR_ERROR(L, ret);
}


/* reader:is_valid() */
static int xmlreader_is_valid(lua_State *L) {
  xmlreader xr = check_xmlreader(L, 1);

  int ret = xmlTextReaderIsValid(xr);
  BOOL_OR_ERROR(L, ret);
}

/* TODO RelaxNG and Schema validation methods unimplemented */

/* reader:xml_version() */
static int xmlreader_xml_version(lua_State *L) {
  xmlreader xr = check_xmlreader(L, 1);
  const char *xml_version = (char*)xmlTextReaderConstXmlVersion(xr);
  if (xml_version) {
    lua_pushstring(L, xml_version);
    return 1;
  } else {
    lua_pushnil(L);
    xmlreader_pusherror(L);
    return 2;
  }
}

/* reader:is_standalone() */
static int xmlreader_is_standalone(lua_State *L) {
  xmlreader xr = check_xmlreader(L, 1);

  int ret = xmlTextReaderStandalone(xr);
  BOOL_OR_ERROR(L, ret);
}

/*** index lookup ***/

/* reader:bytes_consumed() */
static int xmlreader_bytes_consumed(lua_State *L) { xmlreader xr = check_xmlreader(L, 1);
  long ret = xmlTextReaderByteConsumed(xr);
  if (ret != -1) {
    lua_pushinteger(L, ret);
    return 1;
  } else {
    lua_pushnil(L);
    xmlreader_pusherror(L);
    return 2;
  }
}

static int parser_opt_table;

/* assumes table of options at top of stack. Invalid option parameters have no
 * effect
 * */
static int get_parser_option(lua_State *L) {
  int i;
	int len = lua_rawlen(L, -1);
  int opt = 0;
  if (len > 0) {
    lua_pushlightuserdata(L, &parser_opt_table);
    lua_gettable(L, LUA_REGISTRYINDEX);
    for (i = 1; i <= len; i++) {
      /* get option string from user's option table, get int to OR */
      lua_rawgeti(L, -2, i);
      lua_rawget(L, -2);
      opt |= lua_tointeger(L, -1);
      lua_pop(L, 1);
    }
  }
  return opt;
}

/*** new, more complete APIs for simpler creation and reuse of readers ***/

/* xmlreader.from_file(filename [,encoding] [,options]) */
static int xmlreader_from_file(lua_State *L) {
  const char *fn = luaL_checkstring(L, 1);
  const char *enc = luaL_optstring(L, 2, NULL);
  int opt = 0;
  if (lua_gettop(L) > 2) {
    lua_pop(L, lua_gettop(L) - 3);
    luaL_checktype(L, 3, LUA_TTABLE);
    opt = get_parser_option(L);
  }

  xmlreader xr = push_xmlreader(L, xmlReaderForFile(fn, enc, opt));

  if (xr == NULL)
    lua_pushnil(L);
  /*
  else
    xmlTextReaderSetStructuredErrorHandler(xr, xmlreader_error_handler, NULL);
  */
  return 1;
}

/* Differs from xmlReaderForMemory by using xmlParserInputBufferCreateMem,
 * which copys the passed string ensuring that parsing is safe after the
 * string is popped from lua's stack */

xmlreader _xmlreader_from_string(const char *buffer, int size, const char *URL, const char *encoding, int options) {
  xmlTextReaderPtr reader;
  xmlParserInputBufferPtr buf;

  buf = xmlParserInputBufferCreateMem(buffer, size, XML_CHAR_ENCODING_NONE);
  if (buf == NULL)
    return (NULL);
  reader = xmlNewTextReader(buf, URL);
  if (reader == NULL) {
    xmlFreeParserInputBuffer(buf);
    return (NULL);
  }
  xmlTextReaderSetup(reader, buf, URL, encoding, options);
  return (reader);
}

/* xmlreader.from_string(str [, base_url] [, encoding] [,options]) */
static int xmlreader_from_string(lua_State *L) {
  const char *str = luaL_checkstring(L, 1);
  const char *url = luaL_optstring(L, 2, NULL);
  const char *enc = luaL_optstring(L, 3, NULL);
  int opt = 0;
  if (lua_gettop(L) > 3) {
    lua_pop(L, lua_gettop(L) - 4);
    luaL_checktype(L, 4, LUA_TTABLE);
    opt = get_parser_option(L);
  }

  xmlreader xr = push_xmlreader(L, _xmlreader_from_string(str, lua_rawlen(L, -1), url, enc, opt));

  if (xr == NULL)
    lua_pushnil(L);
  /*
  else
    xmlTextReaderSetStructuredErrorHandler(xr, xmlreader_error_handler, L);
  */

  return 1;
}

/*** error handling extensions ***/
/* TODO: Investigate. What does the perl bindings do with these? */

/*
static void xmlreader_error_handler(void *out, xmlError *error)
{
}
*/

static void xmlreader_pusherror(lua_State *L) {
  xmlError *e = xmlGetLastError();
  if (e==NULL)
    luaL_error(L, "error was NULL");
  xml_push_error(L, e);
}

/** Lua setup ***/

static const struct luaL_Reg xmlreader_f [] = {
  {"from_file", xmlreader_from_file},
  {"from_string", xmlreader_from_string},
  {NULL, NULL}
};

static const struct luaL_Reg xmlreader_m [] = {
  {"__gc", xmlreader_gc},
  {"__tostring", xmlreader_tostring},
  {"read", xmlreader_read},
  {"read_inner_xml", xmlreader_read_inner_xml},
  {"read_outer_xml", xmlreader_read_outer_xml},
  {"read_string", xmlreader_read_string},
  {"read_attribute_value", xmlreader_read_attribute_value},
  {"attribute_count", xmlreader_attribute_count},
  {"depth", xmlreader_depth},
  {"has_attributes", xmlreader_has_attributes},
  {"has_value", xmlreader_has_value},
  {"is_default", xmlreader_is_default},
  {"is_empty_element", xmlreader_is_empty_element},
  {"node_type", xmlreader_node_type},
  {"quote_char", xmlreader_quote_char},
  {"read_state", xmlreader_read_state},
  {"is_namespace_declaration", xmlreader_is_namespace_declaration},
  {"base_uri", xmlreader_base_uri},
  {"local_name", xmlreader_local_name},
  {"name", xmlreader_name},
  {"namespace_uri", xmlreader_namespace_uri},
  {"prefix", xmlreader_prefix},
  {"xml_lang", xmlreader_xml_lang},
  {"value", xmlreader_value},
  {"close", xmlreader_close},
  {"get_attribute", xmlreader_get_attribute},
  {"get_attribute_ns", xmlreader_get_attribute_ns},
  {"lookup_namespace", xmlreader_lookup_namespace},
  {"move_to_attribute", xmlreader_move_to_attribute},
  {"move_to_attribute_ns", xmlreader_move_to_attribute_ns},
  {"move_to_first_attribute", xmlreader_move_to_first_attribute},
  {"move_to_next_attribute", xmlreader_move_to_next_attribute},
  {"move_to_element", xmlreader_move_to_element},
  {"encoding", xmlreader_encoding},
  {"get_parser_property", xmlreader_get_parser_property},
  {"line_number", xmlreader_line_number},
  {"column_number", xmlreader_column_number},
  {"next_node", xmlreader_next_node},
  {"is_valid", xmlreader_is_valid},
  {"xml_version", xmlreader_xml_version},
  {"is_standalone", xmlreader_is_standalone},
  {"bytes_consumed", xmlreader_bytes_consumed},
  {NULL, NULL}
};

static struct { const char *key; xmlParserOption value; } parser_opts[] = {
  {"recover", XML_PARSE_RECOVER},
  {"noent", XML_PARSE_NOENT},
  {"dtdload", XML_PARSE_DTDLOAD},
  {"dtdattr", XML_PARSE_DTDATTR},
  {"dtdvalid", XML_PARSE_DTDVALID},
  {"noerror", XML_PARSE_NOERROR},
  {"nowarning", XML_PARSE_NOWARNING},
  {"pedantic", XML_PARSE_PEDANTIC},
  {"noblanks", XML_PARSE_NOBLANKS},
  {"sax1", XML_PARSE_SAX1},
  {"xinclude", XML_PARSE_XINCLUDE},
  {"nonet", XML_PARSE_NONET},
  {"nodict", XML_PARSE_NODICT},
  {"nsclean", XML_PARSE_NSCLEAN},
  {"nocdata", XML_PARSE_NOCDATA},
  {"noxincnode", XML_PARSE_NOXINCNODE},
  {"compact", XML_PARSE_COMPACT},
  {NULL, 0}
};

int luaopen_xmlreader(lua_State *L) {
  int i;

  LIBXML_TEST_VERSION

  lua_pushlightuserdata(L, &parser_opt_table);
  lua_newtable(L);
  for (i=0; parser_opts[i].key != NULL; i++) {
    lua_pushinteger(L, parser_opts[i].value);
    lua_setfield(L, -2, parser_opts[i].key);
  }
  lua_rawset(L, LUA_REGISTRYINDEX);

  luaL_newmetatable(L, LXMLREADER);
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
	luaL_setfuncs(L, xmlreader_m, 0);

  lua_newtable(L);
	luaL_setfuncs(L, xmlreader_f, 0);

  return 1;
}
