#include "ytk_code_gen.h"

void YangCodeGen::generate_decode_xpath_compare_function(const YangStructInfo& st, std::vector<YangElementInfo*>& paramList)
{
    output << "  std::string param_name = query_vec.front();\n";
    output << "  std::string param_key = trim_struct_name_n_fetch_id_from_url(param_name);\n";
    output << "  bool _status = false;\n";

    for(auto& e: paramList) {

        switch(e->yangType) {
            case YangType::CONTAINER:
                output << "  if(param_name == \"" << e->paramName << "\") {\n";
                output << "    if(" << e->pName << ".decodeXPathAndFetchConfig(query_vec, rObj." << e->pName << ")) {\n";
                output << "      rObj." << e->pName << ".isPresent = true;\n";
                output << "      _status = true;\n";
                output << "    }\n";
                output << "  }\n";
                break;

            case YangType::LIST:
                output << "  if(param_name == \"" << e->paramName << "\") {\n";
                output << "    if(param_key.empty()) {\n";
                output << "      for(auto& elm : " << e->pName << ") {\n";
                output << "        rObj." << e->pName << "[elm.first] = elm.second;\n";
                output << "        rObj." << e->pName << "[elm.first].isPresent = true;\n";
                output << "      }\n";
                output << "      _status = true;\n";
                output << "    }\n";
                output << "    else {\n";
                output << "      " << st.structNamespace << "_ns::" << e->pName << " tListObj;\n";
                output << "      if(" << e->pName << ".count(param_key) > 0 && " << e->pName  << "[param_key].decodeXPathAndFetchConfig(query_vec, tListObj)) {\n";
                output << "        rObj." << e->pName << "[param_key] = tListObj;\n";
                output << "        rObj." << e->pName << "[param_key].isPresent = true;\n";
                output << "        _status = true;\n";
                output << "      }\n";
                output << "    }\n";
                output << "  }\n";
                break;

            case YangType::LEAF:
                output << "  if(param_name == \"" << e->paramName << "\") {\n";
                output << "    rObj." << e->pName << " = " << e->pName << ";\n";
                output << "    rObj." << e->pName << ".isPresent = true;\n";
                output << "    _status = true;\n";
                output << "  }\n";
                break;

            default:
                break;
        }
    }

    output << "  return _status;\n";
}

void YangCodeGen::generate_xpath_set_key_function(std::vector<YangElementInfo*>& paramList)
{
    output << '\n';
    for(auto& e: paramList)
    {
        if(e->isListKey) {
            switch(e->leafType) {

                case LY_TYPE_ENUM:
                    output << "  rObj." << e->pName << ".set_value(struct_key);\n";
                    break;

                default:
                    output << "  rObj." << e->pName << ".set_value(struct_key.c_str());\n";
                    break;
            }
        }
    }
    output << '\n';
}


void YangCodeGen::generate_decode_xpath_function(const YangStructInfo& st, std::vector<YangElementInfo*>& paramList)
{
    output << '\n';
    output << "bool " << st.structNamespace << "::decodeXPathAndFetchConfig(std::deque<std::string>& query_vec, struct " << st.structNameWithoutUnderscore << "& rObj) {\n";

    output << "  if(query_vec.size() == 0) return true;\n\n";

    output << "  std::string struct_name = query_vec.front();\n";
    output << "  query_vec.pop_front();\n\n";

    output << "  std::string struct_key = trim_struct_name_n_fetch_id_from_url(struct_name);\n";

    output << "  if(\"" << st.structName << "\" != struct_name) return false;\n\n";

    output << "  if(!struct_key.empty() && struct_key != get_key()) return false;\n";

    generate_xpath_set_key_function(paramList);

    output << "  if(query_vec.size() == 0) {\n";
    output << "    rObj = *this;\n";
    output << "    return true;\n";
    output << "  }\n";


    output << "  else if(query_vec.size() == 1 && query_url_found(query_vec.front())) {\n";

    output << "    std::string query_string = query_vec.front();\n";
    output << "    query_vec.pop_front();\n\n";

    output << "    std::string struct_name = trim_n_fetch_struct_name_from_query(query_string);\n";


    output << "    bool _status = false;\n";
    //output << "    for(auto& kv: kv_list) {\n";
    for(auto& e: paramList) {
        if(e->yangType == YangType::LIST) {
            output << "    if(\"" << e->paramName << "\" == struct_name) {\n";
            output << "        for(auto& elm : " << e->pName << ") {\n";
            output << "          if(elm.second.matchQuerySet(query_string)) {\n";
            output << "            rObj." << e->pName << "[elm.first] = elm.second;\n";
            output << "            rObj." << e->pName << "[elm.first].isPresent = true;\n";
            output << "            _status = true;\n";
            output << "          }\n";
            output << "        }\n";
            output << "    }\n";
        }
    }
    //output << "    }\n";
    output << "    return _status;\n";
    output << "  }\n";


    generate_decode_xpath_compare_function(st, paramList);

    output <<"}\n";
}

