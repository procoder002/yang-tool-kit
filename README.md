# yang-tool-kit

## Overview

The YANG Tool Kit (YTK) is a software development tool, which provides API for building applications based on YANG models. The YTK allows to generate cpp structure and codec function from the corresponding YANG model. Those generated APIs can easily be integrated with various communication protocols like Netconf, Restconf, gNMI etc. Basically this provies a storage facility of YANG models which can send and receive over the wire.

### Key features
* YANG model driven code generation
* Code parsing & generation using libyang
* Flexible & extendible, can a
* 

### Build Requirement:
* g++ compiler (--stdc++11)
* cmake >= 2.8
* libxml2 (sudo apt-get install libxml2-dev libxml2-doc)
* libpcre (sudo apt-get install libpcre2-dev)
* libyang (https://github.com/CESNET/libyang)

## How to use
* Clone the repo.
```
    git clone https://github.com/procoder002/yang-tool-kit.git
```  

* Run configure script which will first clone the 'libyang' library, build it and generate the 'yang_tool_kit' binary under build directory
```
    chmod +x configure
    ./configure
```

* 'yang_tool_kit' will take a 'recipe_file' and 'gen_dir' as input, 'recipe_file' will contain list of YANG filename along with their location and 'gen_dir' will contain all the generated files
```
    USAGE: ./yang_tool_kit  <recipe_file>   <gen_dir>
```

* This command will generate .h and .cpp files under 'build/auto_gen/' directory, corresponding to the yang files mentioned in the recipe file
```
    ./build/yang_tool_kit   recipe/yang_input.txt    build/auto_gen/

    gen_network_device.cpp  gen_network_device.h  
    gen_router_config.cpp  gen_router_config.h
```

### Create your first app
* Go inside app directory, run and following commands, which will create 'ytk_client_app' binary
```
    mkdir build
    cd build
    cmake ..
    make
```

* 'ytk_client_app' takes a xml file as input.
```
    USAGE: ./ytk_client_app <xml_file>
```

*  Run the code like
```
    ./app/build/ytk_client_app xml/network_device.xml
```

* XML file content will get stored in generated c++ structure on calling deserialize API. and those c++ structure can be edited programatically, and calling serialize it will return the structure content in a XML encoded string
```
    network_device deviceConfig;
    deviceConfig.deserializeXML(root);

    deviceConfig.encodeToXML(xml);    
```

* Please refer to the clinet_app.cpp for more details


### Running Unit Tests



### Backward Compatibility


### Future Enhancement:
* Integration with NETCONF, RESTCONF, gNMI etc. Coming Soon ...

### Reference
* https://github.com/CiscoDevNet/ydk-gen
* https://www.nokia.com/model-driven-management/
* https://www.ericsson.com/en/blog/2024/5/how-to-use-data-models-of-domains-in-a-smart-way
* https://medium.com/@k.okasha/yang-and-road-to-a-model-driven-network-e9e52d47148d