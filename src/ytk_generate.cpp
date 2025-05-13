#include "ytk_code_gen.h"
#include "utility.h"

std::string YangCodeGen::generate_namespace_from_xpath(const std::string& structXPath, std::vector<std::string>& namespaceStrs) {
    namespaceStrs = splitIntoVector(structXPath, '/');

    std::string structNamespace;
    for(auto& namespaceStr: namespaceStrs) {
        std::replace( namespaceStr.begin(), namespaceStr.end(), '-', '_');
        namespaceStr = namespaceStr;
        output_h << "namespace " << namespaceStr << "_ns {\n";
        structNamespace += namespaceStr + "_ns::";
    }
    output_h << std::endl;

    return structNamespace;
}

void YangCodeGen::generate_enum_leaf_in_struct(const YangStructInfo& st, YangElementInfo* e) {
    e->pNameUpperCase = e->pName;
    std::transform(e->pNameUpperCase.begin(), e->pNameUpperCase.end(), e->pNameUpperCase.begin(), ::toupper);

    for(auto& val : e->enumValues) {
        std::replace( val.begin(), val.end(), '-', '_');
        std::transform(val.begin(), val.end(), val.begin(), ::toupper);
    }

    std::vector<std::string> enum_str = e->enumValues;

    output_h << std::endl;

    output_h << "  enum e_" << e->pName << " {\n";
    for(auto& val : enum_str) {
        val = "E_" + e->pNameUpperCase + "_" +val;
        output_h << "    " << val << ",\n";
    }
    output_h << "    E_" << e->pNameUpperCase << "_MAX\n";
    output_h << "  };\n"; // endof enum

    output_h << "  static std::map<std::string, e_" << e->pName << "> m_" << e->pName << "_stoe;\n";
    output_h << "  static std::map<e_" << e->pName << ", std::string> m_" << e->pName << "_etos;\n";

    output_h << '\n';
    output_h << "  template<typename T>\n";
    output_h << "  struct " << e->pName << "_es : public YTEntity<T> {\n";

    output_h << "    " << e->pName << "_es() {\n"; // constructtor
    for(int i = 0; i < enum_str.size(); i++) {
        output_h << "      m_" << e->pName << "_stoe[\"" << e->enumValues[i] << "\"] = " << enum_str[i] << ";\n";
        output_h << "      m_" << e->pName << "_etos[" << enum_str[i] << "] = \"" << e->enumValues[i] << "\";\n";
    }
    output_h << "      m_" << e->pName << "_stoe[\"E_" << e->pNameUpperCase << "_MAX\"] = E_" << e->pNameUpperCase << "_MAX;\n";

    output_h << '\n';

    output_h << "      this->value = E_" << e->pNameUpperCase << "_MAX;\n";
    output_h << "      this->isPresent = false;\n";
    output_h << "    }\n"; // endof constructor


    output_h << "    void set_value(std::string element) { \n";
    output_h << "      this->value = m_" << e->pName << "_stoe[element];\n";
    output_h << "      this->isPresent = true;\n";
    output_h << "    };\n";

    output_h << "     std::string get_value() const { \n";
    output_h << "      return m_" << e->pName << "_etos[this->value];\n";
    output_h << "    };\n";


    output_h << "  };\n\n"; // endof struct
}


void YangCodeGen::generate_leaf_in_struct(const YangStructInfo& st, YangElementInfo* e) {

    if(e->isListKey) output_h << "  // " << e->pName << " is a key \n";

    switch(e->leafType) {
        case LY_TYPE_ENUM:
            generate_enum_leaf_in_struct(st, e);
            output_h << "  struct " << e->pName << "_es <e_" << e->pName << ">" << "\t\t" << e->pName << ";";
            break;

        case LY_TYPE_STRING:
        case LY_TYPE_INST:
            output_h << "  " << add_entity_wrapper(e->paramType) << "\t\t" << e->pName << ";";
            break;

        default:
            //output_h << "  " << "bool " << e->pName << "Present;\n";
            output_h << "  " << add_entity_wrapper(e->paramType) << "\t\t" << e->pName << " = 0;";
            break;

    }

    output_h << std::endl;
}