void YangCodeGen::generate_leaf_in_deserialize(YangElementInfo* e)
{
    switch (e->leafType)
    {
    case LY_TYPE_STRING:
        output << "      " << e->pName << " = reinterpret_cast<const char*>(xmlNodeGetContent(child));\n";
        break;

    case LY_TYPE_BOOL:
        output << "      std::string res = reinterpret_cast<const char*>(xmlNodeGetContent(child));\n";
        output << "      " << e->pName << " = (res == \"true\") ? true: false;\n";
        break;

    case LY_TYPE_UINT8:
        output << "      " << e->pName << " = (uint8_t)std::stoi(reinterpret_cast<const char*>(xmlNodeGetContent(child)));\n";
        break;

    default:
        output << "      " << e->pName << ".set_value(reinterpret_cast<const char*>(xmlNodeGetContent(child)));\n";
        break;
    }
}

void YangCodeGen::generate_leaflist_in_deserialize(YangElementInfo* e)
{
    switch (e->leafType)
    {
    case LY_TYPE_STRING:
        output << "      std::string temp = reinterpret_cast<const char*>(xmlNodeGetContent(child));\n";
        break;

    case LY_TYPE_BOOL:
        output << "      std::string res = reinterpret_cast<const char*>(xmlNodeGetContent(child));\n";
        output << "      bool temp = (res == \"true\") ? true: false;\n";
        break;

    case LY_TYPE_ENUM:
        output << "      struct  " << e->pName << "_es <e_" << e->pName  << "> "<< " temp;\n";
        output << "      temp.set_value(reinterpret_cast<const char*>(xmlNodeGetContent(child)));\n";
        break;

    case LY_TYPE_UINT8:
    case LY_TYPE_UINT16:
    case LY_TYPE_UINT32:
    case LY_TYPE_UINT64:
        output << "      " << e->paramType << " temp = (" << e->paramType << ")std::stoull(reinterpret_cast<const char*>(xmlNodeGetContent(child)));\n";
        break;

    default:
        output << "      " << e->paramType << " temp = (" << e->paramType << ")std::stoi(reinterpret_cast<const char*>(xmlNodeGetContent(child)));\n";
        break;
    }


    if(e->maxListElement > -1) output << "    if(" << e->pName << ".size() < " << e->maxListElement << ") \n";
    output << "      " << e->pName << ".insert(temp);\n";
}


void YangCodeGen::generate_deserialize_function(YangStructInfo& structInfo, std::vector<YangElementInfo*>& paramList) {

    output << '\n';
    output << "bool " << structInfo.structNamespace << "::deserializeXML(xmlNodePtr root) {\n";

    output << "  xmlNodePtr child = root->children;\n";
    output << "  while(child != nullptr) {\n";

    for(auto& e: paramList) {
        output << "    if (xmlStrcmp(child->name, BAD_CAST \"" << e->paramName << "\") == 0) {\n";

        switch (e->yangType)
        {
            case YangType::LEAF:
                generate_leaf_in_deserialize(e);
                break;

            case YangType::LIST:
                output << "      " << structInfo.structNamespace << "_ns::" << e->pName << " tListObj;\n";
                //output << "      tListObj.resetStructure(); // clean up default values \n";
                output << "      bool res = tListObj.deserializeXML(child);\n";
                output << "      std::string key = tListObj.get_key();\n";
                output << "      " << e->pName << "[key] = tListObj;\n";
                output << "      " << e->pName << "[key].isPresent = res;\n";
                //output << "      if(tObj." << e->pName << ".count(key) == 0 || tObj." << e->pName << "[key] != tListObj) {\n";
                //output << "         " << e->pName << "[key].compareDiff(temp);\n";
                break;

            case YangType::LEAF_LIST:
                generate_leaflist_in_deserialize(e);
                break;

            case YangType::CONTAINER:
                output << "      " << e->pName << ".deserializeXML(child);\n";
                output << "      " << e->pName << ".isPresent = true;\n";
                break;
        }

        output << "    }\n"; // endof xmlStrcmp
    }

    output << "    child = child->next;\n";
    output << "  }\n"; // endof while loop

    output << "  return containerHasData();\n";
    output << "}\n"; // endof deserializeXML function
}

