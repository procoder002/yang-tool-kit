#include "ytk_code_gen.h"
#include <algorithm>

std::string YangCodeGen::yang_type(LY_DATA_TYPE leaf_type) {
    std::string cpp_type;
    switch (leaf_type) {
        case LY_TYPE_STRING:
        case LY_TYPE_INST:
            cpp_type = "std::string";
            break;
        case LY_TYPE_UINT8:
            cpp_type = "uint8_t";
            break;
        case LY_TYPE_UINT16:
            cpp_type = "uint16_t";
            break;
        case LY_TYPE_UINT32:
            cpp_type = "uint32_t";
            break;
        case LY_TYPE_UINT64:
            cpp_type = "uint64_t";
            break;
        case LY_TYPE_BOOL:
        case LY_TYPE_EMPTY:
        case LY_TYPE_BINARY:
            cpp_type = "bool";
            break;
        case LY_TYPE_INT8:
            cpp_type = "int8_t";
            break;
        case LY_TYPE_INT16:
            cpp_type = "int16_t";
            break;
        case LY_TYPE_INT32:
            cpp_type = "int32_t";
            break;
        case LY_TYPE_INT64:
            cpp_type = "int64_t";
            break;
        case LY_TYPE_ENUM:
            cpp_type = "enum";
            break;
        case LY_TYPE_DEC64:
            cpp_type = "double";
            break;
        case LY_TYPE_BITS:
            cpp_type = "int8_t"; // TODO
            break;
        case LY_TYPE_UNION:
            cpp_type = "union";
            break;

        // Add more cases for other YANG types as needed
        default:
            cpp_type = "UNKNOWN_TYPE";
            break;
    }

    return cpp_type;
}

void YangCodeGen::parse_enum_values(const struct lysc_node_leaf* leaf, struct YangElementInfo* yang_type_name)
{
    const struct lysc_type_enum* enumType = (const struct lysc_type_enum*)leaf->type;

    LY_ARRAY_COUNT_TYPE u;
    LY_ARRAY_FOR(enumType->enums, u) {

        //std::cout << yang_type_name->namespaceString << "::" << yang_type_name->paramName << " Param :" << item.name << std::endl;
        yang_type_name->enumValues.push_back(enumType->enums[u].name);
    }
}

void YangCodeGen::parse_yang_choice_case(const struct lysc_node* choiceNode, const struct lysc_node* parent, int level, std::string xpath)
{
    YangStructInfo* structInfo = new YangStructInfo(choiceNode->name, xpath);
    structInfo->namespaceString = choiceNode->module->ns;
    //std::cout << choiceNode->name  << " choiceNode " <<  xpath << std::endl;
    //print_yang_tree(choiceNode);

    const struct lysc_node *caseNode;
    LY_LIST_FOR(lysc_node_child(choiceNode), caseNode)
    {
        //std::cout << caseNode->name << " Nodetype  :" << lys_nodetype2str(caseNode->nodetype) << std::endl;
        if(caseNode->nodetype == LYS_CASE)
        {
            const struct lysc_node *caseValue;
            LY_LIST_FOR(lysc_node_child(caseNode), caseValue)
            {
                //"  - " << caseValue->name <<
                parse_yang_node(caseValue, choiceNode, level+1, xpath+'/'+choiceNode->name, structInfo);
            }
        }
    }
}


void YangCodeGen::parse_yang_leaflist(const struct lysc_node* child, YangStructInfo* structInfo, int level) {
    //cout << level << " leaf-list  " << child->name << " " << yang_type(leaflist_schema->type->basetype) << endl;
    const struct lysc_node_leaflist* leaflist_schema = (const struct lysc_node_leaflist*)child;

    if(leaflist_schema->type->basetype == LY_TYPE_LEAFREF)
    {
        const struct lysc_type_leafref *leafref = (const struct lysc_type_leafref *)leaflist_schema->type;
        //cout << child->name << " LEAF REF:  " << yang_type(leafref->realtype->basetype) << endl;
        leaflist_schema->type->basetype = leafref->realtype->basetype;
    }


    YangElementInfo* yang_type_name = new YangElementInfo(YangType::LEAF_LIST, leaflist_schema->type->basetype, child->name);

    if (leaflist_schema->type->basetype == LY_TYPE_ENUM) {
        parse_enum_values((struct lysc_node_leaf*)child, yang_type_name);
    }

    if (is_config_false(child)) {
        yang_type_name->operationalData = true;
    }

    if(levelWiseNameTypeMap.size() <= level) levelWiseNameTypeMap.emplace_back();
    levelWiseNameTypeMap[level][structInfo].push_back(yang_type_name);
}