void YangCodeGen::generate_struct_param(const YangStructInfo& st, std::vector<YangElementInfo*>& paramList) {

    for(auto& e: paramList) {
        //cout << " " << e->paramName << " " << e->namespaceStr << endl;

        e->paramType = yang_type(e->leafType);
        e->pName = e->paramName;
        std::replace( e->pName.begin(), e->pName.end(), '-', '_');

        switch (e->yangType)
        {
            case YangType::LEAF:
                generate_leaf_in_struct(st, e);
                break;

            case YangType::LIST:
                output_h << "  " << "std::map<std::string, " << st.structNamespace << "_ns::" << e->pName << "> " << e->pName << ';';
                break;

            case YangType::LEAF_LIST:
                if(e->leafType == LY_TYPE_ENUM) {
                    generate_enum_leaf_in_struct(st, e);
                    output_h << "  " << "std::set<"<< e->pName << "_es <e_" << e->pName << "> > " << e->pName << ';';
                } else {
                    output_h << "  " << "std::set<"<< e->paramType << "> " << e->pName << ';';
                }
                break;

            case YangType::CONTAINER:
                //output_h << "  " << "bool   " << e->pName << "Present;\n";
                output_h << "  " << "struct " << st.structNamespace << "_ns::" << e->pName << ' ' << e->pName << ';';
                break;
        }
        output_h << std::endl;
    }
}

void YangCodeGen::generate_operator_overloded(const YangStructInfo& st, std::vector<YangElementInfo*>& paramList)
{
    std::string build_ne_str, build_eq_str;
    for(auto& e: paramList) {
        build_ne_str += e->pName + " != obj." + e->pName + " || ";
        build_eq_str += e->pName + " == obj." + e->pName + " && ";
    }

    output_h << "  bool operator == (const struct " << st.structNameWithoutUnderscore <<  "& obj) const {\n";
    output_h << "    return " << build_eq_str << "true;\n";
    output_h << "  }\n";

    output_h << "  bool operator != (const struct " << st.structNameWithoutUnderscore <<  "& obj) const {\n";
    output_h << "    return " << build_ne_str << "false;\n";
    output_h << "  }\n";
}


void YangCodeGen::generate_struct_apis(YangStructInfo& structInfo) {

    output_h << '\n';
    output_h << "  " << structInfo.structNameWithoutUnderscore << "() {\n"; // structure constructor
    output_h << "    resetStructure();\n";
    output_h << "  }\n";
    output_h << "  bool containerHasData() const;\n";
    output_h << "  std::string get_key();\n";
    output_h << "  void resetStructure();\n";
    output_h << "  void setDefaultValues();\n\n";
    output_h << "  bool matchQuerySet(std::string query);\n";
    output_h << "  bool decodeXPathAndFetchConfig(std::deque<std::string>& query_vec, struct " << structInfo.structNameWithoutUnderscore << "& rObj);\n";
    output_h << "  bool deserializeXML(xmlNodePtr root);\n";
    output_h << "  bool deserializeYang(struct lyd_node *root);\n";
    output_h << "  void encodeToXML(std::ostringstream& xml, bool isConfig = true, int indent = 0);\n\n";
    output_h << "  void notifySubscriber(std::string xpath = \"\");\n";
}


void YangCodeGen::generate_structure_body(YangStructInfo& structInfo, std::vector<YangElementInfo*>& param_list)
{
    structInfo.structNameWithoutUnderscore = structInfo.structName;

    std::vector<std::string> namespaceStrs;
    structInfo.structNamespace = generate_namespace_from_xpath(structInfo.xpath, namespaceStrs);

    std::replace( structInfo.structNameWithoutUnderscore.begin(), structInfo.structNameWithoutUnderscore.end(), '-', '_');
    output_h << "struct "<< structInfo.structNameWithoutUnderscore << " : YTContainer { " << std::endl;

    structInfo.structNamespace += structInfo.structNameWithoutUnderscore;
    // cout <<  structName << " " << structNamespace << endl;


    generate_struct_apis(structInfo);
    generate_struct_param(structInfo, param_list);
    generate_operator_overloded(structInfo, param_list);

    generate_static_variable_initializer(structInfo, param_list);
    generate_reset_flags_function(structInfo, param_list);
    generate_set_default_values_function(structInfo, param_list);
    generate_container_has_data_function(structInfo, param_list);
    generate_get_key_function(structInfo, param_list);

    generate_match_query_set_function(structInfo, param_list);
    generate_decode_xpath_function(structInfo, param_list);
    generate_serialize_function(structInfo, param_list);
    generate_deserialize_function(structInfo, param_list);
    generate_deserialize_yang_function(structInfo, param_list);
    generate_notify_subscriber_function(structInfo, param_list);

    output_h << "};\n\n"; // ending of structure

    for(int i = namespaceStrs.size()-1; i >= 0; i--) {
        output_h << "} // endof " << namespaceStrs[i] << "_ns \n"; // endof namespace
    }
    output_h << std::endl;
}


