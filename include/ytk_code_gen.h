#ifndef YTK_CODE_GEN_H
#define YTK_CODE_GEN_H

#include <string>
#include <vector>
#include <unordered_map>
#include <set>
#include <fstream>
#include <iostream>
#include <libyang/libyang.h>

enum class YangType{
    LEAF,
    LIST,
    CONTAINER,
    LEAF_LIST,
    MAX
};

struct YangStructInfo
{
    /*  name as per YANG schema. contains dash, ex. network-device
        this will be used in serialize function to build the xml output <network-device/> */
    std::string     structName;

    /* this will be used to encode xml payload ex. "urn:3gpp:sa5:_3gpp-common-managed-element"  */
    std::string     namespaceString;

    /* converted the dash into underscore and stored in this variable ex. network_device */
    std::string     structNameWithoutUnderscore;

    /* path from the top level of the tree till the parent element, ex. /network-device/settings/setting */
    std::string     xpath;

    /* converted xpath to this format to be used as namespace:  /network_device_ns::settings_ns */
    std::string     structNamespace;


    YangStructInfo(const char* name, std::string path) : structName(name), xpath(path) {}

    /*bool operator==(const YangStructInfo& other) const {
        return (structName == other.structName && xpath == other.xpath);
    }*/
};


struct YangElementInfo
{
    /* Yang data type could be either leaf/ list/ container/ leaf-list */
    YangType        yangType;


    /* YANG data type and string to hold the parameter type (LY_DATA_TYPE) so that we don't have to convert everytime */
    LY_DATA_TYPE     leafType;
    std::string      paramType;

    /* name coming directy from YANG schema, contains dash, ex. ip-address */
    std::string      paramName;

    /* converted the dash into underscore and stored in this variable ex. ip_address */
    std::string      pName;

    /* converted the name to uppercase for enum handling */
    std::string      pNameUpperCase;

    /* this will be used to encode xml payload ex. "urn:3gpp:sa5:_3gpp-common-managed-element"  */
    std::string      namespaceString;

    /* holds the default value for this parameter */
    std::string      defaultValue;

    /* if the parameter is a operational data this value will be true */
    bool           operationalData = false;

    /* applicable for list type only. If this paramter is a key of the list */
    bool           isListKey = false;
    /*  list of all the keys in a list */
    std::vector<std::string> listKeys;

    /* maximum and minimum number of element in a list */
    int maxListElement = -1;
    int minListElement = -1;

    std::vector<std::string> enumValues;
    std::vector<std::string> unionValues;

    YangElementInfo(const YangType& y, const LY_DATA_TYPE& t, const std::string& n): yangType(y), leafType(t), paramName(n) {
        if(t == LY_TYPE_IDENT) leafType = LY_TYPE_STRING;
    }
};


class YangCodeGen {
  public:
    bool parse_yang_and_store_data(const char* module_name);

    bool generate(const char* yang_module, std::string output_dir);


  private:
    std::string add_entity_wrapper(std::string type) {
        return "YTEntity<" + type + ">";
    }

    std::string yang_type(LY_DATA_TYPE leaf_type);

    bool is_config_false(const struct lysc_node* schema_node) {
        return (schema_node->flags & LYS_CONFIG_R) != 0;
    }

    /* parsing and storing in map */
    void parse_yang_tree(const struct lysc_node* node, const struct lysc_node* parent, int level, std::string xpath);
    void parse_yang_node(const struct lysc_node* node, const struct lysc_node* parent, int level, std::string xpath, YangStructInfo* structInfo);
    void parse_yang_leaf(const struct lysc_node* child, YangStructInfo* structInfo, int level);
    void parse_yang_list(const struct lysc_node* child, YangStructInfo* structInfo, int level);
    void parse_yang_leaflist(const struct lysc_node* child, YangStructInfo* structInfo, int level);
    void parse_enum_values(const struct lysc_node_leaf* leaf, struct YangElementInfo* yang_type_name);
    void parse_yang_choice_case(const struct lysc_node* case_node, const struct lysc_node* parent, int level, std::string xpath);
    void parse_yang_rpc(const struct lysc_node_action* rpc, int level);
    void parse_yang_notif(const struct lysc_node_notif* notif, int level);

    //for debugging
    void print_yang_tree(const struct lysc_node* schema_node, int indent = 2);