void YangCodeGen::generate_leaf_in_deserialize_yang(YangElementInfo* e)
{
    switch (e->leafType)
    {
    case LY_TYPE_STRING:
        output << "      " << e->pName << " = lyd_get_value(node);\n";
        break;

    case LY_TYPE_INST:
        output << "      char* xml_buffer = NULL;\n";
        output << "      struct lyd_node_any *anyxml_node = (struct lyd_node_any *)node;\n";
        output << "      lyd_print_mem(&xml_buffer, anyxml_node->value.tree, LYD_XML, 0);\n";
        output << "      " << e->pName << " = xml_buffer;\n";
        break;

    case LY_TYPE_BOOL:
        output << "      std::string res = lyd_get_value(node);\n";
        output << "      " << e->pName << " = (res == \"true\") ? true: false;\n";
        break;

    case LY_TYPE_UINT8:
        output << "      " << e->pName << " = (uint8_t)std::stoi(lyd_get_value(node));\n";
        break;

    default:
        output << "      " << e->pName << ".set_value(lyd_get_value(node));\n";
        break;
    }
}

void YangCodeGen::generate_leaflist_in_deserialize_yang(YangElementInfo* e)
{
    switch (e->leafType)
    {
    case LY_TYPE_STRING:
        output << "      std::string temp = lyd_get_value(node);\n";
        break;

    case LY_TYPE_BOOL:
        output << "      std::string res = lyd_get_value(node);\n";
        output << "      bool temp = (res == \"true\") ? true: false;\n";
        break;

    case LY_TYPE_ENUM:
        output << "      struct  " << e->pName << "_es <e_" << e->pName  << "> "<< " temp;\n";
        output << "      temp.set_value(lyd_get_value(node));\n";
        break;

    case LY_TYPE_UINT8:
    case LY_TYPE_UINT16:
    case LY_TYPE_UINT32:
    case LY_TYPE_UINT64:
        output << "      " << e->paramType << " temp = (" << e->paramType << ")std::stoull(lyd_get_value(node));\n";
        break;

    default:
        output << "      " << e->paramType << " temp = (" << e->paramType << ")std::stoi(lyd_get_value(node));\n";
        break;
    }


    if(e->maxListElement > -1) output << "    if(" << e->pName << ".size() < " << e->maxListElement << ") \n";
    output << "      " << e->pName << ".insert(temp);\n";
}


void YangCodeGen::generate_deserialize_yang_function(YangStructInfo& structInfo, std::vector<YangElementInfo*>& paramList) {

    output << '\n';
    output << "bool " << structInfo.structNamespace << "::deserializeYang(struct lyd_node *root) {\n";

    output << "  struct lyd_node* node = nullptr;\n";
    output << "  LY_LIST_FOR(lyd_child(root), node) {\n";

    for(auto& e: paramList) {
        output << "    if(!strcmp(LYD_NAME(node), \"" << e->paramName << "\")) {\n";

        switch (e->yangType)
        {
            case YangType::LEAF:
                generate_leaf_in_deserialize_yang(e);
                break;

            case YangType::LIST:
                output << "      " << structInfo.structNamespace << "_ns::" << e->pName << " tListObj;\n";
                output << "      bool res = tListObj.deserializeYang(node);\n";
                output << "      std::string key = tListObj.get_key();\n";
                output << "      " << e->pName << "[key] = tListObj;\n";
                output << "      " << e->pName << "[key].isPresent = res;\n";
                break;

            case YangType::LEAF_LIST:
                generate_leaflist_in_deserialize_yang(e);
                break;

            case YangType::CONTAINER:
                output << "      " << e->pName << ".deserializeYang(node);\n";
                output << "      " << e->pName << ".isPresent = true;\n";
                break;
        }

        output << "    }\n"; // endof xmlStrcmp
    }

    output << "  }\n"; // endof linked list iteration
    output << "  return containerHasData();\n";
    output << "}\n"; // endof deserializeXML function
}