void YangCodeGen::generate_leaf_notify_subscriber(const std::string& pXPath, YangElementInfo* e) {

    switch(e->leafType) {

        case LY_TYPE_ENUM:
            output << "  if(" << e->pName << ".isPresent) {\n";
            output << "    if(YangSubsMgr::instance()->YangSubsMgr.count(\"" << pXPath <<"\") > 0) {\n";
            output << "      for(auto sub: YangSubsMgr::instance()->xpathSubscriberMap[\"" << pXPath << "\"]) {\n";
            output << "        sub->notify(\"" << pXPath << "\", " << e->pName << ".get_value());\n";
            output << "      }\n";
            output << "    }\n";
            output << "  }\n";
            break;

        default:
            output << "  if(" << e->pName << ".isPresent) {\n";
            output << "    if(YangSubsMgr::instance()->xpathSubscriberMap.count(\"" << pXPath <<"\") > 0) {\n";
            output << "      for(auto sub: YangSubsMgr::instance()->xpathSubscriberMap[\"" << pXPath << "\"]) {\n";
            output << "        sub->notify(\"" << pXPath << "\", " << e->pName  << ".get());\n";
            output << "      }\n";
            output << "    }\n";
            output << "  }\n";

            break;
    }
}

void YangCodeGen::generate_notify_subscriber_function(const YangStructInfo& structInfo, std::vector<YangElementInfo*>& paramList)
{
    output << '\n';
    output << "void " << structInfo.structNamespace << "::notifySubscriber(std::string xpath) {\n";

    output << "  xpath +=  \"/" << structInfo.structName << "\";\n";
    output << "  if(!get_key().empty()) xpath += \"[\" + get_key() + \"]\";\n\n";

    for(auto& e: paramList)
    {
        std::string pXPath = structInfo.xpath +'/'+ structInfo.structName + '/' + e->paramName;

        switch (e->yangType)
        {
        case YangType::LEAF:
            generate_leaf_notify_subscriber(pXPath, e);
            break;

        case YangType::CONTAINER:
            output << "  if(" << e->pName << ".isPresent) {\n";
            output << "    " << e->pName << ".notifySubscriber(xpath);\n";
            output << "  }\n";
            break;

        case YangType::LIST:
            output << "  for(auto& elm: " << e->pName << ") {\n";
            output << "    elm.second.notifySubscriber(xpath);\n";
            output << "  };\n";

            break;

        case YangType::LEAF_LIST:
            output << "  for(auto& elm: " << e->pName << ") {\n";
            output << "    if(YangSubsMgr::instance()->xpathSubscriberMap.count(\"" << pXPath <<"\") > 0) {\n";
            output << "      for(auto sub: YangSubsMgr::instance()->xpathSubscriberMap[\"" << pXPath << "\"]) {\n";
            if(e->leafType == LY_TYPE_ENUM) {
                output << "        sub->notify(\"" << pXPath << "\", elm.get_value());\n";
            } else {
                output << "        sub->notify(\"" << pXPath << "\", elm);\n";
            }
            output << "      }\n";
            output << "    };\n";
            output << "  };\n";
            break;

        default:
            break;
        }
    }

    std::string stXpath = structInfo.xpath +'/'+ structInfo.structName;

    output << "  if (containerHasData()) {\n";
    output << "    std::ostringstream xml_payload;\n";
    output << "    encodeToXML(xml_payload);\n";

    output << "    if(YangSubsMgr::instance()->xpathSubscriberMap.count(xpath) > 0) {\n";
    output << "      for(auto sub: YangSubsMgr::instance()->xpathSubscriberMap[xpath]) {\n";
    output << "        sub->notify(xpath, xml_payload);\n";
    output << "      }\n";
    output << "    }\n";

    output << "    else if(YangSubsMgr::instance()->xpathSubscriberMap.count(\"" << stXpath <<"\") > 0) {\n";

    output << "        for(auto sub: YangSubsMgr::instance()->xpathSubscriberMap[\"" << stXpath << "\"]) {\n";
    output << "          sub->notify(\"" << stXpath << "\", xml_payload);\n";
    output << "        }\n";

    output << "    }\n";

    output << "  }\n";

    output << "}\n"; // endof notifySubscriber
}


