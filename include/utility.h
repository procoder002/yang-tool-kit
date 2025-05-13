#ifndef UTILITY_H
#define UTILITY_H

#include <bits/stdc++.h>

typedef std::vector<std::pair<std::string, std::string>> query_set;

extern std::string get_searchdir_from_filepath(std::string& yang_file);

extern std::vector<std::string> splitIntoVector(const std::string& input, char delimiter);

extern std::deque<std::string> splitIntoDeque(const std::string& input, char delimiter);

extern std::string convertXMLString(std::string& input, char delimiter);

extern std::string trim_struct_name_n_fetch_id_from_url(std::string& query);

extern bool query_url_found(std::string& query);

extern std::string trim_n_fetch_struct_name_from_query(std::string& query);

extern query_set fetch_query_set(std::string& query, char& op);

extern std::string trim_n_fetch_id_from_query_url(std::string& query);


#endif // UTILITY_H
