#include <iostream>
#include <bits/stdc++.h>
#include <libyang/libyang.h>
#include <libxml/parser.h>

#include "gen_network_device.h"

std::string get_xml_content(const char* xml_file) {

    std::ifstream xml_content_file(xml_file);
    if (!xml_content_file.is_open()) {
        std::cerr << "Failed to open XML file: " << xml_file << std::endl;
        return "";
    }

    std::ostringstream xml_content_stream;
    xml_content_stream << xml_content_file.rdbuf();

    xml_content_file.close();

    return xml_content_stream.str();
}


int main(int argc, char* argv[]) {
    if(argc != 2) {
        std::cerr << "USAGE: ./ytk_client_app <xml_file>" << std::endl;
        return 1;
    }

    const char* xml_file    = argv[1];  // "network_device.xml";

    std::string xml_content = get_xml_content(xml_file);
    if(xml_content.empty()) {
        std::cerr << "XML content is empty" << std::endl;
        return 1;
    }

    xmlDocPtr doc = xmlParseMemory(xml_content.c_str(), xml_content.size());
    if (doc == NULL) {
        std::cerr << "Failed to parse XML string" << std::endl;
        return 1;
    }
    xmlNodePtr root = xmlDocGetRootElement(doc);
    if (root == nullptr) {
        std::cerr << "Empty XML document" << std::endl;
        xmlFreeDoc(doc);
        return 1;
    }

    network_device deviceConfig;
    deviceConfig.deserializeXML(root);
    
    std::cout << "Device Name: " << deviceConfig.device_name.get() << std::endl;
    std::cout << "IP Address: " << deviceConfig.ip_address.get() << std::endl;
    std::cout << "Port: " << deviceConfig.port.get() << std::endl;
    std::cout << "Settings:" << std::endl;
    for (auto& setting : deviceConfig.settings.setting) {
        std::cout << "  " << setting.second.name.get() << ": " << setting.second.value.get() << std::endl;
    }
    std::cout << std::endl << std::endl;
    deviceConfig.device_name = "procoder";
    deviceConfig.port = 002;
    
    network_device_ns::settings_ns::setting deviceSetting;
    deviceSetting.name = "DummyKey";
    deviceSetting.value = "DummyValue";

    deviceConfig.settings.setting["id"] = deviceSetting;

    std::ostringstream xml;
    std::cout << "Dumping struct values in xml format" << std::endl;
    deviceConfig.encodeToXML(xml);

    std::cout << xml.str() << std::endl;

    return 0;
}