void YangCodeGen::generate_match_query_set_function(const YangStructInfo& st, std::vector<YangElementInfo*>& paramList)
{
    output << '\n';
    output << "bool " << st.structNamespace << "::matchQuerySet(std::string query) {\n";

    output << "  if(query.empty()) return true;\n";

    output << "  int matched_counter = 0;\n";;
    output << "  std::string struct_name = trim_n_fetch_id_from_query_url(query);\n";

    output << "  char operation;\n";
    output << "  query_set kv_list = fetch_query_set(query, operation);\n\n";
    output << "  for(auto& kv: kv_list) {\n";

    for(auto& e: paramList) {

        switch(e->yangType) {
            case YangType::CONTAINER:
                output << "    if(struct_name == \"" << e->paramName << "\" && " << e->pName << ".matchQuerySet(query)) {\n";
                output << "      ++matched_counter;\n";
                output << "    }\n";
                break;
            case YangType::LEAF:
                if(e->leafType == LY_TYPE_ENUM) {
                    output << "    if(kv.first == \"" << e->paramName << "\" && " << e->pName << ".get_value() == kv.second) {\n";
                } else {
                    output << "    if(kv.first == \"" << e->paramName << "\" && " << e->pName << " == kv.second) {\n";
                }
                output << "      ++matched_counter;\n";
                output << "      continue;\n";
                output << "    }\n";
                break;
        }
    }

    output << "  }\n";

    output << "  if(operation == '&')  return matched_counter == kv_list.size();\n";
    output << "  else if(operation == '|')  return matched_counter > 0;\n";
    output << "  else if(operation == '!')  return matched_counter == 0;\n";
    output << "  return false;\n";
    output << "}\n"; // endof function

}


void YangCodeGen::generate_static_variable_initializer(YangStructInfo& structInfo, std::vector<YangElementInfo*>& paramList)
{
    output << '\n';

    for(auto& e: paramList) {
        if(e->leafType == LY_TYPE_ENUM) {
            if(e->yangType == YangType::LEAF || e->yangType == YangType::LEAF_LIST) {
                //cout << "Enum : " << e->pName << endl;
                std::string namespace_str = structInfo.structNamespace + "::";// + e->pName;
                output << "std::map<std::string, " <<  namespace_str << "e_" << e->pName  << ">  " << namespace_str << "m_" << e->pName << "_stoe;\n";
                output << "std::map<" << namespace_str << "e_" << e->pName << ", std::string>  " << namespace_str << "m_" << e->pName << "_etos;\n";
            }
        }
    }

    output << '\n';
}

void YangCodeGen::generate_reset_leaf_function(YangElementInfo* e) {

    if(e->operationalData) output << "  " << e->pName << ".isOperationalData = true;\n";

    switch(e->leafType) {
        case LY_TYPE_STRING:
            output << "  " << e->pName << ".clear();\n";
            break;

        case LY_TYPE_ENUM:
            output << "  " << e->pName << ".set_value(\"E_" << e->pNameUpperCase << "_MAX\");\n";
            break;

        default:
            //output << "  " << e->pName << " = 0;\n";
            break;
    }
}



