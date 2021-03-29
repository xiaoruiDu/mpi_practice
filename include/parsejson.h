#ifndef  _PARSEJSON_H
#define  _PARSEJSON_H


#include <map>
#include <iostream>
#include <string>
#include <map>
#include <vector>

#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <set>
#include "agent.h"

using json = nlohmann::json;

using namespace std;



class ParseJson{
    public:
        ParseJson();
        vector<agent_S> get_agent_json_data(std::string filename);

    private:
        int x;
        vector<agent_S> agent_list;
};


#endif // _PARSEJSON_H