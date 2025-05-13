#include "ytk_code_gen.h"
#include "utility.h"
#include <dirent.h>
#include <sys/stat.h>
#include <algorithm>

YangCodeGenMgr* YangCodeGenMgr::instancePtr = nullptr;

/* Single threaded application, no need for protection */
YangCodeGenMgr* YangCodeGenMgr::instance() {
    if(instancePtr == nullptr) {
        instancePtr = new YangCodeGenMgr();
    }
    return instancePtr;
}

YangCodeGenMgr::YangCodeGenMgr()
{
    if (ly_ctx_new("", 0, &_yang_ctx) != LY_SUCCESS) {
        std::cerr << "Failed to create libyang context" << std::endl;
    }
}

YangCodeGenMgr::~YangCodeGenMgr() {
    if(_yang_ctx) ly_ctx_destroy(_yang_ctx);
}

bool YangCodeGenMgr::add_search_dir(const char* dirname)
{
    /* Set the yang search path */
    if (ly_ctx_set_searchdir(_yang_ctx, dirname)) {
        //std::cerr << "Failed to add search directory: " << dirname << std::endl;
        return false;
    }

    return load_all_yang_modules_in_dir(dirname);
}

bool YangCodeGenMgr::load_all_yang_modules_in_dir(const char* dirname)
{
    DIR* dir = opendir(dirname); // Open the directory
    if (!dir) {
        std::cerr << "Error: Unable to open directory " << dirname << std::endl;
        return false;
    }

    struct dirent* entry; // Represents a directory entry
    while ((entry = readdir(dir)) != nullptr) {
        std::string fileName = entry->d_name;

        // Skip "." and ".." entries
        if (fileName == "." || fileName == "..") {
            continue;
        }

        // Build the full path to the file
        std::string fullPath = std::string(dirname) + "/" + fileName;

        // Check if the entry is a regular file
        struct stat fileStat;
        if (stat(fullPath.c_str(), &fileStat) == 0 && S_ISREG(fileStat.st_mode)) {
            // Check if the file has a ".yang" extension
            if (fileName.size() >= 5 && fileName.substr(fileName.size() - 5) == ".yang") {

                std::string module_name;

                int pos = fileName.find('@');
                if(pos == std::string::npos) {
                    module_name = fileName.substr(0, fileName.size() - 5);
                } else {
                    module_name = fileName.substr(0, pos);
                }

                std::cout << "Loading YANG Module: " << module_name << std::endl;

                get_module_from_module_name(module_name.c_str());
            }
        }
    }

    closedir(dir); // Close the directory

    return true;
}

struct lys_module* YangCodeGenMgr::get_module_from_module_name(const char* yang_module_name)
{
    const char *features[] = {"*", NULL}; // enable all if-features
    struct lys_module *module = ly_ctx_load_module(_yang_ctx, yang_module_name, NULL, features);
    if(!module) {
        std::cerr << "Couldn't load module: " << yang_module_name << std::endl;
    }

    return module;
}

#if 0
/* Function to find a lyd_node corresponding to a given lysc_node */
const struct lyd_node* YangCodeGenMgr::find_data_node(const struct lyd_node *data_tree, const struct lysc_node *schema_node) {
    const struct lyd_node *node = NULL;

    LYD_TREE_DFS_BEGIN(data_tree, node) {

        if(node->schema == schema_node) return node;

        LYD_TREE_DFS_END(data_tree, node);
    }

    return NULL; // Not found
}
#endif

void YangCodeGen::auto_generate_file() {

    output << "#include \"gen_" << module_name << ".h\"" << std::endl;

    std::transform(module_name.begin(), module_name.end(), module_name.begin(), ::toupper);

    output_h << "#ifndef " << module_name << "_H\n";
    output_h << "#define " << module_name << "_H\n\n";

    //output_h << "#include <bits/stdc++.h>\n";
    output_h << "#include <libxml/parser.h>\n";
    output_h << "#include <libyang/libyang.h>\n";

    output_h << "#include \"yang_subs_mgr.h\"\n\n";

    for(int level = levelWiseNameTypeMap.size()-1; level >= 0; level--) {

        for(auto& structElement: levelWiseNameTypeMap[level]) {

            YangStructInfo structInfo = *structElement.first;

            generate_structure_body(structInfo, structElement.second);

        }
    }

    output_h << "#endif // " << module_name << std::endl;
}

bool YangCodeGen::generate(const char* yang_module, std::string output_dir)
{
    module_name = yang_module;

    std::replace( module_name.begin(), module_name.end(), '-', '_');

    std::string output_header_file_path = output_dir + "gen_" + module_name + ".h";
    std::string output_cpp_file_path = output_dir + "gen_" + module_name + ".cpp";

    output_h.open(output_header_file_path);
    if (!output_h.is_open()) {
        std::cerr << "Failed to open output file: " << output_header_file_path << std::endl;
        output_h.close();
        return false;
    }

    output.open(output_cpp_file_path);
    if (!output.is_open()) {
        std::cerr << "Failed to open output file: " << output_cpp_file_path << std::endl;
        output.close();
        return false;
    }

    auto_generate_file();

    output_h.close();
    output.close();


    std::cout << "Successfully generated " << output_header_file_path << " and " << output_cpp_file_path << std::endl;

    return true;
}

bool YangCodeGenMgr::parse_n_generate(const char* module_name, std::string output_dir) {

    YangCodeGen code_gen;

    if(code_gen.parse_yang_and_store_data(module_name) == false) {
        std::cout << "Failed to load module " << module_name << std::endl;
        return false;
    }

    if(code_gen.generate(module_name, output_dir) == false) {
        std::cout << "Failed to generate file " << module_name << std::endl;
        return false;
    }

    return true;
}

bool checkStringCommented(const std::string& str) {
    // Find the first non-space character
    size_t first_non_space = str.find_first_not_of(" \t\n\r\f\v"); // Includes all whitespace characters

    // If the string is all spaces, return false
    if (first_non_space == std::string::npos) {
        return false;
    }

    // Check if the first non-space character is '#'
    return str[first_non_space] == '#';
}

int main(int argc, char* argv[]) {
    if(argc != 3) {
        std::cerr << "USAGE: ./yang_tool_kit    <recipe_file>   <gen_dir>" << std::endl;
        return 1;
    }

    /* check if input file exists */
    std::ifstream input_file(argv[1]);
    if (!input_file.is_open()) {
        std::cerr << "Error: Unable to open file " << argv[1] << std::endl;
        return 1;
    }

    /* iterate through each line in input file and generate the code */
    std::string line;
    while (std::getline(input_file, line))
    {
        if(checkStringCommented(line)) continue;

        // "usage: <yang_search_dir> <yang_module_name> "
        std::vector<std::string> input = splitIntoVector(line, ' ');

        //const char* yang_dir = input[0].c_str();         //ex- yang/3gpp
        YangCodeGenMgr::instance()->add_search_dir(input[0].c_str());

        // const char* yang_module_name = input[1].c_str(); //ex- network-device
        // const char* output_dir = argv[2]; // ex- auto_gen/

        if(false == YangCodeGenMgr::instance()->parse_n_generate(input[1].c_str(), argv[2]))
        {
            std::cout << "Failed to generate file " << input[1] << std::endl;
            continue;
        }
    }

    input_file.close();

    return 0;
}