void YangCodeGen::generate_leaf_in_serialize(const YangStructInfo& st, YangElementInfo* e)
{
    std::string build_c_str = "(!isOperationalData || !isConfig)";

    if(!e->isListKey)   build_c_str += " && " + e->pName + ".isPresent";

    if(e->leafType == LY_TYPE_STRING) {
        build_c_str += " && !" + e->pName +  ".empty()";
    }

    output << "  if(" << build_c_str << ")";

    output << "  xml << std::string(indent+SPACE, ' ') << \"<"<<  e->paramName;

    if(!e->namespaceString.empty()) output << " xmlns=\\\"" << e->namespaceString << "\\\"";

    switch (e->leafType)
    {
    case LY_TYPE_BOOL:
        output << ">\" << ((" << e->pName << ".get() == true) ? \"true\" : \"false\") << \"</" << e->paramName << ">\" << std::endl;\n";
        break;

    case LY_TYPE_ENUM:
        output << ">\" << " << e->pName << ".get_value() << \"</" << e->paramName << ">\" << std::endl;\n";
        break;

    case LY_TYPE_UINT8:
        output << ">\" << static_cast<int>(" << e->pName << ".get()) << \"</" << e->paramName << ">\" << std::endl;\n";
        break;

    default:
        output << ">\" << " << e->pName << ".get() << \"</" << e->paramName << ">\" << std::endl;\n";
        break;
    }
}

void YangCodeGen::generate_leaflist_in_serialize(YangElementInfo* e)
{
    output << "  for(auto& entry: " << e->pName << ") {\n";
    switch (e->leafType)
    {
    case LY_TYPE_BOOL:
        output << "  xml << std::string(indent+SPACE, ' ') << \"<"<<  e->paramName << ">\" << ((entry == true) ? \"true\" : \"false\") << \"</" << e->paramName << ">\" << std::endl;\n";
        break;

    case LY_TYPE_STRING:
        output << "  if(!" << e->pName << ".empty())";
        output << "  xml << std::string(indent+SPACE, ' ') << \"<"<<  e->paramName << ">\" << entry << \"</" << e->paramName << ">\" << std::endl;\n";
        break;

    case LY_TYPE_ENUM:
        output << "  xml << std::string(indent+SPACE, ' ') << \"<"<<  e->paramName << ">\" << entry.get_value() << \"</" << e->paramName << ">\" << std::endl;\n";
        break;

    case LY_TYPE_UINT8:
        output << "  xml << std::string(indent+SPACE, ' ') << \"<"<<  e->paramName << ">\" << static_cast<int>(entry) << \"</" << e->paramName << ">\" << std::endl;\n";
        break;

    default:
        output << "  xml << std::string(indent+SPACE, ' ') << \"<"<<  e->paramName << ">\" << entry << \"</" << e->paramName << ">\" << std::endl;\n";
        break;
    }
    output << "  };\n";
}


void YangCodeGen::generate_serialize_function(const YangStructInfo& st, std::vector<YangElementInfo*>& paramList) {

    output << '\n';
    output << "void " << st.structNamespace << "::encodeToXML(std::ostringstream& xml, bool isConfig, int indent) {\n";
    if(st.namespaceString.empty()) {
        output << "  xml << std::string(indent, ' ') << \"<" << st.structName << ">\" << std::endl;\n";
    } else {
        output << "  xml << std::string(indent, ' ') << \"<" << st.structName << " xmlns=\\\"" << st.namespaceString << "\\\">\" << std::endl;\n";
    }

    for(auto& e: paramList) {
        switch (e->yangType)
        {
            case YangType::LEAF:
                generate_leaf_in_serialize(st, e);
                break;

            case YangType::LIST:
                output << '\n';
                output << "  for(auto& e: " << e->pName << " ) {\n";
                output << "    e.second.encodeToXML(xml, isConfig, indent+SPACE);\n";
                output << "   };\n";
                break;

            case YangType::LEAF_LIST:
                generate_leaflist_in_serialize(e);
                break;

            case YangType::CONTAINER:
                output << "  if(" << e->pName << ".isPresent) " << e->pName << ".encodeToXML(xml, isConfig, indent+SPACE);\n";
                break;
        }
    }
    output << "  xml << std::string(indent, ' ') << \"</" << st.structName << ">\" << std::endl;\n";
    output << "};\n"; // endof encodeToXML function
}