void YangCodeGen::generate_reset_flags_function(YangStructInfo& structInfo, std::vector<YangElementInfo*>& paramList)
{
    output << '\n';
    output << "void " << structInfo.structNamespace  << "::resetStructure() {\n";
    for(auto& e: paramList) {

        switch (e->yangType) {
            case YangType::LEAF:
                generate_reset_leaf_function(e);
                output << "  " << e->pName << ".isPresent = false;\n"; // mindful of the order
                break;

            case YangType::CONTAINER:
                output << "  " << e->pName << ".resetStructure();\n";
                output << "  " << e->pName << ".isPresent = false;\n";
                break;

            case YangType::LIST:
                output << "  for(auto& elm: " << e->pName << ") {\n";
                output << "    elm.second.resetStructure();\n";
                output << "  }\n";
                output << "  " << e->pName << ".clear();\n";
                break;

            case YangType::LEAF_LIST:
                output << "  " << e->pName << ".clear();\n";
                break;

        }
    }
    output << "}\n";
}

void YangCodeGen::generate_leaf_set_default_function(YangElementInfo* e) {

    if(e->defaultValue.empty()) return; // default value not present

    output << "  if(!" << e->pName << ".isPresent)  ";

    switch (e->leafType) {

        case LY_TYPE_STRING:
            output << e->pName << " = \"" << e->defaultValue << "\";";
            break;

        case LY_TYPE_ENUM:
            output << e->pName << ".set_value(\"" << e->defaultValue << "\");";
            break;

        // Add more cases for other YANG types as needed
        default:
            output << e->pName << " = " << e->defaultValue << ";";
            break;
    }
    output << '\n';
}

void YangCodeGen::generate_set_default_values_function(YangStructInfo& structInfo, std::vector<YangElementInfo*>& paramList)
{
    output << '\n';
    output << "void " << structInfo.structNamespace  << "::setDefaultValues() {\n";

    for(auto& e: paramList) {

        switch(e->yangType)
        {
            case YangType::LEAF:
                generate_leaf_set_default_function(e);
                break;

            case YangType::CONTAINER:
                output << "  " << e->pName << ".setDefaultValues();\n";
                break;

            case YangType::LIST:
                output << "  for(auto& elm: " << e->pName << ") {\n";
                output << "    elm.second.setDefaultValues();\n";
                output << "  }\n";
                break;
        }
    }

    output << "}\n";
}


void YangCodeGen::generate_container_has_data_function(const YangStructInfo& st, std::vector<YangElementInfo*>& paramList)
{
    output << '\n';
    output << "bool " << st.structNamespace << "::containerHasData() const { \n";

    std::string build_pp_str; // check if any of the parameter is set in the object

    for(auto& e: paramList)
    {
        if(e->isListKey) continue; // assumption is key will always exits, so check for other params

        switch(e->yangType)
        {
        case YangType::LEAF:
        case YangType::CONTAINER:
            build_pp_str += e->pName + ".isPresent || ";
            break;

        case YangType::LIST:
        case YangType::LEAF_LIST:
            build_pp_str += e->pName + ".size() > 0 || ";
        }
    }

    output << "  return " << build_pp_str << "false;\n";
    output << "}\n";
}

void YangCodeGen::generate_get_key_function(const YangStructInfo& st, std::vector<YangElementInfo*>& paramList)
{
    output << "\n";
    output << "std::string " << st.structNamespace << "::get_key() { \n";
    output << "  std::ostringstream build_key;\n";

    output << "  build_key << ";
    for(auto& e: paramList) {
        if(e->isListKey) {
            if(e->leafType == LY_TYPE_ENUM) {
                output << e->pName << ".get_value() << ";
            }
            else {
                output << e->pName << ".get() << ";
            }
        }
    }
    output << "endl;\n";
    output << "  std::string key = build_key.str();\n"; // removal of newline character at the end
    output << "  key.pop_back();\n";
    output << "  return key;\n";
    //output << "  return build_key.str().size() == 1 ? \"\" : build_key.str();\n";
    output << "}\n";
}

void YangCodeGen::print_yang_tree(const struct lysc_node* schema_node, int indent)
{
    if(schema_node == NULL) return;
    std::cout << '\n';

    const struct lysc_node *node = NULL;
    LY_LIST_FOR(lysc_node_child(schema_node), node) {

        std::string space(indent, ' ');
        std::cout << space << node->name;

        print_yang_tree(node, indent+2);
    }
}