void YangCodeGen::parse_yang_list(const struct lysc_node* child, YangStructInfo* structInfo, int level)
{
    YangElementInfo* yang_element_info = new YangElementInfo(YangType::LIST, LY_TYPE_UNKNOWN, child->name);

    const struct lysc_node_list* list_schema = (const struct lysc_node_list*)child;
    //cout << level << " LIST " << child->name << " Min: " << list_schema->min << " Max: " << list_schema->max << endl;
    yang_element_info->minListElement = list_schema->min;
    yang_element_info->maxListElement = list_schema->max;

    const struct lysc_node* list_child = nullptr;
    LY_LIST_FOR(lysc_node_child(child), list_child) {
        // Check if the child node is a key (LYS_KEY flag is set)
        if (list_child->flags & LYS_KEY) {

            const struct lysc_node_leaf* child_leaf = (const struct lysc_node_leaf*)list_child;
            yang_element_info->listKeys.push_back(yang_type(child_leaf->type->basetype));
        }
    }
    if(levelWiseNameTypeMap.size() <= level) levelWiseNameTypeMap.emplace_back();
    levelWiseNameTypeMap[level][structInfo].push_back(yang_element_info);
}


/**
 * @brief Takes a leaf node and structure as input and store it as a member of the structure
 *
 * @param child
 * @param structInfo
 * @param level
 */

void YangCodeGen::parse_yang_leaf(const struct lysc_node* child, YangStructInfo* structInfo, int level)
{
    struct lysc_node_leaf* leaf = (struct lysc_node_leaf*)child;
    if(leaf->type->basetype == LY_TYPE_UNION) return;

    if(leaf->type->basetype == LY_TYPE_LEAFREF)
    {
        const struct lysc_type_leafref *leafref = (const struct lysc_type_leafref *)leaf->type;
        //cout << child->name << " LEAF REF:  " << yang_type(leafref->realtype->basetype) << endl;
        leaf->type = leafref->realtype;
    }


    YangElementInfo* yang_type_name = new YangElementInfo(YangType::LEAF, leaf->type->basetype, child->name);
    if(levelWiseNameTypeMap.size() <= level) levelWiseNameTypeMap.emplace_back();
    levelWiseNameTypeMap[level][structInfo].push_back(yang_type_name);

    //const struct lys_module* leaf_module = child->module;
    if(child->module->ns != structInfo->namespaceString) yang_type_name->namespaceString = child->module->ns;
    //std::cout << child->name << " Namespace " << child->module->ns << " Parent " << structInfo->namespaceString << std::endl;

    // default Value
    if (leaf->dflt) {
        const struct lyd_value* default_value = leaf->dflt;
        //cout << child->name << " Default value" << lyd_value_get_canonical(ctx, default_value) << endl;

        yang_type_name->defaultValue = lyd_value_get_canonical(YangCodeGenMgr::instance()->get_context(), default_value);
    }

    if (is_config_false(child)) {
        yang_type_name->operationalData = true;
    }

    if (leaf->type->basetype == LY_TYPE_ENUM) {
        parse_enum_values(leaf, yang_type_name);
    }

    if (leaf->flags & LYS_KEY) {
        yang_type_name->isListKey = true;
    }
}


/**
 * @brief Takes a node and struct as input, determines what type of node it is (i.e. leaf, list, container) which will eventually become a member of the structure
 *
 * @param node
 * @param parent
 * @param level
 * @param xpath
 * @param structInfo
 */
