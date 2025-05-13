#include "utility.h"

std::string get_searchdir_from_filepath(std::string& yang_file)
{
    int dot_pos = yang_file.rfind('.');
    int slash_pos = yang_file.rfind('/');

    if(dot_pos == std::string::npos && slash_pos == std::string::npos) return yang_file;

    else if(dot_pos == std::string::npos) return yang_file.substr(slash_pos+1);

    yang_file.erase(dot_pos);

    if(slash_pos == std::string::npos) return "";

    std::string search_dir = yang_file.substr(0, slash_pos);
    yang_file = yang_file.substr(slash_pos+1);

    return search_dir;
}

std::vector<std::string> splitIntoVector(const std::string& input, char delimiter) {
    size_t pos = 0;
    std::vector<std::string> V;
    std::string str = input;
    while ((pos = str.find(delimiter)) != std::string::npos) {
        std::string token = str.substr(0, pos);
        if(!token.empty())
        {
            V.push_back(token);
        }
        str.erase(0, pos + 1);
    }
    std::string token = str.substr(0, pos);
    if(!token.empty()) V.push_back(token);
    return V;
}

std::deque<std::string> splitIntoDeque(const std::string& input, char delimiter)
{
    std::deque<std::string> dq;
    if(input.empty()) return dq;

    size_t pos = 0;
    std::string str = input;

    while ((pos = str.find(delimiter)) != std::string::npos) {
        std::string token = str.substr(0, pos);
        if(!token.empty())
        {
            dq.push_back(token);
        }
        str.erase(0, pos + 1);
    }
    std::string token = str.substr(0, pos);
    if(!token.empty()) dq.push_back(token);
    return dq;
}

/**
 * @brief this function takes a query string as input, strip the structure name and return the struct key or empty string
  Ex.  Input: query - ManagedElement[46]
       Output: query will become ManagedElement and return 46

       Input: query - ManagedElement
       Output: no change in query just return empty string
 */

std::string trim_struct_name_n_fetch_id_from_url(std::string& query)
{
    int open_pos = query.find('[');
    int close_pos = query.find(']');

    if(open_pos == std::string::npos || close_pos == std::string::npos) return "";

    std::string struct_key = query.substr(open_pos+1, close_pos-open_pos-1);
    query = query.substr(0, open_pos);

    return struct_key;
}

std::string trim_n_fetch_id_from_query_url(std::string& query)
{
    int open_pos = query.find('{');
    int close_pos = query.find('}');

    if(open_pos == std::string::npos || close_pos == std::string::npos) return "";

    std::string struct_key = query.substr(0, open_pos);
    query = query.substr(open_pos+1, close_pos-open_pos-1);

    return struct_key;
}


bool query_url_found(std::string& query)
{
    return query.find('?') != std::string::npos;
}

std::string trim_n_fetch_struct_name_from_query(std::string& query)
{
    int question_pos = query.find('?');
    if(question_pos == std::string::npos) return "";

    std::string struct_name = query.substr(0, question_pos);
    query = query.substr(question_pos+1);

    return struct_name;
}


/**
 * @brief this function takes a query string as input and return the (key, value) query set
        id=1&id=2
        attributes.administrativeState=UNLOCKED&attributes.cellState=IDLE
 */

query_set fetch_query_set(std::string& query, char& op)
{
    query_set ans;

    if(query.find('&') != std::string::npos) {
        op = '&';
    } else if(query.find('|') != std::string::npos) {
        op = '|';
    } else if(query.find('!') != std::string::npos) {
        op = '!';
        query.erase(0, 1); // removing ! character
    }
    else {
        op = '&'; // default is and
    }

    std::vector<std::string> query_list = splitIntoVector(query, op);
    for(auto& kv_pair: query_list)
    {
        std::vector<std::string> kv = splitIntoVector(kv_pair, '=');
        if(kv.size() == 2)
        {
        ans.push_back({kv[0], kv[1]});
        }
    }

    return ans;
}


/* convertXMLString: remove newline and space from the string and escapse quoble quote */
std::string convertXMLString(std::string& input, char delimiter) {

    input.erase(std::remove_if(input.begin(), input.end(), [](unsigned char c) {
        return c == '\t' || c == '\n';
    }), input.end());

    std::string escapedString;
    for (char c : input) {
        // If the current character matches the delimiter, add a backslash before it
        if (c == delimiter) {
            escapedString += '\\';
        }
        // Add the current character to the result
        escapedString += c;
    }
    return escapedString;
}