    std::string generate_namespace_from_xpath(const std::string& structXPath, std::vector<std::string>& namespaceStrs);

    // header file functions
    void generate_operator_overloded(const YangStructInfo& st, std::vector<YangElementInfo*>& paramList);
    void generate_enum_leaf_in_struct(const YangStructInfo& st, YangElementInfo* e);
    void generate_leaf_in_struct(const YangStructInfo& st, YangElementInfo* e);
    void generate_struct_param(const YangStructInfo& st, std::vector<YangElementInfo*>& paramList);
    void generate_struct_apis(YangStructInfo& structInfo);
    void generate_structure_body(YangStructInfo& structInfo, std::vector<YangElementInfo*>& param_list);
    void auto_generate_file();

    // source file functions
    void generate_static_variable_initializer(YangStructInfo& structInfo, std::vector<YangElementInfo*>& paramList);
    void generate_reset_leaf_function(YangElementInfo* e);
    void generate_reset_flags_function(YangStructInfo& structInfo, std::vector<YangElementInfo*>& paramList);
    void generate_leaf_set_default_function(YangElementInfo* e);
    void generate_set_default_values_function(YangStructInfo& structInfo, std::vector<YangElementInfo*>& paramList);
    void generate_container_has_data_function(const YangStructInfo& st, std::vector<YangElementInfo*>& paramList);
    void generate_get_key_function(const YangStructInfo& st, std::vector<YangElementInfo*>& paramList);

    // decoder function
    void generate_xpath_set_key_function(std::vector<YangElementInfo*>& paramList);
    void generate_decode_xpath_compare_function(const YangStructInfo& st, std::vector<YangElementInfo*>& paramList);
    void generate_decode_xpath_function(const YangStructInfo& st, std::vector<YangElementInfo*>& paramList);
    void generate_leaf_in_deserialize(YangElementInfo* e);
    void generate_leaflist_in_deserialize(YangElementInfo* e);
    void generate_deserialize_function(YangStructInfo& structInfo, std::vector<YangElementInfo*>& paramList);
    void generate_leaf_in_deserialize_yang(YangElementInfo* e);
    void generate_leaflist_in_deserialize_yang(YangElementInfo* e);
    void generate_deserialize_yang_function(YangStructInfo& structInfo, std::vector<YangElementInfo*>& paramList);

    // encoder functions
    void generate_leaf_in_serialize(const YangStructInfo& st, YangElementInfo* e);
    void generate_leaflist_in_serialize(YangElementInfo* e);
    void generate_serialize_function(const YangStructInfo& st, std::vector<YangElementInfo*>& paramList);


    // utility functions
    void generate_leaf_notify_subscriber(const std::string& pXPath, YangElementInfo* e);
    void generate_notify_subscriber_function(const YangStructInfo& structInfo, std::vector<YangElementInfo*>& paramList);
    void generate_match_query_set_function(const YangStructInfo& st, std::vector<YangElementInfo*>& paramList);

    std::string      module_name;
    std::ofstream    output_h;
    std::ofstream    output;


    /* container name <-> list of parameter Mapping*/
    typedef std::unordered_map<YangStructInfo*, std::vector<YangElementInfo*>> structEntity;

    /* for each level in the yang tree a structure is maintained */
    std::vector<structEntity> levelWiseNameTypeMap;

};


/**
 * This structure is used to generate the common header and cpp file list file for application to consume
 */

struct YtModuleInfo
{
    std::string module_name;
    std::string struct_name;

    YtModuleInfo(const std::string& module_name, const std::string& struct_name): module_name(module_name), struct_name(struct_name) {}
};

class YangCodeGenMgr {
    public:
      static YangCodeGenMgr* instance();

      bool parse_n_generate(const char* module_name, std::string output_dir);

      // Auto gen API
      struct ly_ctx* get_context() { return _yang_ctx; }
      struct lys_module* get_module_from_module_name(const char* yang_module_name);
      bool   add_search_dir(const char* dirname);

    private:
      YangCodeGenMgr();
      ~YangCodeGenMgr();
      static YangCodeGenMgr* instancePtr;

      bool load_all_yang_modules_in_dir(const char* dirname);

      struct ly_ctx* _yang_ctx = nullptr;
};


#endif // YANG_CODE_GEN_H