void YangCodeGen::parse_yang_node(const struct lysc_node* node, const struct lysc_node* parent, int level, std::string xpath, YangStructInfo* structInfo)
{
    if(levelWiseNameTypeMap.size() <= level) levelWiseNameTypeMap.emplace_back();
    //std::cout << node->name << "  NodeType: " << lys_nodetype2str(node->nodetype) << std::endl;

    switch(node->nodetype)
    {
        case LYS_LEAF:
            parse_yang_leaf(node, structInfo, level);
            break;

        case LYS_LIST:
            parse_yang_list(node, structInfo, level);
            parse_yang_tree(node, parent, level, xpath);
            break;

        case LYS_CONTAINER:
            levelWiseNameTypeMap[level][structInfo].push_back(new YangElementInfo(YangType::CONTAINER, LY_TYPE_UNKNOWN, node->name));
            parse_yang_tree(node, parent, level, xpath);
            break;

        case LYS_LEAFLIST:
            parse_yang_leaflist(node, structInfo, level);
            break;

        case LYS_CHOICE:
            levelWiseNameTypeMap[level][structInfo].push_back(new YangElementInfo(YangType::CONTAINER, LY_TYPE_UNKNOWN, node->name));
            parse_yang_choice_case(node, parent, level, xpath);
            break;

        case LYS_RPC:
        case LYS_NOTIF:
        case LYS_INPUT:
        case LYS_OUTPUT:
            std::cout << node->name << " - "<< lys_nodetype2str(node->nodetype) <<" Handled in different way !" << std::endl;
            break;

        case LYS_ANYXML:
            levelWiseNameTypeMap[level][structInfo].push_back(new YangElementInfo(YangType::LEAF, LY_TYPE_INST, node->name));
            // considering LY_TYPE_INST as ANYXML
            break;

        default:
            std::cout << level << " Unkonwn type " << node->name << std::endl;
    }
}



/**
 * @brief Takes a YANG container node as input, store it as a Map key which means it's nominated to become a structure
          and it's child will be become the structure member
 *
 * @param node
 * @param parent
 * @param level
 * @param xpath
 */
void YangCodeGen::parse_yang_tree(const struct lysc_node* node, const struct lysc_node* parent, int level, std::string xpath)
{
    YangStructInfo* structInfo = new YangStructInfo(node->name, xpath);
    //std::cout << node->name << "  NodeType: " << lys_nodetype2str(node->nodetype) << std::endl;

    /* Assumption is if leaf has different namespace that parent then print it and for container anyway print the namespace */
    //if(parent == NULL || parent->module->ns != node->module->ns) {
        structInfo->namespaceString = node->module->ns;
    //}


    const struct lysc_node* child;
    LY_LIST_FOR(lysc_node_child(node), child) {  // Iterate over child nodes

       // std::cout << " child " << child->name << "  NodeType: " << lys_nodetype2str(child->nodetype) << std::endl;

        parse_yang_node(child, node, level+1, xpath+'/'+node->name, structInfo);
    }
}

void YangCodeGen::parse_yang_rpc(const struct lysc_node_action* rpc, int level)
{
    YangStructInfo* structInfo = new YangStructInfo(rpc->name, "");
    //cout << "RPC: " << rpc->name << endl;
    structInfo->namespaceString = rpc->module->ns;

    struct lysc_node* node;
    LY_LIST_FOR(rpc->input.child, node) {
        //std::cout << rpc->name  << " RPC " << node->name << "  NodeType: " << lys_nodetype2str(node->nodetype) << std::endl;

        parse_yang_node(node, NULL, level+1, rpc->name, structInfo);
    }

    LY_LIST_FOR(rpc->output.child, node) {

        parse_yang_node(node, NULL, level+1, rpc->name, structInfo);
    }

}

void YangCodeGen::parse_yang_notif(const struct lysc_node_notif* notif, int level)
{
    YangStructInfo* structInfo = new YangStructInfo(notif->name, "");
    //std::cout << "Notif: " << notif->name << std::endl;
    structInfo->namespaceString = notif->module->ns;

    const struct lysc_node* notif_node;
    LY_LIST_FOR(notif->child, notif_node) {

        //std::cout << "Child: " << notif_node->name << std::endl;
        parse_yang_node(notif_node, NULL, level, notif->name, structInfo);
    }
}

// Function to read XML and store data in the structure using libxml2
bool YangCodeGen::parse_yang_and_store_data(const char* yang_module_name) {

    std::cout << "Root YANG Module: " << yang_module_name << std::endl;

    struct lys_module* root_module =  YangCodeGenMgr::instance()->get_module_from_module_name(yang_module_name);

    if(root_module == nullptr) return false;

    levelWiseNameTypeMap.emplace_back();

    /* iterate over top level containers */
    const struct lysc_node* node;
    LY_LIST_FOR(root_module->compiled->data, node) {

        //if (node->nodetype == LYS_CONTAINER) {
            parse_yang_tree(node, NULL, 0, "");

            // multiple top level nodes in a module
        //}
    }


    /* iterate over rpcs */
    const struct lysc_node_action* rpc;
    LY_LIST_FOR(root_module->compiled->rpcs, rpc) {

        parse_yang_rpc(rpc, 0);
    }

    /* iterate over notificatins */
    const struct lysc_node_notif* notif;
    LY_LIST_FOR(root_module->compiled->notifs, notif) {

        parse_yang_notif(notif, 0);
    }

    return true;
}
